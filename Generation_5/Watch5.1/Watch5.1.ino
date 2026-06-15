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

#define totalFunctions 13

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};
WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;
int currentWiFiIndex = 0;

const char *Functions[] = {"Time", "Outputs","Maths", "Random", "Score", "Snake", "Metronome", "Notes", "Calendar", "WiFi Setup", "WiFi Funcs","Shell", "Settings"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

const byte buttonPin = 2;

// Default button resistance values
const int defBtn1 = 1499;  // 5.1K
const int defBtn2 = 1017;  // 3K
const int defBtn3 = 215;   // 510
const int defBtn4 = 400;   // 1K
const int defBtn5 = 2238;  // 10K
const int defBtn6 = 1805;  // 6.8K

int btn1;
int btn2;
int btn3;
int btn4;
int btn5;
int btn6;

int buttonOffset = 0;
int buttonValRange = 30;

unsigned long lastActivityTime = 0;
unsigned long inactivityPeriod = 120000;

byte Func1 = 0;
byte Func2 = 21;
byte Func3 = 0; // currently func3 is not implemented yet

uint16_t colourBG   =  display.color565(0, 0, 0);        // Black
uint16_t colourText =  display.color565(255, 255, 255);  // White
uint16_t colour1    =  display.color565(123, 125, 123);  // Dark Grey
uint16_t colour2    =  display.color565(255, 0, 0);      // Red
uint16_t colour3    =  display.color565(0, 255, 0);      // Green
uint16_t colour4    =  display.color565(255, 255, 0);    // Yellow
uint16_t colour5    =  display.color565(0, 0, 123);      // Navy
uint16_t colour6    =  display.color565(0, 255, 255);    // Cyan

bool inverted = false;

int blinkTime1 = 500000;
int blinkTime2 = 1;
int blinkTime3 = 10000;

int selectedFunction = 1;

bool wifiConnected = false;

unsigned long lastNavTime = 0;
const unsigned long NAV_DEBOUNCE = 120;

bool staticUIdrawn = false;
int prevFuncShown = -1;
int oldFuncShown = -1;
int oldSelectedFunction = -1;
bool prevWifiConnected = false;
int lastDisplayedHour = -1;
int lastDisplayedMin = -1;

bool a_button_is_pressed(){
  return (analogRead(buttonPin) != 4095);
}

bool button_is_pressed(int btnVal, bool onlyOnce = false) {
  int pinVal = analogRead(buttonPin) - buttonOffset;
  int errorVal = pinVal - btnVal;
  int absErrorVal = abs(errorVal);

  unsigned long now = millis();

  if (now - lastActivityTime > inactivityPeriod){
    while (lightSleep()){}
    lastActivityTime = millis();
    while (a_button_is_pressed()){}
    return false;
  }

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
    lastActivityTime = millis();
    return true;
  }
  return false;
}

void updateTimeDisplay() {
  int dst = getDSTOffset();
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour + dst;
  int minute = timeinfo->tm_min;
  
  if (hour != lastDisplayedHour || minute != lastDisplayedMin) {
    display.fillRect(SCREEN_CENTER_X - 40, SCREEN_HEIGHT - 70, 80, 12, colourBG);
    
    display.setTextSize(1);
    display.setTextColor(colour1);
    int16_t x1, y1;
    uint16_t w, h;
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_CENTER_X - w/2, SCREEN_HEIGHT - 69);
    display.print(timeStr);
    
    lastDisplayedHour = hour;
    lastDisplayedMin = minute;
  }
}

void drawCyborgLogo(int cx = 50, int cy=50, float scale = 1.0f)
{
auto S = [&](int v) { return (int)(v * scale); };

// ===== Circuit traces =====
display.fillRoundRect(cx - S(85), cy - S(4), S(55), S(8), S(4), colour6);

display.drawLine(cx - S(85), cy - S(40),
cx - S(55), cy - S(10),
colour6);
display.drawLine(cx - S(55), cy - S(10),
cx - S(30), cy - S(10),
colour6);

display.drawLine(cx - S(85), cy + S(40),
cx - S(55), cy + S(10),
colour6);
display.drawLine(cx - S(55), cy + S(10),
cx - S(30), cy + S(10),
colour6);

display.fillCircle(cx - S(85), cy - S(40), S(7), colour6);
display.fillCircle(cx - S(85), cy, S(7), colour6);
display.fillCircle(cx - S(85), cy + S(40), S(7), colour6);

display.fillCircle(cx - S(30), cy, S(18), colourText);
display.fillCircle(cx - S(30), cy, S(12), colourBG);
display.fillCircle(cx - S(30), cy, S(8), colour6);

// ===== Main head shape =====

// Helmet dome
display.fillCircle(cx + S(20), cy - S(25), S(55), colourText);

// Cut rear half to form profile
display.fillRect(cx - S(40), cy - S(90), S(60), S(180), colourBG);

// Jaw
display.fillTriangle(
cx + S(10), cy + S(30),
cx + S(45), cy + S(65),
cx + S(40), cy + S(15),
colourText);

// Face
display.fillTriangle(
cx + S(20), cy - S(25),
cx + S(70), cy - S(10),
cx + S(65), cy + S(40),
colourText);

// Chin
display.fillTriangle(
cx + S(25), cy + S(20),
cx + S(65), cy + S(40),
cx + S(45), cy + S(65),
colourText);

// Neck cutout
display.fillTriangle(
cx - S(5), cy + S(25),
cx + S(20), cy + S(85),
cx + S(25), cy + S(20),
colourBG);

// Eye slot
display.fillRoundRect(
cx + S(20),
cy - S(8),
S(40),
S(10),
S(5),
colourBG);

display.fillRoundRect(
cx + S(28),
cy - S(5),
S(26),
S(4),
S(2),
colour6);
}

void drawMainUI() {
  int headerR = SCREEN_RADIUS-24;

  if (!staticUIdrawn) {
    display.fillScreen(colourBG);
    display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, colourBG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, colourText);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-2, colour1);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, headerR, colour5);
    display.setTextSize(1);
    display.setTextColor(colourText);
    int16_t x1, y1; uint16_t w, h;
    String title = "Watch 5.1";
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_CENTER_X - w/2, 60);
    display.print(title);
    display.setTextSize(1);
    display.setTextColor(colourText);
    display.getTextBounds("< SEL >", 0,0, &x1,&y1,&w,&h);
    display.setCursor(SCREEN_CENTER_X - w/2, SCREEN_HEIGHT-15);
    display.print("< SEL >");
    staticUIdrawn = true;
    prevWifiConnected = !wifiConnected; // force first WiFi draw
    prevFuncShown = -1;
    oldFuncShown = -1;
    oldSelectedFunction = -1;
    lastDisplayedHour = -1;
    lastDisplayedMin = -1;
  }

  if (wifiConnected != prevWifiConnected) {
    display.fillRect(SCREEN_CENTER_X + headerR - 30, 18, 38, 10, colourBG); // erase old
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X + headerR - 30, 18);
    display.setTextColor(wifiConnected ? colour3 : colourBG);
    if (wifiConnected) display.print("WiFi");
    prevWifiConnected = wifiConnected;
  }
  if (oldFuncShown != selectedFunction) {
    display.setTextSize(2);
    display.fillRect(10, 120, SCREEN_WIDTH-20, 22, colourBG);
    String fname = Functions[selectedFunction-1];
    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(fname, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_CENTER_X - w/2, 120);
    display.setTextColor(colourText);
    display.print(fname);
    oldFuncShown = selectedFunction;
  }

  if (selectedFunction != oldSelectedFunction) {
    float arcStart = -120.0;
    float arcEnd = 120.0;
    float arcSpan = arcEnd - arcStart;
    for (int i=0; i<totalFunctions; i++) {
      float angle = (arcStart + arcSpan * i/(float)(totalFunctions-1)) * 3.1416 / 180.0;
      int cx = SCREEN_CENTER_X + cos(angle)*(headerR-8);
      int cy = SCREEN_CENTER_Y + sin(angle)*(headerR-8);
      display.fillCircle(cx, cy, 8, colourBG);
      display.drawCircle(cx, cy, 5, colour1); 
    }
    float angle = (arcStart + arcSpan * (selectedFunction-1)/(float)(totalFunctions-1)) * 3.1416 / 180.0;
    int cx = SCREEN_CENTER_X + cos(angle)*(headerR-8);
    int cy = SCREEN_CENTER_Y + sin(angle)*(headerR-8);
    display.fillCircle(cx, cy, 7, colour4);
    oldSelectedFunction = selectedFunction;
  }

  updateTimeDisplay();
}

void timeSyncAndUI() {
  if (wifiNetworkCount == 0) {
    delay(2000);
    return;
  }

  display.fillScreen(colourBG);
  display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, colourBG);
  display.setTextSize(1);
  display.setTextColor(colourText);
  String msg = "INITIALIZING";
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_CENTER_Y - 34);
  display.print(msg);

  for (int wifiIndex = 0; wifiIndex < wifiNetworkCount; wifiIndex++) {
    WiFi.begin(wifiNetworks[wifiIndex].ssid, wifiNetworks[wifiIndex].password);
	  WiFi.setTxPower(WIFI_POWER_8_5dBm);

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

      if (currentStep != lastPercentDrawn) {
        display.fillRect(SCREEN_CENTER_X-30, SCREEN_CENTER_Y-16, 60, 10, colourBG);

        int dotCount = (attempts / 2) % 4;
        display.setTextSize(1);
        display.setTextColor(colourText);
        display.setCursor(SCREEN_CENTER_X-12, SCREEN_CENTER_Y - 16);
        for (int i = 0; i < dotCount; i++) display.print(".");

        display.setCursor(SCREEN_CENTER_X-12, SCREEN_CENTER_Y + 20);
        display.print(analogRead(buttonPin));

        const int arc_radius = SCREEN_RADIUS - 12;
        const int arc_thickness = 10;
        const float start_angle = 200.0, end_angle = -20.0;
        float percent = currentStep * 1.0 / totalSteps;
        float prog_end = start_angle + (end_angle - start_angle) * percent;

        display.fillRect(SCREEN_CENTER_X-62, SCREEN_HEIGHT-54, 124, 40, colourBG);

        for (float a = start_angle; a >= end_angle; a -= 2) {
          float rad = a * 3.14159 / 180.0;
          int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness/2);
          int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness/2);
          int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness/2);
          int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness/2);
          display.drawLine(x1a, y1a, x2a, y2a, colour1);
        }
        for (float a = start_angle; a >= prog_end; a -= 2) {
          float rad = a * 3.14159 / 180.0;
          int x1a = SCREEN_CENTER_X + cos(rad) * (arc_radius - arc_thickness / 2);
          int y1a = SCREEN_CENTER_Y + sin(rad) * (arc_radius - arc_thickness / 2);
          int x2a = SCREEN_CENTER_X + cos(rad) * (arc_radius + arc_thickness / 2);
          int y2a = SCREEN_CENTER_Y + sin(rad) * (arc_radius + arc_thickness / 2);
          display.drawLine(x1a, y1a, x2a, y2a, colour6);
        }

        char percentStr[8];
        sprintf(percentStr, "%3d%%", currentStep);
        display.setTextSize(2);
        display.setTextColor(colourText);
        display.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_HEIGHT - 50);
        display.print(percentStr);

        lastPercentDrawn = currentStep;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      break;
    }
  }

  display.fillRect(SCREEN_CENTER_X-62, SCREEN_HEIGHT-54, 124, 40, colourBG);
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
    display.drawLine(x1a, y1a, x2a, y2a, colour6);
  }
  char percentStr[8];
  sprintf(percentStr, "%3d%%", 100);
  display.setTextSize(2);
  display.setTextColor(colourText);
  display.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_CENTER_X - w / 2, SCREEN_HEIGHT - 50);
  display.print(percentStr);

  if (WiFi.status() == WL_CONNECTED) {
    configTime(0, 0, "uk.pool.ntp.org", "time.nist.gov");
    //while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) delay(50);
    delay(1000);
  }
  wifiConnected = false;
  WiFi.disconnect();
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

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  esp_sleep_enable_timer_wakeup(500000); // 500ms
  loadBtnVals();
  randomSeed(analogRead(1));
  display.begin();
  Serial.begin(115200);

  randomiseMac();

  initializeNotesNVS(); 
  loadWiFiNetworksFromNVS();
  //drawCyborgLogo();
  //delay(1000);
  timeSyncAndUI();

  delay(1000);

  if (a_button_is_pressed()) {
    display.fillScreen(colourBG);
    while (a_button_is_pressed()){}
    tuneButtonVals();
  }

  drawMainUI();
}

bool lightSleep(){
  display.fillScreen(GC9A01A_BLACK);
  delay(1000);
  while (!a_button_is_pressed()){
    esp_light_sleep_start();
  }
  // vcc is cut off from display which seems to require calling display.begin again. 
  display.begin();
  display.fillScreen(colour1);
  if (button_is_pressed(btn3, true)){
    displayTime();
    return true;
  }
  else {
    delay(1000);
    staticUIdrawn = false; // force redraw when returning to UI
    return false;
  }
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.peek();
    if (cmd == 'w' || cmd == 'W') {
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

  drawMainUI();

  unsigned long now = millis();

  if (button_is_pressed(btn1)) {
    // returns true if going back to sleep, otherwise false
    while (lightSleep()){}
  }
  /*else if (button_is_pressed(btn3)) {
    display.fillScreen(GC9A01A_BLACK);
    delay(1000);
    esp_deep_sleep_enable_gpio_wakeup(2, ESP_GPIO_WAKEUP_GPIO_HIGH);
    esp_deep_sleep_start();
  }*/

  else if (button_is_pressed(btn4) && (now - lastNavTime) > NAV_DEBOUNCE) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
    lastNavTime = now;
  } 
  else if (button_is_pressed(btn5) && (now - lastNavTime) > NAV_DEBOUNCE) {
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
    lastNavTime = now;
  } 
  else if (button_is_pressed(btn6)) {
    delay(100);
    switch (selectedFunction) {
      case 1:  timeMenu(); break;
      case 2:  watchFuncs(); break;
      case 3:  maths(); break;
      case 4:  randomNum(); break;
      case 5:  counter(); break;
      case 6:  snake(); break;
      case 7:  metronome(); break;
      case 8:  notesFunction(); break;
      case 9:  calendar(); break;
      case 10: wifiMenu(); break;
      case 11: wifiFuncs(); break;
      case 12: shell(); break;
      case 13: settings(); break;
    }
    staticUIdrawn = false; // force redraw when returning to UI
  }
}