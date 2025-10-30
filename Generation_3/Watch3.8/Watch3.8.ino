// Watch 3.8: Computation focused watch

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define totalFunctions 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool button_is_pressed(const byte btn, bool onlyOnce = false);

int selectedFunction = 1;

const byte btn1 = 2;
const byte btn2 = 4;
const byte btn3 = 5;
const byte btn4 = 6;
const byte btn5 = 7;
const byte btn6 = 3;

const byte Torch = A0;

volatile bool wakeup = false;

void setup(){
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(btn5, INPUT_PULLUP);
  pinMode(btn6, INPUT_PULLUP);
  pinMode(Torch, OUTPUT);
  
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
  display.setCursor(0,0);
  display.print("Welcome to");
  display.setCursor(20, 20);
  display.print("Watch 8");
  display.setCursor(30, 50);
  display.print("Gen 3");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();

  delay(100);

  for (int i = 0; i < 200; i++){
    delay(20);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
    if (button_is_pressed(btn1)){
      digitalWrite(Torch, HIGH);
    }
  }
}

bool button_is_pressed(const byte btn, bool onlyOnce = false) {
  if (digitalRead(btn) == LOW) {
    if (onlyOnce){
      while (digitalRead(btn) == LOW){}
    }
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
    attachInterrupt(digitalPinToInterrupt(btn6), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn6));
 
    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);

}

void torch(void){
  bool blink = false;
  bool keepOn = false;
  int blinkTime = 500;
  int power = 1023;

  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.print("1. Quick Flash");
    display.setCursor(0, 24);
    display.print("2. Always On");
    display.setCursor(0, 32);
    display.print("3. Blink");
    display.setCursor(0, 40);
    display.print("4. Decrease");
    display.setCursor(0, 48);
    display.print("5. Increase");
    display.setCursor(0, 56);
    display.print("6. Return");
    
    display.setTextSize(2);
    display.setCursor(100, 0);
    display.print(" S: ");
    display.print(digitalRead(Torch));
    display.print(" P: ");
    display.print(power);
    display.print(" B: ");
    display.print(blinkTime);
    display.display();
    
    if (!blink) delay(50);
    if (keepOn){
      analogWrite(Torch, HIGH);
    } 
    else if (blink){
      analogWrite(Torch, !digitalRead(Torch));
      delay(blinkTime / 2);
    } 
    else{
      if (button_is_pressed(btn1)) digitalWrite(Torch, HIGH);
      else analogWrite(Torch, LOW);
    }

    if (button_is_pressed(btn2, true)){
      keepOn = !keepOn;
      if (keepOn) blink = false;
    } 
    else if (button_is_pressed(btn3, true)){
      blink = !blink;
      if (blink) keepOn = false;
    }
    else if (button_is_pressed(btn4)){
      if (blink) blinkTime--;
      else power--;
    }
    else if (button_is_pressed(btn5)){
      if (blink) blinkTime++;
      else power++;
    }
    else if (button_is_pressed(btn6)){
      return;
    delay(250);
    }
  }
}

void calculator(void){
    const char* options[] ={"1","2","3","4","5","6","7","8","9","0",".","+","-","*","/","^","r","%","(",")"};
    const int numOptions = sizeof(options)/sizeof(options[0]);
    int currentOption = 0;
    char expr[25] = "";
    int exprLen = 0;
    bool showResult = false;
    double result = 0;
    bool error = false;

    while (true){
        display.clearDisplay();

        display.setTextSize(2);
        display.setCursor(0, 0);
        if (showResult){
            if (error){
                display.print("Error!");
            } 
            else{
                display.print(result, 6);
            }
        }

        display.setTextSize(1);
        int startX = 0;
        int yOptions = 20;
        int spacing = 16;
        for (int i = 0; i < numOptions; i++){
            int x = startX + (i % 7) * spacing;
            int y = yOptions + (i / 7) * 10;
            if (i == currentOption){
                display.fillRect(x-1, y-1, 12, 10, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
                display.setCursor(x, y);
                display.print(options[i]);
                display.setTextColor(SSD1306_WHITE);
            } 
            else{
                display.setCursor(x, y);
                display.print(options[i]);
            }
        }

        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print(expr);

        display.display();

        if (button_is_pressed(btn1)){
            currentOption = (currentOption + 1) % numOptions;
            delay(150);
        }
        else if (button_is_pressed(btn2, true)){
            if (showResult){
                expr[0] = '\0';
                exprLen = 0;
                showResult = false;
            }
            if (exprLen < 24){
                strcat(expr, options[currentOption]);
                exprLen = strlen(expr);
            }
            delay(250);
        }
        else if (button_is_pressed(btn3)){
            
        }
        else if (button_is_pressed(btn4, true)){
          if (!showResult){
            error = false;
            result = evaluateExpression(expr, error);
            showResult = true;
            delay(550);
          }
          else return;
        }
        delay(100);
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
        display.setCursor(0, 25);
        display.print("Type: ");
        display.print(types[selectedType]);

        display.setCursor(0, 50);
        display.print("Input: ");
        display.setTextSize(2);
        display.setCursor(50, 45);
        display.print(inputValue, 2);

        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("= ");
        if (!enteringValue)
            display.print(result, 4);

        display.display();

        if (button_is_pressed(btn1)){
            selectedType = (selectedType + 1) % numTypes;
            enteringValue = true;
            delay(250);
        } 
        else if (button_is_pressed(btn3)){
            if (enteringValue) inputValue--;
            delay(150);
        } 
        else if (button_is_pressed(btn2)){
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
                delay(500);
            } 
            else{
                return;
            }
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
  volatile byte Func = Torch;
  
  const unsigned long HOLD_INITIAL_MS = 400; 
  const unsigned long HOLD_MIN_MS = 1;      
  const float HOLD_ACCEL_FACTOR = 0.75f;     

  unsigned long now = 0;
  unsigned long interval = 60000UL / (unsigned long)max(1, bpm);

  
  bool btn1Held = false;
  bool btn2Held = false;
  unsigned long btn1NextRepeat = 0;
  unsigned long btn2NextRepeat = 0;
  unsigned long btn1RepeatDelay = HOLD_INITIAL_MS;
  unsigned long btn2RepeatDelay = HOLD_INITIAL_MS;

  digitalWrite(Torch, LOW);

  while (true){
    now = millis();

    static int lastShownBpm = -1;
    if (bpm != lastShownBpm){
      lastShownBpm = bpm;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.print("Metronome");
      display.setCursor(0,30);
      display.setTextSize(3);
      display.print(bpm);
      display.print("BPM");
      display.display();
      
      interval = 60000UL / (unsigned long)max(1, bpm);
    }
    
    if ((unsigned long)(now - lastBeat) >= interval){
      digitalWrite(Func, HIGH);
      ledOffAt = now + PULSE_MS;
      lastBeat = now;
    }

    if (ledOffAt && now >= ledOffAt){
      digitalWrite(Func, LOW);
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
      digitalWrite(Func, LOW);
      return;
    }  
    yield();
  }
}

void randomInt(){
  int range = 1;
  while (true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Random Int");
    display.setCursor(30, 25);
    display.print("Range:");
    display.setCursor(30, 45);
    display.print(("0 - "));
    display.print(range);
    display.display();
    delay(100);
    if (button_is_pressed(btn1)){
      range += 10;
      delay(250);
    } 
    else if (button_is_pressed(btn2)){
      range--;
      delay(250);
    } 
    else if (button_is_pressed(btn3)){
      int randNumber = random(0, range + 1);
      display.clearDisplay();
      display.setTextSize(4);
      display.setCursor(10, 25);
      display.print(randNumber);
      display.display();
      delay(2000);
    } 
    else if (button_is_pressed(btn4, true)){
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
    display.setCursor(10, 5);
    display.print("Score Counter");

    display.setTextSize(3);
    display.setCursor(0, 30);
    display.print(score1);
    display.print(" : ");
    display.print(score2);
    display.display();

    if (button_is_pressed(btn1)){
      ++score1;
      delay(150);
    }
    else if (button_is_pressed(btn2)){
      ++score2;
      delay(150);
    }
    else if (button_is_pressed(btn4)){
      delay(1000);
      return;
    }
    delay(50);
  }
}

void loop(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Home     ");
  display.print(selectedFunction);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("1. Torch   5. Counter");
  display.setCursor(0, 32);
  display.print("2. Maths   6. Metron");
  display.setCursor(0, 44);
  display.print("3. Convert 7. -");
  display.setCursor(0, 56);
  display.print("4. Random  8. Power");

  display.display();

  delay(50);

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
  else if (button_is_pressed(btn4, true)){
    switch (selectedFunction){
      case 1:
        torch();
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
      case 8:
        goToSleep();
        break;
    }
  }
  else if (button_is_pressed(btn3)){
    reset();
  }
}
