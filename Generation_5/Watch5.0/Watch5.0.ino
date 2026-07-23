// Watch 5.0: Initial 5th gen watch - ESP32C3 with SSD1306 OLED display

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
#include <Wire.h>
#include <ctype.h>
#include <math.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences preferences;

#define totalFunctions 13
#define numSettings 5

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64


const char *Functions[] = {"Outputs", "Maths", "Random", "Score", "Games", "Metronome", "Notes", "Calendar", "WiFi Menu", "WiFi Tools","Shell", "Settings", "Sleep"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

const byte buttonPin = 2;

// Default button resistance values (Ordered by frequency used)
const int defBtn1 = 1433;  // 4.7K
const int defBtn2 = 812;   // 2.2K
const int defBtn3 = 202;   // 470
const int defBtn4 = 409;   // 1K
const int defBtn5 = 95;    // 220
const int defBtn6 = 2304;  // 10K

int btn1;
int btn2;
int btn3;
int btn4;
int btn5;
int btn6;

// As power reduces, btn values increase.
// Offset is a temporary fix for this.
int buttonOffset = 0;
// How much button vals are allowed to differ from read value
int buttonValRange = 30;

byte Func1 = 3;
byte Func2 = 0;
byte Func3 = 1;

// blink time in microseconds
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

bool lightSleep(){
  display.ssd1306_command(SSD1306_DISPLAYOFF); 
  delay(500);
  while (!a_button_is_pressed()){
    esp_light_sleep_start();
  }
  if (!a_button_is_pressed()) return true;
  else {
    display.ssd1306_command(SSD1306_DISPLAYON);
    return false;
  }
}

void drawMainUI() {
  display.clearDisplay();
  
  display.drawLine(0, 0, SCREEN_WIDTH, 0, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(3, 2);
  display.print("Watch 5.0");
  
  if (wifiConnected) {
    display.setCursor(80, 2);
    display.print("W");
  }
  
  display.setCursor(100, 2);
  display.print("[");
  if (selectedFunction < 10) display.print("0");
  display.print(selectedFunction);
  display.print("]");
  
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  
  display.drawRect(5, 20, SCREEN_WIDTH - 10, 25, SSD1306_WHITE);
  display.setTextSize(2);
  int titleLen = strlen(Functions[selectedFunction - 1]);
  int titleX = (SCREEN_WIDTH - (titleLen * 12)) / 2;
  display.setCursor(titleX, 26);
  display.print(Functions[selectedFunction - 1]);
  
  display.drawLine(0, 48, SCREEN_WIDTH, 48, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(40, 51);
  display.print("< SEL >");
  
  int barWidth = (selectedFunction - 1) * SCREEN_WIDTH / (totalFunctions-1);
  display.drawRect(0, 60, SCREEN_WIDTH, 4, SSD1306_WHITE);
  display.fillRect(0, 60, barWidth, 4, SSD1306_WHITE);
  
  display.display();
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
      // Skip button allows early exit
      if (button_is_pressed(btn6)) {
        WiFi.disconnect();
        return;
      }

      delay(500);
      attempts++;
      currentStep = min(totalSteps - 10, (int)((millis() - startTime) * totalSteps / timeout));

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      
      display.setCursor(34, 5);
      display.print("WATCH 5.0");
      
      display.setCursor(25, 22);
      display.print("INITIALIZING");
      
      int dotCount = (attempts / 2) % 4;
      display.setCursor(96, 22);
      for (int i = 0; i < dotCount; i++) display.print(".");
      
      int barWidth = (currentStep * (SCREEN_WIDTH - 10)) / totalSteps;
      display.drawRect(5, 40, SCREEN_WIDTH - 10, 8, SSD1306_WHITE);
      display.fillRect(6, 41, barWidth, 6, SSD1306_WHITE);
      
      display.setCursor(5, 52);
      display.print(currentStep);
      display.print("%");
      
      display.display();
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true; // This is not strictly necessary, it is just done for consistency and in case the loop breaks somehow
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }
  }
  // This needs to be programmed more cleanly:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(35, 5);
  display.print("WATCH 5.0");
  
  display.setCursor(25, 22);
  display.print("INITIALIZING");
  display.print(".");
  
  int barWidth = SCREEN_WIDTH - 12;
  display.drawRect(5, 40, SCREEN_WIDTH - 10, 8, SSD1306_WHITE);
  display.fillRect(6, 41, barWidth, 6, SSD1306_WHITE);
  
  display.setCursor(5, 52);
  display.print("100%");
  
  display.display();
  wifiConnected = false;
  WiFi.disconnect();
  delay(500);
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);

  esp_sleep_enable_timer_wakeup(100000); // 100ms

  loadBtnVals();
  
  randomSeed(analogRead(1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) {
      digitalWrite(Func1, HIGH); delay(50);
      digitalWrite(Func1, LOW); delay(200);
    }
  }

  Serial.begin(115200);

  initializeNotesNVS(); 
  loadWiFiNetworksFromNVS();

  // Sync time and display startup message
  timeSyncAndUI();
  
  delay(1000);
  
  // If any button is pressed, enter button tuning
  if (a_button_is_pressed()) {
    display.clearDisplay();
    display.display();
    tuneButtonVals();
  }
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
  drawMainUI();

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
  else if (button_is_pressed(btn3, true)) {
    delay(100);
    switch (selectedFunction) {
      case 1:
        watchFuncs();
        break;
      case 2:
        maths();
        break;
      case 3:
        randomNum();
        break;
      case 4:
        counter();
        break;
      case 5:
        games();
        break;
      case 6:
        metronome();
        break;
      case 7:
        notesFunction();
        break;
      case 8:
        calendar();
        break;
      case 9:
        wifiMenu();
        break;
      case 10:
        wifiFuncs();
        break;
      case 11:
        shell();
        break;
      case 12 :
        settings();
        break;
      case 13:
        display.clearDisplay();
        while (lightSleep()){}
        break;
    }
  }
}
