// Calendar & Alarm System

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern bool wifiConnected;
extern Preferences preferences;
extern bool inputStringOnWatch(const char* label, char* buffer, int maxLen);

#define MAX_CALENDAR_EVENTS 20
#define MAX_EVENT_MESSAGE 128

struct CalendarEvent {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  char message[MAX_EVENT_MESSAGE];
  bool used;
  bool alarmed;
};

void selectDateTime(CalendarEvent* event);
void incrementField(CalendarEvent* event, int field);
void decrementField(CalendarEvent* event, int field);
void triggerAlarm(CalendarEvent* event);
void deleteEvent(int idx);
void editEvent(int idx);
void serialAddEvent();
void serialViewEvents();
void serialDeleteEvent();
void serialEditEvent();

CalendarEvent events[MAX_CALENDAR_EVENTS];
int eventCount = 0;

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
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Calendar");
    display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    display.print("Events: ");
    display.println(eventCount);
    
    display.setCursor(0, 35);
    display.println("3:New Event");
    display.setCursor(0, 45);
    display.println("2:View Events");
    display.setCursor(0, 55);
    display.println("6:Back");
    
    display.display();
    
    if (button_is_pressed(btn3)) {
      createNewEvent();
      delay(200);
    }
    else if (button_is_pressed(btn2)) {
      viewEvents();
      delay(200);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(50);
  }
}

void createNewEvent() {
  if (eventCount >= MAX_CALENDAR_EVENTS) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Max events reached!");
    display.display();
    delay(2000);
    return;
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
  
  if (!inputStringOnWatch("Message:", newEvent.message, MAX_EVENT_MESSAGE)) {
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Confirm Event?");
  display.setCursor(0, 20);
  display.printf("%04d-%02d-%02d", newEvent.year, newEvent.month, newEvent.day);
  display.setCursor(0, 30);
  display.printf("%02d:%02d", newEvent.hour, newEvent.minute);
  display.setCursor(0, 40);
  display.println(newEvent.message);
  display.setCursor(0, 56);
  display.println("3:Save 6:Cancel");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn3)) {
      newEvent.used = true;
      newEvent.alarmed = false;
      events[eventCount] = newEvent;
      eventCount++;
      saveCalendarToNVS();
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Event saved!");
      display.display();
      delay(1500);
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

void selectDateTime(CalendarEvent* event) {
  int field = 0; // 0=year, 1=month, 2=day, 3=hour, 4=minute
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Set Date & Time");
    
    display.setCursor(0, 20);
    
    if (field == 0) display.print(">");
    display.printf("%04d", event->year);
    display.print("-");
    
    if (field == 1) display.print(">");
    display.printf("%02d", event->month);
    display.print("-");
    
    if (field == 2) display.print(">");
    display.printf("%02d", event->day);
    
    display.setCursor(0, 40);
    if (field == 3) display.print(">");
    display.printf("%02d", event->hour);
    display.print(":");
    
    if (field == 4) display.print(">");
    display.printf("%02d", event->minute);
    
    display.setCursor(0, 56);
    display.println("1:- 2:+ 3:Next 6:Done");
    
    display.display();
    
    if (button_is_pressed(btn1)) {
      decrementField(event, field);
      delay(100);
    }
    else if (button_is_pressed(btn2)) {
      incrementField(event, field);
      delay(100);
    }
    else if (button_is_pressed(btn3)) {
      field = (field + 1) % 5;
      delay(150);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(50);
  }
}

void incrementField(CalendarEvent* event, int field) {
  switch (field) {
    case 0: // year
      event->year++;
      if (event->year > 2100) event->year = 2024;
      break;
    case 1: // month
      event->month++;
      if (event->month > 12) event->month = 1;
      break;
    case 2: // day
      event->day++;
      if (event->day > 31) event->day = 1;
      break;
    case 3: // hour
      event->hour++;
      if (event->hour > 23) event->hour = 0;
      break;
    case 4: // minute
      event->minute++;
      if (event->minute > 59) event->minute = 0;
      break;
  }
}

void decrementField(CalendarEvent* event, int field) {
  switch (field) {
    case 0: // year
      event->year--;
      if (event->year < 2024) event->year = 2100;
      break;
    case 1: // month
      event->month--;
      if (event->month < 1) event->month = 12;
      break;
    case 2: // day
      event->day--;
      if (event->day < 1) event->day = 31;
      break;
    case 3: // hour
      event->hour--;
      if (event->hour < 0) event->hour = 23;
      break;
    case 4: // minute
      event->minute--;
      if (event->minute < 0) event->minute = 59;
      break;
  }
}

void viewEvents() {
  if (eventCount == 0) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("No events");
    display.display();
    delay(1500);
    return;
  }
  
  int selectedIdx = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Events (");
    display.print(selectedIdx + 1);
    display.print("/");
    display.print(eventCount);
    display.println(")");
    
    if (events[selectedIdx].used) {
      display.setCursor(0, 20);
      display.printf("%04d-%02d-%02d", events[selectedIdx].year, events[selectedIdx].month, events[selectedIdx].day);
      display.setCursor(0, 30);
      display.printf("%02d:%02d", events[selectedIdx].hour, events[selectedIdx].minute);
      
      display.setCursor(0, 40);
      display.println(events[selectedIdx].message);
      
      display.setCursor(0, 56);
      display.println("4:Del 5:Edit 6:Back");
    }
    
    display.display();
    
    if (button_is_pressed(btn1)) {
      selectedIdx = (selectedIdx - 1 + eventCount) % eventCount;
      delay(150);
    }
    else if (button_is_pressed(btn2)) {
      selectedIdx = (selectedIdx + 1) % eventCount;
      delay(150);
    }
    else if (button_is_pressed(btn4)) {
      deleteEvent(selectedIdx);
      if (selectedIdx >= eventCount && eventCount > 0) {
        selectedIdx = eventCount - 1;
      }
      delay(200);
    }
    else if (button_is_pressed(btn5)) {
      editEvent(selectedIdx);
      delay(200);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(50);
  }
}

void deleteEvent(int idx) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Delete?");
  display.setCursor(0, 20);
  display.println(events[idx].message);
  display.setCursor(0, 50);
  display.println("3:Yes 6:Cancel");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn3)) {
      for (int i = idx; i < eventCount - 1; i++) {
        events[i] = events[i + 1];
      }
      eventCount--;
      saveCalendarToNVS();
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Deleted!");
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

void editEvent(int idx) {
  selectDateTime(&events[idx]);
  
  char newMessage[MAX_EVENT_MESSAGE];
  strncpy(newMessage, events[idx].message, MAX_EVENT_MESSAGE - 1);
  
  if (inputStringOnWatch("Message:", newMessage, MAX_EVENT_MESSAGE)) {
    strncpy(events[idx].message, newMessage, MAX_EVENT_MESSAGE - 1);
    saveCalendarToNVS();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Event updated!");
    display.display();
    delay(1500);
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
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("ALARM!");
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println(event->message);
  
  display.setCursor(0, 56);
  display.println("6:Dismiss");
  display.display();

  while (!button_is_pressed(btn6)) {
    display.invertDisplay(true);
    delay(250);
    display.invertDisplay(false);
    delay(250);
  }
}

// Serial menu functions
void serialCalendarMenu() {
  while (true) {
    Serial.println("\n========== CALENDAR MENU ==========");
    Serial.println("1. Add Event");
    Serial.println("2. View Events");
    Serial.println("3. Delete Event");
    Serial.println("4. Edit Event");
    Serial.println("5. Exit");
    Serial.println("===================================");
    Serial.print("Enter option (1-5): ");
    
    while (!Serial.available()) delay(10);
    char option = Serial.read();
    Serial.println(option);
    
    while (Serial.available()) Serial.read();
    
    switch (option) {
      case '1':
        serialAddEvent();
        break;
      case '2':
        serialViewEvents();
        break;
      case '3':
        serialDeleteEvent();
        break;
      case '4':
        serialEditEvent();
        break;
      case '5':
        Serial.println("\nExiting calendar menu...");
        return;
      default:
        Serial.println("✗ Invalid option");
    }
    
    delay(500);
  }
}

void serialAddEvent() {
  if (eventCount >= MAX_CALENDAR_EVENTS) {
    Serial.println("\n✗ Max events reached!");
    return;
  }
  
  CalendarEvent newEvent;
  
  Serial.println("\n--- Add New Event ---");
  
  Serial.print("Enter year (2024-2100): ");
  while (!Serial.available()) delay(10);
  newEvent.year = Serial.parseInt();
  
  Serial.print("Enter month (1-12): ");
  while (!Serial.available()) delay(10);
  newEvent.month = Serial.parseInt();
  
  Serial.print("Enter day (1-31): ");
  while (!Serial.available()) delay(10);
  newEvent.day = Serial.parseInt();
  
  Serial.print("Enter hour (0-23): ");
  while (!Serial.available()) delay(10);
  newEvent.hour = Serial.parseInt();
  
  Serial.print("Enter minute (0-59): ");
  while (!Serial.available()) delay(10);
  newEvent.minute = Serial.parseInt();
  
  Serial.print("Enter message: ");
  while (!Serial.available()) delay(10);
  String message = Serial.readStringUntil('\n');
  message.trim();
  strncpy(newEvent.message, message.c_str(), MAX_EVENT_MESSAGE - 1);
  newEvent.message[MAX_EVENT_MESSAGE - 1] = '\0';
  
  newEvent.used = true;
  newEvent.alarmed = false;
  
  events[eventCount] = newEvent;
  eventCount++;
  saveCalendarToNVS();
  
  Serial.println("\n✓ Event added!");
  Serial.printf("  %04d-%02d-%02d %02d:%02d\n", newEvent.year, newEvent.month, newEvent.day, newEvent.hour, newEvent.minute);
  Serial.println(newEvent.message);
}

void serialViewEvents() {
  Serial.println("\n--- Events ---");
  
  if (eventCount == 0) {
    Serial.println("No events");
    return;
  }
  
  for (int i = 0; i < eventCount; i++) {
    if (events[i].used) {
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.printf("%04d-%02d-%02d %02d:%02d - ", events[i].year, events[i].month, events[i].day, events[i].hour, events[i].minute);
      Serial.println(events[i].message);
    }
  }
}

void serialDeleteEvent() {
  if (eventCount == 0) {
    Serial.println("\n✗ No events to delete!");
    return;
  }
  
  Serial.println("\n--- Delete Event ---");
  serialViewEvents();
  
  Serial.print("\nEnter event number (1-");
  Serial.print(eventCount);
  Serial.print("): ");
  while (!Serial.available()) delay(10);
  int num = Serial.parseInt();
  Serial.println(num);
  
  if (num < 1 || num > eventCount) {
    Serial.println("✗ Invalid number!");
    return;
  }
  
  int idx = num - 1;
  Serial.print("Delete '");
  Serial.print(events[idx].message);
  Serial.print("'? (y/n): ");
  while (!Serial.available()) delay(10);
  char response = Serial.read();
  Serial.println(response);
  
  if (response == 'y' || response == 'Y') {
    for (int i = idx; i < eventCount - 1; i++) {
      events[i] = events[i + 1];
    }
    eventCount--;
    saveCalendarToNVS();
    Serial.println("✓ Event deleted!");
  } else {
    Serial.println("Cancelled");
  }
}

void serialEditEvent() {
  if (eventCount == 0) {
    Serial.println("\n✗ No events to edit!");
    return;
  }
  
  Serial.println("\n--- Edit Event ---");
  serialViewEvents();
  
  Serial.print("\nEnter event number (1-");
  Serial.print(eventCount);
  Serial.print("): ");
  while (!Serial.available()) delay(10);
  int num = Serial.parseInt();
  Serial.println(num);
  
  if (num < 1 || num > eventCount) {
    Serial.println("✗ Invalid number!");
    return;
  }
  
  int idx = num - 1;
  
  Serial.print("New message (leave blank to keep): ");
  while (!Serial.available()) delay(10);
  String newMsg = Serial.readStringUntil('\n');
  newMsg.trim();
  
  if (newMsg.length() > 0) {
    strncpy(events[idx].message, newMsg.c_str(), MAX_EVENT_MESSAGE - 1);
    events[idx].message[MAX_EVENT_MESSAGE - 1] = '\0';
  }
  
  saveCalendarToNVS();
  Serial.println("✓ Event updated!");
}
