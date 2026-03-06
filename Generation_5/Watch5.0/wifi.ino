// Includes: WiFi connection, Weather data, Time display, Serial WiFi Menu

#include <HTTPClient.h>
#include <ArduinoJson.h>

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const int btn1, btn2, btn3, btn4, btn5, btn6;
extern char ssid[32];
extern char password[64];
extern bool wifiConnected;
extern Preferences preferences;

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

void saveWiFiCredentials(void) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
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
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi Menu");
    display.setCursor(0, 20);
    
    if (strlen(ssid) == 0) {
      display.println("No credentials!");
      display.println("Use Serial menu");
      display.println("to configure.");
    } else {
      display.print("SSID: ");
      display.print(ssid);
      display.setCursor(0, 30);
      if (wifiConnected) {
        display.print("Status: Connected");
      } else {
        display.print("Status: Disconnected");
      }
    }
    display.setCursor(0, 40);
    display.print(WiFi.RSSI());
    display.print(" dBm");

    display.setCursor(0, 56);
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

// ============ SERIAL WiFi MENU FUNCTIONS ============

void serialConfigureWiFi(void) {
  Serial.println("\n--- WiFi Configuration ---");
  
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
  
  Serial.print("Enter Password: ");
  while (!Serial.available()) delay(10);
  String inputPassword = Serial.readStringUntil('\n');
  inputPassword.trim();
  
  if (inputPassword.length() > 63) {
    Serial.println("Password too long (max 63 characters)");
    return;
  }
  
  strncpy(ssid, inputSSID.c_str(), 31);
  ssid[31] = '\0';
  strncpy(password, inputPassword.c_str(), 63);
  password[63] = '\0';
  
  saveWiFiCredentials();
  
  Serial.println("\n✓ Credentials saved successfully!");
  Serial.print("  SSID: ");
  Serial.println(ssid);
  Serial.println("  Password: (hidden)");
  
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
    Serial.println("��� Connected!");
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
    Serial.println("\n========== WATCH 5.0 SERIAL MENU ==========");
    Serial.println("1. Configure WiFi Credentials");
    Serial.println("2. Connect to WiFi");
    Serial.println("3. Disconnect WiFi");
    Serial.println("4. Show WiFi Status");
    Serial.println("5. Clear Credentials");
    Serial.println("6. Exit Menu");
    Serial.println("==========================================");
    Serial.print("Enter option (1-6): ");
    
    while (!Serial.available()) delay(10);
    char option = Serial.read();
    Serial.println(option);
    
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
    display.print("WiFi not connected");
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
  String url = "https://api.open-meteo.com/v1/forecast?latitude=51.754642&longitude=0.0&current=temperature_2m,weather_code,relative_humidity_2m,wind_speed_10m&timezone=auto";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    
    float temp = doc["current"]["temperature_2m"];
    int weatherCode = doc["current"]["weather_code"];
    int humidity = doc["current"]["relative_humidity_2m"];
    float windSpeed = doc["current"]["wind_speed_10m"];
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Weather");
    
    display.setCursor(0, 20);
    display.print("Temp: ");
    display.print((int)temp);
    display.print("C");
    
    display.setCursor(0, 30);
    const char* weatherDesc;
    if (weatherCode == 0) weatherDesc = "Sunny";
    else if (weatherCode == 1) weatherDesc = "Clear";
    else if (weatherCode < 3) weatherDesc = "Cloudy";
    else if (weatherCode < 50) weatherDesc = "Rainy";
    else if (weatherCode < 60) weatherDesc = "Rain";
    else if (weatherCode < 80) weatherDesc = "Snow";
    else if (weatherCode < 100) weatherDesc = "Thunder";
    else weatherDesc = "Unknown";
    
    display.print("Condition: ");
    display.print(weatherDesc);
    
    display.setCursor(0, 40);
    display.print("Humidity: ");
    display.print(humidity);
    display.print("%");
    
    display.setCursor(0, 50);
    display.print("Wind: ");
    display.print((int)windSpeed);
    display.print(" km/h");
    
    display.display();
    while (true) {
      if(button_is_pressed(btn6)) break;  
    }
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