  // Watch 5.0: Initial 5th gen watch - ESP32C3 with SSD1306 OLED display

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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

const char *Functions[] = {"Outputs", "Maths", "Random", "Score", "Games", "Metronome", "Notes", "WiFi", "Weather", "Time", "Shell","Calendar", "Settings"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

const byte buttonPin = 2;

// Button resistance values (Ordered by frequency used)
int btn1 = 1433;  // 4.7K
int btn2 = 812;   // 2.2K
int btn3 = 202;   // 470
int btn4 = 409;   // 1K
int btn5 = 95;    // 220
int btn6 = 2304;  // 10K

// As power reduces, btn values increase.
// Offset is a temporary fix for this.
int buttonOffset = 0;
// How much button vals are allowed to differ from read value
int buttonValRange = 30;

byte Func1 = 3;
byte Func2 = 0;
byte Func3 = 1;

int selectedFunction = 1;

bool button_is_pressed(int btnVal, bool onlyOnce = true) {
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

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);

  randomSeed(analogRead(1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) {
      digitalWrite(Func1, HIGH); delay(50);
      digitalWrite(Func1, LOW); delay(200);
    }
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(7, 0);
  display.print("Welcome to");
  display.setCursor(20, 20);
  display.print("Watch 0");
  display.setCursor(30, 50);
  display.print("Gen 5");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();
  Serial.begin(115200);

  initializeNotesNVS(); 
  loadWiFiNetworksFromNVS();

  // Attempt to connect to WiFi for time sync
  timeSync();

  // If any button is pressed, no matter the value, enter button tuning
  if (a_button_is_pressed()) {
    display.clearDisplay();
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
  
  display.clearDisplay();
  
  display.drawLine(0, 0, SCREEN_WIDTH, 0, SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  int titleX = (SCREEN_WIDTH - (strlen(Functions[selectedFunction - 1]) * 12)) / 2;
  display.setCursor(titleX, 20);
  display.print(Functions[selectedFunction - 1]);
  
  display.setTextSize(1);
  display.setCursor(90, 5);
  display.print(selectedFunction);
  display.print("/");
  display.print(totalFunctions);
  
  if (wifiConnected) {
    display.setCursor(3, 5);
    display.print("W");
  }
  
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  
  display.display();

  delay(100);
  
  if (button_is_pressed(btn2)) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  } 
  else if (button_is_pressed(btn1)) {
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
  } 
  else if (button_is_pressed(btn6)) {
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
        wifiMenu();
        break;
      case 9:
        getWeather();
        break;
      case 10:
        displayTime();
        break;
      case 11:
        shell();
        break;
      case 12:
        calendar();
        break;
      case 13 :
        settings();
        break;
    }
  }
}
