#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <ctype.h>
#include <math.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define totalFunctions 8

#define btn1 2
#define btn2 4
#define btn3 5
#define btn4 6
#define btn5 7
#define btn6 3

#define Torch 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool button_is_pressed(const byte btn, bool onlyOnce = false);

int selectedFunction = 1;

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
  display.print("Watch 6");
  display.setCursor(30, 50);
  display.print("Gen 3");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();

  for (int i = 0; i < 200; i++){
    delay(10);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

bool button_is_pressed(const byte btn, bool onlyOnce) {
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
    attachInterrupt(digitalPinToInterrupt(btn1), wakeUp, FALLING);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn1));
    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);
}

void torch(){
  bool blink = false;
  bool keepOn = false;
  int blinkTime = 500;
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
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(digitalRead(Torch) ? "On" : "Off");
    display.print(" B:");
    display.print(blinkTime);
    display.display();

    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(torch, HIGH);
    } 
    else if (blink){
      digitalWrite(torch, !digitalRead(torch));
      delay(blinkTime);
    } 
    else{
      if (button_is_pressed(btn1)) digitalWrite(torch, HIGH);
      else digitalWrite(torch, LOW);
    }

    if (button_is_pressed(btn2, true)){
      keepOn = !keepOn;
      if (keepOn) blink = false;
    } 
    else if (button_is_pressed(btn3, true)){
      blink = !blink;
      if (blink) keepOn = false;
    }
    else if (button_is_pressed(btn4)) blinkTime--;
    else if (button_is_pressed(btn5)) blinkTime++;
    else if (button_is_pressed(btn6)){
      return;
    }
  }
}


void calculator(void){
    const char* options[] ={"1","2","3","4","5","6","7","8","9","0",".","+","-","*","/","^","%","(",")"};
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
        else if (button_is_pressed(btn2)){
          currentOption = (currentOption - 1) % numOptions;
          delay(150);
        }
        else if (button_is_pressed(btn3)){
            expr[exprLen-1] = '\0';
            exprLen = strlen(expr);
        }
        else if (button_is_pressed(btn4, true)){
          expr[0] = '\0';
          exprLen = 0;
        }
        else if (button_is_pressed(btn6, true)){
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

void randomNum(void){
  int range = 10;
  bool floatMode = false;
  int decimals = 2;
  while (true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Random");
    display.setCursor(0, 22);
    display.setTextSize(1);
    display.print("Max:");
    display.print(range);
    display.setCursor(64, 22);
    display.print(floatMode ? "Float" : "Int");
    display.setCursor(0, 44);
    display.print("Dec:");
    display.print(decimals);
    display.setCursor(0, 56);
    display.print("1+ 2- 3 gen 5 toggle 6 dec");
    display.display();
    delay(100);
    if (button_is_pressed(btn1)){
      range += 10;
      delay(250);
    } 
    else if (button_is_pressed(btn2)){
      range = max(1, range - 1);
      delay(250);
    } 
    else if (button_is_pressed(btn3)){
      if (!floatMode){
        long r = random(0, range + 1);
        display.clearDisplay();
        display.setTextSize(4);
        display.setCursor(10, 25);
        display.print(r);
        display.display();
        delay(2000);
      } else {
        double u = (random(0, 32767) / 32767.0);
        double val = u * range;
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(0, 18);
        display.print(val, decimals);
        display.display();
        delay(2000);
      }
    } 
    else if (button_is_pressed(btn5)){
      floatMode = !floatMode;
      delay(250);
    }
    else if (button_is_pressed(btn6)){
      decimals = min(6, decimals + 1);
      delay(200);
    }
    else if (button_is_pressed(btn4, true)){
      return;
    }
  }
}

const int EEPROM_MAGIC_ADDR = 0;
const uint32_t EEPROM_MAGIC = 0xCAFEBABE;
const int EEPROM_PLAYERS_ADDR = 4;
const int EEPROM_SCORES_ADDR = 8;
void saveScoresToEEPROM(int players, int scores[], int maxPlayers){
  EEPROM.put(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
  EEPROM.put(EEPROM_PLAYERS_ADDR, players);
  for (int i = 0; i < maxPlayers; ++i){
    EEPROM.put(EEPROM_SCORES_ADDR + i * sizeof(int), scores[i]);
  }
}
bool loadScoresFromEEPROM(int &players, int scores[], int maxPlayers){
  uint32_t m = 0;
  EEPROM.get(EEPROM_MAGIC_ADDR, m);
  if (m != EEPROM_MAGIC) return false;
  EEPROM.get(EEPROM_PLAYERS_ADDR, players);
  if (players < 1 || players > maxPlayers) return false;
  for (int i = 0; i < maxPlayers; ++i){
    EEPROM.get(EEPROM_SCORES_ADDR + i * sizeof(int), scores[i]);
  }
  return true;
}

void counter(void){
  const int MAX_PLAYERS = 8;
  int players = 2;
  int scores[MAX_PLAYERS];
  for (int i = 0; i < MAX_PLAYERS; ++i) scores[i] = 0;
  const int MAX_HISTORY = 256;
  int history[MAX_HISTORY];
  int head = 0;
  int count = 0;
  int current = 0;
  auto pushAction = [&](int player){
    history[head] = player;
    head = (head + 1) % MAX_HISTORY;
    if (count < MAX_HISTORY) count++;
  };
  auto undoAction = [&]()->bool{
    if (count == 0) return false;
    int idx = (head - 1 + MAX_HISTORY) % MAX_HISTORY;
    int last = history[idx];
    head = idx;
    count--;
    if (last >= 0 && last < MAX_PLAYERS){
      if (scores[last] > 0) scores[last]--;
      return true;
    }
    return false;
  };
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Counter");
    display.setCursor(0, 10);
    display.print("Players:");
    display.print(players);
    display.setTextSize(1);
    display.setCursor(0, 22);
    for (int i = 0; i < players; ++i){
      display.setCursor((i % 4) * 32, 30 + (i / 4) * 18);
      if (i == current) display.print(">");
      else display.print(" ");
      display.print(i+1);
      display.print(":");
      display.print(scores[i]);
    }
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("1 prev 2 next 3 + 5 undo 6 players 4 exit");
    display.setCursor(90, 56);
    display.print("H:");
    display.print(count);
    display.display();
    if (button_is_pressed(btn1)){
      current = (current - 1 + players) % players;
      delay(150);
    }
    else if (button_is_pressed(btn2)){
      current = (current + 1) % players;
      delay(150);
    }
    else if (button_is_pressed(btn3)){
      scores[current]++;
      pushAction(current);
      delay(150);
    }
    else if (button_is_pressed(btn5)){
      bool ok = undoAction();
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 10);
      if (ok){
        display.print("Undone");
        display.setTextSize(1);
        display.setCursor(0, 36);
        display.print("P");
        display.print(current+1);
        display.print(":");
        display.print(scores[current]);
      } else {
        display.print("No actions");
      }
      display.display();
      delay(300);
    }
    else if (button_is_pressed(btn6)){
      players = players % MAX_PLAYERS + 1;
      if (players < 2) players = 2;
      if (current >= players) current = players - 1;
      delay(250);
    }
    else if (button_is_pressed(btn4)){
      delay(500);
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
  else if (button_is_pressed(btn6, true)){
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
        randomNum();
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
