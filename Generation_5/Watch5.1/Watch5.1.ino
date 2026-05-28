<<<<<<< Updated upstream
#include "Watch5.1.h"

#define DC  8
#define RST 9
#define CS 10

=======
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
>>>>>>> Stashed changes

#define totalFunctions 12
#define numSettings 5

<<<<<<< Updated upstream
Adafruit_GC9A01A display(CS, DC, RST);
Preferences preferences;
=======
#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64
>>>>>>> Stashed changes

const char *Functions[] = {"Outputs", "Maths", "Random", "Score", "Games", "Metronome", "Notes", "Calendar", "WiFi Menu", "WiFi Tools","Shell", "Settings"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

<<<<<<< Updated upstream
=======
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
int prevSelectedFunction = 0; 

bool wifiConnected = false;

struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};

>>>>>>> Stashed changes
WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;
int currentWiFiIndex = 0;

<<<<<<< Updated upstream
=======
unsigned long lastNavTime = 0;
const unsigned long NAV_DEBOUNCE = 120;

// Helper to check if a point is within the visible circle
bool inCircle(int x, int y) {
  int dx = x - SCREEN_CENTER_X;
  int dy = y - SCREEN_CENTER_Y;
  return (dx*dx + dy*dy) <= (SCREEN_RADIUS-2)*(SCREEN_RADIUS-2);
}

>>>>>>> Stashed changes
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

void drawMainUI() {
  display.fillScreen(GC9A01A_BLACK);

  display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, GC9A01A_BLACK);

  display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, GC9A01A_WHITE);
  display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-2, GC9A01A_DARKGREY);

  int headerY = 42;
  int headerR = SCREEN_RADIUS-24;
  display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, headerR, GC9A01A_NAVY);

  display.setTextColor(GC9A01A_WHITE);
  display.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;

  if (wifiConnected) {
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X + headerR - 30, 18);
    display.setTextColor(GC9A01A_GREEN);
    display.print("WiFi");
  }

  display.setTextColor(GC9A01A_CYAN);
  display.setTextSize(1);
  display.setCursor(SCREEN_CENTER_X - 12, 150);
  display.print("[");
  if (selectedFunction < 10) display.print("0");
  display.print(selectedFunction);
  display.print("]");
  display.setTextColor(GC9A01A_WHITE);

  // Draw function title in the "middle"
  display.setTextSize(2);
  display.getTextBounds(Functions[selectedFunction-1], 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w/2, 120);
  display.setTextColor(GC9A01A_WHITE);
  display.print(Functions[selectedFunction-1]);

  // Draw selector bar at bottom (with arc)
  int barY = SCREEN_HEIGHT - 40;
  int barW = 120;
  int barH = 28;

  // Draw arc to represent current function position
  float arcStart = -120.0;
  float arcEnd   = 120.0;
  float arcSpan  = arcEnd - arcStart;

  for (int i=0; i<totalFunctions; i++) {
    float angle = (arcStart + arcSpan * i/(float)(totalFunctions-1)) * 3.1416 / 180.0;
    int cx = SCREEN_CENTER_X + cos(angle)*(headerR-8);
    int cy = SCREEN_CENTER_Y + sin(angle)*(headerR-8);
    if (i+1 == selectedFunction)
      display.fillCircle(cx, cy, 7, GC9A01A_YELLOW);
    else
      display.drawCircle(cx, cy, 5, GC9A01A_DARKGREY);
  }
  
  // Draw selector
  display.setTextSize(1);
  display.setTextColor(GC9A01A_WHITE);
  display.getTextBounds("< SEL >", 0,0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w/2, SCREEN_HEIGHT-15);
  display.print("< SEL >");
}

void updateMainUI() {
  if (selectedFunction != prevSelectedFunction) {
    drawMainUI();
    prevSelectedFunction = selectedFunction;
  }
}

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

void timeSyncAndUI() {
  if (wifiNetworkCount == 0) {
    delay(2000);
    return;
  }

  int totalSteps = 100;
  int currentStep = 0;
  unsigned long startTime = millis();
  unsigned long timeout = 10000;

  for (int wifiIndex = 0; wifiIndex < wifiNetworkCount; wifiIndex++) {
    if (WiFi.status() == WL_CONNECTED) {
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }

    WiFi.begin(wifiNetworks[wifiIndex].ssid, wifiNetworks[wifiIndex].password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
<<<<<<< Updated upstream
      if (button_is_pressed(btn6, true)) {
=======
      // Allow escape if a skip button is pressed
      if (button_is_pressed(btn6)) {
>>>>>>> Stashed changes
        WiFi.disconnect();
        return;
      }

      delay(500);
      attempts++;
      currentStep = min(totalSteps - 10, (int)((millis() - startTime) * totalSteps / timeout));

      display.fillScreen(GC9A01A_BLACK);
      display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, GC9A01A_BLACK);

      display.setTextSize(1);
      display.setTextColor(GC9A01A_WHITE);

      String msg = "INITIALIZING";
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_CENTER_Y - 34);
      display.print(msg);

      int dotCount = (attempts / 2) % 4;
      display.setCursor(SCREEN_CENTER_X - 12, SCREEN_CENTER_Y - 16);
      for (int i = 0; i < dotCount; i++) display.print(".");

      const int arc_radius = SCREEN_RADIUS - 12;
      const int arc_thickness = 10;
      const float start_angle = 200.0;   // left bottom, degrees
      const float end_angle = -20.0;     // right bottom, degrees

      for (float a = start_angle; a >= end_angle; a -= 2) {
        float rad = a * 3.14159 / 180.0;
        int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness / 2);
        int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness / 2);
        int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness / 2);
        int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness / 2);
        display.drawLine(x1a, y1a, x2a, y2a, GC9A01A_DARKGREY);
      }

      float percent = currentStep * 1.0 / totalSteps;
      float prog_end = start_angle + (end_angle - start_angle) * percent;
      for (float a = start_angle; a >= prog_end; a -= 2) {
        float rad = a * 3.14159 / 180.0;
        int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness / 2);
        int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness / 2);
        int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness / 2);
        int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness / 2);
        display.drawLine(x1a, y1a, x2a, y2a, GC9A01A_CYAN);
      }

      char percentStr[8];
      sprintf(percentStr, "%3d%%", currentStep);
      display.setTextSize(2);
      display.setTextColor(GC9A01A_WHITE);
      display.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_HEIGHT - 38);
      display.print(percentStr);
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }
  }

  display.fillScreen(GC9A01A_BLACK);
  display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, GC9A01A_BLACK);

  display.setTextSize(1);
  display.setTextColor(GC9A01A_WHITE);

  String msg = "INITIALIZING";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_CENTER_Y - 12);
  display.print(msg);
  display.print(".");

  // Progress arc (full)
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
  // ---------------------------------------------------------

  wifiConnected = false;
  WiFi.disconnect();
  delay(500);
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

  drawMainUI();
  prevSelectedFunction = selectedFunction;
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

  updateMainUI();

  unsigned long now = millis();

  if (button_is_pressed(btn4) && (now - lastNavTime) > NAV_DEBOUNCE) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
    lastNavTime = now;
  } 
  else if (button_is_pressed(btn6) && (now - lastNavTime) > NAV_DEBOUNCE) {
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
    lastNavTime = now;
  } 
  else if (button_is_pressed(btn5, true)) {
    delay(100);
    int prevSel = selectedFunction;
    bool prevWifi = wifiConnected;
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
  }
}