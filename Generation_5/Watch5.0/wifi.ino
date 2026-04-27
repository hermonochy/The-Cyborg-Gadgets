// Includes: WiFi connection, Weather data, Time display, Serial WiFi Menu, Multi-network support, Time Sync

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

#define TIME_SETTINGS "time_settings"
#define DEFAULT_TIME_OFFSET 0
#define DEFAULT_12_HOUR_MODE false
#define DEFAULT_DST_OFFSET 0


struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern bool wifiConnected;
extern Preferences preferences;

char ssid[32] = "";
char password[64] = "";

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

void timeSync() {
  if (wifiNetworkCount == 0) {
    delay(2000);
    return;
  }
  int totAttempts;
  int attempts;
  for (int wifiIndex=0; wifiIndex<=wifiNetworkCount; wifiIndex++){
    if (WiFi.status() == WL_CONNECTED) {
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }
    WiFi.begin(wifiNetworks[wifiIndex].ssid, wifiNetworks[wifiIndex].password);
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      totAttempts++;
      attempts++;
      // If there are no wifi's nearby you can just skip this function.
      if (button_is_pressed(btn6)) return;
    }
  }
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

void scanWiFiNetworks() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Scanning networks...");
  display.display();
  
  int numNetworks = WiFi.scanNetworks();
  
  if (numNetworks == 0) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("No networks found");
    display.display();
    delay(2000);
    return;
  }
  
  int indices[numNetworks];
  for (int i = 0; i < numNetworks; i++) {
    indices[i] = i;
  }
  
  for (int i = 0; i < numNetworks - 1; i++) {
    for (int j = 0; j < numNetworks - i - 1; j++) {
      if (WiFi.RSSI(indices[j]) < WiFi.RSSI(indices[j + 1])) {
        int temp = indices[j];
        indices[j] = indices[j + 1];
        indices[j + 1] = temp;
      }
    }
  }
  
  int selectedIdx = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Found: ");
    display.println(numNetworks);

    display.setTextSize(1);

    int displayCount = min(5, numNetworks);
    for (int i = 0; i < displayCount; i++) {
      int idx = indices[i];
      int rssi = WiFi.RSSI(idx);
      
      display.setCursor(0, 20 + i * 9);
      if (i == selectedIdx) {
        display.print("> ");
      } else {
        display.print("  ");
      }
      
      String ssid = WiFi.SSID(idx);
      if (ssid.length() > 12) {
        ssid = ssid.substring(0, 12);
      }
      display.print(ssid);
            
      display.setCursor(110, 20 + i * 9);
      display.print(rssi);
    }
    
    display.display();
    
    if (button_is_pressed(btn1)) {
      selectedIdx = (selectedIdx - 1 + min(5, numNetworks)) % min(5, numNetworks);
      delay(150);
    }
    else if (button_is_pressed(btn2)) {
      selectedIdx = (selectedIdx + 1) % min(5, numNetworks);
      delay(150);
    }
    else if (button_is_pressed(btn3)) {
      int idx = indices[selectedIdx];
      String selectedSSID = WiFi.SSID(idx);
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("SSID: ");
      display.println(selectedSSID);
      display.setCursor(0, 20);
      display.println("Add to saved");
      display.println("networks?");
      display.setCursor(0, 45);
      display.println("3:Yes 6:No");
      display.display();
      
      while (true) {
        if (button_is_pressed(btn3)) {
          if (wifiNetworkCount < MAX_WIFI_NETWORKS) {
            strncpy(wifiNetworks[wifiNetworkCount].ssid, selectedSSID.c_str(), MAX_WIFI_SSID - 1);
            wifiNetworks[wifiNetworkCount].ssid[MAX_WIFI_SSID - 1] = '\0';
            
            char newPassword[MAX_WIFI_PASS] = "";
            if (inputStringOnWatch("Password:", newPassword, MAX_WIFI_PASS)) {
              strncpy(wifiNetworks[wifiNetworkCount].password, newPassword, MAX_WIFI_PASS - 1);
              wifiNetworks[wifiNetworkCount].password[MAX_WIFI_PASS - 1] = '\0';
              
              wifiNetworkCount++;
              saveWiFiNetworksToNVS();
              
              display.clearDisplay();
              display.setTextSize(1);
              display.setCursor(0, 20);
              display.print("Network saved!");
              display.display();
              delay(1500);
            }
          }
          return;
        }
        if (button_is_pressed(btn6)) {
          return;
        }
        delay(50);
      }
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(50);
  }
}

void disconnectWiFi() {
  if (!wifiConnected) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Not connected");
    display.display();
    delay(1500);
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Disconnect?");
  display.setCursor(0, 20);
  display.print("SSID: ");
  display.println(WiFi.SSID());
  display.setCursor(0, 45);
  display.println("3:Yes 6:Cancel");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn3)) {
      WiFi.disconnect(true);
      wifiConnected = false;
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Disconnected!");
      display.display();
      delay(1500);
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
      display.println("No networks saved!");
    } else {
      display.print("Networks: ");
      display.print(wifiNetworkCount);
      display.println("/5");
      display.setCursor(0, 30);
      display.print("Cur: ");
      display.println(wifiNetworks[currentWiFiIndex].ssid);
      display.setCursor(0, 45);
      display.print(WiFi.RSSI());
      display.print(" dBm");
      display.setCursor(0, 56);
      if (wifiConnected) {
        display.print("Status: Connected");
      } else {
        display.print("Status: Offline");
      }
    }
    
    display.display();
    
    if (button_is_pressed(btn4)) {
      scanWiFiNetworks();
      delay(200);
    }
    else if (button_is_pressed(btn3)) {
      wifiNetworkMenu();
      delay(200);
    }
    else if (button_is_pressed(btn5)) {
      disconnectWiFi();
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
    display.setCursor(0, 40);
    display.print("Connect?");
    display.display();
    while(true){
      if(button_is_pressed(btn3)) {
        wifiNetworkMenu();
        break;
      }
      else if (button_is_pressed(btn6)) return;
    }
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Fetching weather...");
  display.display();
  
  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=51.752&longitude=-1.258&current=temperature_2m,weather_code,relative_humidity_2m,wind_speed_10m&timezone=auto";
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
    else if (weatherCode < 50) weatherDesc = "Drizzle";
    else if (weatherCode < 60) weatherDesc = "Heavy Rain";
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

void initializeTimeSettings() {
  preferences.begin(TIME_SETTINGS, false);
  
  if (!preferences.isKey("offset")) {
    preferences.putInt("offset", DEFAULT_TIME_OFFSET);
  }
  if (!preferences.isKey("12hour")) {
    preferences.putBool("12hour", DEFAULT_12_HOUR_MODE);
  }
  if (!preferences.isKey("dst")) {
    preferences.putInt("dst", DEFAULT_DST_OFFSET);
  }
  
  preferences.end();
}

int getTimeOffset() {
  preferences.begin(TIME_SETTINGS, true);
  int offset = preferences.getInt("offset", DEFAULT_TIME_OFFSET);
  preferences.end();
  return offset;
}

void setTimeOffset(int offset) {
  preferences.begin(TIME_SETTINGS, false);
  preferences.putInt("offset", offset);
  preferences.end();
}

bool get12HourMode() {
  preferences.begin(TIME_SETTINGS, true);
  bool mode = preferences.getBool("12hour", DEFAULT_12_HOUR_MODE);
  preferences.end();
  return mode;
}

void set12HourMode(bool mode) {
  preferences.begin(TIME_SETTINGS, false);
  preferences.putBool("12hour", mode);
  preferences.end();
}

int getDSTOffset() {
  preferences.begin(TIME_SETTINGS, true);
  int dst = preferences.getInt("dst", DEFAULT_DST_OFFSET);
  preferences.end();
  return dst;
}

void setDSTOffset(int dst) {
  preferences.begin(TIME_SETTINGS, false);
  preferences.putInt("dst", dst);
  preferences.end();
}

void timeSettingsMenu() {
  int selectedOption = 0;
  int timeOffset = getTimeOffset();
  bool use12Hour = get12HourMode();
  int dstOffset = getDSTOffset();
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Time Settings");
    display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    if (selectedOption == 0) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print("Offset: ");
    display.print(timeOffset);
    display.println(" hrs");
    
    display.setCursor(0, 35);
    if (selectedOption == 1) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print("12/24h: ");
    display.println(use12Hour ? "12h" : "24h");
    
    display.setCursor(0, 50);
    if (selectedOption == 2) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print("DST: ");
    display.print(dstOffset);
    display.print(" hrs");
    
    display.display();
    
    if (button_is_pressed(btn1)) {
      selectedOption = (selectedOption - 1 + 3) % 3;
      delay(150);
    }
    else if (button_is_pressed(btn2)) {
      selectedOption = (selectedOption + 1) % 3;
      delay(150);
    }
    else if (button_is_pressed(btn3)) {
      if (selectedOption == 0) {
        while (true) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("Time Offset (hours)");
          display.setCursor(0, 20);
          display.print("1:Dec 2:Inc");
          display.setCursor(0, 35);
          display.setTextSize(2);
          display.print(timeOffset);
          display.display();
          
          if (button_is_pressed(btn1)) {
            timeOffset--;
            if (timeOffset < -12) timeOffset = -12;
            delay(150);
          }
          else if (button_is_pressed(btn2)) {
            timeOffset++;
            if (timeOffset > 12) timeOffset = 12;
            delay(150);
          }
          else if (button_is_pressed(btn6)) {
            setTimeOffset(timeOffset);
            break;
          }
          delay(50);
        }
      }
      else if (selectedOption == 1) {
        use12Hour = !use12Hour;
        set12HourMode(use12Hour);
        delay(200);
      }
      else if (selectedOption == 2) {
        while (true) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("DST Offset (hours)");
          display.setCursor(0, 20);
          display.print("1:Dec 2:Inc");
          display.setCursor(0, 35);
          display.setTextSize(2);
          display.print(dstOffset);
          display.display();
          
          if (button_is_pressed(btn1)) {
            dstOffset--;
            if (dstOffset < 0) dstOffset = 0;
            delay(150);
          }
          else if (button_is_pressed(btn2)) {
            dstOffset++;
            if (dstOffset > 2) dstOffset = 2;
            delay(150);
          }
          else if (button_is_pressed(btn6)) {
            setDSTOffset(dstOffset);
            break;
          }
          delay(50);
        }
      }
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(50);
  }
}

void displayTime(void) {
  initializeTimeSettings();
  
  int timeOffset = getTimeOffset();
  bool use12Hour = get12HourMode();
  int dstOffset = getDSTOffset();
  
  while (true) {
    time_t now = time(nullptr);
    
    now += (timeOffset + dstOffset) * 3600;
    
    struct tm* timeinfo = localtime(&now);
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Time");
    
    // If the year is less than 2000, clearly the time is not correct, most likely because it is not synced.
    if (timeinfo->tm_year + 1900 < 2000){
      if (!wifiConnected) display.print(" (Not Synced!)");
      // If WiFi is connected and the time is still impossible, something strange has happened.
      else display.print(" Unknown Error!");
    }
    
    display.setTextSize(2);
    display.setCursor(10, 20);
    
    int hour = timeinfo->tm_hour;
    const char* ampm = "";
    
    if (use12Hour) {
      ampm = (hour >= 12) ? "PM" : "AM";
      hour = hour % 12;
      if (hour == 0) hour = 12;
    }
    
    display.printf("%02d:%02d:%02d", hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    if (use12Hour) {
      display.setTextSize(1);
      display.setCursor(100, 40);
      display.print(ampm);
    }
    
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.printf("%s %d, %d", 
      (timeinfo->tm_mon == 0 ? "Jan" : timeinfo->tm_mon == 1 ? "Feb" : timeinfo->tm_mon == 2 ? "Mar" : 
       timeinfo->tm_mon == 3 ? "Apr" : timeinfo->tm_mon == 4 ? "May" : timeinfo->tm_mon == 5 ? "Jun" : 
       timeinfo->tm_mon == 6 ? "Jul" : timeinfo->tm_mon == 7 ? "Aug" : timeinfo->tm_mon == 8 ? "Sep" : 
       timeinfo->tm_mon == 9 ? "Oct" : timeinfo->tm_mon == 10 ? "Nov" : "Dec"),
      timeinfo->tm_mday, timeinfo->tm_year + 1900);
    
    display.display();
    
    if (button_is_pressed(btn1)) timeSync();

    else if (button_is_pressed(btn4)) {
      timeSettingsMenu();
      timeOffset = getTimeOffset();
      use12Hour = get12HourMode();
      dstOffset = getDSTOffset();
      delay(100);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(1000);
  }
}
