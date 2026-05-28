// Watch 5.1: 5th gen watch - ESP32C3 with GC9A01A TFT display

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <esp_wifi.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SCREEN_RADIUS (SCREEN_WIDTH/2)
#define SCREEN_CENTER_X (SCREEN_WIDTH/2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT/2)

#define TFT_CS   10
#define TFT_DC    8
#define TFT_RST   9

Adafruit_GC9A01A display(TFT_CS, TFT_DC, TFT_RST);
Preferences preferences;

#define totalFunctions 12
#define numSettings 5

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

const char *Functions[] = {"Outputs", "Maths", "Random", "Score", "Games", "Metronome", "Notes", "Calendar", "WiFi Menu", "WiFi Tools","Shell", "Settings"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

const byte buttonPin = 2;

// Default button resistance values (Ordered by frequency used)
const int defBtn1 = 1499;  // 5.1K
const int defBtn2 = 1017;   // 3K
const int defBtn3 = 215;   // 510
const int defBtn4 = 400;   // 1K
const int defBtn5 = 1805;    // 6.8K
const int defBtn6 = 2238;  // 10K

int btn1;
int btn2;
int btn3;
int btn4;
int btn5;
int btn6;

int buttonOffset = 0;
int buttonValRange = 30;

byte Func1 = 3;
byte Func2 = 0;
byte Func3 = 1;

int blinkTime1 = 500000;
int blinkTime2 = 1;
int blinkTime3 = 10000;

int selectedFunction = 1;

bool wifiConnected = false;

struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};

WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;
int currentWiFiIndex = 0;

unsigned long lastNavTime = 0;
const unsigned long NAV_DEBOUNCE = 120;

// PARTIAL REDRAW STATE (for flicker-free UI)
bool staticUIdrawn = false;
int prevFuncShown = -1;
int oldFuncShown = -1;
int oldSelectedFunction = -1;
bool prevWifiConnected = false;

// BUTTONS
bool button_is_pressed(int btnVal, bool onlyOnce = false) {
  int pinVal = analogRead(buttonPin) - buttonOffset;
  int errorVal = pinVal - btnVal;
  int absErrorVal = abs(errorVal);

  if (absErrorVal <= buttonValRange) {    
    if (onlyOnce) {
      while (true) {
        delay(10);
        pinVal = analogRead(buttonPin);
        errorVal = pinVal - btnVal;
        absErrorVal = abs(errorVal);
        if (absErrorVal > 10) break;
      }
    }
    return true;
  }
  return false;
}

bool a_button_is_pressed(){
  return (analogRead(buttonPin) != 4095);
}

// MAIN UI (flicker-free, round, modern)
void drawMainUI() {
  int headerR = SCREEN_RADIUS-24;

  // --- Draw static UI only once ---
  if (!staticUIdrawn) {
    display.fillScreen(GC9A01A_BLACK);
    display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, GC9A01A_BLACK);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, GC9A01A_WHITE);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-2, GC9A01A_DARKGREY);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, headerR, GC9A01A_NAVY);
    display.setTextSize(2);
    display.setTextColor(GC9A01A_WHITE);
    int16_t x1, y1; uint16_t w, h;
    String title = "Watch 5.1";
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_CENTER_X - w/2, 20);
    display.print(title);
    // "< SEL >" label
    display.setTextSize(1);
    display.setTextColor(GC9A01A_WHITE);
    display.getTextBounds("< SEL >", 0,0, &x1,&y1,&w,&h);
    display.setCursor(SCREEN_CENTER_X - w/2, SCREEN_HEIGHT-25);
    display.print("< SEL >");
    staticUIdrawn = true;
    prevWifiConnected = !wifiConnected; // force first WiFi draw
    prevFuncShown = -1;
    oldFuncShown = -1;
    oldSelectedFunction = -1;
  }

  // --- Dynamic: WiFi indicator ---
  if (wifiConnected != prevWifiConnected) {
    display.fillRect(SCREEN_CENTER_X + headerR - 30, 18, 38, 10, GC9A01A_BLACK); // erase old
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X + headerR - 30, 18);
    display.setTextColor(wifiConnected ? GC9A01A_GREEN : GC9A01A_BLACK);
    if (wifiConnected) display.print("WiFi");
    prevWifiConnected = wifiConnected;
  }

  // --- Dynamic: Function number bar ---
  if (selectedFunction != prevFuncShown) {
    display.fillRect(SCREEN_CENTER_X - headerR + 12, 18, 35, 10, GC9A01A_BLACK); // erase
    display.setTextSize(1);
    display.setTextColor(GC9A01A_CYAN);
    display.setCursor(SCREEN_CENTER_X - headerR + 12, 18);
    display.print("[");
    if (selectedFunction < 10) display.print("0");
    display.print(selectedFunction);
    display.print("]");
    prevFuncShown = selectedFunction;
  }

  // --- Dynamic: Function name (center) ---
  if (oldFuncShown != selectedFunction) {
    display.fillRect(10, 76, SCREEN_WIDTH-20, 22, GC9A01A_BLACK);
    String fname = Functions[selectedFunction-1];
    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(fname, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_CENTER_X - w/2, 76);
    display.setTextColor(GC9A01A_WHITE);
    display.print(fname);
    oldFuncShown = selectedFunction;
  }

  // --- Dynamic: Selector bar (arcs/circles along bottom) ---
  if (selectedFunction != oldSelectedFunction) {
    float arcStart = -120.0;
    float arcEnd = 120.0;
    float arcSpan = arcEnd - arcStart;
    for (int i=0; i<totalFunctions; i++) {
      float angle = (arcStart + arcSpan * i/(float)(totalFunctions-1)) * 3.1416 / 180.0;
      int cx = SCREEN_CENTER_X + cos(angle)*(headerR-8);
      int cy = SCREEN_CENTER_Y + sin(angle)*(headerR-8);
      display.fillCircle(cx, cy, 8, GC9A01A_BLACK); // erase previous
      display.drawCircle(cx, cy, 5, GC9A01A_DARKGREY); // draw all as grey
    }
    // Draw highlight
    float angle = (arcStart + arcSpan * (selectedFunction-1)/(float)(totalFunctions-1)) * 3.1416 / 180.0;
    int cx = SCREEN_CENTER_X + cos(angle)*(headerR-8);
    int cy = SCREEN_CENTER_Y + sin(angle)*(headerR-8);
    display.fillCircle(cx, cy, 7, GC9A01A_YELLOW);
    oldSelectedFunction = selectedFunction;
  }
}

// FLICKER-FREE ROUND LOADING UI
void timeSyncAndUI() {
  if (wifiNetworkCount == 0) {
    delay(2000);
    return;
  }

  display.fillScreen(GC9A01A_BLACK);
  display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, GC9A01A_BLACK);
  display.setTextSize(1);
  display.setTextColor(GC9A01A_WHITE);
  String msg = "INITIALIZING";
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_CENTER_Y - 34);
  display.print(msg);

  for (int wifiIndex = 0; wifiIndex < wifiNetworkCount; wifiIndex++) {
    if (WiFi.status() == WL_CONNECTED) {
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }

    WiFi.begin(wifiNetworks[wifiIndex].ssid, wifiNetworks[wifiIndex].password);

    int totalSteps = 100;
    int currentStep = 0;
    int attempts = 0;
    unsigned long startTime = millis();
    unsigned long timeout = 10000;
    int lastPercentDrawn = -1;

    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      if (button_is_pressed(btn6)) {
        WiFi.disconnect();
        return;
      }
      delay(500);
      attempts++;
      currentStep = min(totalSteps - 10, (int)((millis() - startTime) * totalSteps / timeout));

      // Only redraw if changed!
      if (currentStep != lastPercentDrawn) {
        // Erase dots, arc, percent area only
        display.fillRect(SCREEN_CENTER_X-30, SCREEN_CENTER_Y-16, 60, 10, GC9A01A_BLACK);

        int dotCount = (attempts / 2) % 4;
        display.setTextSize(1);
        display.setTextColor(GC9A01A_WHITE);
        display.setCursor(SCREEN_CENTER_X-12, SCREEN_CENTER_Y - 16);
        for (int i = 0; i < dotCount; i++) display.print(".");

        // Erase & redraw progress arc
        const int arc_radius = SCREEN_RADIUS - 12;
        const int arc_thickness = 10;
        const float start_angle = 200.0, end_angle = -20.0;
        float percent = currentStep * 1.0 / totalSteps;
        float prog_end = start_angle + (end_angle - start_angle) * percent;

        // Erase arc+percent area
        display.fillRect(SCREEN_CENTER_X-62, SCREEN_HEIGHT-54, 124, 40, GC9A01A_BLACK);

        // Arc background
        for (float a = start_angle; a >= end_angle; a -= 2) {
          float rad = a * 3.14159 / 180.0;
          int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness/2);
          int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness/2);
          int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness/2);
          int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness/2);
          display.drawLine(x1a, y1a, x2a, y2a, GC9A01A_DARKGREY);
        }
        // Progress arc
        for (float a = start_angle; a >= prog_end; a -= 2) {
          float rad = a * 3.14159 / 180.0;
          int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness / 2);
          int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness / 2);
          int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness / 2);
          int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness / 2);
          display.drawLine(x1a, y1a, x2a, y2a, GC9A01A_CYAN);
        }

        // Redraw progress percent
        char percentStr[8];
        sprintf(percentStr, "%3d%%", currentStep);
        display.setTextSize(2);
        display.setTextColor(GC9A01A_WHITE);
        display.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_HEIGHT - 38);
        display.print(percentStr);

        lastPercentDrawn = currentStep;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }
  }

  // Draw 100% arc and label
  display.fillRect(SCREEN_CENTER_X-62, SCREEN_HEIGHT-54, 124, 40, GC9A01A_BLACK);
  const int arc_radius = SCREEN_RADIUS - 12;
  const int arc_thickness = 10;
  const float start_angle = 200.0;
  const float end_angle = -20.0;
  for (float a = start_angle; a >= end_angle; a -= 2) {
    float rad = a * 3.14159 / 180.0;
    int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness / 2);
    int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness / 2);
    int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness / 2);
    int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness / 2);
    display.drawLine(x1a, y1a, x2a, y2a, GC9A01A_CYAN);
  }
  char percentStr[8];
  sprintf(percentStr, "%3d%%", 100);
  display.setTextSize(2);
  display.setTextColor(GC9A01A_WHITE);
  display.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_HEIGHT - 38);
  display.print(percentStr);

  wifiConnected = false;
  WiFi.disconnect();
  delay(500);
}

// --- BUTTON CALIBRATION AND STORAGE ---

void saveBtnVals() {
  preferences.begin("btns", false);
  preferences.putInt("btn1", btn1);
  preferences.putInt("btn2", btn2);
  preferences.putInt("btn3", btn3);
  preferences.putInt("btn4", btn4);
  preferences.putInt("btn5", btn5);
  preferences.putInt("btn6", btn6);
  preferences.end();
}

void loadBtnVals(){
  preferences.begin("btns", true);
  btn1 = preferences.getInt("btn1", defBtn1);
  btn2 = preferences.getInt("btn2", defBtn2);
  btn3 = preferences.getInt("btn3", defBtn3);
  btn4 = preferences.getInt("btn4", defBtn4);
  btn5 = preferences.getInt("btn5", defBtn5);
  btn6 = preferences.getInt("btn6", defBtn6);
  preferences.end();
}

void randomiseMac(){
  uint8_t mac[6];
  for (int i = 0; i < 6; i++) {
    mac[i] = random(0, 256);
  }
  mac[0] = (mac[0] | 0x02) & 0xFE;
  esp_wifi_set_mac(WIFI_IF_STA, mac);
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);

  loadBtnVals();
  randomSeed(analogRead(1));
  display.begin();
  Serial.begin(115200);

  randomiseMac();

  initializeNotesNVS(); 
  loadWiFiNetworksFromNVS();

  // Sync time and display startup message
  timeSyncAndUI();

  delay(1000);

  // If any button is pressed, enter button tuning
  if (a_button_is_pressed()) {
    display.fillScreen(GC9A01A_BLACK);
    tuneButtonVals();
  }

  // Draw main UI at the start
  drawMainUI();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.peek();
    if (cmd == 's' || cmd == 'S') {
      Serial.read();
      serialWiFiMenu();
    } 
    else if (cmd == 'n' || cmd == 'N') {
      Serial.read();
      serialNotesMenu();
    } 
    else if (cmd == 'c' || cmd == 'C') {
      Serial.read();
      serialCalendarMenu();
    } 
    else {
      Serial.read();
    }
  }

  checkCalendarAlarms();

  drawMainUI(); // only updates what is needed

  unsigned long now = millis();

  if (button_is_pressed(btn2) && (now - lastNavTime) > NAV_DEBOUNCE) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
    lastNavTime = now;
  } 
  else if (button_is_pressed(btn1) && (now - lastNavTime) > NAV_DEBOUNCE) {
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
    lastNavTime = now;
  } 
  else if (button_is_pressed(btn3)) {
    delay(100);
    switch (selectedFunction) {
      case 1: watchFuncs(); break;
      case 2: maths(); break;
      case 3: randomNum(); break;
      case 4: counter(); break;
      case 5: games(); break;
      case 6: metronome(); break;
      case 7: notesFunction(); break;
      case 8: calendar(); break;
      case 9: wifiMenu(); break;
      case 10: wifiFuncs(); break;
      case 11: shell(); break;
      case 12: settings(); break;
    }
    staticUIdrawn = false; // force redraw when returning to UI
  }
}