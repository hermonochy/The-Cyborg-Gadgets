#include <HTTPClient.h>
#include <ArduinoJson.h>

extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern void randomiseMac();
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG, colourText, colour1, colour2, colour3, colour4, colour5, colour6;
extern bool wifiConnected, didWifiConnect;
extern Preferences preferences;

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

static int redrawMenu = 1;

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
  if (wifiNetworkCount == 0) return;
  randomiseMac();
  WiFi.begin(wifiNetworks[currentWiFiIndex].ssid, wifiNetworks[currentWiFiIndex].password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 12) {
    display.setTextSize(2);
    display.setTextColor(colour3);
    display.setCursor(76, 120);
    display.print(".");
    delay(250);
    attempts++;
  }
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    didWifiConnect = true;
    configTime(0, 0, "uk.pool.ntp.org", "time.nist.gov");
    display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
    display.setTextColor(colour3);
    display.setCursor(66, 110);
    display.print("Connected!");
    delay(1500);
  } else {
    display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
    display.setTextColor(colour2);
    display.setCursor(40, 110);
    display.print("Connect failed");
    delay(1200);
  }
  redrawMenu = 1;
}

bool inputStringOnWatch(const char* label, char* buffer, int maxLen) {
  buffer[0] = '\0';
  int cursor = 0, charIndex = 0;
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_@";
  int clen = strlen(charset);
  while (true) {
    display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
    display.setTextSize(1);
    display.setTextColor(colourText);
    display.setCursor(44, 32);
    display.print(label);
    display.setCursor(60, 60);
    display.print(buffer);
    display.print("_");
    display.setTextSize(2);
    display.setTextColor(colour1);
    display.setCursor(110, 100);
    display.print(charset[charIndex]);
    display.setTextSize(1);
    display.setTextColor(colour4);
    display.setCursor(58, 160);
    display.print("1/2: <-/>  3:Add  4:Del  6:OK");
    if (button_is_pressed(btn1, true)) {
      charIndex = (charIndex - 1 + clen) % clen;
    } else if (button_is_pressed(btn2, true)) {
      charIndex = (charIndex + 1) % clen;
    } else if (button_is_pressed(btn3, true)) {
      int l = strlen(buffer);
      if (l < maxLen - 1) {
        buffer[l] = charset[charIndex];
        buffer[l + 1] = '\0';
      }
    } else if (button_is_pressed(btn4, true)) {
      int l = strlen(buffer);
      if (l > 0) buffer[l - 1] = '\0';
    } else if (button_is_pressed(btn6, true)) return strlen(buffer) > 0;
    delay(90);
  }
}

void addWiFiNetworkOnWatch() {
  if (wifiNetworkCount >= MAX_WIFI_NETWORKS) return;
  char newSSID[MAX_WIFI_SSID] = "";
  char newPassword[MAX_WIFI_PASS] = "";
  if (!inputStringOnWatch("SSID:", newSSID, MAX_WIFI_SSID)) return;
  if (!inputStringOnWatch("Pass:", newPassword, MAX_WIFI_PASS)) return;
  strncpy(wifiNetworks[wifiNetworkCount].ssid, newSSID, MAX_WIFI_SSID - 1);
  wifiNetworks[wifiNetworkCount].ssid[MAX_WIFI_SSID - 1] = '\0';
  strncpy(wifiNetworks[wifiNetworkCount].password, newPassword, MAX_WIFI_PASS - 1);
  wifiNetworks[wifiNetworkCount].password[MAX_WIFI_PASS - 1] = '\0';
  wifiNetworkCount++;
  saveWiFiNetworksToNVS();
  redrawMenu = 1;
  display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
  display.setTextColor(colour3);
  display.setCursor(66, 120);
  display.print("Saved!");
  delay(1200);
}

void deleteWiFiNetwork(int idx) {
  if (idx < 0 || idx >= wifiNetworkCount) return;
  display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
  display.setTextSize(2);
  display.setTextColor(colour2);
  display.setCursor(56, 110);
  display.print("Delete?");
  display.setTextSize(1);
  display.setTextColor(colourText);
  display.setCursor(40, 135);
  display.print(wifiNetworks[idx].ssid);
  display.setCursor(77, 160);
  display.print("3:Yes  6:No");
  while (true) {
    if (button_is_pressed(btn3, true)) {
      for (int i = idx; i < wifiNetworkCount - 1; i++) wifiNetworks[i] = wifiNetworks[i + 1];
      wifiNetworkCount--;
      saveWiFiNetworksToNVS();
      redrawMenu = 1;
      return;
    }
    if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}

void wifiNetworkMenu() {
  int selectedIdx = 0, oldSel = -1;
  while (true) {
    if (redrawMenu || selectedIdx != oldSel) {
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(80, 20);
      display.print("NETWORKS");
      int y0 = 62;
      for (int i = 0; i < wifiNetworkCount; i++) {
        display.setTextSize(selectedIdx == i ? 2 : 1);
        display.setTextColor(selectedIdx == i ? colour6 : colour3);
        display.setCursor(38, y0 + i * 28);
        display.print(wifiNetworks[i].ssid);
      }
      display.setTextSize(1);
      display.setTextColor(colour4);
      if (wifiNetworkCount == 0) {
        display.setCursor(72, 140);
        display.print("btn4: Add");
      } else {
        display.setCursor(16, 190);
        display.print("1/2:Up/Dwn 3:Conn 4:Add 5:Del");
      }
      oldSel = selectedIdx;
      redrawMenu = 0;
    }
    if (wifiNetworkCount > 0) {
      if (button_is_pressed(btn1, true)) {
        selectedIdx = (selectedIdx - 1 + wifiNetworkCount) % wifiNetworkCount;
      } else if (button_is_pressed(btn2, true)) {
        selectedIdx = (selectedIdx + 1) % wifiNetworkCount;
      } else if (button_is_pressed(btn3, true)) {
        currentWiFiIndex = selectedIdx;
        connectWiFi();
      } else if (button_is_pressed(btn5, true)) {
        deleteWiFiNetwork(selectedIdx);
        if (selectedIdx >= wifiNetworkCount && wifiNetworkCount > 0) selectedIdx = wifiNetworkCount - 1;
      }
    }
    if (button_is_pressed(btn4, true)) { addWiFiNetworkOnWatch(); }
    if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}

void scanWiFiNetworks() {
  display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
  display.setTextSize(2);
  display.setTextColor(colour1);
  display.setCursor(54, 70);
  display.print("Scanning...");
  int numNetworks = WiFi.scanNetworks();
  int selectedIdx = 0, showList = (numNetworks > 0);
  int indices[numNetworks]; 
  for (int i = 0; i < numNetworks; i++) indices[i] = i;
  for (int i = 0; i < numNetworks - 1; i++)
    for (int j = 0; j < numNetworks - 1 - i; j++)
      if (WiFi.RSSI(indices[j]) < WiFi.RSSI(indices[j + 1])) {
        int t = indices[j];
        indices[j] = indices[j + 1];
        indices[j + 1] = t;
      }
  while (showList) {
    display.setTextSize(2);
    display.setTextColor(colour3, colourBG);
    display.setCursor(107, 22);
    display.print("SCAN");
    display.setTextSize(1);
    for (int i = 0; i < min(numNetworks, 5); i++) {
      int idx = indices[i];
      display.setTextColor(i == selectedIdx ? colour6 : colour3, colourBG);
      display.setCursor(42, 56 + i * 24);
      String ssid = WiFi.SSID(idx);
      if (ssid.length() > 14) ssid = ssid.substring(0, 14);
      display.print(ssid);
      display.setCursor(170, 56 + i * 24);
      display.print(WiFi.RSSI(idx));
    }
    display.setTextColor(colour4, colourBG);
    display.setCursor(27, 180);
    display.print("1/2:Up/Dn 3:Add");
    if (button_is_pressed(btn1, true)) {
      selectedIdx = (selectedIdx - 1 + min(numNetworks, 5)) % min(numNetworks, 5);
    } else if (button_is_pressed(btn2, true)) {
      selectedIdx = (selectedIdx + 1) % min(numNetworks, 5);
    } else if (button_is_pressed(btn3, true)) {
      String selSSID = WiFi.SSID(indices[selectedIdx]);
      char newPW[MAX_WIFI_PASS] = "";
      if (inputStringOnWatch("PW:", newPW, MAX_WIFI_PASS)) {
        if (wifiNetworkCount < MAX_WIFI_NETWORKS) {
          strncpy(wifiNetworks[wifiNetworkCount].ssid, selSSID.c_str(), MAX_WIFI_SSID - 1);
          wifiNetworks[wifiNetworkCount].ssid[MAX_WIFI_SSID - 1] = '\0';
          strncpy(wifiNetworks[wifiNetworkCount].password, newPW, MAX_WIFI_PASS - 1);
          wifiNetworks[wifiNetworkCount].password[MAX_WIFI_PASS - 1] = '\0';
          wifiNetworkCount++;
          saveWiFiNetworksToNVS();
          redrawMenu = 1;
        }
      }
    } else if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}

void disconnectWiFi() {
  if (!wifiConnected) {
    display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
    display.setTextColor(colour2);
    display.setTextSize(2);
    display.setCursor(70, 114);
    display.print("Offline");
    delay(1000);
    return;
  }
  display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
  display.setTextColor(colour2);
  display.setTextSize(2);
  display.setCursor(56, 106);
  display.print("Disconnect?");
  display.setTextSize(1);
  display.setCursor(68, 140);
  display.print("3:Yes  6:No");
  while (true) {
    if (button_is_pressed(btn3, true)) {
      WiFi.disconnect();
      wifiConnected = false;
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextColor(colour3);
      display.setCursor(60, 60);
      display.setTextSize(3);
      display.print("Done!");
      delay(1000);
      redrawMenu = 1;
      return;
    }
    if (button_is_pressed(btn6, true)) return;
    delay(50);
  }
}

void wifiMenu(void) {
  int sel = 0, oldSel = -1;
  const char* menu[] = { "Connect", "Scan", "Net Mgr", "Disconnect", "Scramble MAC"};
  while (true) {
    if (sel != oldSel || redrawMenu) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(84, 22);
      display.print("WiFi");
      for (int i = 0; i < 5; ++i) {
        display.setTextSize(i == sel ? 2 : 1);
        display.setTextColor(i == sel ? colour6 : colour3);
        display.setCursor(50, 54 + i * 30);
        display.print(menu[i]);
      }
      display.setTextSize(1);
      display.setTextColor(colour4, colourBG);
      display.setCursor(50, 200);
      display.print("1/2:Up/Dn 3:Sel");
      oldSel = sel;
      redrawMenu = 0;
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % 5;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel + 4) % 5;
    } else if (button_is_pressed(btn3, true)) {
      if (sel == 0) connectWiFi();
      else if (sel == 1) scanWiFiNetworks();
      else if (sel == 2) wifiNetworkMenu();
      else if (sel == 3) disconnectWiFi();
      else if (sel == 4) randomiseMac();
      redrawMenu = 1;
      oldSel = -1;
    } else if (button_is_pressed(btn6, true)) return;
    delay(40);
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
  
  display.fillScreen(colourBG);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Network added via");
  display.println("Serial!");
  
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
  
  display.fillScreen(colourBG);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi");
  display.println("via Serial...");
  
  
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
    
    display.fillScreen(colourBG);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Connected!");
    display.setCursor(0, 20);
    display.print("IP: ");
    display.println(WiFi.localIP());
    
    delay(2000);
    
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("  Syncing time with NTP...");
  } else {
    wifiConnected = false;
    Serial.println("✗ Failed to connect");
    Serial.print("  WiFi Status: ");
    Serial.println(WiFi.status());
    
    display.fillScreen(colourBG);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Connection failed!");
    
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

void wifiFuncs(){
  while (!button_is_pressed(btn6, true)) {
    display.fillScreen(colourBG);
    display.setTextSize(1);
    display.setCursor(50, 50);
    display.print("1. Weather");
    display.setCursor(50, 100);
    display.print("2. Time");
    display.setCursor(50, 150);
    display.print("3. Dictionary");
    
    delay(50);
    
    if (button_is_pressed(btn1)) getWeather();
    else if (button_is_pressed(btn2)) displayTime();
    else if (button_is_pressed(btn3)) dictionary();
  }
}

void getWeather(void) {
  if (!wifiConnected) {
    display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
    display.setTextSize(2); display.setTextColor(colour2);
    display.setCursor(72, 104); display.print("No WiFi!");
    delay(1200); return;
  }
  int hourOffset = 0, prevHour = -1;
  bool redraw = true;
  while (true) {
    if (hourOffset != prevHour || redraw) {
      display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
      display.setTextSize(2); display.setTextColor(colour6);
      display.setCursor(62,28); display.print("WEATHER");
      display.setTextColor(colour4); display.setTextSize(1);
      display.setCursor(40,70); display.print("Hour (+0..+23): ");
      display.print(hourOffset);
      display.setCursor(32, 204); display.print("1/2:Change 3:Go  6:Bk");
      prevHour = hourOffset; redraw = false;
    }
    if (button_is_pressed(btn1,true)) { hourOffset = (hourOffset+23)%24; redraw=true;}
    else if (button_is_pressed(btn2,true)) { hourOffset = (hourOffset+1)%24; redraw=true;}
    else if (button_is_pressed(btn6,true)) return;
    else if (button_is_pressed(btn3,true)) break;
    delay(40);
  }

  display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
  display.setTextSize(1); display.setTextColor(colourText);
  display.setCursor(78, 90); display.print("Loading...");
  // Oxford, UK
  String url = "https://api.open-meteo.com/v1/forecast?latitude=51.752&longitude=-1.258"
               "&hourly=temperature_2m,weather_code,relative_humidity_2m,wind_speed_10m"
               "&forecast_days=1&timezone=auto";
  HTTPClient http; http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(8192); deserializeJson(doc, payload);
    JsonArray tempArr = doc["hourly"]["temperature_2m"];
    JsonArray codeArr = doc["hourly"]["weather_code"];
    JsonArray humidityArr = doc["hourly"]["relative_humidity_2m"];
    JsonArray windArr = doc["hourly"]["wind_speed_10m"];
    JsonArray timeArr = doc["hourly"]["time"];
    int nHours = tempArr.size();
    if (hourOffset >= nHours) hourOffset = nHours - 1;
    float temp = tempArr[hourOffset];
    int weatherCode = codeArr[hourOffset];
    int humidity = humidityArr[hourOffset];
    float windSpeed = windArr[hourOffset];
    const char* timeStr = timeArr[hourOffset];

    const char* weatherDesc = "Cloud";
    if      (weatherCode == 0)  weatherDesc = "Sunny";
    else if (weatherCode == 1)  weatherDesc = "Clear";
    else if (weatherCode < 3)   weatherDesc = "Cloudy";
    else if (weatherCode < 50)  weatherDesc = "Drizzle";
    else if (weatherCode < 60)  weatherDesc = "Rain";
    else if (weatherCode < 80)  weatherDesc = "Snow";
    else if (weatherCode < 100) weatherDesc = "Thunder";

    display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
    display.setTextColor(colourText); display.setTextSize(2); display.setCursor(64,24); display.print("FORECAST");
    display.setTextSize(1);
    display.setCursor(26, 64); display.print("At "); display.print(&timeStr[11]); //HH:MM
    display.setCursor(26, 84); display.print("Temp "); display.print((int)temp); display.print(" C");
    display.setCursor(26, 104); display.print("Cond: "); display.print(weatherDesc);
    display.setCursor(26, 124); display.print("Humidity "); display.print(humidity); display.print("%");
    display.setCursor(26, 144); display.print("Wind "); display.print((int)windSpeed); display.print("km/h");
    display.setTextColor(colour6); display.setCursor(56,185); display.print("6:Back");
    while (true) { if (button_is_pressed(btn6,true)) break; delay(40);}
  } else {
    display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
    display.setTextColor(colour2); display.setCursor(70,114); display.print("No data.");
    delay(1400);
  }
  http.end();
}

struct DictResult {
  char word[32], phonetic[32], partOfSpeech[16], definition[256], example[128];
  int definitionCount, currentDefinition;
};
DictResult dictResult;
bool dictDataValid = false;

bool fetchWordDefinition(const char* word) {
  if (!wifiConnected) return false;
  HTTPClient http;
  char url[128]; snprintf(url, sizeof(url), "https://api.dictionaryapi.dev/api/v2/entries/en/%s", word);
  http.begin(url); int httpCode = http.GET();
  if (httpCode != 200) { http.end(); return false; }
  String payload = http.getString();
  http.end();
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) return false;
  strncpy(dictResult.word, doc[0]["word"].as<const char*>(), 31); dictResult.word[31]='\0';
  if (doc[0]["phonetic"].is<const char*>()) strncpy(dictResult.phonetic, doc[0]["phonetic"].as<const char*>(), 31); else dictResult.phonetic[0]=0; dictResult.phonetic[31]='\0';
  if (doc[0]["meanings"].size() > 0) {
    JsonObject meaning = doc[0]["meanings"][0];
    strncpy(dictResult.partOfSpeech, meaning["partOfSpeech"].as<const char*>(), 15); dictResult.partOfSpeech[15]='\0';
    if (meaning["definitions"].size() > 0) strncpy(dictResult.definition, meaning["definitions"][0]["definition"].as<const char*>(), 255),dictResult.definition[255]='\0';
    if (meaning["definitions"][0]["example"].is<const char*>()) strncpy(dictResult.example, meaning["definitions"][0]["example"].as<const char*>(), 127),dictResult.example[127]='\0'; else dictResult.example[0]=0;
  } else dictResult.definition[0]=0, dictResult.example[0]=0,dictResult.partOfSpeech[0]=0;
  dictResult.definitionCount = 1;
  dictResult.currentDefinition = 0;
  dictDataValid = true;
  return true;
}

void dictCharacterInput(char* buffer, int maxLen) {
  buffer[0] = '\0';
  int charIndex = 0;
  const char charset[] = "abcdefghijklmnopqrstuvwxyz";
  int charsetSize = strlen(charset);
  while (true) {
    display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
    display.setTextSize(2);
    display.setTextColor(colour6);
    display.setCursor(58, 30); display.print("DICTIONARY");
    display.setTextSize(1); display.setTextColor(colourText);
    display.setCursor(52, 68); display.print("Type: ");
    display.setTextSize(2); 
    display.setTextColor(colour4);
    display.setCursor(85, 88); display.print(charset[charIndex]);
    display.setTextSize(1); display.setTextColor(colourText);
    display.setCursor(52, 128); display.print(buffer); display.print("_");
    display.setTextColor(colour4);
    display.setCursor(44,185); display.print("1/2:Prev/Next  3:Add  4:Del  6:Go");
    if (button_is_pressed(btn1,true)) { charIndex=(charIndex-1+charsetSize)%charsetSize;}
    else if (button_is_pressed(btn2,true)) { charIndex=(charIndex+1)%charsetSize;}
    else if (button_is_pressed(btn3,true)) { int l=strlen(buffer); if(l<maxLen-1){ buffer[l]=charset[charIndex]; buffer[l+1]='\0';}}
    else if (button_is_pressed(btn4,true)) { int l=strlen(buffer); if(l>0) buffer[l-1]='\0';}
    else if (button_is_pressed(btn6,true)) { if(strlen(buffer)>0) return; }
    delay(90);
  }
}

void dictDisplayWord() {
  display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
  display.setTextSize(2); display.setTextColor(colour1);
  display.setCursor(54,18); display.print(dictResult.word);
  display.setTextSize(1); display.setTextColor(colour3);
  display.setCursor(54,42); display.print(dictResult.phonetic);
  display.setCursor(54,60); display.print(dictResult.partOfSpeech);
  display.setTextColor(colourText); display.setCursor(24,80);
  display.print(dictResult.definition);
  display.setTextColor(colour4); display.setCursor(24,158);
  display.print(dictResult.example);
  display.setTextColor(colour6); display.setCursor(58,200); display.print("6:Back");
}

void dictionary(void) {
  char searchWord[32] = "";
  while (true) {
    dictCharacterInput(searchWord, 32);
    if(strlen(searchWord)==0) return;
    display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
    display.setTextSize(2); display.setTextColor(colourText);
    display.setCursor(64, 110); display.print("Searching...");
    if (fetchWordDefinition(searchWord)) {
      while (true) {
        dictDisplayWord();
        if(button_is_pressed(btn6,true)) break;
        delay(80);
      }
    } else {
      display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
      display.setTextColor(colour2); display.setTextSize(2);
      display.setCursor(54,110); display.print("Not found.");
      display.setTextColor(colour6); display.setTextSize(1);
      display.setCursor(66,150); display.print("6:Back");
      while (!button_is_pressed(btn6,true)) delay(80);
    }
    searchWord[0]=0;
  }
}
