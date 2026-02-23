// Watch 5.0: Initial 5th gen watch with OLED - ESP32C3 with WiFi & Serial Input.

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "tinyexpr.h"
#include <Wire.h>
#include <ctype.h>
#include <math.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define totalFunctions 12
#define MAX_NOTES 5
#define MAX_NOTE_LENGTH 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences preferences;

int selectedFunction = 1;

const char *Functions[] = {"Outputs", "Maths", "Random", "Convert", "Score", "Shooter", "Metronome", "Notes", "WiFi", "Weather", "Time", "Debug"};

const byte buttonPin = 2;

// Button resistance values (Ordered by frequency used)
const int btn1 = 1433; // 4.7K
const int btn2 = 812;  // 2.2K
const int btn3 = 202;  // 470
const int btn4 = 409;  // 1K
const int btn5 = 95;   // 220
const int btn6 = 2304; // 10K

const byte Func1 = 3;
const byte Func2 = 0;
const byte Func3 = 1;

// WiFi credentials (stored in NVS)
char ssid[32] = "";
char password[64] = "";
bool wifiConnected = false;

// Notes storage (in RAM for quick access)
struct Note {
  char text[MAX_NOTE_LENGTH];
  bool used;
};

Note notes[MAX_NOTES];

bool button_is_pressed(int btnVal, bool onlyOnce = true) {
  int pinVal = analogRead(buttonPin);
  int errorVal = pinVal - btnVal;
  int absErrorVal = abs(errorVal);
  
  if (absErrorVal <= 10) {    
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

void saveWiFiCredentials(void) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
}

void loadWiFiCredentials(void) {
  preferences.begin("wifi", true);
  String storedSSID = preferences.getString("ssid", "");
  String storedPassword = preferences.getString("password", "");
  preferences.end();
  
  if (storedSSID.length() > 0) {
    strncpy(ssid, storedSSID.c_str(), 31);
    strncpy(password, storedPassword.c_str(), 63);
  }
}

void wifiSetup(void) {
  bool enteringSSID = true;
  int charIndex = 0;
  char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_";
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi Setup");
  display.setCursor(0, 20);
  display.print("Use Serial Console!");
  display.display();
  delay(1500);
}

void connectWiFi(void) {
  if (strlen(ssid) == 0) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Configure WiFi first!");
    display.println("Use Serial menu.");
    display.display();
    delay(2000);
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Connecting to WiFi...");
  display.setCursor(0, 20);
  display.print(ssid);
  display.display();
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    display.setCursor(0, 30);
    display.print(".");
    display.display();
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Connected!");
    display.setCursor(0, 20);
    display.print("IP: ");
    display.print(WiFi.localIP());
    display.display();
    delay(2000);
    
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  } else {
    wifiConnected = false;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Failed to connect");
    display.display();
    delay(2000);
  }
}

void wifiMenu(void) {
  int option = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi Menu");
    display.setCursor(0, 18);
    
    if (strlen(ssid) == 0) {
      display.println("No credentials!");
      display.println("Use Serial menu");
      display.println("to configure.");
    } else {
      display.print("SSID: ");
      display.println(ssid);
      
      if (wifiConnected) {
        display.println("Status: Connected");
      } else {
        display.println("Status: Disconnected");
      }
    }
    
    display.setCursor(0, 45);
    display.setTextSize(0.8);
    display.println("3:Connect 6:Menu");
    display.display();
    
    if (button_is_pressed(btn3)) {
      connectWiFi();
      delay(200);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    delay(50);
  }
}

void printSerialMenu(void) {
  Serial.println("\n========== WATCH 5.0 SERIAL MENU ==========");
  Serial.println("1. Configure WiFi Credentials");
  Serial.println("2. Connect to WiFi");
  Serial.println("3. Disconnect WiFi");
  Serial.println("4. Show WiFi Status");
  Serial.println("5. Clear Credentials");
  Serial.println("6. Exit Menu");
  Serial.println("==========================================");
  Serial.print("Enter option (1-6): ");
}

void serialConfigureWiFi(void) {
  Serial.println("\n--- WiFi Configuration ---");
  
  // Get SSID
  Serial.print("Enter SSID: ");
  while (!Serial.available()) delay(10);
  String inputSSID = Serial.readStringUntil('\n');
  inputSSID.trim();
  
  if (inputSSID.length() == 0) {
    Serial.println("SSID cannot be empty!");
    return;
  }
  
  if (inputSSID.length() > 31) {
    Serial.println("SSID too long (max 31 characters)");
    return;
  }
  
  // Get Password
  Serial.print("Enter Password: ");
  while (!Serial.available()) delay(10);
  String inputPassword = Serial.readStringUntil('\n');
  inputPassword.trim();
  
  if (inputPassword.length() > 63) {
    Serial.println("Password too long (max 63 characters)");
    return;
  }
  
  // Save credentials
  strncpy(ssid, inputSSID.c_str(), 31);
  ssid[31] = '\0';
  strncpy(password, inputPassword.c_str(), 63);
  password[63] = '\0';
  
  saveWiFiCredentials();
  
  Serial.println("\n✓ Credentials saved successfully!");
  Serial.print("  SSID: ");
  Serial.println(ssid);
  Serial.println("  Password: (hidden)");
  
  // Update display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("WiFi credentials");
  display.println("saved via Serial!");
  display.display();
  delay(2000);
}

void serialConnectWiFi(void) {
  if (strlen(ssid) == 0) {
    Serial.println("\n✗ No WiFi credentials configured!");
    Serial.println("  Please configure first.");
    return;
  }
  
  Serial.print("\nConnecting to WiFi: ");
  Serial.println(ssid);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi");
  display.println("via Serial...");
  display.display();
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("✓ Connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Connected!");
    display.setCursor(0, 20);
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
    
    // Sync time with NTP
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("  Syncing time with NTP...");
  } else {
    wifiConnected = false;
    Serial.println("✗ Failed to connect");
    Serial.print("  WiFi Status: ");
    Serial.println(WiFi.status());
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Connection failed!");
    display.display();
    delay(2000);
  }
}

void serialDisconnectWiFi(void) {
  WiFi.disconnect();
  wifiConnected = false;
  Serial.println("\n✓ Disconnected from WiFi");
}

void serialShowWiFiStatus(void) {
  Serial.println("\n--- WiFi Status ---");
  
  if (wifiConnected && WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ Connected");
    Serial.print("  SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("✗ Not connected");
    if (strlen(ssid) > 0) {
      Serial.print("  Configured SSID: ");
      Serial.println(ssid);
    } else {
      Serial.println("  No credentials configured");
    }
  }
}

void serialClearCredentials(void) {
  Serial.print("\nClear WiFi credentials? (y/n): ");
  while (!Serial.available()) delay(10);
  char response = Serial.read();
  Serial.println(response);
  
  if (response == 'y' || response == 'Y') {
    ssid[0] = '\0';
    password[0] = '\0';
    preferences.begin("wifi", false);
    preferences.putString("ssid", "");
    preferences.putString("password", "");
    preferences.end();
    Serial.println("✓ Credentials cleared");
  } else {
    Serial.println("Cancelled");
  }
}

void serialWiFiMenu(void) {
  while (true) {
    printSerialMenu();
    
    while (!Serial.available()) delay(10);
    char option = Serial.read();
    Serial.println(option);
    
    // Clear any remaining serial data
    while (Serial.available()) Serial.read();
    
    switch (option) {
      case '1':
        serialConfigureWiFi();
        break;
      case '2':
        serialConnectWiFi();
        break;
      case '3':
        serialDisconnectWiFi();
        break;
      case '4':
        serialShowWiFiStatus();
        break;
      case '5':
        serialClearCredentials();
        break;
      case '6':
        Serial.println("\nExiting menu...");
        return;
      default:
        Serial.println("✗ Invalid option");
    }
    
    delay(500);
  }
}

void getWeather(void) {
  if (!wifiConnected) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Please connect to WiFi");
    display.display();
    delay(2000);
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Fetching weather...");
  display.display();
  
  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=51.754642&longitude=0.0&current=temperature_2m,weather_code&timezone=auto";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    
    float temp = doc["current"]["temperature_2m"];
    int weatherCode = doc["current"]["weather_code"];
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Weather");
    
    display.setTextSize(3);
    display.setCursor(20, 20);
    display.print((int)temp);
    display.print("F");
    
    display.setTextSize(1);
    display.setCursor(0, 50);
    const char* weatherDesc = "Unknown";
    if (weatherCode == 0) weatherDesc = "Clear";
    else if (weatherCode < 3) weatherDesc = "Cloudy";
    else if (weatherCode < 50) weatherDesc = "Rainy";
    else if (weatherCode < 60) weatherDesc = "Rain";
    else if (weatherCode < 80) weatherDesc = "Snow";
    else weatherDesc = "Thunder";
    
    display.print(weatherDesc);
    display.display();
    
    delay(5000);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Weather fetch failed");
    display.display();
    delay(2000);
  }
  
  http.end();
}

void displayTime(void) {
  while (true) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Current Time");
    
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.printf("%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.printf("%s %d, %d", 
      (timeinfo->tm_mon == 0 ? "Jan" : timeinfo->tm_mon == 1 ? "Feb" : timeinfo->tm_mon == 2 ? "Mar" : 
       timeinfo->tm_mon == 3 ? "Apr" : timeinfo->tm_mon == 4 ? "May" : timeinfo->tm_mon == 5 ? "Jun" : 
       timeinfo->tm_mon == 6 ? "Jul" : timeinfo->tm_mon == 7 ? "Aug" : timeinfo->tm_mon == 8 ? "Sep" : 
       timeinfo->tm_mon == 9 ? "Oct" : timeinfo->tm_mon == 10 ? "Nov" : "Dec"),
      timeinfo->tm_mday, timeinfo->tm_year + 1900);
    
    display.display();
    
    if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(1000);
  }
}

void saveNotesToNVS(void) {
  preferences.begin("notes", false);
  
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20];
    char keyUsed[20];
    
    sprintf(keyText, "note%d_text", i);
    sprintf(keyUsed, "note%d_used", i);
    
    preferences.putString(keyText, notes[i].text);
    preferences.putBool(keyUsed, notes[i].used);
  }
  
  preferences.end();
}

void loadNotesFromNVS(void) {
  preferences.begin("notes", true);
  
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20];
    char keyUsed[20];
    
    sprintf(keyText, "note%d_text", i);
    sprintf(keyUsed, "note%d_used", i);
    
    String noteText = preferences.getString(keyText, "");
    notes[i].used = preferences.getBool(keyUsed, false);
    
    strncpy(notes[i].text, noteText.c_str(), MAX_NOTE_LENGTH - 1);
    notes[i].text[MAX_NOTE_LENGTH - 1] = '\0';
  }
  
  preferences.end();
}

void initializeNotesNVS(void) {
  loadNotesFromNVS();
}

// ===== OTHER FUNCTIONS (from previous code) =====
void activateFunc(byte func, int blinkTime = 500){
  bool blink = false;
  bool keepOn = false;

  while (true){
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    } 
    else {
      if (button_is_pressed(btn1)) digitalWrite(func, HIGH);
      else digitalWrite(func, LOW);
    }

    if (button_is_pressed(btn2, true)){
      keepOn = !keepOn;
      if (keepOn) blink = false;
    } 
    else if (button_is_pressed(btn3, true)){
      blink = !blink;
      if (blink) keepOn = false;
    }
    else if (button_is_pressed(btn6)){
      return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Quick Flash");
    display.setCursor(0, 30);
    display.println("2. Always On");
    display.setCursor(0, 40);
    display.println("3. Blink");
    display.setCursor(0, 50);
    display.println("6. Return");
    
    display.setTextSize(2);
    display.setCursor(5, 0);
    display.print(digitalRead(func) ? "On" : "Off");
    display.display();
  }
}

void watchFuncs(void) {

  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. White LED");
    display.setCursor(0, 30);
    display.println("2. Laser");
    display.setCursor(0, 40);
    display.println("3. UV LED");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) activateFunc(Func1);
    else if (button_is_pressed(btn2)) activateFunc(Func2);
    else if (button_is_pressed(btn3)) activateFunc(Func3);
    else if (button_is_pressed(btn6)) return;
  }
}

void calculator() {
  const char* screen1[] = {"1","2","3","4","5","6","7","8","9","0",".","pi"};
  const int screen1_size = sizeof(screen1) / sizeof(screen1[0]);
  const char* screen2[] = {"+","-","*","/","(",")","^","%","!","sin","cos","tan","asn","acs","atn"};
  const int screen2_size = sizeof(screen2) / sizeof(screen2[0]);
  
  bool onScreen1 = true;
  int selected = 0;
  char expr[64] = "";
  int exprLen = 0;
  bool showResult = false;
  double result = 0;
  bool error = false;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    if (showResult) {
      if (error) display.print("Error!");
      else {
        display.print("= ");
        display.print(result);
      }
    }
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print(expr);

    int y;
    int x;
    int spacing = 20;
    const char** activeScreen = onScreen1 ? screen1 : screen2;
    int activeSize = onScreen1 ? screen1_size : screen2_size;
    for (int i = 0; i < activeSize; i++) {
      x = (i % 6) * spacing;
      y = 20 + (i / 6) * 10;
      if (i == selected) {
        display.fillRect(x-1, y-1, spacing, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(x, y);
        display.print(activeScreen[i]);
        display.setTextColor(SSD1306_WHITE);
      } else {
        display.setCursor(x, y);
        display.print(activeScreen[i]);
      }
    }
    display.display();

    if (button_is_pressed(btn2)) {
      selected = (selected+1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn1)) {
      selected = (selected-1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn3)) {
      if (exprLen < 63) {
        if (onScreen1 && strcmp(activeScreen[selected], "pi") == 0) {
          strcat(expr, "pi");
          exprLen += 2;
        } else {
          strcat(expr, activeScreen[selected]);
          exprLen = strlen(expr);
        }
      }
      delay(120);
    }
    else if (button_is_pressed(btn4)) {
      int len = strlen(expr);
      if (len > 0) {
        if (len >= 2 && expr[len-2] == 'p' && expr[len-1] == 'i') {
          expr[len-2] = '\0';
        } else {
          expr[len-1] = '\0';
        }
        exprLen = strlen(expr);
      }
      delay(120);
    }
    else if (button_is_pressed(btn5)) {
      onScreen1 = !onScreen1;
      selected = 0;
      delay(120);
    }
    else if (button_is_pressed(btn6)) {
      if (showResult) return;
      int err;
      te_variable vars[] = {};
      te_expr* te = te_compile(expr, vars, 1, &err);
      if (te) {
        result = te_eval(te);
        te_free(te);
        showResult = true;
        error = false;
      } else {
        showResult = true;
        error = true;
      }
      delay(300);
    }
    delay(40);
  }
}

void unitConverter(void){
    const char* types[] ={
        "cm->in", "in->cm",
        "C->F",   "F->C",
        "kg->lb", "lb->kg",
        "km->mi", "mi->km",
        "g->oz",  "oz->g",
        "L->gal", "gal->L"
    };
    enum{LEN=0, LEN2, TEMP, TEMP2, WT, WT2, KM_MI, MI_KM, G_OZ, OZ_G, L_GAL, GAL_L};
    const int numTypes = sizeof(types) / sizeof(types[0]);
    int selectedType = 0;
    float inputValue = 0;
    bool enteringValue = true;
    float result = 0;
    while (true){
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 25);
        display.print("Type: ");
        display.print(types[selectedType]);
        display.setCursor(0, 50);
        display.print("Input: ");
        display.setTextSize(2);
        display.setCursor(50, 45);
        display.print(inputValue, 2);
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("= ");
        if (!enteringValue)
            display.print(result, 4);
        display.display();
        if (button_is_pressed(btn1)){
            selectedType = (selectedType - 1) % numTypes;
            enteringValue = true;
            delay(250);
        } 
        else if (button_is_pressed(btn2)){
            selectedType = (selectedType + 1) % numTypes;
            enteringValue = true;
            delay(250);
        } 
        else if (button_is_pressed(btn4)){
            if (enteringValue) inputValue--;
            delay(150);
        } 
        else if (button_is_pressed(btn5)){
            if (enteringValue) inputValue++;
            delay(150);
        } 
        else if (button_is_pressed(btn6)){
            if (enteringValue){
                switch (selectedType){
                    case LEN:   result = inputValue / 2.54; break;
                    case LEN2:  result = inputValue * 2.54; break;
                    case TEMP:  result = inputValue * 9.0 / 5.0 + 32.0; break;
                    case TEMP2: result = (inputValue - 32.0) * 5.0 / 9.0; break;
                    case WT:    result = inputValue * 2.20462; break;
                    case WT2:   result = inputValue / 2.20462; break;
                    case KM_MI: result = inputValue * 0.621371; break;
                    case MI_KM: result = inputValue / 0.621371; break;
                    case G_OZ:  result = inputValue * 0.035274; break;
                    case OZ_G:  result = inputValue / 0.035274; break;
                    case L_GAL: result = inputValue * 0.264172; break;
                    case GAL_L: result = inputValue / 0.264172; break;
                }
                enteringValue = false;
                delay(500);
            } 
            else return;
        }
        delay(50);
    }
}

void randomNum(void) {
  int range = 10;
  bool floatMode = false;
  int decimals = 2;
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Random");
    display.setCursor(0, 22);
    display.setTextSize(1);
    display.print("Max:");
    display.print(range);
    display.setCursor(64, 22);
    display.print(floatMode ? "Float" : "Int");
    display.setCursor(0, 44);
    display.print("Dec:");
    display.print(decimals);
    display.setCursor(0, 56);
    display.print("3:tog 4:d.p 5:gen");
    display.display();
    delay(100);
    
    if (button_is_pressed(btn1)) {
      range += 10;
      delay(250);
    } 
    else if (button_is_pressed(btn2)) {
      range = max(1, range - 1);
      delay(250);
    } 
    else if (button_is_pressed(btn5)) {
      if (!floatMode) {
        int r = random(0, range + 1);
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(10, 25);
        display.print(r);
        display.display();
        delay(2000);
      } else {
        double u = (random(0, 32767) / 32767.0);
        double val = u * range;
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(0, 18);
        display.print(val, decimals);
        display.display();
        delay(2000);
      }
    } 
    else if (button_is_pressed(btn3)) {
      floatMode = !floatMode;
      delay(250);
    }
    else if (button_is_pressed(btn4)) {
      decimals = min(6, decimals + 1);
      delay(200);
    }
    else if (button_is_pressed(btn6, true)) {
      return;
    }
  }
}

int score1 = 0;
int score2 = 0;
void counter(void){
  while (true){
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(10, 5);
    display.print("Score Counter");

    display.setTextSize(3);
    display.setCursor(0, 30);
    display.print(score1);
    display.print(" : ");
    display.print(score2);
    display.display();

    if (button_is_pressed(btn1)){
      ++score1;
      delay(150);
    }
    else if (button_is_pressed(btn2)){
      ++score2;
      delay(150);
    }
    else if (button_is_pressed(btn3)){
    score1 = 0;
    score2 = 0;  
    }
    
    else if (button_is_pressed(btn4)){
      --score1;
      delay(150);
    }
    else if (button_is_pressed(btn5)){
      --score2;
      delay(150);
    }
    else if (button_is_pressed(btn6)){
      return;
    }
    delay(50);
  }
}

int bpm = 100;
void metronome(void){
  const int MIN_BPM = 1;
  const unsigned long PULSE_MS = 10;
  unsigned long lastBeat = millis();
  unsigned long ledOffAt = 0;
  volatile byte Func = Func2;
  
  const unsigned long HOLD_INITIAL_MS = 400; 
  const unsigned long HOLD_MIN_MS = 1; 
  const float HOLD_ACCEL_FACTOR = 0.75f;     

  unsigned long now = 0;
  unsigned long interval = 60000UL / (unsigned long)max(1, bpm);
  
  bool btn1Held = false;
  bool btn2Held = false;
  unsigned long btn1NextRepeat = 0;
  unsigned long btn2NextRepeat = 0;
  unsigned long btn1RepeatDelay = HOLD_INITIAL_MS;
  unsigned long btn2RepeatDelay = HOLD_INITIAL_MS;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Metronome");
  display.setCursor(0,30);
  display.print(bpm);
  display.print("BPM");
  display.display();
  
  digitalWrite(Func, LOW);

  while (true){
    now = millis();
    static int lastShownBpm = -1;
    if (bpm != lastShownBpm){
      lastShownBpm = bpm;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.print("Metronome");
      display.setCursor(0,30);
      display.print(bpm);
      display.print("BPM");
      display.display();
      
      interval = 60000UL / (unsigned long)max(1, bpm);
    }
    
    if ((unsigned long)(now - lastBeat) >= interval){
      digitalWrite(Func, HIGH);
      ledOffAt = now + PULSE_MS;
      lastBeat = now;
    }

    if (ledOffAt && now >= ledOffAt){
      digitalWrite(Func, LOW);
      ledOffAt = 0;
    }
    
    if (button_is_pressed(btn1)){
      if (!btn1Held){
        btn1Held = true;
        btn1RepeatDelay = HOLD_INITIAL_MS;
        btn1NextRepeat = now + btn1RepeatDelay;
        if (bpm > MIN_BPM) bpm--;
      } else {
        if (now >= btn1NextRepeat){
          if (bpm > MIN_BPM) bpm--;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn1RepeatDelay * HOLD_ACCEL_FACTOR);
          btn1RepeatDelay = newDelay;
          btn1NextRepeat = now + btn1RepeatDelay;
        }
      }
    } else {
      btn1Held = false;
    }

    if (button_is_pressed(btn2)){
      if (!btn2Held){
        btn2Held = true;
        btn2RepeatDelay = HOLD_INITIAL_MS;
        btn2NextRepeat = now + btn2RepeatDelay;
        bpm++;
      } else {
        if (now >= btn2NextRepeat){
          bpm++;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn2RepeatDelay * HOLD_ACCEL_FACTOR);
          btn2RepeatDelay = newDelay;
          btn2NextRepeat = now + btn2RepeatDelay;
        }
      }
    } else {
      btn2Held = false;
    }
    if (button_is_pressed(btn6)){
      digitalWrite(Func, LOW);
      return;
    }  
    yield();
  }
}

void shooterGame(void) {
  int playerX = 120;
  int playerY = 55;
  int score = 0;
  int health = 3;
  
  struct Bullet { int x; int y; bool active; };
  Bullet bullets[8];
  for (int i = 0; i < 8; i++) bullets[i].active = false;
  
  struct Enemy { int x; int y; bool active; };
  Enemy enemies[3];
  for (int i = 0; i < 3; i++) {
    enemies[i].x = (i * 40) + 10;
    enemies[i].y = 5;
    enemies[i].active = true;
  }

  unsigned long lastShot = 0;
  unsigned long lastSpawn = 0;

  while (health > 0) {
    unsigned long now = millis();

    if (button_is_pressed(btn1, false) && playerX > 0) {
      playerX -= 3;
    }
    if (button_is_pressed(btn2, false) && playerX < 120) {
      playerX += 3;
    }
    if (button_is_pressed(btn3, false) && now - lastShot > 200) {
      for (int i = 0; i < 8; i++) {
        if (!bullets[i].active) {
          bullets[i].x = playerX + 2;
          bullets[i].y = playerY - 2;
          bullets[i].active = true;
          lastShot = now;
          break;
        }
      }
    }
    if (button_is_pressed(btn6)) return;

    for (int i = 0; i < 8; i++) {
      if (bullets[i].active) {
        bullets[i].y -= 4;
        if (bullets[i].y < 0) bullets[i].active = false;
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        enemies[i].y += 1;
        if (enemies[i].y > 64) {
          enemies[i].active = false;
          health--;
        }
      }
    }

    if (now - lastSpawn > 2500) {
      for (int i = 0; i < 3; i++) {
        if (!enemies[i].active) {
          enemies[i].x = random(10, 118);
          enemies[i].y = 5;
          enemies[i].active = true;
          lastSpawn = now;
          break;
        }
      }
    }

    for (int b = 0; b < 8; b++) {
      if (bullets[b].active) {
        for (int e = 0; e < 3; e++) {
          if (enemies[e].active) {
            if (bullets[b].x >= enemies[e].x - 2 && bullets[b].x <= enemies[e].x + 6 &&
                bullets[b].y >= enemies[e].y - 2 && bullets[b].y <= enemies[e].y + 6) {
              bullets[b].active = false;
              enemies[e].active = false;
              score += 10;
            }
          }
        }
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        if (playerX >= enemies[i].x - 4 && playerX <= enemies[i].x + 6 &&
            playerY >= enemies[i].y - 4 && playerY <= enemies[i].y + 8) {
          enemies[i].active = false;
          health--;
        }
      }
    }

    display.clearDisplay();
    
    for (int dx = 0; dx < 4; dx++) {
      for (int dy = 0; dy < 6; dy++) {
        display.drawPixel(playerX + dx, playerY + dy, 1);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (bullets[i].active) {
        display.drawPixel(bullets[i].x, bullets[i].y, 1);
        display.drawPixel(bullets[i].x + 1, bullets[i].y, 1);
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        for (int dx = 0; dx < 4; dx++) {
          for (int dy = 0; dy < 4; dy++) {
            display.drawPixel(enemies[i].x + dx, enemies[i].y + dy, 1);
          }
        }
      }
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(score);
    display.setCursor(100, 0);
    display.print("H:");
    display.print(health);
    
    display.display();
    delay(30);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(3000);
}

void notesFunction(void) {
  int selectedNote = 0;
  bool viewingMode = true;

  while (true) {
    if (viewingMode) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Notes [NVS]");
      
      display.setCursor(0, 18);
      for (int i = 0; i < MAX_NOTES; i++) {
        if (i == selectedNote) {
          display.print(">");
        } else {
          display.print(" ");
        }
        display.print(i + 1);
        display.print(": ");
        
        if (notes[i].used) {
          char preview[13];
          strncpy(preview, notes[i].text, 12);
          preview[12] = '\0';
          display.print(preview);
        } else {
          display.print("[Empty]");
        }
        display.setCursor(0, 18 + ((i + 1) * 10));
      }
     
      display.display();
      
      if (button_is_pressed(btn1)) {
        selectedNote = (selectedNote - 1 + MAX_NOTES) % MAX_NOTES;
        delay(150);
      }
      else if (button_is_pressed(btn2)) {
        selectedNote = (selectedNote + 1) % MAX_NOTES;
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        viewingMode = false;
        delay(200);
      }
      else if (button_is_pressed(btn6)) {
        saveNotesToNVS();
        return;
      }
    } else {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Note ");
      display.print(selectedNote + 1);
      display.print(" - Edit");
      
      display.setCursor(0, 18);
      display.print("Text:");
      display.setCursor(0, 30);
      display.print(notes[selectedNote].text);
      display.print("_");
      
      display.setCursor(0, 40);
      display.print("1:Del 2:Add 3:Char");
      display.setCursor(0, 50);
      display.print("4:Clr 5:Done 6:Back");
      display.display();
      
      if (button_is_pressed(btn1)) {
        int len = strlen(notes[selectedNote].text);
        if (len > 0) {
          notes[selectedNote].text[len - 1] = '\0';
          if (len - 1 == 0) {
            notes[selectedNote].used = false;
          }
        }
        delay(150);
      }
      else if (button_is_pressed(btn2)) {
        int len = strlen(notes[selectedNote].text);
        if (len < MAX_NOTE_LENGTH - 1) {
          notes[selectedNote].text[len] = ' ';
          notes[selectedNote].text[len + 1] = '\0';
          notes[selectedNote].used = true;
        }
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-:,!?";
        int charIndex = 0;
        bool selectingChar = true;
        
        while (selectingChar) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("Select Char:");
          display.setCursor(0, 18);
          display.setTextSize(2);
          display.print(charset[charIndex]);
          display.setTextSize(1);
          display.setCursor(0, 30);
          display.print("1:< 2:> 3:Select");
          display.setCursor(0, 40);
          display.print("6:Cancel");
          display.display();
          
          if (button_is_pressed(btn1)) {
            charIndex = (charIndex - 1 + (int)strlen(charset)) % (int)strlen(charset);
            delay(100);
          }
          else if (button_is_pressed(btn2)) {
            charIndex = (charIndex + 1) % (int)strlen(charset);
            delay(100);
          }
          else if (button_is_pressed(btn3)) {
            int len = strlen(notes[selectedNote].text);
            if (len < MAX_NOTE_LENGTH - 1) {
              notes[selectedNote].text[len] = charset[charIndex];
              notes[selectedNote].text[len + 1] = '\0';
              notes[selectedNote].used = true;
            }
            selectingChar = false;
            delay(200);
          }
          else if (button_is_pressed(btn6)) {
            selectingChar = false;
            delay(200);
          }
          delay(50);
        }
      }
      else if (button_is_pressed(btn4)) {
        notes[selectedNote].text[0] = '\0';
        notes[selectedNote].used = false;
        delay(200);
      }
      else if (button_is_pressed(btn5)) {
        saveNotesToNVS();
        viewingMode = true;
        delay(200);
      }
      else if (button_is_pressed(btn6)) {
        saveNotesToNVS();
        viewingMode = true;
        delay(200);
      }
    }
    
    delay(50);
  }
}

void debug() {
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Debug Info");
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("ADC: ");
    display.print(analogRead(buttonPin));
    display.setCursor(0, 30);
    display.print("Func1: ");
    display.print(digitalRead(Func1));
    display.setCursor(0, 40);
    display.print("Func2: ");
    display.print(digitalRead(Func2));
    display.setCursor(0, 50);
    display.print("Func3: ");
    display.print(digitalRead(Func3));
    display.display();
    
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(100);
  }
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
  delay(2000);
  Serial.println("\n\n========================================");
  Serial.println("     Watch 5.0 - Serial Configuration");
  Serial.println("========================================");
  Serial.println("Press any key in Serial Monitor to enter menu...");
  Serial.println("(You have 5 seconds)");
  Serial.println("========================================\n");
  
  // Wait for serial input for 5 seconds
  unsigned long startTime = millis();
  bool enterSerialMenu = false;
  while (millis() - startTime < 5000) {
    if (Serial.available()) {
      Serial.read();
      enterSerialMenu = true;
      break;
    }
    delay(10);
  }
  
  if (enterSerialMenu) {
    serialWiFiMenu();
  }

  initializeNotesNVS();
  loadWiFiCredentials();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.peek();
    if (cmd == 's' || cmd == 'S') {
      Serial.read();
      serialWiFiMenu();
    } else {
      Serial.read();
    }
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Watch 5.0");
  display.setCursor(0, 30);
  display.print(Functions[selectedFunction - 1]);
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
        calculator();
        break;
      case 3:
        randomNum();
        break;
      case 4:
        unitConverter();
        break;
      case 5:
        counter();
        break;
      case 6:
        shooterGame();
        break;
      case 7:
        metronome();
        break;
      case 8:
        notesFunction();
        break;
      case 9:
        wifiMenu();
        break;
      case 10:
        getWeather();
        break;
      case 11:
        displayTime();
        break;
      case 12:
        debug();
        break;
    }
  }
}