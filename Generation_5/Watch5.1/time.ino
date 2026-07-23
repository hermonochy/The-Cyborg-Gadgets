extern TFT_eSPI display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG, colourText, colour1, colour2, colour3, colour4, colour5, colour6;
extern bool wifiConnected;
extern Preferences preferences;

#define TIME_SETTINGS "time_settings"
#define DEFAULT_TIME_OFFSET 0
#define DEFAULT_12_HOUR_MODE false
#define DEFAULT_DST_OFFSET 0
#define SCREEN_W 240
#define SCREEN_H 240

void initializeTimeSettings() {
  preferences.begin(TIME_SETTINGS, false);
  if (!preferences.isKey("offset")) preferences.putInt("offset", DEFAULT_TIME_OFFSET);
  if (!preferences.isKey("12hour")) preferences.putBool("12hour", DEFAULT_12_HOUR_MODE);
  if (!preferences.isKey("dst")) preferences.putInt("dst", DEFAULT_DST_OFFSET);
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

const char* timeFuncs[] = { "Clock", "Alarm", "Timer", "Stopwatch", "World Clock", "Settings" };
#define TIMEFUNCS_COUNT 6

void alarmMenu();
void timerMenu();
void stopwatchMenu();
void worldClockMenu();

void timeMenu() {
  int sel = 0, oldSel = -1;
  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.setTextSize(2);
  display.setTextColor(colour4);
  display.setCursor(60, 20);
  display.print("TIME");
  
  while (1) {
    if (sel != oldSel) {
      int baseY = 70, spacing = 30;
      for (int i = 0; i < TIMEFUNCS_COUNT; i++) {
        display.fillRect(35, baseY + i * spacing - 4, 170, 22, colourBG);
        if (i == sel) {
          display.fillRoundRect(32, baseY + i * spacing - 6, 176, 26, 8, colour6);
          display.setTextSize(2);
          display.setTextColor(colourBG);
        } else {
          display.setTextSize(1);
          display.setTextColor(colour3);
        }
        display.setCursor(50, baseY + i * spacing - 2);
        display.print(timeFuncs[i]);
      }
      oldSel = sel;
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % TIMEFUNCS_COUNT;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel + TIMEFUNCS_COUNT - 1) % TIMEFUNCS_COUNT;
    } else if (button_is_pressed(btn3, true)) {
      if (sel == 0) displayTime();
      else if (sel == 1) alarmMenu();
      else if (sel == 2) timerMenu();
      else if (sel == 3) stopwatchMenu();
      else if (sel == 4) worldClockMenu();
      else if (sel == 5) timeSettingsMenu();
      display.fillScreen(colourBG);
      oldSel = -1;
      continue;
    } else if (button_is_pressed(btn6, true)) {
      return;
    }
    delay(40);
  }
}

void setRTCMenu() {
  struct tm tnow;
  time_t now = time(nullptr);
  localtime_r(&now, &tnow);

  int fields[6] = { tnow.tm_year + 1900, tnow.tm_mon + 1, tnow.tm_mday, tnow.tm_hour, tnow.tm_min, tnow.tm_sec };
  const char *labels[6] = { "Year", "Month", "Day", "Hour", "Minute", "Second" };
  int minval[6] = { 1970, 1, 1, 0, 0, 0 }, maxval[6] = { 2099, 12, 31, 23, 59, 59 };
  int sel = 0, oldsel = -1, oldfields[6] = { -1, -1, -1, -1, -1, -1 };
  const int cx = SCREEN_W/2, cy = SCREEN_H/2;

  display.fillCircle(cx, cy, cx, colourBG);
  display.setTextColor(colour6);
  display.setTextSize(1);
  display.setCursor(40, 16);
  display.print("SET TIME & DATE");

  while (true) {
    int y = 75, sep = 40;
    for (int i = 0; i < 6; ++i) {
      if (oldfields[i] != fields[i] || oldsel != sel) {
        int dx = (i<3)?-65:50;
        int fy = y+sep*(i%3);
        display.fillRect(cx+dx-3, fy-3, 95, 38, colourBG);
        if (i == sel) {
          display.drawRoundRect(cx+dx-5, fy-5, 99, 42, 6, colour6);
          display.setTextColor(colour6);
        } else {
          display.setTextColor(colour3);
        }
        display.setTextSize(1);
        display.setCursor(cx+dx+2, fy+3);
        display.setTextColor(colour1);
        display.print(labels[i]);
        display.setCursor(cx+dx+2, fy+15);
        if (i == sel) display.setTextColor(colour4);
        else display.setTextColor(colourText);
        display.setTextSize(2);
        if (i==1 || i==2 || i==4 || i==5)
          display.printf("%02d", fields[i]);
        else
          display.printf("%d", fields[i]);
      }
    }

    if (oldfields[0]!=fields[0] || oldfields[1]!=fields[1] || oldfields[2]!=fields[2]) {
      char datebuf[20];
      sprintf(datebuf, "%04d-%02d-%02d", fields[0], fields[1], fields[2]);
      display.fillRect(cx-80, 32, 160, 22, colourBG);
      display.setTextSize(1);
      display.setTextColor(colour6);
      display.setCursor(cx-70, 35);
      display.print(datebuf);
    }
    if (oldfields[3]!=fields[3] || oldfields[4]!=fields[4] || oldfields[5]!=fields[5]) {
      char timebuf[16];
      sprintf(timebuf, "%02d:%02d:%02d", fields[3], fields[4], fields[5]);
      display.fillRect(cx-60, 175, 120, 24, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour4);
      display.setCursor(cx-50, 178);
      display.print(timebuf);
    }

    for (int i = 0; i < 6; ++i) oldfields[i] = fields[i];
    oldsel = sel;

    if (button_is_pressed(btn1)) {
      fields[sel]--;
      if (fields[sel] < minval[sel]) fields[sel] = maxval[sel];
    }
    else if (button_is_pressed(btn2)) {
      fields[sel]++;
      if (fields[sel] > maxval[sel]) fields[sel] = minval[sel];
    }
    else if (button_is_pressed(btn3, true)) {
      sel = (sel+1)%6;
    }
    else if (button_is_pressed(btn4, true)) {
      struct tm tset;
      tset.tm_year = fields[0] - 1900;
      tset.tm_mon  = fields[1] - 1;
      tset.tm_mday = fields[2];
      tset.tm_hour = fields[3];
      tset.tm_min  = fields[4];
      tset.tm_sec  = fields[5];
      tset.tm_isdst = -1;
      time_t tt = mktime(&tset);
      struct timeval tvnow = { tt, 0 };
      settimeofday(&tvnow, nullptr);
      return;
    }
    else if (button_is_pressed(btn6, true)) return;
    delay(60);
  }
}

void displayTime(void) {
  initializeTimeSettings();
  int timeOffset = getTimeOffset();
  bool use12Hour = get12HourMode();
  int dstOffset = getDSTOffset();
  int lastHour = -1, lastMin = -1, lastSec = -1, lastDay = -1, lastMonth = -1, lastYear = -1;
  bool last12h = !use12Hour;
  char lastAMP[4] = "";

  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
  
  while (true) {
    time_t now = time(nullptr);
    now += (timeOffset + dstOffset) * 3600;
    struct tm* timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour;
    const char* ampm = "";
    if (use12Hour) {
      ampm = (hour >= 12) ? "PM" : "AM";
      hour = hour % 12;
      if (hour == 0) hour = 12;
    }
    
    if (lastHour != hour || lastMin != timeinfo->tm_min || lastSec != timeinfo->tm_sec || last12h != use12Hour || strcmp(lastAMP, ampm) != 0 || lastDay != timeinfo->tm_mday || lastMonth != timeinfo->tm_mon || lastYear != timeinfo->tm_year) {
      display.fillRect(30, 45, 180, 100, colourBG);
      
      display.setTextSize(4);
      display.setTextColor(colour4);
      char timestr[16];
      sprintf(timestr, "%02d:%02d:%02d", hour, timeinfo->tm_min, timeinfo->tm_sec);
      display.setCursor(60, 52);
      display.print(timestr);
      
      if (use12Hour) {
        display.setTextSize(2);
        display.setTextColor(colour3);
        display.setCursor(170, 70);
        display.print(ampm);
        strncpy(lastAMP, ampm, 4);
      }
      
      display.fillRect(40, 125, 160, 20, colourBG);
      display.setTextSize(1);
      display.setTextColor(colour1);
      const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
      display.setCursor(45, 128);
      display.printf("%s %d, %d", months[timeinfo->tm_mon], timeinfo->tm_mday, timeinfo->tm_year + 1900);
      
      lastHour = hour;
      lastMin = timeinfo->tm_min;
      lastSec = timeinfo->tm_sec;
      lastDay = timeinfo->tm_mday;
      lastMonth = timeinfo->tm_mon;
      lastYear = timeinfo->tm_year;
      last12h = use12Hour;
    }
    
    display.fillRect(20, 205, 200, 14, colourBG);
    display.setTextSize(1);
    display.setTextColor(colour1);
    display.setCursor(35, 208);
    display.print("btn1:SET    btn6:BACK");
    
    if (button_is_pressed(btn1, true)) setRTCMenu();
    else if (button_is_pressed(btn6, true)) return;
    delay(200);
  }
}

void quickTimeDisplay() {
  initializeTimeSettings();
  int timeOffset = getTimeOffset();
  bool use12Hour = get12HourMode();
  int dstOffset = getDSTOffset();
  
  time_t now = time(nullptr);
  now += (timeOffset + dstOffset) * 3600;
  struct tm* timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  const char* ampm = "";
  if (use12Hour) {
    ampm = (hour >= 12) ? "PM" : "AM";
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  
  display.fillScreen(colourBG);
  display.fillCircle(SCREEN_W / 2, SCREEN_H / 2, SCREEN_W / 2, colourBG);
  display.drawCircle(SCREEN_W / 2, SCREEN_H / 2, SCREEN_W / 2-2, display.color565(40, 40, 40));
  
  display.setTextSize(5);
  display.setTextColor(colour4);
  char timestr[16];
  sprintf(timestr, "%02d:%02d", hour, timeinfo->tm_min);
  display.setCursor(40, 65);
  display.print(timestr);
  
  if (use12Hour) {
    display.setTextSize(2);
    display.setTextColor(colour3);
    display.setCursor(170, 85);
    display.print(ampm);
  }
  
  display.setTextSize(1);
  display.setTextColor(colour1);
  const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  display.setCursor(65, 140);
  display.printf("%s %d, %d", months[timeinfo->tm_mon], timeinfo->tm_mday, timeinfo->tm_year + 1900);

  delay(5000);
}

#define MAX_ALARMS 5
struct Alarm {
  int hour, minute;
  bool enabled;
} alarms[MAX_ALARMS] = { 0 };
int numAlarms = 0;

void alarmMenu() {
  int sel = 0, oldSel = -1, mode = 0, aIdx = 0, ah = 7, am = 0, lastNumAlarms = -1;
  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
  display.setTextSize(2);
  display.setTextColor(colour6);
  display.setCursor(50, 20);
  display.print("ALARMS");
  
  while (1) {
    if (mode == 0 && (oldSel != sel || lastNumAlarms != numAlarms)) {
      int baseY = 70, spacing = 32;
      for (int i = 0; i < max(1, numAlarms); i++) {
        display.fillRect(30, baseY + i * spacing - 4, 180, 26, colourBG);
      }
      
      if (numAlarms == 0) {
        display.setTextSize(1);
        display.setTextColor(colour1);
        display.setCursor(50, 105);
        display.print("No alarms set");
      }
      
      for (int i = 0; i < numAlarms; i++) {
        if (i == sel) {
          display.fillRoundRect(28, baseY + i * spacing - 6, 184, 30, 8, colour6);
          display.setTextSize(2);
          display.setTextColor(colourBG);
        } else {
          display.setTextSize(1);
          display.setTextColor(colour3);
        }
        display.setCursor(50, baseY + i * spacing - 1);
        display.printf("%02d:%02d  %s", alarms[i].hour, alarms[i].minute, alarms[i].enabled ? "ON" : "off");
      }
      oldSel = sel;
      lastNumAlarms = numAlarms;
    }
    
    if (mode == 0 && button_is_pressed(btn2, true)) {
      sel = (sel + 1) % max(1, numAlarms);
    } else if (mode == 0 && button_is_pressed(btn1, true)) {
      sel = (sel - 1 + max(1, numAlarms)) % max(1, numAlarms);
    } else if (mode == 0 && button_is_pressed(btn3, true) && numAlarms > 0) {
      mode = 1;
      aIdx = sel;
      ah = alarms[aIdx].hour;
      am = alarms[aIdx].minute;
    } else if (mode == 0 && button_is_pressed(btn4, true) && numAlarms < MAX_ALARMS) {
      ah = 7;
      am = 0;
      mode = 2;
    } else if (mode == 0 && button_is_pressed(btn5, true) && numAlarms > 0) {
      for (int i = sel; i < numAlarms - 1; i++) alarms[i] = alarms[i + 1];
      numAlarms--;
      if (sel > 0) sel--;
      oldSel = -1;
    } else if (mode == 0 && button_is_pressed(btn6, true)) return;
    else if ((mode == 1 || mode == 2)) {
      display.fillRect(30, 135, 180, 55, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour4);
      display.setCursor(70, 145);
      display.printf("%02d:%02d", ah, am);
      display.setTextSize(1);
      display.setTextColor(colour1);
      display.fillRect(20, 195, 200, 14, colourBG);
      display.setCursor(35, 198);
      display.print("1:-h 2:+h 3:-m 4:+m 5:OK");
      
      if (button_is_pressed(btn1, true)) {
        ah = (ah + 23) % 24;
      } else if (button_is_pressed(btn2, true)) {
        ah = (ah + 1) % 24;
      } else if (button_is_pressed(btn3, true)) {
        am = (am + 59) % 60;
      } else if (button_is_pressed(btn4, true)) {
        am = (am + 1) % 60;
      } else if (button_is_pressed(btn5, true)) {
        if (mode == 1) {
          alarms[aIdx].hour = ah;
          alarms[aIdx].minute = am;
        } else if (mode == 2) {
          alarms[numAlarms].hour = ah;
          alarms[numAlarms].minute = am;
          alarms[numAlarms].enabled = true;
          numAlarms++;
        }
        mode = 0;
        oldSel = -1;
      } else if (button_is_pressed(btn6, true)) {
        mode = 0;
        oldSel = -1;
      }
    }
    delay(40);
  }
}

bool checkAlarmsTriggered() {
  time_t now = time(nullptr);
  struct tm* tmnow = localtime(&now);
  for (int i = 0; i < numAlarms; i++) {
    if (!alarms[i].enabled) continue;
    if (tmnow->tm_hour == alarms[i].hour && tmnow->tm_min == alarms[i].minute && tmnow->tm_sec == 0) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
      display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
      display.setTextSize(3);
      display.setTextColor(colour2);
      display.setCursor(60, 95);
      display.print("ALARM!");
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(65, 145);
      display.printf("%02d:%02d", alarms[i].hour, alarms[i].minute);
      for (int t = 0; t < 60; t++) {
        if (button_is_pressed(btn3, true) || button_is_pressed(btn6, true)) break;
        delay(1000);
      }
      return true;
    }
  }
  return false;
}

void timerMenu() {
  int setSec = 0, timerMode = 0;
  int remainSec = 0;
  unsigned long startMs = 0, oldRem = -1;
  
  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
  display.setTextSize(2);
  display.setTextColor(colour6);
  display.setCursor(70, 20);
  display.print("TIMER");
  
  while (1) {
    if (timerMode == 0) {
      display.fillRect(40, 65, 160, 75, colourBG);
      display.setTextSize(4);
      display.setTextColor(colour4);
      display.setCursor(55, 80);
      int min = setSec / 60, sec = setSec % 60;
      display.printf("%02d:%02d", min, sec);
      display.fillRect(20, 195, 200, 14, colourBG);
      display.setTextSize(1);
      display.setTextColor(colour1);
      display.setCursor(30, 198);
      display.print("1/2:-/+m  3/4:-/+s  5:GO");
      
      if (button_is_pressed(btn1, true)) {
        setSec -= 60;
        if (setSec < 0) setSec = 0;
      } else if (button_is_pressed(btn2, true)) {
        setSec += 60;
        if (setSec > 3599) setSec = 3599;
      } else if (button_is_pressed(btn3, true)) {
        setSec -= 1;
        if (setSec < 0) setSec = 0;
      } else if (button_is_pressed(btn4, true)) {
        setSec += 1;
        if (setSec > 3599) setSec = 3599;
      } else if (button_is_pressed(btn5, true) && setSec > 0) {
        remainSec = setSec;
        startMs = millis();
        timerMode = 1;
      } else if (button_is_pressed(btn6, true)) return;
    } else if (timerMode == 1) {
      int left = remainSec - ((millis() - startMs) / 1000);
      if (left < 0) left = 0;
      if (left != oldRem) {
        display.fillRect(40, 65, 160, 75, colourBG);
        display.setTextSize(4);
        display.setTextColor(colour4);
        display.setCursor(55, 80);
        int min = left / 60, sec = left % 60;
        display.printf("%02d:%02d", min, sec);
        oldRem = left;
      }
      display.fillRect(20, 195, 200, 14, colourBG);
      display.setTextSize(1);
      display.setTextColor(colour1);
      display.setCursor(35, 198);
      display.print("3:PAUSE  4:RESET  6:BACK");
      
      if (left == 0) {
        timerMode = 2;
        continue;
      }
      if (button_is_pressed(btn3, true)) {
        timerMode = 0;
        oldRem = -1;
      } else if (button_is_pressed(btn4, true)) {
        timerMode = 0;
        oldRem = -1;
        setSec = remainSec;
      } else if (button_is_pressed(btn6, true)) return;
    } else if (timerMode == 2) {
      display.fillRect(40, 65, 160, 75, colourBG);
      display.setTextSize(3);
      display.setTextColor(colour2);
      display.setCursor(60, 95);
      display.print("DONE!");
      display.fillRect(20, 195, 200, 14, colourBG);
      display.setTextSize(1);
      display.setTextColor(colour1);
      display.setCursor(70, 198);
      display.print("3/6:BACK");
      
      if (button_is_pressed(btn3, true) || button_is_pressed(btn6, true)) {
        timerMode = 0;
        oldRem = -1;
        setSec = 0;
      }
    }
    delay(40);
  }
}

void stopwatchMenu() {
  int mode = 0;
  unsigned long swStart = 0, elapsed = 0, lastShown = UINT32_MAX;
  
  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
  display.setTextSize(2);
  display.setTextColor(colour6);
  display.setCursor(30, 20);
  display.print("STOPWATCH");
  
  while (true) {
    if (mode == 0 || (mode == 1 && millis() - swStart != lastShown)) {
      unsigned long shown = mode == 1 ? (millis() - swStart) : elapsed;
      int ms = (shown % 1000) / 10;
      int s = (shown / 1000) % 60, m = (shown / 60000) % 60, h = shown / 3600000;
      
      display.fillRect(30, 60, 180, 80, colourBG);
      display.setTextSize(3);
      display.setTextColor(colour4);
      display.setCursor(40, 75);
      display.printf("%02d:%02d:%02d", h, m, s);
      display.setTextSize(1);
      display.setTextColor(colour3);
      display.setCursor(105, 110);
      display.printf("%02d", ms);
      
      lastShown = millis() - swStart;
    }
    
    display.fillRect(20, 195, 200, 14, colourBG);
    display.setTextSize(1);
    display.setTextColor(colour1);
    display.setCursor(30, 198);
    display.print("3:START  4:STOP  5:RESET  6:BACK");
    
    if (mode == 0 && button_is_pressed(btn3, true)) {
      swStart = millis() - elapsed;
      mode = 1;
    } else if (mode == 1 && button_is_pressed(btn4, true)) {
      elapsed = millis() - swStart;
      mode = 0;
    } else if (mode == 0 && button_is_pressed(btn5, true)) {
      elapsed = 0;
    } else if (button_is_pressed(btn6, true)) return;
    delay(50);
  }
}

const struct {
  const char* name;
  int offset;
} worldZones[] = {
  { "London", 0 }, { "Berlin", 1}, { "Tokyo", 9 }, { "Sydney", 10 }, { "LA", -8 }, { "NY", -5 }
};
#define NUM_WORLDS (sizeof(worldZones) / sizeof(worldZones[0]))

void worldClockMenu() {
  int sel = 0, oldSel = -1;
  
  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
  display.setTextSize(2);
  display.setTextColor(colour6);
  display.setCursor(35, 20);
  display.print("WORLD CLOCK");
  
  while (1) {
    if (sel != oldSel) {
      int baseY = 70, spacing = 32;
      for (int i = 0; i < NUM_WORLDS && i < 6; ++i) {
        display.fillRect(30, baseY + i * spacing - 4, 180, 26, colourBG);
        if (i == sel) {
          display.fillRoundRect(28, baseY + i * spacing - 6, 184, 30, 8, colour6);
          display.setTextSize(2);
          display.setTextColor(colourBG);
        } else {
          display.setTextSize(1);
          display.setTextColor(colour3);
        }
        display.setCursor(50, baseY + i * spacing - 1);
        display.print(worldZones[i].name);
      }
      oldSel = sel;
    }
    
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % NUM_WORLDS;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel + NUM_WORLDS - 1) % NUM_WORLDS;
    } else if (button_is_pressed(btn3, true)) {
      display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
      display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
      display.setTextSize(1);
      display.setTextColor(colour6);
      display.setCursor(60, 30);
      display.print(worldZones[sel].name);
      
      unsigned long lastUpdateTime = 0;
      while (1) {
        if (millis() - lastUpdateTime > 500) {
          time_t now = time(nullptr) + (worldZones[sel].offset * 3600);
          struct tm* tminfo = localtime(&now);
          display.fillRect(40, 75, 160, 60, colourBG);
          display.setTextSize(4);
          display.setTextColor(colour4);
          display.setCursor(50, 85);
          display.printf("%02d:%02d:%02d", tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec);
          lastUpdateTime = millis();
        }
        if (button_is_pressed(btn3, true) || button_is_pressed(btn6, true)) break;
        delay(40);
      }
      display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
      display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(35, 20);
      display.print("WORLD CLOCK");
      oldSel = -1;
    } else if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}

void timeSettingsMenu() {
  int selectedOption = 0;
  int timeOffset = getTimeOffset();
  bool use12Hour = get12HourMode();
  int dstOffset = getDSTOffset();
  int prevSelected = -1, lastTimeOffset = -100, lastDstOffset = -100;
  bool last12Hour = !use12Hour;
  
  display.fillCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2, colourBG);
  display.drawCircle(SCREEN_W/2, SCREEN_H/2, SCREEN_W/2-2, display.color565(40, 40, 40));
  display.setTextSize(2);
  display.setTextColor(colour6);
  display.setCursor(30, 20);
  display.print("SETTINGS");
  
  while (true) {
    if (button_is_pressed(btn5, true)) setRTCMenu();
    
    if (selectedOption != prevSelected || timeOffset != lastTimeOffset || use12Hour != last12Hour || dstOffset != lastDstOffset) {
      int baseY = 75, spacing = 40;
      for (int i = 0; i < 3; ++i) {
        display.fillRect(30, baseY + i * spacing - 4, 180, 28, colourBG);
        bool selected = (i == selectedOption);
        if (selected) {
          display.fillRoundRect(28, baseY + i * spacing - 6, 184, 32, 8, colour6);
          display.setTextSize(2);
          display.setTextColor(colourBG);
        } else {
          display.setTextSize(1);
          display.setTextColor(colour3);
        }
        display.setCursor(50, baseY + i * spacing);
        if (i == 0) {
          display.print("Offset ");
          if (selected) display.setTextColor(colour4);
          if (timeOffset >= 0) display.print("+");
          display.print(timeOffset);
        } else if (i == 1) {
          display.print("Mode ");
          if (selected) display.setTextColor(colour4);
          display.print(use12Hour ? "12h" : "24h");
        } else {
          display.print("DST ");
          if (selected) display.setTextColor(colour4);
          display.print(dstOffset);
        }
      }
      prevSelected = selectedOption;
      lastTimeOffset = timeOffset;
      last12Hour = use12Hour;
      lastDstOffset = dstOffset;
    }
    
    if (button_is_pressed(btn1, true)) {
      selectedOption = (selectedOption + 2) % 3;
    } else if (button_is_pressed(btn2, true)) {
      selectedOption = (selectedOption + 1) % 3;
    } else if (button_is_pressed(btn3, true)) {
      if (selectedOption == 0) {
        while (true) {
          display.fillRect(30, 155, 180, 30, colourBG);
          display.setTextSize(2);
          display.setTextColor(colour4);
          display.setCursor(55, 160);
          display.print("Offset ");
          if (timeOffset >= 0) display.print("+");
          display.print(timeOffset);
          if (button_is_pressed(btn1, true)) {
            timeOffset--;
            if (timeOffset < -12) timeOffset = -12;
          } else if (button_is_pressed(btn2, true)) {
            timeOffset++;
            if (timeOffset > 12) timeOffset = 12;
          } else if (button_is_pressed(btn6, true)) {
            setTimeOffset(timeOffset);
            break;
          }
          delay(80);
        }
      } else if (selectedOption == 1) {
        use12Hour = !use12Hour;
        set12HourMode(use12Hour);
      } else {
        while (true) {
          display.fillRect(30, 155, 180, 30, colourBG);
          display.setTextSize(2);
          display.setTextColor(colour4);
          display.setCursor(75, 160);
          display.print("DST ");
          display.print(dstOffset);
          if (button_is_pressed(btn1, true)) {
            dstOffset--;
            if (dstOffset < 0) dstOffset = 0;
          } else if (button_is_pressed(btn2, true)) {
            dstOffset++;
            if (dstOffset > 2) dstOffset = 2;
          } else if (button_is_pressed(btn6, true)) {
            setDSTOffset(dstOffset);
            break;
          }
          delay(80);
        }
      }
      prevSelected = -1;
    } else if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}