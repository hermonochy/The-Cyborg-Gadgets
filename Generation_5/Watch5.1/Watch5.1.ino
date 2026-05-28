#include "Watch5.1.h"

#define DC  8
#define RST 9
#define CS 10


#define totalFunctions 12
#define numSettings 5

Adafruit_GC9A01A display(CS, DC, RST);
Preferences preferences;

const char *Functions[] = {"Outputs", "Maths", "Random", "Score", "Games", "Metronome", "Notes", "Calendar", "WiFi Menu", "WiFi Tools","Shell", "Settings"};
const char *settingFuncs[] = {"Button Offset", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Display Settings"};

WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;
int currentWiFiIndex = 0;

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
  display.fillScreen(COLOR_BG);

  display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, COLOR_ACCENT);
  display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-6, COLOR_FG);

  display.setTextColor(COLOR_FG);
  display.setTextSize(1);
  const char* title = "Watch 5.1";
  display.setCursor(SCREEN_CENTER_X - STR_W(title,1)/2, 32);
  display.print(title);

  if (wifiConnected) {
    int iconY = 50;
    display.drawCircle(SCREEN_CENTER_X, iconY, 10, COLOR_ACCENT);
    display.drawCircle(SCREEN_CENTER_X, iconY, 7, COLOR_ACCENT);
    display.drawCircle(SCREEN_CENTER_X, iconY, 4, COLOR_ACCENT);
    display.fillCircle(SCREEN_CENTER_X, iconY+7, 2, COLOR_ACCENT);
  }

  for (int i = 0; i < totalFunctions; i++) {
    float angle = (2 * PI * i / totalFunctions) - PI/2;
    int rDots = SCREEN_RADIUS - 26;
    int dotX = SCREEN_CENTER_X + (int)(rDots * cos(angle));
    int dotY = SCREEN_CENTER_Y + (int)(rDots * sin(angle));
    if ((i+1) == selectedFunction)
      display.fillCircle(dotX, dotY, 7, COLOR_SELECTED);
    else
      display.fillCircle(dotX, dotY, 4, COLOR_UNSELECTED);
  }

  display.setTextColor(COLOR_FG);
  display.setTextSize(2);
  const char* funcName = Functions[selectedFunction - 1];
  display.setCursor(SCREEN_CENTER_X - STR_W(funcName,2)/2, SCREEN_CENTER_Y - 18);
  display.print(funcName);

  char numBuf[8];
  sprintf(numBuf, "[%02d]", selectedFunction);
  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(2);
  display.setCursor(SCREEN_CENTER_X - STR_W(numBuf,2)/2, SCREEN_CENTER_Y + 20);
  display.print(numBuf);

  display.setTextColor(COLOR_FG);
  display.setTextSize(1);
  const char* navHint = "< NAV >   SEL";
  display.setCursor(SCREEN_CENTER_X - STR_W(navHint,1)/2, SCREEN_HEIGHT - 22);
  display.print(navHint);
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
      if (button_is_pressed(btn6, true)) {
        WiFi.disconnect();
        return;
      }

      delay(500);
      attempts++;
      currentStep = min(totalSteps - 10, (int)((millis() - startTime) * totalSteps / timeout));

      display.fillScreen(COLOR_BG);
      display.setTextSize(1);
      display.setTextColor(COLOR_FG);

      const char* title = "WATCH 5.1";
      display.setCursor(SCREEN_CENTER_X - STR_W(title,1)/2, SCREEN_CENTER_Y - 44);
      display.print(title);

      const char* initTxt = "INITIALIZING";
      display.setCursor(SCREEN_CENTER_X - STR_W(initTxt,1)/2, SCREEN_CENTER_Y - 26);
      display.print(initTxt);

      int dotCount = (attempts / 2) % 4;
      for (int i = 0; i < dotCount; i++) {
        display.print(".");
      }

      float angleF = 2 * PI * (float)currentStep / (float)totalSteps;
      int arcR1 = 94, arcR2 = 106;
      for (float theta = -PI/2; theta < -PI/2 + angleF; theta += 0.04) {
        int x1 = SCREEN_CENTER_X + (int)(arcR1 * cos(theta));
        int y1 = SCREEN_CENTER_Y + (int)(arcR1 * sin(theta));
        int x2 = SCREEN_CENTER_X + (int)(arcR2 * cos(theta));
        int y2 = SCREEN_CENTER_Y + (int)(arcR2 * sin(theta));
        display.drawLine(x1, y1, x2, y2, COLOR_ACCENT);
      }
      char percentText[8];
      sprintf(percentText, "%d%", currentStep);
      display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X - STR_W(percentText, 2)/2, SCREEN_CENTER_Y + 20);
      display.print(percentText);
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      break;
    }
  }
  display.fillScreen(COLOR_BG);
  display.setTextSize(1);
  display.setTextColor(COLOR_FG);

  const char* title = "WATCH 5.1";
  display.setCursor(SCREEN_CENTER_X - STR_W(title,1)/2, SCREEN_CENTER_Y - 44);
  display.print(title);

  const char* initDone = "INITIALIZING.";
  display.setCursor(SCREEN_CENTER_X - STR_W(initDone,1)/2, SCREEN_CENTER_Y - 26);
  display.print(initDone);

  int arcR1 = 94, arcR2 = 106;
  for (float theta = -PI/2; theta < -PI/2 + 2*PI; theta += 0.04) {
    int x1 = SCREEN_CENTER_X + (int)(arcR1 * cos(theta));
    int y1 = SCREEN_CENTER_Y + (int)(arcR1 * sin(theta));
    int x2 = SCREEN_CENTER_X + (int)(arcR2 * cos(theta));
    int y2 = SCREEN_CENTER_Y + (int)(arcR2 * sin(theta));
    display.drawLine(x1, y1, x2, y2, COLOR_ACCENT);
  }
  char percentText[8];
  sprintf(percentText, "100%");
  display.setTextSize(2);
  display.setCursor(SCREEN_CENTER_X - STR_W(percentText, 2)/2, SCREEN_CENTER_Y + 20);
  display.print(percentText);

  delay(1500);
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

  timeSyncAndUI();
  delay(1000);

  if (a_button_is_pressed()) {
    display.fillScreen(COLOR_BG);
    tuneButtonVals();
  }
}

void loop() {
  // Only redraw UI if needed
  if (selectedFunction != lastSelectedFunction) {
    drawMainUI();
    lastSelectedFunction = selectedFunction;
  }

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
  else if (button_is_pressed(btn3)) {
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
        //shell();
        break;
      case 12 :
        settings();
        break;
    }
  }
}