// Watch 5.1: 5th gen watch - ESP32C3 with GC9A01A TFT display

#include <TFT_eSPI.h>
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

TFT_eSPI display = TFT_eSPI();
Preferences preferences;

#define totalFunctions 14

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

const char *Functions[] = {"Time", "Outputs","Maths", "Random", "Score", "Games", "Metronome", "Notes", "Calendar", "WiFi Setup", "WiFi Funcs","Shell", "Settings", "Sleep"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

const byte buttonPin = 2;

const int defBtn1 = 1800;
const int defBtn2 = 1240;
const int defBtn3 = 315;
const int defBtn4 = 540;
const int defBtn5 = 2660;
const int defBtn6 = 2160;

int btn1, btn2, btn3, btn4, btn5, btn6;
int buttonOffset = 0;
int buttonValRange = 30;

unsigned long lastActivityTime = 0;
unsigned long inactivityPeriod = 120000;

byte Func1 = 0;
byte Func2 = 21;
byte Func3 = 0;

int blinkTime1 = 500000;
int blinkTime2 = 1;
int blinkTime3 = 10000;

uint16_t colourBG   = display.color565(0, 0, 0);
uint16_t colourText = display.color565(255, 255, 255);
uint16_t colour1    = display.color565(123, 125, 123);
uint16_t colour2    = display.color565(255, 0, 0);
uint16_t colour3    = display.color565(10, 225, 80);
uint16_t colour4    = display.color565(255, 255, 0);
uint16_t colour5    = display.color565(0, 0, 123);
uint16_t colour6    = display.color565(0, 255, 255);

bool inverted = false;
int selectedFunction = 1;
bool wifiConnected = false;
bool didWifiConnect = false;

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
  randomiseMac();
  display.fillScreen(colourBG);
  display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, colourBG);
  display.setTextColor(colourText);
  display.setTextDatum(CC_DATUM);
  display.drawString("INITIALIZING", SCREEN_CENTER_X, SCREEN_CENTER_Y - 34);
  
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
        display.setTextColor(colourText);
        display.setTextDatum(CC_DATUM);
        
        String dots = "";
        int dotCount = (attempts / 2) % 4;
        for (int i = 0; i < dotCount; i++) dots += ".";
        display.drawString(dots, SCREEN_CENTER_X, SCREEN_CENTER_Y - 16);
        display.drawNumber(analogRead(buttonPin), SCREEN_CENTER_X, SCREEN_CENTER_Y + 20);

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

        display.setTextColor(colourText);
        display.setTextDatum(CC_DATUM);
        display.drawString(String(currentStep) + "%", SCREEN_CENTER_X, SCREEN_HEIGHT - 50);

        lastPercentDrawn = currentStep;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      didWifiConnect = true;
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

  display.setTextColor(colourText);
  display.setTextDatum(CC_DATUM);
  display.drawString("100%", SCREEN_CENTER_X, SCREEN_HEIGHT - 50);

  if (WiFi.status() == WL_CONNECTED) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
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

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  esp_sleep_enable_timer_wakeup(100000);
  loadBtnVals();
  randomSeed(analogRead(1));
  
  display.init();
  display.setRotation(0);
  display.fillScreen(colourBG);
  
  Serial.begin(115200);

  initializeNotesNVS(); 
  loadWiFiNetworksFromNVS();
  timeSyncAndUI();

  delay(1000);

  if (a_button_is_pressed()) {
    display.fillScreen(colourBG);
    while (a_button_is_pressed()){}
    tuneButtonVals();
  }

  drawMainUI();
}

void updateTimeDisplay() {
  int dst = getDSTOffset();
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour + dst;
  int minute = timeinfo->tm_min;
  
  if (hour != lastDisplayedHour || minute != lastDisplayedMin) {
    display.fillRect(SCREEN_CENTER_X - 40, SCREEN_HEIGHT - 70, 80, 12, colourBG);
    
    display.setTextColor(colour1);
    display.setTextDatum(CC_DATUM);
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    display.drawString(timeStr, SCREEN_CENTER_X, SCREEN_HEIGHT - 69);
    
    lastDisplayedHour = hour;
    lastDisplayedMin = minute;
  }
}z

void drawMainUI() {
  int headerR = SCREEN_RADIUS-24;
  uint16_t circleColour;

  if (!staticUIdrawn) {
    if (wifiConnected) circleColour = colour3;
    else if (didWifiConnect) circleColour = colour6;
    else circleColour = colour2;
    
    display.fillScreen(colourBG);
    display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, colourBG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, circleColour);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, headerR+1, colour5);
    
    display.setTextColor(colourText);
    display.setTextDatum(CC_DATUM);
    display.drawString("Watch 5.1", SCREEN_CENTER_X, 60);
    
    display.drawString("< SEL >", SCREEN_CENTER_X, SCREEN_HEIGHT-15);
    
    staticUIdrawn = true;
    prevWifiConnected = !wifiConnected;
    prevFuncShown = -1;
    oldFuncShown = -1;
    oldSelectedFunction = -1;
    lastDisplayedHour = -1;
    lastDisplayedMin = -1;
  }

  if (wifiConnected != prevWifiConnected) {
    display.setTextColor(colour3, colourBG);
    display.setTextDatum(TL_DATUM);
    if (wifiConnected) {
      display.drawString("WiFi", SCREEN_CENTER_X + headerR - 30, 18);
    } else {
      display.fillRect(SCREEN_CENTER_X + headerR - 30, 18, 38, 10, colourBG);
    }
    prevWifiConnected = wifiConnected;
  }

  if (oldFuncShown != selectedFunction) {
    display.fillRect(30, 120, SCREEN_WIDTH-60, 22, colourBG);
    String fname = Functions[selectedFunction-1];
    display.setTextColor(colourText);
    display.setTextDatum(CC_DATUM);
    display.drawString(fname, SCREEN_CENTER_X, 120);
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

bool lightSleep(){
  display.fillScreen(TFT_BLACK);
  delay(500);
  while (!a_button_is_pressed()){
    esp_light_sleep_start();
  }
  display.init();
  quickTimeDisplay();
  if (!a_button_is_pressed()) return true;
  else {
    staticUIdrawn = false;
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

  if (button_is_pressed(btn4) && (now - lastNavTime) > NAV_DEBOUNCE) {
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
    while (a_button_is_pressed()){}
    switch (selectedFunction) {
      case 1:  timeMenu(); break;
      case 2:  watchFuncs(); break;
      case 3:  maths(); break;
      case 4:  randomNum(); break;
      case 5:  counter(); break;
      case 6:  games(); break;
      case 7:  metronome(); break;
      case 8:  notesFunction(); break;
      case 9:  calendar(); break;
      case 10: wifiMenu(); break;
      case 11: wifiFuncs(); break;
      case 12: shell(); break;
      case 13: settings(); break;
      case 14: while (lightSleep()){} break;
    }
    while(a_button_is_pressed()){}
    staticUIdrawn = false;
  }
}