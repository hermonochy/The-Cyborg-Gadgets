extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG, colourText, colour1, colour2, colour3, colour4, colour5, colour6;
extern bool wifiConnected;
extern Preferences preferences;

#define TIME_SETTINGS "time_settings"
#define DEFAULT_TIME_OFFSET 0
#define DEFAULT_12_HOUR_MODE false
#define DEFAULT_DST_OFFSET 0

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

// -------- MAIN MENU --------
const char* timeFuncs[] = { "Clock", "Alarm", "Timer", "Stopwatch", "World Clock", "Settings" };
#define TIMEFUNCS_COUNT 6

void alarmMenu();
void timerMenu();
void stopwatchMenu();
void worldClockMenu();

void timeMenu() {
  int sel = 0, oldSel = -1;
  while (1) {
    if (sel != oldSel) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds("TIME", 0, 0, &x1, &y1, &w, &h);
      display.setCursor(SCREEN_WIDTH / 2 - w / 2, 18);
      display.print("TIME");
      int baseY = 56, spacing = 30;
      for (int i = 0; i < TIMEFUNCS_COUNT; i++) {
        display.setTextSize(i == sel ? 2 : 1);
        display.setTextColor(i == sel ? colour1 : colour3);
        display.setCursor(44, baseY + i * spacing);
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
  const int cx = SCREEN_WIDTH/2, cy = SCREEN_HEIGHT/2;

  auto drawCore = [&](bool redrawLabels = false) {
    display.setTextSize(2);
    display.setTextColor(colourText, colourBG);
    display.fillCircle(cx, cy, cx, colourBG);
    if (redrawLabels) {
      display.setTextColor(colour4, colourBG);
      display.setTextSize(1);
      display.setCursor(cx-52, 14);
      display.print("SET TIME & DATE");
    }

    int y = 80;
    int sep = 34;
    for (int i = 0; i < 6; ++i) {
      if (oldfields[i] != fields[i] || oldsel != sel || redrawLabels) {
        int dx = (i<3)?-58:42;
        int fy = y+sep*(i%3);
        display.fillRect(cx+dx-2, fy-2, 84, 36, colourBG);
        if (i == sel)
          display.fillRoundRect(cx+dx-6, fy-6, 94, 40, 12, colour1);
        display.setCursor(cx+dx, fy);
        display.setTextColor(i==sel?colourBG:colourText, i==sel?colour1:colourBG);
        if (i==1 || i==2) // 2-digit
          display.printf("%s: %02d", labels[i], fields[i]);
        else if (i==4 || i==5)
          display.printf("%s: %02d", labels[i], fields[i]);
        else
          display.printf("%s: %d", labels[i], fields[i]);
      }
    }

    if (redrawLabels || oldfields[0]!=fields[0] || oldfields[1]!=fields[1] || oldfields[2]!=fields[2]) {
      char datebuf[20];
      sprintf(datebuf, "%04d-%02d-%02d", fields[0], fields[1], fields[2]);
      display.setTextColor(colour2, colourBG);
      display.fillRect(cx-76, 34, 152, 28, colourBG);
      display.setCursor(cx-60, 36);
      display.print(datebuf);
    }
    if (redrawLabels || oldfields[3]!=fields[3] || oldfields[4]!=fields[4] || oldfields[5]!=fields[5]) {
      char timebuf[16];
      sprintf(timebuf, "%02d:%02d:%02d", fields[3], fields[4], fields[5]);
      display.setTextColor(colour3, colourBG);
      display.fillRect(cx-52, 174, 104, 32, colourBG);
      display.setCursor(cx-48, 178);
      display.print(timebuf);
    }

    display.setTextColor(colour4, colourBG);
    display.fillRect(0, 218, SCREEN_WIDTH, 22, colourBG);
    display.setCursor(cx-90, 222);
    display.print("1/2:-/+ 3:Nxt 4:Save");
  };

  while (true) {
    if (oldsel != sel
      || fields[0]!=oldfields[0] || fields[1]!=oldfields[1] || fields[2]!=oldfields[2]
      || fields[3]!=oldfields[3] || fields[4]!=oldfields[4] || fields[5]!=oldfields[5]
      ) drawCore(oldsel == -1);

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
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour1);
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds("TIME", 0, 0, &x1, &y1, &w, &h);
      display.setCursor(SCREEN_WIDTH / 2 - w / 2, 18);
      display.print("TIME");

      display.setTextSize(3);
      display.setTextColor(colourText);
      char timestr[16];
      sprintf(timestr, "%02d:%02d:%02d", hour, timeinfo->tm_min, timeinfo->tm_sec);
      display.getTextBounds(timestr, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(SCREEN_WIDTH / 2 - w / 2, 66);
      display.print(timestr);
      if (use12Hour) {
        display.setTextSize(2);
        display.setTextColor(colour2);
        display.setCursor(SCREEN_WIDTH / 2 + 54, 85);
        display.print(ampm);
        strncpy(lastAMP, ampm, 4);
      }
      display.setTextSize(1);
      display.setTextColor(colour3);
      display.setCursor(SCREEN_WIDTH / 2 - 60, 128);
      const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
      display.printf("%s %d, %d", months[timeinfo->tm_mon], timeinfo->tm_mday, timeinfo->tm_year + 1900);
      lastHour = hour;
      lastMin = timeinfo->tm_min;
      lastSec = timeinfo->tm_sec;
      lastDay = timeinfo->tm_mday;
      lastMonth = timeinfo->tm_mon;
      lastYear = timeinfo->tm_year;
      last12h = use12Hour;
    }
    display.setTextSize(1);
    display.setTextColor(colour4);
    display.setCursor(40, 190);
    display.print("Btn3:Menu Btn6:Back");
    if (button_is_pressed(btn3, true)) return;
    else if (button_is_pressed(btn6, true)) return;
    delay(333);
  }
}

// -------- ALARM --------
#define MAX_ALARMS 5
struct Alarm {
  int hour, minute;
  bool enabled;
} alarms[MAX_ALARMS] = { 0 };
int numAlarms = 0;

void alarmMenu() {
  int sel = 0, oldSel = -1, mode = 0, aIdx = 0, ah = 7, am = 0, lastNumAlarms = -1;
  while (1) {
    if (mode == 0 && (oldSel != sel || lastNumAlarms != numAlarms)) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour1);
      display.setCursor(SCREEN_WIDTH / 2 - 36, 20);
      display.print("ALARMS");
      if (numAlarms == 0) {
        display.setTextSize(1);
        display.setCursor(60, 90);
        display.setTextColor(colour3);
        display.print("No alarms...");
      }
      for (int i = 0; i < numAlarms; i++) {
        display.setTextSize(sel == i ? 2 : 1);
        display.setTextColor(sel == i ? colour2 : colour3);
        int y = 68 + i * 30;
        display.setCursor(54, y);
        display.printf("%02d:%02d %s", alarms[i].hour, alarms[i].minute, alarms[i].enabled ? "On" : "Off");
      }
      display.setTextSize(1);
      display.setTextColor(colour5);
      display.setCursor(34, 198);
      display.print("btn1/2:Up/Down btn3:Edit btn4:New btn5:Del btn6:Bk");
      oldSel = sel;
      lastNumAlarms = numAlarms;
    }
    if (mode == 0 && button_is_pressed(btn2, true)) {
      sel = (sel + 1) % max(1, numAlarms);
    } else if (mode == 0 && button_is_pressed(btn1, true)) {
      sel = (sel - 1 + (max(1, numAlarms))) % max(1, numAlarms);
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
      display.fillRect(24, 144, 192, 52, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour1);
      display.setCursor(60, 154);
      display.printf("Set: %02d:%02d", ah, am);
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(56, 196);
      display.print("btn1/hr- btn2/hr+ btn3/min- btn4/min+ btn5:OK");
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
      // show an alert
      display.fillScreen(colourBG);
      display.setTextSize(2);
      display.setTextColor(colour3);
      display.setCursor(32, 100);
      display.print("ALARM!");
      display.setTextSize(3);
      display.setTextColor(colour1);
      display.setCursor(40, 140);
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
  int setSec = 0, timerMode = 0;  // 0=set, 1=running, 2=finished
  int remainSec = 0;
  unsigned long startMs = 0, oldRem = -1;
  while (1) {
    if (timerMode == 0) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour2);
      display.setCursor(82, 28);
      display.print("TIMER");
      display.setTextSize(3);
      display.setTextColor(colourText);
      display.setCursor(54, 82);
      int min = setSec / 60, sec = setSec % 60;
      display.printf("%02d:%02d", min, sec);
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(30, 170);
      display.print("btn1/2:-/+min btn3/4:-/+sec btn5:Go btn6:Bk");
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
        display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
        display.setTextSize(3);
        display.setTextColor(colourText);
        display.setCursor(54, 82);
        int min = left / 60, sec = left % 60;
        display.printf("%02d:%02d", min, sec);
        int arcR = 90;
        int arcT = 14;
        float frac = left / (float)setSec;
        for (int a = 0; a < 360; a += 4) {
          int x = SCREEN_WIDTH / 2 + cos(a * 3.14159 / 180.0) * (arcR - arcT / 2);
          int y = SCREEN_HEIGHT / 2 + sin(a * 3.14159 / 180.0) * (arcR - arcT / 2);
          int x2 = SCREEN_WIDTH / 2 + cos(a * 3.14159 / 180.0) * (arcR + arcT / 2);
          int y2 = SCREEN_HEIGHT / 2 + sin(a * 3.14159 / 180.0) * (arcR + arcT / 2);
          display.drawLine(x, y, x2, y2, (a / 360.0) < frac ? colour2 : colour6);
        }
        oldRem = left;
      }
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(44, 170);
      display.print("btn3:Pause btn4:Reset btn6:Bk");
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
      } else if (button_is_pressed(btn6, true)) return;
    } else if (timerMode == 2) {
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour5);
      display.setCursor(54, 100);
      display.print("TIMER DONE!");
      for (int t = 0; t < 60; t++) {
        if (button_is_pressed(btn3, true) || button_is_pressed(btn6, true)) break;
        delay(400);
      }
      timerMode = 0;
      oldRem = -1;
    }
    delay(40);
  }
}

void stopwatchMenu() {
  int mode = 0;  // 0=stopped 1=running
  unsigned long swStart = 0, elapsed = 0, lastShown = UINT32_MAX;
  while (true) {
    if (mode == 0 || (mode == 1 && millis() - swStart != lastShown)) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour1);
      display.setCursor(62, 24);
      display.print("STOPWATCH");
      unsigned long shown = mode == 1 ? (millis() - swStart) : elapsed;
      int ms = (shown % 1000) / 10;
      int s = (shown / 1000) % 60, m = (shown / 60000) % 60, h = shown / 3600000;
      display.setTextSize(3);
      display.setTextColor(colour2);
      display.setCursor(34, 82);
      display.printf("%02d:%02d:%02d.%02d", h, m, s, ms);
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(30, 170);
      display.print("btn3:Start btn4:Stop btn5:Reset btn6:Bk");
      lastShown = millis() - swStart;
    }
    if (mode == 0 && (button_is_pressed(btn3, true))) {
      swStart = millis() - elapsed;
      mode = 1;
    } else if (mode == 1 && (button_is_pressed(btn4, true))) {
      elapsed = millis() - swStart;
      mode = 0;
    } else if (mode == 0 && (button_is_pressed(btn5, true))) {
      elapsed = 0;
    } else if (button_is_pressed(btn6, true)) return;
    delay(50);
  }
}

const struct {
  const char* name;
  int offset;
} worldZones[] = {
  { "London", 0 }, { "Berlin", 1}
};
#define NUM_WORLDS (sizeof(worldZones) / sizeof(worldZones[0]))

void worldClockMenu() {
  int sel = 0, oldSel = -1;
  while (1) {
    if (sel != oldSel) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(38, 14);
      display.print("WORLD CLOCK");
      int baseY = 52;
      for (int i = 0; i < NUM_WORLDS && i < 6; ++i) {
        display.setTextSize(i == sel ? 2 : 1);
        display.setTextColor(i == sel ? colour2 : colour3);
        display.setCursor(37, baseY + i * 30);
        display.print(worldZones[i].name);
      }
      oldSel = sel;
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % NUM_WORLDS;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel + NUM_WORLDS - 1) % NUM_WORLDS;
    } else if (button_is_pressed(btn3, true)) {
      // show world time
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);
      time_t now = time(nullptr) + (worldZones[sel].offset * 3600);
      struct tm* tminfo = localtime(&now);
      display.setTextSize(2);
      display.setTextColor(colour2);
      display.setCursor(62, 32);
      display.print(worldZones[sel].name);
      display.setTextSize(3);
      display.setTextColor(colour1);
      display.setCursor(44, 82);
      display.printf("%02d:%02d:%02d", tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec);
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(60, 170);
      display.print("Btn3:Back Btn6:Quit");
      delay(500);
      while (1) {
        if (button_is_pressed(btn3, true) || button_is_pressed(btn6, true)) break;
        time_t now2 = time(nullptr) + (worldZones[sel].offset * 3600);
        struct tm* tminfo2 = localtime(&now2);
        display.fillRect(44, 82, 150, 32, colourBG);
        display.setTextSize(3);
        display.setTextColor(colour1);
        display.setCursor(44, 82);
        display.printf("%02d:%02d:%02d", tminfo2->tm_hour, tminfo2->tm_min, tminfo2->tm_sec);
        delay(400);
      }
      oldSel = -1;
    } else if (button_is_pressed(btn6, true)) return;
    delay(50);
  }
}

void timeSettingsMenu() {
  int selectedOption = 0;
  int timeOffset = getTimeOffset();
  bool use12Hour = get12HourMode();
  int dstOffset = getDSTOffset();
  int prevSelected = -1, lastTimeOffset = -100, lastDstOffset = -100;
  bool last12Hour = !use12Hour;
  while (true) {
    if (button_is_pressed(btn5, true)) setRTCMenu();
    if (selectedOption != prevSelected || timeOffset != lastTimeOffset || use12Hour != last12Hour || dstOffset != lastDstOffset) {
      display.fillScreen(colourBG);
      display.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, colourBG);

      display.setTextSize(2);
      display.setTextColor(colourText);
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds("TIME SETTINGS", 0, 0, &x1, &y1, &w, &h);
      display.setCursor(SCREEN_WIDTH / 2 - w / 2, 28);
      display.print("TIME SETTINGS");

      int baseY = 75, spacing = 38;
      for (int i = 0; i < 3; ++i) {
        bool selected = (i == selectedOption);
        display.setTextSize(selected ? 2 : 1);
        display.setTextColor(selected ? colour1 : colour3);
        display.setCursor(48, baseY + i * spacing);
        if (i == 0) {
          display.print("Time offset: ");
          if (timeOffset >= 0) display.print("+");
          display.print(timeOffset);
          display.print("h");
        } else if (i == 1) {
          display.print("Mode: ");
          display.print(use12Hour ? "12h" : "24h");
        } else {
          display.print("DST: ");
          display.print(dstOffset);
          display.print("h");
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
        int prev = timeOffset;
        while (true) {
          display.fillRect(0, 140, SCREEN_WIDTH, 45, colourBG);
          display.setTextSize(2);
          display.setTextColor(colour1);
          display.setCursor(60, 150);
          display.print("Offset: ");
          if (timeOffset >= 0) display.print("+");
          display.print(timeOffset);
          display.print("h");
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
        int prev = dstOffset;
        while (true) {
          display.fillRect(0, 140, SCREEN_WIDTH, 45, colourBG);
          display.setTextSize(2);
          display.setTextColor(colour1);
          display.setCursor(75, 150);
          display.print("DST: ");
          display.print(dstOffset);
          display.print("h");
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