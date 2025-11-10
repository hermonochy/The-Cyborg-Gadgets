// Watch 3.4: similar to watch 3.2 but adapted for 128x32 OLED

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

#define totalFunctions 8
#define totalParts 4

bool button_is_pressed(const byte btn, bool onlyOnce = false);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastActivityTime = 0;
unsigned long inactivityPeriod = 30000;

int selectedFunction = 1;
int selectedPart = 1;

const char *settingFuncs[] = {"Inactivity Period", "Func1 Settings", "Func1 Settings", "Func2 Settings", "Func3 Settings", "Func4 Settings"};

int blinkTime1 = 1000;
int blinkTime2 = 2;
int blinkTime3 = 100;
int blinkTime4 = 100;

const byte btn1 = 7;
const byte btn2 = 4;
const byte btn3 = 5;
const byte btn4 = 2; // needs to be pin2 for interrupts

byte Func1 = 9;
byte Func2 = 8;
byte Func3 = 10;
byte Func4 = 13;

volatile bool wakeup = false;

void setup(){
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  pinMode(Func4, OUTPUT);
  randomSeed(analogRead(1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    while (true){
      digitalWrite(LED_BUILTIN, HIGH); delay(50);
      digitalWrite(LED_BUILTIN, LOW); delay(200);
    }
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.print("Watch 4");
  display.setTextSize(1);
  display.setCursor(16, 20);
  display.print("of Generation 3");
  display.display();

  delay(100);

  for (int i = 0; i < 120; i++){
    delay(20);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    if (button_is_pressed(btn1)){
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
    if (button_is_pressed(btn2)){
      digitalWrite(Func2, HIGH);
    } 
    else if (button_is_pressed(btn3)){
      digitalWrite(LED_BUILTIN, LOW);
      calculator();
      break;
    }
  }
}

bool button_is_pressed(const byte btn, bool onlyOnce = false) {
  unsigned long now = millis();

  if (now - lastActivityTime > inactivityPeriod) {
    goToSleep();
    lastActivityTime = millis();
    // Ensure buttons do not have immediate effect
    return false;
  }
  if (digitalRead(btn) == LOW) {
    if (onlyOnce){
      while (digitalRead(btn) == LOW){}
    }
    lastActivityTime = millis();
    return true;
  }
  return false;
}

void(* reset)(void) = 0;

void wakeUp(){
    wakeup = true;
}

void goToSleep(){

    display.ssd1306_command(SSD1306_DISPLAYOFF);

    ADCSRA &= ~(1 << ADEN);
    sleep_bod_disable();

    wakeup = false;
    attachInterrupt(digitalPinToInterrupt(btn4), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn4));
 
    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);

}

void activateFunc(const byte func, int blinkTime){
  bool blink = false;
  bool keepOn = false;

  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 12);
    display.println("1. Flash");
    display.setCursor(60, 12);
    display.println("2. Keep On");
    display.setCursor(0, 22);
    display.println("3. Blink");
    display.setCursor(60, 22);
    display.println("4. Return");
    
    display.setCursor(0, 0);
    display.print(digitalRead(func));
    display.display();
    
    if (!blink) delay(30);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    } 
    else{
      if (button_is_pressed(btn1)) digitalWrite(func, HIGH);
      else digitalWrite(func, LOW);
    }

    if (button_is_pressed(btn2)){
      keepOn = !keepOn;
      if (keepOn) blink = false;
    } 
    else if (button_is_pressed(btn3)){
      blink = !blink;
      if (blink) keepOn = false;
    }
    else if (button_is_pressed(btn4)){
      return;
    delay(150);
    }
  }
}

void watchFuncs(void){
  delay(50);
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("1. White");
    display.setCursor(0, 8);
    display.print("2. Laser");
    display.setCursor(0, 16);
    display.print("3. UV");
    display.setCursor(0, 24);
    display.print("4. BuiltIn");
    display.setTextSize(4);
    display.setCursor(100, 0);
    display.print(selectedPart);
    display.display();

    delay(150);

    if (button_is_pressed(btn2)){
      selectedPart++;
      if (selectedPart > totalParts) selectedPart = 1;
    } 
    else if (button_is_pressed(btn1)){
      selectedPart--;
      if (selectedPart < 1) selectedPart = totalParts;
    } 
    else if (button_is_pressed(btn4)) return;
    else if (button_is_pressed(btn3)){
      switch (selectedPart){
        case 1:
          activateFunc(Func1, blinkTime1);
          continue;
        case 2:
          activateFunc(Func2, blinkTime2);
          continue;
        case 3:
          activateFunc(Func3, blinkTime3);
          continue;
        case 4:
          activateFunc(LED_BUILTIN, blinkTime4);
          continue;
      }
    }
  }
}

void calculator(void){
  const char* options[]={"1","2","3","4","5","6","7","8","9","0",".","+","-","*","/","^","r","%","(",")"};
  const int numOptions = sizeof(options)/sizeof(options[0]);
  int currentOption = 0;
  char expr[25] = "";
  int exprLen = 0;
  bool showResult = false;
  double result = 0;
  bool error = false;

  while (true){
    display.clearDisplay();
    if (showResult){
      display.setTextSize(2);
      display.setCursor(0,0);
      if (error) display.print("Err");
      else display.print(result,6);
    } else {
      display.setTextSize(1);
      display.setCursor(0,0);
      display.print(expr);
      display.setCursor(exprLen*6,0);
      display.print(options[currentOption]);
    }
    display.display();

    if (button_is_pressed(btn1)){
      currentOption = (currentOption + 1) % numOptions;
    } else if (button_is_pressed(btn2)){
      if (showResult){
        expr[0]=0; exprLen=0; showResult=false; currentOption=0;
      } else {
        const char* ch = options[currentOption];
        int chlen = strlen(ch);
        if (exprLen + chlen < (int)sizeof(expr)-1){
          strcat(expr,ch);
          exprLen += chlen;
        }
      }
    } else if (button_is_pressed(btn3)){
      error = false;
      result = evaluateExpression(expr,error);
      showResult = true;
    } else if (button_is_pressed(btn4)){
      return;
    }
    yield();
  }
}

double parsePrimary(const char* &s, bool &error);

double parseNumber(const char* &s, bool &error){
    double value = 0.0;
    bool hasDecimal = false;
    double frac = 0.1;
    while (isdigit(*s) || *s == '.'){
        if (*s == '.'){
            if (hasDecimal){ error = true; return 0; }
            hasDecimal = true;
        } 
        else if (hasDecimal){
            value += (*s - '0') * frac;
            frac *= 0.1;
        } 
        else{
            value = value * 10 + (*s - '0');
        }
        s++;
    }
    return value;
}

double parseFactor(const char* &s, bool &error){
    while (*s == ' ') s++;
    if (*s == '+'){ s++; return parseFactor(s, error); }
    if (*s == '-'){ s++; return -parseFactor(s, error); }
    if (*s == 'r'){ s++; double val = parseFactor(s, error); return val<0?(error=true,0):sqrt(val); }
    if (*s == '('){
        s++;
        double val = parsePrimary(s, error);
        if (*s == ')') s++;
        else error = true;
        return val;
    }
    return parseNumber(s, error);
}

double parseExponent(const char* &s, bool &error){
    double left = parseFactor(s, error);
    while (*s == '^'){
        s++;
        double right = parseFactor(s, error);
        left = pow(left, right);
    }
    return left;
}

double parseTerm(const char* &s, bool &error){
    double left = parseExponent(s, error);
    while (*s == '*' || *s == '/' || *s == '%'){
        char op = *s++;
        double right = parseExponent(s, error);
        if (op == '*') left *= right;
        else if (op == '/'){ if (right == 0){ error = true; return 0; } left /= right; }
        else if (op == '%'){ if (right == 0){ error = true; return 0; } left = fmod(left, right); }
    }
    return left;
}

double parsePrimary(const char* &s, bool &error){
    double left = parseTerm(s, error);
    while (*s == '+' || *s == '-'){
        char op = *s++;
        double right = parseTerm(s, error);
        if (op == '+') left += right;
        else left -= right;
    }
    return left;
}

double evaluateExpression(const char* expr, bool &error){
    const char* s = expr;
    error = false;
    double result = parsePrimary(s, error);
    while (*s && !error){
        if (!isspace(*s)) error = true;
        s++;
    }
    return result;
}

void unitConverter(void){
    const char* types[] ={
        "cm->in", "in->cm",
        "C->F",   "F->C",
        "kg->lb", "lb->kg",
        "km->mi", "mi->km",
        "g->oz",  "oz->g",
        "L->gal", "gal->L"
    };
    enum{LEN=0, LEN2, TEMP, TEMP2, WT, WT2, KM_MI, MI_KM, G_OZ, OZ_G, L_GAL, GAL_L};
    const int numTypes = sizeof(types) / sizeof(types[0]);
    int selectedType = 0;
    float inputValue = 0;
    bool enteringValue = true;
    float result = 0;

    while (true){
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(types[selectedType]);

        display.setCursor(80, 0);
        display.print(inputValue, 1);

        display.setTextSize(2);
        display.setCursor(5, 16);
        display.print("= ");
        if (!enteringValue)
            display.print(result, 4);

        display.display();

        if (button_is_pressed(btn3)){
            selectedType = (selectedType + 1) % numTypes;
            enteringValue = true;
            delay(200);
        } 
        else if (button_is_pressed(btn2)){
            if (enteringValue) inputValue--;
            delay(150);
        } 
        else if (button_is_pressed(btn1)){
            if (enteringValue) inputValue++;
            delay(150);
        } 
        else if (button_is_pressed(btn4)){
            if (enteringValue){
                switch (selectedType){
                    case LEN:   result = inputValue / 2.54; break;
                    case LEN2:  result = inputValue * 2.54; break;
                    case TEMP:  result = inputValue * 9.0 / 5.0 + 32.0; break;
                    case TEMP2: result = (inputValue - 32.0) * 5.0 / 9.0; break;
                    case WT:    result = inputValue * 2.20462; break;
                    case WT2:   result = inputValue / 2.20462; break;
                    case KM_MI: result = inputValue * 0.621371; break;
                    case MI_KM: result = inputValue / 0.621371; break;
                    case G_OZ:  result = inputValue * 0.035274; break;
                    case OZ_G:  result = inputValue / 0.035274; break;
                    case L_GAL: result = inputValue * 0.264172; break;
                    case GAL_L: result = inputValue / 0.264172; break;
                }
                enteringValue = false;
                delay(300);
            } 
            else{
                return;
            }
        }
        delay(20);
    }
}

void randomInt(){
  int range = 1;
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Range: ");
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.print("0 - ");
    display.print(range);
    display.display();
    delay(150);
    if (button_is_pressed(btn1)){
      range += 10;
      delay(200);
    } 
    else if (button_is_pressed(btn2)){
      range--;
      delay(200);
    } 
    else if (button_is_pressed(btn3)){
      int randNumber = random(0, range + 1);
      display.clearDisplay();
      display.setTextSize(3);
      display.setCursor(20,8);
      display.print(randNumber);
      display.display();
      delay(1500);
    } 
    else if (button_is_pressed(btn4)){
      return;
    }
  }
}

void counter(void){
  int score1 = 0;
  int score2 = 0;

  while (true){
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Score:");

    display.setTextSize(2);
    display.setCursor(10, 12);
    display.print(score1);
    display.print(" : ");
    display.print(score2);
    display.display();

    if (button_is_pressed(btn1)){
      score1++;
      delay(200);
    }
    else if (button_is_pressed(btn2)){
      score2++;
      delay(200);
    }
    else if (button_is_pressed(btn4)){
      delay(500);
      return;
    }
    delay(50);
  }
}

void metronome(void){
  int bpm = 100;
  const int MIN_BPM = 10;
  const unsigned long PULSE_MS = 10;
  unsigned long lastBeat = millis();
  unsigned long ledOffAt = 0;

  
  const unsigned long HOLD_INITIAL_MS = 400; 
  const unsigned long HOLD_MIN_MS = 40;      
  const float HOLD_ACCEL_FACTOR = 0.75f;     

  unsigned long now = 0;
  unsigned long interval = 60000UL / (unsigned long)max(1, bpm);

  
  bool btn1Held = false;
  bool btn2Held = false;
  unsigned long btn1NextRepeat = 0;
  unsigned long btn2NextRepeat = 0;
  unsigned long btn1RepeatDelay = HOLD_INITIAL_MS;
  unsigned long btn2RepeatDelay = HOLD_INITIAL_MS;

  digitalWrite(Func1, LOW);

  while (true){
    now = millis();

    static int lastShownBpm = -1;
    if (bpm != lastShownBpm){
      lastShownBpm = bpm;
      display.clearDisplay();
      display.setTextSize(3);
      display.setCursor(0, 10);
      display.print(bpm);
      display.print("BPM");
      display.display();
      
      interval = 60000UL / (unsigned long)max(1, bpm);
    }

    
    if ((unsigned long)(now - lastBeat) >= interval){
      digitalWrite(Func1, HIGH);
      ledOffAt = now + PULSE_MS;
      lastBeat = now;
    }

    if (ledOffAt && now >= ledOffAt){
      digitalWrite(Func1, LOW);
      ledOffAt = 0;
    }
    
    if (button_is_pressed(btn1)){
      if (!btn1Held){
        
        btn1Held = true;
        btn1RepeatDelay = HOLD_INITIAL_MS;
        btn1NextRepeat = now + btn1RepeatDelay;
        if (bpm > MIN_BPM) bpm--;
      } else {
        
        if (now >= btn1NextRepeat){
          if (bpm > MIN_BPM) bpm--;
          
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn1RepeatDelay * HOLD_ACCEL_FACTOR);
          btn1RepeatDelay = newDelay;
          btn1NextRepeat = now + btn1RepeatDelay;
        }
      }
    } else {
      
      btn1Held = false;
    }

    if (button_is_pressed(btn2)){
      if (!btn2Held){
        
        btn2Held = true;
        btn2RepeatDelay = HOLD_INITIAL_MS;
        btn2NextRepeat = now + btn2RepeatDelay;
        bpm++;
      } else {
        
        if (now >= btn2NextRepeat){
          bpm++;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn2RepeatDelay * HOLD_ACCEL_FACTOR);
          btn2RepeatDelay = newDelay;
          btn2NextRepeat = now + btn2RepeatDelay;
        }
      }
    } else {
      
      btn2Held = false;
    }
    if (button_is_pressed(btn4)){
      digitalWrite(Func1, LOW);
      return;
    }  
    yield();
  }
}

void settings() {
  const byte numSettings = 5;
  int settingIndex = 0;
  while(!button_is_pressed(btn4, true)){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print(settingFuncs[settingIndex]);

    display.setCursor(0, 12);

    switch(settingIndex) {
      case 0:
        display.print(inactivityPeriod/1000);
        display.print(" s");
        if(button_is_pressed(btn2)) {
          inactivityPeriod += 5000;
          if(inactivityPeriod > 600000) inactivityPeriod = 10000;
        }
        if(button_is_pressed(btn1)) {
          inactivityPeriod -= 50000;
          if(inactivityPeriod < 10000) inactivityPeriod = 120000;
        }
        break;
      case 1:
        display.print(blinkTime1);
        display.setCursor(0, 24);
        display.print(Func2);
        if(button_is_pressed(btn1)) {
          blinkTime1 += 5;
          if(blinkTime1 > 5000) blinkTime1 = 1;
        }
        if(button_is_pressed(btn2)) {
          Func1++;
          if(Func1 >= 12) Func1 = 1;
        }
        break;
      case 2:
        display.print(blinkTime2);
        display.setCursor(0, 24);
        display.print(Func2);
        if(button_is_pressed(btn1)) {
          blinkTime2 += 5;
          if(blinkTime2 > 5000) blinkTime2 = 1;
        }
        if(button_is_pressed(btn2)) {
          Func2++;
          if(Func2 >= 12) Func2 = 2;
        }
        break;
      case 3:
        display.print(blinkTime3);
        display.setCursor(0, 24);
        display.print(Func3);
        if(button_is_pressed(btn1)) {
          blinkTime3 += 5;
          if(blinkTime3 > 5000) blinkTime3 = 1;
        }
        if(button_is_pressed(btn2)) {
          Func3++;
          if(Func3 >= 12) Func3 = 2;
        }
        break;
      case 4:
        display.print(blinkTime4);
        display.setCursor(0, 24);
        display.print(Func4);
        if(button_is_pressed(btn1)) {
          blinkTime4 += 5;
          if(blinkTime4 > 5000) blinkTime4 = 1;
        }
        if(button_is_pressed(btn2)) {
          Func4++;
          if(Func4 >= 12) Func4 = 2;
        }
        break;
    }
    
    display.display();
    
    if(button_is_pressed(btn3, true)) {
      settingIndex = (settingIndex + 1) % numSettings;
    }
  }
}

void loop(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Home");
  display.setCursor(120, 0);
  display.print(selectedFunction);
  display.setCursor(0, 14);
  display.print("1.Out 2Mth 3.Cnv 4Rnd");
  display.setCursor(0, 24);
  display.print("5.Cnt 6Blk 7.Set 8Pwr");

  display.display();

  delay(150);

  if (button_is_pressed(btn2)){
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  } 
  else if (button_is_pressed(btn1)){
    selectedFunction--;
    if (selectedFunction < 1){
      selectedFunction = totalFunctions;
    }
  } 
  else if (button_is_pressed(btn4)){
    while(button_is_pressed(btn4)){}
    switch (selectedFunction){
      case 1:
        watchFuncs();
        break;
      case 2:
        calculator();
        break;
      case 3:
        unitConverter();
        break;
      case 4:
        randomInt();
        break;
      case 5:
        counter();
        break;
      case 6:
        metronome();
        break;
      case 7:
        settings();
        break;
      case 8:
        while (button_is_pressed(btn4));
        goToSleep();
        break;
    }
  }
  else if (button_is_pressed(btn3)){
    reset();
  }
}
