// Modern Round 240x240 Calendar & Alarm System

#include "Watch5.1.h"

extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern bool wifiConnected;
extern Preferences preferences;
extern bool inputStringOnWatch(const char* label, char* buffer, int maxLen);

#define MAX_CALENDAR_EVENTS 20
#define MAX_EVENT_MESSAGE 64

CalendarEvent events[MAX_CALENDAR_EVENTS];
int eventCount = 0;

void selectDateTime(CalendarEvent* event);
void incrementField(CalendarEvent* event, int field);
void decrementField(CalendarEvent* event, int field);
void triggerAlarm(CalendarEvent* event);
void deleteEvent(int idx);
void editEvent(int idx);
void viewEvents();
void createNewEvent();

void initializeCalendar() {
  for (int i = 0; i < MAX_CALENDAR_EVENTS; i++) {
    events[i].used = false;
    events[i].alarmed = false;
  }
}

void saveCalendarToNVS() {
  preferences.begin("calendar", false);
  preferences.putInt("count", eventCount);
  for (int i = 0; i < eventCount; i++) {
    String keyYear = "evt_" + String(i) + "_y";
    String keyMonth = "evt_" + String(i) + "_mo";
    String keyDay = "evt_" + String(i) + "_d";
    String keyHour = "evt_" + String(i) + "_h";
    String keyMin = "evt_" + String(i) + "_mi";
    String keyMsg = "evt_" + String(i) + "_msg";
    String keyUsed = "evt_" + String(i) + "_u";
    preferences.putInt(keyYear.c_str(), events[i].year);
    preferences.putInt(keyMonth.c_str(), events[i].month);
    preferences.putInt(keyDay.c_str(), events[i].day);
    preferences.putInt(keyHour.c_str(), events[i].hour);
    preferences.putInt(keyMin.c_str(), events[i].minute);
    preferences.putString(keyMsg.c_str(), events[i].message);
    preferences.putBool(keyUsed.c_str(), events[i].used);
  }
  preferences.end();
}

void loadCalendarFromNVS() {
  preferences.begin("calendar", true);
  eventCount = preferences.getInt("count", 0);
  for (int i = 0; i < eventCount && i < MAX_CALENDAR_EVENTS; i++) {
    String keyYear = "evt_" + String(i) + "_y";
    String keyMonth = "evt_" + String(i) + "_mo";
    String keyDay = "evt_" + String(i) + "_d";
    String keyHour = "evt_" + String(i) + "_h";
    String keyMin = "evt_" + String(i) + "_mi";
    String keyMsg = "evt_" + String(i) + "_msg";
    String keyUsed = "evt_" + String(i) + "_u";
    events[i].year = preferences.getInt(keyYear.c_str(), 0);
    events[i].month = preferences.getInt(keyMonth.c_str(), 0);
    events[i].day = preferences.getInt(keyDay.c_str(), 0);
    events[i].hour = preferences.getInt(keyHour.c_str(), 0);
    events[i].minute = preferences.getInt(keyMin.c_str(), 0);
    String msg = preferences.getString(keyMsg.c_str(), "");
    strncpy(events[i].message, msg.c_str(), MAX_EVENT_MESSAGE - 1);
    events[i].message[MAX_EVENT_MESSAGE - 1] = '\0';
    events[i].used = preferences.getBool(keyUsed.c_str(), false);
    events[i].alarmed = false;
  }
  preferences.end();
}

void calendar(void) {
  loadCalendarFromNVS();
  int redraw = 1;
  while (true) {
    if (redraw) {
      display.fillScreen(COLOR_BG);
      display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, COLOR_ACCENT);
      display.setTextColor(COLOR_FG); display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X-48, 32); display.print("Calendar");

      display.setTextColor(COLOR_ACCENT); display.setTextSize(1);
      char buf[32]; sprintf(buf,"Events: %d", eventCount);
      display.setCursor(SCREEN_CENTER_X-STR_W(buf,1)/2, 72); display.print(buf);

      display.setTextColor(COLOR_FG); display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X-48, 108); display.print("3:New Event");
      display.setCursor(SCREEN_CENTER_X-48, 128); display.print("2:View Events");
      display.setCursor(SCREEN_CENTER_X-48, 148); display.print("6:Back");

      redraw = 0;
    }
    if (button_is_pressed(btn3)) { createNewEvent(); redraw = 1; delay(240);}
    else if (button_is_pressed(btn2)) { viewEvents(); redraw = 1; delay(240);}
    else if (button_is_pressed(btn6, true)) { return; }
    delay(40);
  }
}

void createNewEvent() {
  if (eventCount >= MAX_CALENDAR_EVENTS) {
    display.fillScreen(COLOR_ERROR);
    display.setTextColor(COLOR_FG);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - 84, SCREEN_CENTER_Y-8);
    display.print("Max events!");
    delay(1300); return;
  }
  CalendarEvent newEvent;
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  newEvent.year = timeinfo->tm_year + 1900;
  newEvent.month = timeinfo->tm_mon + 1;
  newEvent.day = timeinfo->tm_mday;
  newEvent.hour = timeinfo->tm_hour;
  newEvent.minute = timeinfo->tm_min;
  selectDateTime(&newEvent);
  if (!inputStringOnWatch("Message:", newEvent.message, MAX_EVENT_MESSAGE)) { return; }
  display.fillScreen(COLOR_BG);
  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(1);
  display.setCursor(SCREEN_CENTER_X-40,SCREEN_CENTER_Y-32);
  display.print("Confirm Event?");
  char buf[40];
  sprintf(buf,"%04d-%02d-%02d",newEvent.year,newEvent.month,newEvent.day);
  display.setCursor(SCREEN_CENTER_X-STR_W(buf,1)/2, SCREEN_CENTER_Y-18); display.print(buf);
  sprintf(buf,"%02d:%02d",newEvent.hour,newEvent.minute);
  display.setCursor(SCREEN_CENTER_X-STR_W(buf,1)/2, SCREEN_CENTER_Y-6); display.print(buf);
  display.setCursor(SCREEN_CENTER_X-STR_W(newEvent.message,1)/2,SCREEN_CENTER_Y+8); display.print(newEvent.message);
  display.setCursor(SCREEN_CENTER_X-44, SCREEN_HEIGHT-32); display.print("3:Save      6:Cancel");
  while (1) {
    if (button_is_pressed(btn3)) {
      newEvent.used = true; newEvent.alarmed = false;
      events[eventCount++] = newEvent;
      saveCalendarToNVS();
      display.fillScreen(COLOR_ACCENT);
      display.setTextColor(COLOR_BG); display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X-40,SCREEN_CENTER_Y-9); display.print("Saved!");
      delay(900); return;
    }
    if (button_is_pressed(btn6, true)) {
      display.fillScreen(COLOR_ACCENT);
      display.setTextColor(COLOR_ERROR); display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X-76,SCREEN_CENTER_Y-9); display.print("Cancelled");
      delay(650); return;
    }
    delay(40);
  }
}

void selectDateTime(CalendarEvent* event) {
  int field=0; // 0=year, 1=month, 2=day, 3=hour, 4=minute
  while (1) {
    display.fillScreen(COLOR_BG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, COLOR_ACCENT);
    display.setTextColor(COLOR_FG); display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X-56,34); display.print("Date & Time");
    display.setTextColor(COLOR_ACCENT); display.setTextSize(2);

    int cy = SCREEN_CENTER_Y-20;
    char buf[32];
    // Date (YYYY-MM-DD)
    display.setCursor(SCREEN_CENTER_X-48, cy);
    if(field==0) display.setTextColor(COLOR_SELECTED); else display.setTextColor(COLOR_ACCENT);
    sprintf(buf,"%04d",event->year); display.print(buf); display.setTextColor(COLOR_ACCENT); display.print("-");
    if(field==1) display.setTextColor(COLOR_SELECTED); else display.setTextColor(COLOR_ACCENT);
    sprintf(buf,"%02d",event->month); display.print(buf); display.setTextColor(COLOR_ACCENT); display.print("-");
    if(field==2) display.setTextColor(COLOR_SELECTED); else display.setTextColor(COLOR_ACCENT);
    sprintf(buf,"%02d",event->day); display.print(buf);

    // Time (HH:MM)
    display.setTextSize(2);
    cy+=34;
    display.setCursor(SCREEN_CENTER_X-34,cy);
    if(field==3) display.setTextColor(COLOR_SELECTED); else display.setTextColor(COLOR_ACCENT);
    sprintf(buf,"%02d",event->hour); display.print(buf); display.setTextColor(COLOR_ACCENT); display.print(":");
    if(field==4) display.setTextColor(COLOR_SELECTED); else display.setTextColor(COLOR_ACCENT);
    sprintf(buf,"%02d",event->minute); display.print(buf);

    display.setTextColor(COLOR_FG); display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X-56,SCREEN_HEIGHT-24);
    display.print("1:- 2:+ 3:Next 6:Done");
    if (button_is_pressed(btn1)) { decrementField(event, field); delay(100); }
    else if (button_is_pressed(btn2)) { incrementField(event, field); delay(100); }
    else if (button_is_pressed(btn3)) { field=(field+1)%5; delay(130);}
    else if (button_is_pressed(btn6, true)) return;
    delay(26);
  }
}

void incrementField(CalendarEvent* event, int field) {
  switch (field) {
    case 0: event->year++; if (event->year > 2100) event->year = 2024; break;
    case 1: event->month++; if (event->month > 12) event->month = 1; break;
    case 2: event->day++; if (event->day > 31) event->day = 1; break;
    case 3: event->hour++; if (event->hour > 23) event->hour = 0; break;
    case 4: event->minute++; if (event->minute > 59) event->minute = 0; break;
  }
}
void decrementField(CalendarEvent* event, int field) {
  switch (field) {
    case 0: event->year--; if (event->year < 2024) event->year = 2100; break;
    case 1: event->month--; if (event->month < 1) event->month = 12; break;
    case 2: event->day--; if (event->day < 1) event->day = 31; break;
    case 3: event->hour--; if (event->hour < 0) event->hour = 23; break;
    case 4: event->minute--; if (event->minute < 0) event->minute = 59; break;
  }
}

void viewEvents() {
  if (eventCount == 0) {
    display.fillScreen(COLOR_ERROR); display.setTextColor(COLOR_BG); display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X-44,SCREEN_CENTER_Y-10);
    display.print("No events!"); delay(1000); return;
  }
  int selectedIdx = 0;
  while (1) {
    display.fillScreen(COLOR_BG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS-1, COLOR_ACCENT);
    display.setTextColor(COLOR_ACCENT); display.setTextSize(2);
    char buf[20];
    sprintf(buf, "%d / %d", selectedIdx+1, eventCount);
    display.setCursor(SCREEN_CENTER_X-STR_W(buf,2)/2, 32);
    display.print(buf);

    const CalendarEvent* ev = &events[selectedIdx];
    display.setTextColor(COLOR_FG); display.setTextSize(1);
    sprintf(buf,"%04d-%02d-%02d",ev->year,ev->month,ev->day);
    display.setCursor(SCREEN_CENTER_X-STR_W(buf,1)/2, 72); display.print(buf);
    sprintf(buf,"%02d:%02d",ev->hour,ev->minute);
    display.setCursor(SCREEN_CENTER_X-STR_W(buf,1)/2, 92); display.print(buf);
    display.setTextColor(COLOR_ACCENT); display.setCursor(SCREEN_CENTER_X-STR_W(ev->message,1)/2,112);
    display.print(ev->message);

    display.setTextSize(1); display.setTextColor(COLOR_FG);
    display.setCursor(SCREEN_CENTER_X-66, SCREEN_HEIGHT-34);
    display.print("1/2:Prev/Next 4:Del 5:Edit 6:Back");
    if (button_is_pressed(btn1)) { selectedIdx = (selectedIdx - 1 + eventCount) % eventCount; delay(120);}
    else if (button_is_pressed(btn2)) { selectedIdx = (selectedIdx + 1) % eventCount; delay(120);}
    else if (button_is_pressed(btn4)) { deleteEvent(selectedIdx); if(selectedIdx>=eventCount)selectedIdx=eventCount-1; delay(200);}
    else if (button_is_pressed(btn5)) { editEvent(selectedIdx); delay(200);}
    else if (button_is_pressed(btn6, true)) { return; }
    delay(34);
  }
}

void deleteEvent(int idx) {
  display.fillScreen(COLOR_ERROR);
  display.setTextColor(COLOR_BG); display.setTextSize(2);
  display.setCursor(SCREEN_CENTER_X-48,SCREEN_CENTER_Y-10);
  display.print("Delete?");
  display.setTextSize(1);
  display.setCursor(SCREEN_CENTER_X-STR_W(events[idx].message,1)/2,SCREEN_CENTER_Y+16);
  display.print(events[idx].message);
  display.setCursor(SCREEN_CENTER_X-36,SCREEN_HEIGHT-28);
  display.print("3:Yes 6:Cancel");
  while (1) {
    if (button_is_pressed(btn3)) {
      for (int i = idx; i < eventCount - 1; i++) events[i] = events[i + 1];
      eventCount--; saveCalendarToNVS();
      display.fillScreen(COLOR_ACCENT); display.setTextColor(COLOR_BG); display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X-40,SCREEN_CENTER_Y-12); display.print("Deleted!");
      delay(800); return;
    }
    if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}

void editEvent(int idx) {
  selectDateTime(&events[idx]);
  char newMessage[MAX_EVENT_MESSAGE];
  strncpy(newMessage, events[idx].message, MAX_EVENT_MESSAGE - 1);
  if (inputStringOnWatch("Message:", newMessage, MAX_EVENT_MESSAGE)) {
    strncpy(events[idx].message, newMessage, MAX_EVENT_MESSAGE - 1);
    saveCalendarToNVS();
    display.fillScreen(COLOR_ACCENT); display.setTextColor(COLOR_BG); display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X-52,SCREEN_CENTER_Y-8); display.print("Updated!");
    delay(900);
  }
}

void checkCalendarAlarms() {
  if (!wifiConnected) return;
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int currentYear = timeinfo->tm_year + 1900;
  int currentMonth = timeinfo->tm_mon + 1;
  int currentDay = timeinfo->tm_mday;
  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;
  for (int i = 0; i < eventCount; i++) {
    if (!events[i].used || events[i].alarmed) continue;
    if (events[i].year == currentYear &&
        events[i].month == currentMonth &&
        events[i].day == currentDay &&
        events[i].hour == currentHour &&
        events[i].minute == currentMinute) {
      triggerAlarm(&events[i]);
      events[i].alarmed = true;
      saveCalendarToNVS();
    }
  }
}

void triggerAlarm(CalendarEvent* event) {
  int c = 0;
  while (!button_is_pressed(btn6, true)) {
    c++;
    display.fillScreen((c%2)?COLOR_BG:COLOR_ERROR);
    display.setTextColor(COLOR_BG); display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X-38,SCREEN_CENTER_Y-20); display.print("ALARM!");
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X-STR_W(event->message,1)/2,SCREEN_CENTER_Y); display.print(event->message);
    display.setCursor(SCREEN_CENTER_X-36,SCREEN_HEIGHT-24); display.print("6:Dismiss");
    int t=0; while(t<13 && !button_is_pressed(btn6, true)){ delay(45); t++;}
  }
}