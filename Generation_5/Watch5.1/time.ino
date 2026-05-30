extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG,colourText,colour1,colour2,colour3,colour4,colour5,colour6;
extern bool wifiConnected;
extern Preferences preferences;

// Clock:
#define TIME_SETTINGS "time_settings"
#define DEFAULT_TIME_OFFSET 0
#define DEFAULT_12_HOUR_MODE false
#define DEFAULT_DST_OFFSET 0

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
    display.fillScreen(colourBG);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Time Settings");
    display.drawLine(0, 10, SCREEN_WIDTH, 10, colourText);
    
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
          display.fillScreen(colourBG);
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("Time Offset (hours)");
          display.setCursor(0, 20);
          display.print("1:Dec 2:Inc");
          display.setCursor(0, 35);
          display.setTextSize(2);
          display.print(timeOffset);
          
          
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
          else if (button_is_pressed(btn6, true)) {
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
          display.fillScreen(colourBG);
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("DST Offset (hours)");
          display.setCursor(0, 20);
          display.print("1:Dec 2:Inc");
          display.setCursor(0, 35);
          display.setTextSize(2);
          display.print(dstOffset);
          
          
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
          else if (button_is_pressed(btn6, true)) {
            setDSTOffset(dstOffset);
            break;
          }
          delay(50);
        }
      }
    }
    else if (button_is_pressed(btn6, true)) {
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
    
    display.fillScreen(colourBG);
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
    
    
    
    if (button_is_pressed(btn1)) timeSync();

    else if (button_is_pressed(btn4)) {
      timeSettingsMenu();
      timeOffset = getTimeOffset();
      use12Hour = get12HourMode();
      dstOffset = getDSTOffset();
      delay(100);
    }
    else if (button_is_pressed(btn6, true)) {
      return;
    }
    
    delay(1000);
  }
}