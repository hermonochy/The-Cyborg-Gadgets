// Includes: WiFi connection, Weather data, Time display, Serial WiFi Menu, Multi-network support

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const int btn1, btn2, btn3, btn4, btn5, btn6;
extern bool wifiConnected;
extern Preferences preferences;

WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;
int currentWiFiIndex = 0;

void saveWiFiNetworksToNVS() {
  preferences.begin("wifi", false);
  preferences.putInt("count", wifiNetworkCount);
  for (int i = 0; i < wifiNetworkCount; i++) {
    String keySSID = "ssid" + String(i);
    String keyPASS = "pass" + String(i);
    preferences.putString(keySSID.c_str(), wifiNetworks[i].ssid);
    preferences.putString(keyPASS.c_str(), wifiNetworks[i].password);
  }
  preferences.end();
}

void loadWiFiNetworksFromNVS() {
  preferences.begin("wifi", true);
  wifiNetworkCount = preferences.getInt("count", 0);
  for (int i = 0; i < wifiNetworkCount && i < MAX_WIFI_NETWORKS; i++) {
    String keySSID = "ssid" + String(i);
    String keyPASS = "pass" + String(i);
    String storedSSID = preferences.getString(keySSID.c_str(), "");
    String storedPASS = preferences.getString(keyPASS.c_str(), "");
    strncpy(wifiNetworks[i].ssid, storedSSID.c_str(), MAX_WIFI_SSID - 1);
    wifiNetworks[i].ssid[MAX_WIFI_SSID - 1] = '\0';
    strncpy(wifiNetworks[i].password, storedPASS.c_str(), MAX_WIFI_PASS - 1);
    wifiNetworks[i].password[MAX_WIFI_PASS - 1] = '\0';
  }
  preferences.end();
  currentWiFiIndex = 0;
}

void connectWiFi() {
  if (wifiNetworkCount == 0) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("No WiFi networks saved!");
    display.display();
    delay(2000);
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Connecting to:");
  display.setCursor(0, 20);
  display.print(wifiNetworks[currentWiFiIndex].ssid);
  display.display();
  
  WiFi.begin(wifiNetworks[currentWiFiIndex].ssid, wifiNetworks[currentWiFiIndex].password);
  
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
    display.println(WiFi.localIP());
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

void addWiFiNetworkOnWatch() {
  if (wifiNetworkCount >= MAX_WIFI_NETWORKS) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Max networks (5) reached!");
    display.display();
    delay(2000);
    return;
  }
  
  char newSSID[MAX_WIFI_SSID] = "";
  char newPassword[MAX_WIFI_PASS] = "";
  
  if (!inputStringOnWatch("SSID:", newSSID, MAX_WIFI_SSID)) return;
  
  if (!inputStringOnWatch("Password:", newPassword, MAX_WIFI_PASS)) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Confirm?");
  display.setCursor(0, 20);
  display.print("SSID: ");
  display.println(newSSID);
  display.setCursor(0, 35);
  display.print("Pass: ");
  for (int i = 0; i < strlen(newPassword); i++) display.print("*");
  display.setCursor(0, 50);
  display.print("3:Yes 6:Cancel");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn3)) {
      strncpy(wifiNetworks[wifiNetworkCount].ssid, newSSID, MAX_WIFI_SSID - 1);
      wifiNetworks[wifiNetworkCount].ssid[MAX_WIFI_SSID - 1] = '\0';
      strncpy(wifiNetworks[wifiNetworkCount].password, newPassword, MAX_WIFI_PASS - 1);
      wifiNetworks[wifiNetworkCount].password[MAX_WIFI_PASS - 1] = '\0';
      
      wifiNetworkCount++;
      saveWiFiNetworksToNVS();
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Network added!");
      display.setCursor(0, 35);
      display.print("Total: ");
      display.print(wifiNetworkCount);
      display.print("/5");
      display.display();
      delay(2000);
      return;
    }
    if (button_is_pressed(btn6)) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Cancelled");
      display.display();
      delay(1000);
      return;
    }
    delay(50);
  }
}

bool inputStringOnWatch(const char* label, char* buffer, int maxLen) {
  int cursorPos = 0;
  buffer[0] = '\0';
  
  char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_@";
  int charsetSize = strlen(charset);
  int charIndex = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(label);
    
    display.setCursor(0, 20);
    display.print("> ");
    display.print(buffer);
    if (cursorPos == strlen(buffer)) {
      display.print("_");
    }
    
    display.setCursor(0, 30);
    display.print("Char: ");
    display.setTextSize(2);
    display.setCursor(40, 28);
    display.print(charset[charIndex]);
    
    display.display();
    
    if (button_is_pressed(btn1)) {
      charIndex = (charIndex - 1 + charsetSize) % charsetSize;
      delay(100);
    }
    else if (button_is_pressed(btn2)) {
      charIndex = (charIndex + 1) % charsetSize;
      delay(100);
    }
    else if (button_is_pressed(btn3)) {
      if (strlen(buffer) < maxLen - 1) {
        buffer[strlen(buffer)] = charset[charIndex];
        buffer[strlen(buffer) + 1] = '\0';
      }
      delay(150);
    }
    else if (button_is_pressed(btn4)) {
      if (strlen(buffer) > 0) {
        buffer[strlen(buffer) - 1] = '\0';
      }
      delay(150);
    }
    else if (button_is_pressed(btn5)) {
      buffer[0] = '\0';
      delay(150);
    }
    else if (button_is_pressed(btn6)) {
      if (strlen(buffer) > 0) {
        return true;
      } else {
        return false;
      }
    }
    delay(30);
  }
}

void wifiNetworkMenu() {
  int selectedIdx = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi Networks (");
    display.print(wifiNetworkCount);
    display.print("/");
    display.print(MAX_WIFI_NETWORKS);
    display.println(")");
    
    if (wifiNetworkCount == 0) {
      display.setCursor(0, 20);
      display.println("No networks saved.");
      display.setCursor(0, 35);
      display.println("Button 4 to add");
      display.println("a new network.");
    } else {
      for (int i = 0; i < wifiNetworkCount; i++) {
        display.setCursor(0, 20 + i * 10);
        if (i == selectedIdx) {
          display.print("> ");
        } else {
          display.print("  ");
        }
        display.print(i + 1);
        display.print(": ");
        display.print(wifiNetworks[i].ssid);
        
        if (wifiConnected && i == currentWiFiIndex) {
          display.setCursor(SCREEN_WIDTH - 18, 20 + i * 10);
          display.print("[*]");
        }
      }
    }
    
    display.display();
    
    if (wifiNetworkCount > 0) {
      if (button_is_pressed(btn1)) {
        selectedIdx = (selectedIdx - 1 + wifiNetworkCount) % wifiNetworkCount;
        delay(150);
      }
      else if (button_is_pressed(btn2)) {
        selectedIdx = (selectedIdx + 1) % wifiNetworkCount;
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        currentWiFiIndex = selectedIdx;
        connectWiFi();
        delay(200);
      }
      else if (button_is_pressed(btn5)) {
        deleteWiFiNetwork(selectedIdx);
        if (selectedIdx >= wifiNetworkCount && wifiNetworkCount > 0) {
          selectedIdx = wifiNetworkCount - 1;
        }
        delay(200);
      }
    } else {
      if (button_is_pressed(btn4)) {
        addWiFiNetworkOnWatch();
        delay(200);
      }
    }
    
    if (wifiNetworkCount > 0 && button_is_pressed(btn4)) {
      addWiFiNetworkOnWatch();
      delay(200);
    }
    
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(50);
  }
}

void deleteWiFiNetwork(int idx) {
  if (idx < 0 || idx >= wifiNetworkCount) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Delete?");
  display.setCursor(0, 20);
  display.print(wifiNetworks[idx].ssid);
  display.setCursor(0, 50);
  display.print("3:Yes 6:Cancel");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn3)) {
      for (int i = idx; i < wifiNetworkCount - 1; i++) {
        wifiNetworks[i] = wifiNetworks[i + 1];
      }
      wifiNetworkCount--;
      saveWiFiNetworksToNVS();
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.println("Network deleted!");
      display.display();
      delay(1000);
      return;
    }
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(50);
  }
}

void wifiMenu(void) {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi Menu");
    display.setCursor(0, 20);
    
    if (wifiNetworkCount == 0) {
      display.println("No networks saved.");
      display.setCursor(0, 35);
      display.println("Button 3 to setup");
      display.println("new network.");
    } else {
      display.print("Networks: ");
      display.print(wifiNetworkCount);
      display.println("/5");
      display.setCursor(0, 35);
      display.print("Current: ");
      display.println(wifiNetworks[currentWiFiIndex].ssid);
      display.setCursor(0, 48);
      if (wifiConnected) {
        display.print("Status: Connected");
      } else {
        display.print("Status: Offline");
      }
    }
    
    display.setCursor(0, 56);
    display.print("3:Networks 6:Back");
    display.display();
    
    if (button_is_pressed(btn3)) {
      wifiNetworkMenu();
      delay(200);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    delay(50);
  }
}

void addWiFiNetworkSerial() {
  if (wifiNetworkCount >= MAX_WIFI_NETWORKS) {
    Serial.println("\n✗ Max WiFi networks (5) reached!");
    return;
  }
  
  Serial.println("\n--- Add New WiFi Network ---");
  
  Serial.print("Enter SSID: ");
  while (!Serial.available()) delay(10);
  String inputSSID = Serial.readStringUntil('\n');
  inputSSID.trim();
  
  if (inputSSID.length() == 0) {
    Serial.println("✗ SSID cannot be empty!");
    return;
  }
  
  if (inputSSID.length() > MAX_WIFI_SSID - 1) {
    Serial.println("✗ SSID too long (max 31 characters)");
    return;
  }
  
  Serial.print("Enter Password: ");
  while (!Serial.available()) delay(10);
  String inputPassword = Serial.readStringUntil('\n');
  inputPassword.trim();
  
  if (inputPassword.length() > MAX_WIFI_PASS - 1) {
    Serial.println("✗ Password too long (max 63 characters)");
    return;
  }
  
  strncpy(wifiNetworks[wifiNetworkCount].ssid, inputSSID.c_str(), MAX_WIFI_SSID - 1);
  wifiNetworks[wifiNetworkCount].ssid[MAX_WIFI_SSID - 1] = '\0';
  strncpy(wifiNetworks[wifiNetworkCount].password, inputPassword.c_str(), MAX_WIFI_PASS - 1);
  wifiNetworks[wifiNetworkCount].password[MAX_WIFI_PASS - 1] = '\0';
  
  wifiNetworkCount++;
  saveWiFiNetworksToNVS();
  
  Serial.println("\n✓ Network added!");
  Serial.print("  SSID: ");
  Serial.println(inputSSID);
  Serial.print("  Total networks: ");
  Serial.print(wifiNetworkCount);
  Serial.println("/5");
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Network added via");
  display.println("Serial!");
  display.display();
  delay(1500);
}

void listWiFiNetworksSerial() {
  Serial.println("\n--- Saved WiFi Networks ---");
  
  if (wifiNetworkCount == 0) {
    Serial.println("No networks saved.");
  } else {
    for (int i = 0; i < wifiNetworkCount; i++) {
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.println(wifiNetworks[i].ssid);
    }
  }
}

void deleteWiFiNetworkSerial() {
  if (wifiNetworkCount == 0) {
    Serial.println("\n✗ No networks to delete!");
    return;
  }
  
  Serial.println("\n--- Delete WiFi Network ---");
  listWiFiNetworksSerial();
  
  Serial.print("\nEnter network number to delete (1-");
  Serial.print(wifiNetworkCount);
  Serial.print("): ");
  while (!Serial.available()) delay(10);
  int netNum = Serial.parseInt();
  Serial.println(netNum);
  
  if (netNum < 1 || netNum > wifiNetworkCount) {
    Serial.println("✗ Invalid network number!");
    return;
  }
  
  int idx = netNum - 1;
  Serial.print("Delete '");
  Serial.print(wifiNetworks[idx].ssid);
  Serial.print("'? (y/n): ");
  while (!Serial.available()) delay(10);
  char response = Serial.read();
  Serial.println(response);
  
  if (response == 'y' || response == 'Y') {
    for (int i = idx; i < wifiNetworkCount - 1; i++) {
      wifiNetworks[i] = wifiNetworks[i + 1];
    }
    wifiNetworkCount--;
    saveWiFiNetworksToNVS();
    Serial.println("✓ Network deleted!");
  } else {
    Serial.println("Cancelled");
  }
}

void connectWiFiSerial() {
  if (wifiNetworkCount == 0) {
    Serial.println("\n✗ No WiFi networks saved!");
    return;
  }
  
  Serial.println("\n--- Connect to WiFi ---");
  listWiFiNetworksSerial();
  
  Serial.print("\nEnter network number (1-");
  Serial.print(wifiNetworkCount);
  Serial.print("): ");
  while (!Serial.available()) delay(10);
  int netNum = Serial.parseInt();
  Serial.println(netNum);
  
  if (netNum < 1 || netNum > wifiNetworkCount) {
    Serial.println("✗ Invalid network number!");
    return;
  }
  
  currentWiFiIndex = netNum - 1;
  Serial.print("\nConnecting to: ");
  Serial.println(wifiNetworks[currentWiFiIndex].ssid);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi");
  display.println("via Serial...");
  display.display();
  
  WiFi.begin(wifiNetworks[currentWiFiIndex].ssid, wifiNetworks[currentWiFiIndex].password);
  
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
  }
  
  Serial.print("\nSaved Networks: ");
  Serial.print(wifiNetworkCount);
  Serial.println("/5");
  listWiFiNetworksSerial();
}

void serialWiFiMenu(void) {
  while (true) {
    Serial.println("\n========== WATCH 5.0 SERIAL WiFi MENU ==========");
    Serial.println("1. Add WiFi Network");
    Serial.println("2. List Networks");
    Serial.println("3. Connect to Network");
    Serial.println("4. Delete Network");
    Serial.println("5. Show WiFi Status");
    Serial.println("6. Disconnect WiFi");
    Serial.println("7. Exit Menu");
    Serial.println("================================================");
    Serial.print("Enter option (1-7): ");
    
    while (!Serial.available()) delay(10);
    char option = Serial.read();
    Serial.println(option);
    
    while (Serial.available()) Serial.read();
    
    switch (option) {
      case '1':
        addWiFiNetworkSerial();
        break;
      case '2':
        listWiFiNetworksSerial();
        break;
      case '3':
        connectWiFiSerial();
        break;
      case '4':
        deleteWiFiNetworkSerial();
        break;
      case '5':
        serialShowWiFiStatus();
        break;
      case '6':
        serialDisconnectWiFi();
        break;
      case '7':
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
