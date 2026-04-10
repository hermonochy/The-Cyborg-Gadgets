// Watch 3.3: 64x32 OLED display watch
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

#define totalFunctions 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool button_is_pressed(const byte btn, bool onlyOnce = false);

unsigned long lastActivityTime = 0;
unsigned long inactivityPeriod = 60000;

int selectedFunction = 1;

const char *Functions[] = {"Outputs", "Maths", "Random", "Score", "Dino Game", "Metronome", "Settings", "Sleep"};

const char *settingFuncs[] = {"Inactivity", "Func1", "Func2", "Func3", "Display"};

int blinkTime1 = 1000;
int blinkTime2 = 2;
int blinkTime3 = 100;

const byte btn1 = 2;
const byte btn2 = 4;
const byte btn3 = 5;
const byte btn4 = 3;
const byte btnInterrupt1 = btn1;
const byte btnInterrupt2 = btn4;

byte Func1 = 8;
byte Func2 = 7;
byte Func3 = 6;

volatile bool wakeup = false;
int rotation = 0;

void setup(){
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  randomSeed(analogRead(1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    while (true){
      digitalWrite(LED_BUILTIN, HIGH); delay(50);
      digitalWrite(LED_BUILTIN, LOW); delay(200);
    }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 5);
  display.print("Watch 3");
  display.setCursor(0, 20);
  display.print("Gen 3");
  display.display();

  delay(100);

  for (int i = 0; i < 100; i++){
    delay(20);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
    if (button_is_pressed(btn1)){
      digitalWrite(Func1, HIGH);
    } 
    else if (button_is_pressed(btn2)){
      digitalWrite(Func2, HIGH);
    }
    else if (button_is_pressed(btn3, true)){
      digitalWrite(LED_BUILTIN, LOW);
      calculator();
      break;
    }
    if (button_is_pressed(btn4, true)){
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
  }
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
    attachInterrupt(digitalPinToInterrupt(btnInterrupt1), wakeUp, FALLING);
    attachInterrupt(digitalPinToInterrupt(btnInterrupt2), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    
    detachInterrupt(digitalPinToInterrupt(btnInterrupt1));
    detachInterrupt(digitalPinToInterrupt(btnInterrupt2));

    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);
}

bool button_is_pressed(const byte btn, bool onlyOnce = false) {
  unsigned long now = millis();

  if (now - lastActivityTime > inactivityPeriod){
    goToSleep();
    lastActivityTime = millis();
    while (digitalRead(btnInterrupt1) == LOW){}
    while (digitalRead(btnInterrupt2) == LOW){}
    return false;
  }
  if (digitalRead(btn) == LOW){
    if (onlyOnce){
      while (digitalRead(btn) == LOW){}
    }
    lastActivityTime = millis();
    return true;
  }
  return false;
}

void activateFunc(const byte func, int blinkTime){
  bool blink = false;
  bool keepOn = false;

  while (true){
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      lastActivityTime = millis();
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    } 
    else{
      if (button_is_pressed(btn1)) digitalWrite(func, HIGH);
      else digitalWrite(func, LOW);
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
      return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("1. Flash");
    display.setCursor(0, 8);
    display.println("2. On");
    display.setCursor(0, 16);
    display.println("3. Blink");
    display.setCursor(0, 24);
    display.println("4. Return");
    
    display.setCursor(52, 0);
    display.print(digitalRead(func));
    display.display();
    delay(250);
  }
}

void watchFuncs(void){
  delay(50);
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("1. White");
    display.setCursor(0, 8);
    display.println("2. Laser");
    display.setCursor(0, 16);
    display.println("3. UV");
    display.display();

    delay(100);

    if (button_is_pressed(btn1, true)) activateFunc(Func1, blinkTime1);
    else if (button_is_pressed(btn2, true)) activateFunc(Func2, blinkTime2); 
    else if (button_is_pressed(btn3, true)) activateFunc(Func3, blinkTime3);
    else if (button_is_pressed(btn4, true)) return;
  }
}

void calculator(void){
    const char buttons[] = "1234567890.+-*/^%";
    const byte numButtons = 17;
    byte selected = 0;
    char expr[20] = "";
    byte exprLen = 0;
    bool showResult = false;
    double result = 0;
    bool error = false;
    byte displayMode = 0;

    while (true){
        display.clearDisplay();
        display.setTextSize(1);

        if (displayMode == 0) {
            display.setCursor(0, 0);
            if (showResult) {
                if (error) {
                    display.print("Err");
                } else {
                    display.print(result);
                }
            } else {
                display.print("Expr:");
                display.print(expr);
            }
            
            display.display();

            if (button_is_pressed(btn1, true)) {
                displayMode = 1;
                delay(100);
            }
            else if (button_is_pressed(btn2, true)) {
              expr[exprLen-1] = '\0';
              exprLen = strlen(expr);
              showResult = false;
              showResult = false;

                delay(100);
            }
            else if (button_is_pressed(btn3, true)) {
                if (exprLen > 0) {
                    error = false;
                    result = evaluateExpression(expr, error);
                    showResult = true;
                }
                delay(100);
            }
            else if (button_is_pressed(btn4, true)) {
                return;
            }
        }
        else if (displayMode == 1) {
            byte pageStart = (selected / 3) * 3;
            
            display.setCursor(0, 0);
            display.print(expr);
            for (byte i = 0; i < 3 && pageStart + i < numButtons; i++) {
                byte buttonIndex = pageStart + i;
                byte xPos = i * 20 + 2;
                
                if (buttonIndex == selected) {
                    display.setTextColor(SSD1306_BLACK);
                    display.fillRect(xPos, 11, 19, 8, SSD1306_WHITE);
                    display.setCursor(xPos + 6, 12);
                    display.print(buttons[buttonIndex]);
                    display.setTextColor(SSD1306_WHITE);
                } else {
                    display.setCursor(xPos + 6, 12);
                    display.print(buttons[buttonIndex]);
                }
            }
            display.display();

            if (button_is_pressed(btn1, true)) {
                if (selected == 0) {
                    selected = numButtons - 1;
                } else {
                    selected--;
                }
                delay(100);
            }
            else if (button_is_pressed(btn2, true)) {
                selected = (selected + 1) % numButtons;
                delay(100);
            }
            else if (button_is_pressed(btn3, true)) {
                if (exprLen < 19) {
                    expr[exprLen++] = buttons[selected];
                    expr[exprLen] = '\0';
                }
                delay(100);
            }
            else if (button_is_pressed(btn4, true)) {
                displayMode = 0;
                delay(100);
            }
        }
        delay(50);
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

void dinoRunner() {
  byte dinoY = 20, velocity = 0;
  int gravity = 1;
  int jumpPower = 5;
  byte cactusX = 64, cactusY = 20;
  const byte cactusW = 2, cactusH = 8;
  bool jumping = false, gameOver = false;
  unsigned long lastMove = millis(), lastCactusMove = millis();
  bool paused = false;
  byte score = 0;
  byte nextSpawnDist;

  while (true) {
    if (button_is_pressed(btn4)) return;

    if (button_is_pressed(btn3, true)) {
      paused = !paused;
    }
    if (paused) {
      delay(10);
      continue;
    }

    if (!gameOver && !jumping && (button_is_pressed(btn1) || button_is_pressed(btn2))) {
      velocity = -jumpPower; 
      jumping = true;
    }

    if (!gameOver && millis() - lastMove > 40) {
      if (jumping) {
        dinoY += velocity;
        velocity += gravity;
        if (dinoY >= 20) {
          dinoY = 20;
          jumping = false;
          velocity = 0; 
        }
      }
      lastMove = millis();
    }

    if (!gameOver && cactusX < 12 && cactusX + cactusW > 8 && dinoY + 8 > cactusY) {
      gameOver = true;
    }

    if (!gameOver && millis() - lastCactusMove > 24) {
      cactusX -= 2 + (score / 5);
      if (cactusX < -cactusW) { 
        nextSpawnDist = random(50);
        cactusX = 50 + nextSpawnDist;
        score++;
      }
      lastCactusMove = millis();
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(55, 0);
    display.print(score);

    display.drawLine(0, 28, 64, 28, SSD1306_WHITE);

    display.fillRect(8, dinoY, 5, 8, SSD1306_WHITE);

    display.drawLine(cactusX, cactusY, cactusX + cactusW, cactusY + cactusH, SSD1306_WHITE);
    display.drawLine(cactusX + cactusW, cactusY + cactusH, cactusX - cactusW, cactusY + cactusH, SSD1306_WHITE);
    display.drawLine(cactusX - cactusW, cactusY + cactusH, cactusX, cactusY, SSD1306_WHITE);

    if (gameOver) {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.print("Game Over!");
      display.setCursor(5, 20);
      display.print("Score: ");
      display.print(score);
      display.display();
      
      if (button_is_pressed(btn3, true)) {
        return; 
      }
      delay(100);
      continue;
    }
    display.display();
    delay(10);
  }
}

int bpm = 100;
void metronome(void){
  const int MIN_BPM = 1;
  const unsigned long PULSE_MS = 10;
  unsigned long lastBeat = millis();
  unsigned long ledOffAt = 0;
  volatile byte Func = Func1;
  
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

  digitalWrite(Func1, LOW);

  while (true){
    now = millis();
    lastActivityTime = millis();
    static int lastShownBpm = -1;
    if (bpm != lastShownBpm){
      lastShownBpm = bpm;
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Metronome");
      display.setCursor(0,16);
      display.print(bpm);
      display.print(" BPM");
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
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Random");
    display.setCursor(0, 10);
    display.print("0-");
    display.print(range);
    display.display();
    delay(100);
    if (button_is_pressed(btn1)){
      range += 10;
      delay(100);
    } 
    else if (button_is_pressed(btn2)){
      range = max(1, range - 1);
      delay(100);
    } 
    else if (button_is_pressed(btn3)){
      int randNumber = random(0, range + 1);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 8);
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
  int score1 = 0, score2 = 0;
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print(score1);
    display.print(" : ");
    display.print(score2);
    display.display();

    if (button_is_pressed(btn1)){
      score1++;
      delay(150);
    }
    else if (button_is_pressed(btn2)){
      score2++;
      delay(150);
    }
    else if (button_is_pressed(btn4)){
      delay(1000);
      return;
    }
    delay(50);
  }
}

void settings() {
  const byte numSettings = 5;
  byte settingIndex = 0;
  while(!button_is_pressed(btn4, true)){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(settingFuncs[settingIndex]);

    display.setCursor(0, 10);

    switch(settingIndex) {
      case 0:
        display.print(inactivityPeriod/1000);
        display.print("s");
        if(button_is_pressed(btn2)) {
          inactivityPeriod += 5000;
          if(inactivityPeriod > 600000) inactivityPeriod = 10000;
          delay(100);
        }
        if(button_is_pressed(btn1)) {
          inactivityPeriod -= 5000;
          if(inactivityPeriod < 10000) inactivityPeriod = 600000;
          delay(100);
        }
        break;
      case 1:
        display.print("Blk:");
        display.print(blinkTime1);
        display.setCursor(0, 20);
        display.print("Pin:");
        display.print(Func1);
        if(button_is_pressed(btn1)) {
          blinkTime1 += 50;
          if(blinkTime1 > 2000) blinkTime1 = 1;
          delay(100);
        }
        if(button_is_pressed(btn2, true)) {
          Func1++;
          if(Func1 > 13) Func1 = 1;
        }
        break;
      case 2:
        display.print("Blk:");
        display.print(blinkTime2);
        display.setCursor(0, 20);
        display.print("Pin:");
        display.print(Func2);
        if(button_is_pressed(btn1)) {
          blinkTime2 += 5;
          if(blinkTime2 > 2000) blinkTime2 = 1;
          delay(100);
        }
        if(button_is_pressed(btn2, true)) {
          Func2++;
          if(Func2 > 13) Func2 = 2;
        }
        break;
      case 3:
        display.print("Blk:");
        display.print(blinkTime3);
        display.setCursor(0, 20);
        display.print("Pin:");
        display.print(Func3);
        if(button_is_pressed(btn1)) {
          blinkTime3 += 5;
          if(blinkTime3 > 2000) blinkTime3 = 1;
          delay(100);
        }
        if(button_is_pressed(btn2, true)) {
          Func3++;
          if(Func3 > 13) Func3 = 3;
        }
        break;
      case 4:
        display.print("Rot:");
        display.print(rotation%4);
        if(button_is_pressed(btn1)) {
          rotation++;
          display.setRotation(rotation%4);
          delay(100);
        }
        if(button_is_pressed(btn2, true)) {
          display.ssd1306_command(SSD1306_DISPLAYOFF);
        }
        break;
    }
    display.display();
    
    if(button_is_pressed(btn3, true)) {
      settingIndex = (settingIndex + 1) % numSettings;
      delay(100);
    }
  }
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print(Functions[selectedFunction - 1]);
  display.display();

  delay(150);
  
  if (button_is_pressed(btn2)) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  } 
  else if (button_is_pressed(btn1)) {
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
  } 
  else if (button_is_pressed(btn4, true)){
    switch (selectedFunction){
      case 1:
        watchFuncs();
        break;
      case 2:
        calculator();
        break;
      case 3:
        randomInt();
        break;
      case 4:
        counter();
        break;
      case 5:
        dinoRunner();
        break;
      case 6:
        metronome();
        break;
      case 7:
        settings();
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
