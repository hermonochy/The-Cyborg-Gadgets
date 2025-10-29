// Watch 3.3: Uses a DS1302 chip to become an actual watch, rather than a mere extension.
// Requires 3D printed parts.

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Ds1302.h>

#define PIN_ENA 8
#define PIN_CLK 12
#define PIN_DAT 11

Ds1302 rtc(PIN_ENA, PIN_CLK, PIN_DAT);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte btn1 = 4;
const byte btn2 = 5;
const byte btn3 = 3;
const byte btn4 = 6;
const byte btn5 = 7;

const byte Func1 = A2;
const byte Func2 = 10;

unsigned long lastActivityTime = 0;
const unsigned long inactivityPeriod = 60000;

volatile bool wakeup = false;

enum WatchFunction {
  FUNC_CLOCK,
  FUNC_TIMER,
  FUNC_STOPWATCH,
  FUNC_ALARM,
  FUNC_METRONOME,
  FUNC_CALCULATOR,
  FUNC1,
  FUNC2,
  FUNC_BUILTINLED,
  POWER_CONTROL,
  NUM_FUNCTIONS
};

int currentFunction = FUNC_CLOCK;

struct FuncState{
  bool blink = false;
  bool keepOn = false;
  int blinkTime = 500;
  unsigned long lastBlink = 0;
  bool outputState = false;
};

FuncState funcStates[NUM_FUNCTIONS];

bool button_is_pressed(const byte btn){
    unsigned long now = millis();

    if (now - lastActivityTime > inactivityPeriod){
        display.clearDisplay();
        display.display();
        goToSleep();
        lastActivityTime = millis();
        // Ensure interrupt does not trigger immediately
        while (button_is_pressed(btn3)) delay(10);
        return false;
    }

    if (digitalRead(btn) == LOW) {
        delay(50);
        lastActivityTime = millis();
        return true;
    }
    return false;
}

void clock() {
  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(5, 30);
  char buf[9];
  sprintf(buf, "%02d:%02d", now.hour, now.minute);
  display.print(buf);
  display.setTextSize(1);
  display.setCursor(105, 45);
  sprintf(buf, "%02d", now.second);
  display.print(buf);



  display.setCursor(5, 5);

  const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  display.print(daysOfWeek[now.dow % 7]);

  sprintf(buf, "   %02d / %02d / %04d", now.day, now.month, 2000 + now.year);
  display.print(buf);


  display.display();
}

unsigned long timerSet = 60;
unsigned long timerLeft = 0;
bool timerRunning = false;
unsigned long timerLast = 0;

void timer(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  unsigned long t = timerRunning ? timerLeft : timerSet;
  byte m = t / 60;
  byte s = t % 60;
  char buf[6];
  sprintf(buf, "%02d:%02d", m, s);
  display.print("TMR ");
  display.print(buf);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print(timerRunning ? "Running" : "Set: Btn2/Btn3 +/-");
  display.setCursor(0, 40);
  display.print("Btn1: Start/Stop");

  display.display();

  if (!timerRunning) {
    if (button_is_pressed(btn2) && timerSet < 3600) timerSet += 10;
    if (button_is_pressed(btn3) && timerSet >= 10) timerSet -= 10;
    if (button_is_pressed(btn1)) {
      timerLeft = timerSet;
      timerRunning = true;
      timerLast = millis();
    }
  } else {
    if (millis() - timerLast >= 1000 && timerLeft > 0) {
      lastActivityTime = millis(); // ensure watch does not go to sleep during timer
      timerLeft--;
      timerLast += 1000;
    }
    if (timerLeft == 0) timerRunning = false;
    if (button_is_pressed(btn1)) timerRunning = false;
  }
}
/*

unsigned long stopwatchTime = 0;
bool stopwatchRunning = false;
unsigned long stopwatchLast = 0;

void stopWatch(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  unsigned long t = stopwatchTime;
  if (stopwatchRunning) {
    unsigned long now = millis();
    t += now - stopwatchLast;
  }
  byte m = (t / 1000) / 60;
  byte s = (t / 1000) % 60;
  char buf[9];
  sprintf(buf, "%02d:%02d.%01d", m, s, (int)((t % 1000) / 100));
  display.print("SW ");
  display.setCursor(0, 30);
  display.print(buf);

  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print("Btn1: Start/Stop  Btn2: Reset");

  display.display();

  if (button_is_pressed(btn1)) {
    if (!stopwatchRunning) {
      stopwatchLast = millis();
      stopwatchRunning = true;
    } else {
      stopwatchTime += millis() - stopwatchLast;
      stopwatchRunning = false;
    }
    delay(200);
  }
  if (button_is_pressed(btn2)) {
    stopwatchTime = 0;
    if (stopwatchRunning) stopwatchLast = millis();
    delay(200);
  }
  if (!stopwatchRunning && stopwatchTime > 0) stopwatchLast = millis();
}

byte alarmHour = 7, alarmMin = 0;
bool alarmSet = false, alarmActive = false;

void alarm() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  char buf[6];
  sprintf(buf, "%02d:%02d", alarmHour, alarmMin);
  display.print("AL ");
  display.print(buf);

  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print(alarmSet ? "Set" : "OFF");
  display.setCursor(0, 35);
  display.print("Btn1: Set ON/OFF");
  display.setCursor(0, 45);
  display.print("Btn2/3: Hour/Min");

  display.display();

  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  if (button_is_pressed(btn1)) {
    alarmSet = !alarmSet;
    delay(200);
  }
  if (button_is_pressed(btn2)) {
    alarmHour = (alarmHour + 1) % 24;
    delay(100);
  }
  if (button_is_pressed(btn3)) {
    alarmMin = (alarmMin + 1) % 60;
    delay(100);
  }

  if (alarmSet && now.hour == alarmHour && now.minute == alarmMin && now.second == 0 && !alarmActive) {
    alarmActive = true;
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_BUILTIN, HIGH); delay(100);
      digitalWrite(LED_BUILTIN, LOW); delay(100);
    }
  }
  if (now.minute != alarmMin) alarmActive = false;
}


int metronomeBPM = 120;
bool metronomeRunning = false;
Ds1302::DateTime lastBeatTime;

void metronome(){
  static bool showSetting = true;
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);

  if (showSetting) {
    display.print("BPM: ");
    display.print(metronomeBPM);
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.print("Btn2/3 +/-  Btn1:Start");
    display.setCursor(0, 50);
    display.print("Btn4: Menu");
    display.display();

    if (button_is_pressed(btn2)) {
      metronomeBPM += 1;
      if (metronomeBPM > 300) metronomeBPM = 300;
      delay(120);
    }
    if (button_is_pressed(btn3)) {
      metronomeBPM -= 1;
      if (metronomeBPM < 30) metronomeBPM = 30;
      delay(120);
    }
    if (button_is_pressed(btn1)) {
      showSetting = false;
      metronomeRunning = true;
      rtc.getDateTime(&lastBeatTime);
      ledState = true;
      digitalWrite(LED_BUILTIN, HIGH);
      lastBlink = millis();
      delay(200);
    }
    return;
  }

  display.print("Metronome");
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("BPM: ");
  display.print(metronomeBPM);
  display.setCursor(0, 40);
  display.print("Btn1: Stop Btn4: Menu");

  int msPerBeat = 60000 / metronomeBPM;

  rtc.getDateTime(&now);
  unsigned long nowMs = (now.hour * 3600UL + now.minute * 60UL + now.second) * 1000UL;
  nowMs += now.ms;
  unsigned long lastBeatMs = (lastBeatTime.hour * 3600UL + lastBeatTime.minute * 60UL + lastBeatTime.second) * 1000UL;
  lastBeatMs += lastBeatTime.ms; // if library supports ms

  if (nowMs - lastBeatMs >= (unsigned long)msPerBeat) {
    rtc.getDateTime(&lastBeatTime);
    ledState = true;
    digitalWrite(LED_BUILTIN, HIGH);
    lastBlink = millis();
  }

  if (ledState && millis() - lastBlink > 100) {
    ledState = false;
    digitalWrite(LED_BUILTIN, LOW);
  }

  display.display();

  if (button_is_pressed(btn1)) {
    metronomeRunning = false;
    showSetting = true;
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void calculator() {
  static char expr[25] = "";
  static int exprLen = 0;
  static bool showResult = false;
  static double result = 0;
  static bool error = false;
  const char *options[] = {"1","2","3","4","5","6","7","8","9","0",".","+","-","*","/","(",")"};
  const int numOptions = sizeof(options)/sizeof(options[0]);
  static int currentOption = 0;

  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  if (showResult) {
    if (error) display.print("Error!");
    else display.print(result, 6);
  }

  display.setTextSize(1);
  int startX = 0;
  int yOptions = 20;
  int spacing = 16;
  for (int i = 0; i < numOptions; i++) {
    int x = startX + (i % 7) * spacing;
    int y = yOptions + (i / 7) * 10;
    if (i == currentOption) {
      display.fillRect(x-1, y-1, 12, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(x, y);
      display.print(options[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(x, y);
      display.print(options[i]);
    }
  }
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(expr);

  display.display();

  if (button_is_pressed(btn2)) {
    currentOption = (currentOption + 1) % numOptions;
    delay(100);
  } else if (button_is_pressed(btn3)) {
    if (showResult) {
      expr[0] = '\0';
      exprLen = 0;
      showResult = false;
    }
    if (exprLen < 24) {
      strcat(expr, options[currentOption]);
      exprLen = strlen(expr);
    }
    delay(200);
  } else if (button_is_pressed(btn1)) {
    error = false;
    result = evaluateExpression(expr, error);
    showResult = true;
    delay(500);
  }
}

double parsePrimary(const char *&s, bool &error);

double parseNumber(const char *&s, bool &error) {
  double value = 0.0;
  bool hasDecimal = false;
  double frac = 0.1;
  while (isdigit(*s) || *s == '.') {
    if (*s == '.') {
      if (hasDecimal) { error = true; return 0; }
      hasDecimal = true;
    } else if (hasDecimal) {
      value += (*s - '0') * frac;
      frac *= 0.1;
    } else {
      value = value * 10 + (*s - '0');
    }
    s++;
  }
  return value;
}

double parseFactor(const char *&s, bool &error) {
  while (*s == ' ') s++;
  if (*s == '+') { s++; return parseFactor(s, error); }
  if (*s == '-') { s++; return -parseFactor(s, error); }
  if (*s == '(') {
    s++;
    double val = parsePrimary(s, error);
    if (*s == ')') s++;
    else error = true;
    return val;
  }
  return parseNumber(s, error);
}

double parseTerm(const char *&s, bool &error) {
  double left = parseFactor(s, error);
  while (*s == '*' || *s == '/') {
    char op = *s++;
    double right = parseFactor(s, error);
    if (op == '*') left *= right;
    else if (op == '/') {
      if (right == 0) { error = true; return 0; }
      left /= right;
    }
  }
  return left;
}

double parsePrimary(const char *&s, bool &error) {
  double left = parseTerm(s, error);
  while (*s == '+' || *s == '-') {
    char op = *s++;
    double right = parseTerm(s, error);
    if (op == '+') left += right;
    else left -= right;
  }
  return left;
}

double evaluateExpression(const char *expr, bool &error) {
  const char *s = expr;
  error = false;
  double result = parsePrimary(s, error);
  while (*s && !error) {
    if (!isspace(*s)) error = true;
    s++;
  }
  return result;
}
*/
void showFunc(const char *name, const byte func, int idx) {
  FuncState &state = funcStates[idx];
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print("1. Quick Flash");
  display.setCursor(0, 30);
  display.print("2. Always On");
  display.setCursor(0, 40);
  display.print("3. Blink");
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.print(name);
  display.display();

  if (state.blink) {
    lastActivityTime = millis(); // ensure watch does not go to sleep during blink
    unsigned long now = millis();
    if (now - state.lastBlink >= state.blinkTime) {
      state.outputState = !state.outputState;
      digitalWrite(func, state.outputState ? HIGH : LOW);
      state.lastBlink = now;
    }
  } else if (state.keepOn) {
    digitalWrite(func, HIGH);
  } else {
    if (digitalRead(btn1) == LOW) digitalWrite(func, HIGH);
    else digitalWrite(func, LOW);
  }

  if (button_is_pressed(btn2)) {
    state.keepOn = !state.keepOn;
    if (state.keepOn) state.blink = false;
    delay(200);
  } else if (button_is_pressed(btn3)) {
    state.blink = !state.blink;
    if (state.blink) {
      state.keepOn = false;
      state.lastBlink = millis();
      state.outputState = false;
      digitalWrite(func, LOW);
    }
    delay(200);
  }
}

void wakeUp(){
    wakeup = true;
}

void(* reset) (void) = 0;

void goToSleep(){
    display.ssd1306_command(SSD1306_DISPLAYOFF);

    ADCSRA &= ~(1 << ADEN);
    sleep_bod_disable();

    wakeup = false;
    attachInterrupt(digitalPinToInterrupt(btn3), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn3));

    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);

}

void power(){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.print("Power");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("1. Low Power");
  display.setCursor(0, 40);
  display.print("2. Reset");
  display.display();
  
  if (button_is_pressed(btn2)) reset();
  
  else if (button_is_pressed(btn1)){
    goToSleep();
  }
}

void setup() {
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(btn5, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);

  rtc.init();
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
  while (true){
    digitalWrite(LED_BUILTIN, HIGH); delay(50);
    digitalWrite(LED_BUILTIN, LOW); delay(200);
    }
  }
}

void loop() {
  if (button_is_pressed(btn4)) {
    currentFunction++;
    if (currentFunction >= NUM_FUNCTIONS) currentFunction = 0;
    delay(200);
  }

  else if (button_is_pressed(btn5)) {
    currentFunction--;
    if (currentFunction < 1) currentFunction = NUM_FUNCTIONS;
    delay(200);
  }

  switch (currentFunction) {
    case FUNC_CLOCK:
      clock();
      break;
    case FUNC_TIMER:
      timer();
      break;
    case FUNC_STOPWATCH:
      //stopWatch();
      break;
    case FUNC_ALARM:
      //alarm();
      break;
    case FUNC_CALCULATOR:
      //calculator();
      break;
    case FUNC_METRONOME:
      //metronome();
      break;
    case FUNC1:
      showFunc("White LED", Func1, FUNC1);
      break;
    case FUNC2:
      showFunc("Laser", Func2, FUNC2);
      break;
    case FUNC_BUILTINLED:
      showFunc("BuiltIn", LED_BUILTIN, FUNC_BUILTINLED);
      break;
    case POWER_CONTROL:
      power();
      break;
  }
  delay(50);
}
