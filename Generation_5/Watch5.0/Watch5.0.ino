// Watch 5.0: Initial 5th gen watch with OLED - ESP32C3 with NVS.

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "tinyexpr.h"
#include <Wire.h>
#include <ctype.h>
#include <math.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define totalFunctions 9
#define MAX_NOTES 5
#define MAX_NOTE_LENGTH 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences preferences;

int selectedFunction = 1;

const char *Functions[] = {"Outputs", "Maths", "Random", "Convert", "Score", "Shooter", "Metronome", "Notes", "Debug"};

const byte buttonPin = 2;

// Button resistance values (Ordered by frequency used)
const int btn1 = 1433; // 4.7K
const int btn2 = 812;  // 2.2K
const int btn3 = 202;  // 470
const int btn4 = 409;  // 1K
const int btn5 = 95;   // 220
const int btn6 = 2304; // 10K

const byte Func1 = 1;
const byte Func2 = 0;
const byte Func3 = 3;

// Notes storage (in RAM for quick access)
struct Note {
  char text[MAX_NOTE_LENGTH];
  bool used;
};

Note notes[MAX_NOTES];

bool button_is_pressed(int btnVal, bool onlyOnce = true) {
  int pinVal = analogRead(buttonPin);
  int errorVal = pinVal - btnVal;
  int absErrorVal = abs(errorVal);
  
  if (absErrorVal <= 10) {    
      if (onlyOnce) {
        while (true) {
          delay(10);
          pinVal = analogRead(buttonPin);
          errorVal = pinVal - btnVal;
          absErrorVal = abs(errorVal);
          if (absErrorVal > 10) break;
        }
      }
      return true;
    }
  return false;
}

// ===== NVS FUNCTIONS =====
void saveNotesToNVS(void) {
  preferences.begin("notes", false); // false = read/write mode
  
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20];
    char keyUsed[20];
    
    sprintf(keyText, "note%d_text", i);
    sprintf(keyUsed, "note%d_used", i);
    
    preferences.putString(keyText, notes[i].text);
    preferences.putBool(keyUsed, notes[i].used);
  }
  
  preferences.end();
}

void loadNotesFromNVS(void) {
  preferences.begin("notes", true); // true = read-only mode
  
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20];
    char keyUsed[20];
    
    sprintf(keyText, "note%d_text", i);
    sprintf(keyUsed, "note%d_used", i);
    
    String noteText = preferences.getString(keyText, "");
    notes[i].used = preferences.getBool(keyUsed, false);
    
    strncpy(notes[i].text, noteText.c_str(), MAX_NOTE_LENGTH - 1);
    notes[i].text[MAX_NOTE_LENGTH - 1] = '\0';
  }
  
  preferences.end();
}

void initializeNotesNVS(void) {
  // Load notes from NVS on startup
  loadNotesFromNVS();
}

void activateFunc(byte func, int blinkTime = 500){
  bool blink = false;
  bool keepOn = false;

  while (true){
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    } 
    else {
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
    else if (button_is_pressed(btn6)){
      return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Quick Flash");
    display.setCursor(0, 30);
    display.println("2. Always On");
    display.setCursor(0, 40);
    display.println("3. Blink");
    display.setCursor(0, 50);
    display.println("6. Return");
    
    display.setTextSize(2);
    display.setCursor(5, 0);
    display.print(digitalRead(func) ? "On" : "Off");
    display.display();
  }
}

void watchFuncs(void) {

  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. White LED");
    display.setCursor(0, 30);
    display.println("2. Laser");
    display.setCursor(0, 40);
    display.println("3. UV LED");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) activateFunc(Func1);
    else if (button_is_pressed(btn2)) activateFunc(Func2);
    else if (button_is_pressed(btn3)) activateFunc(Func3);
    else if (button_is_pressed(btn6)) return;
  }
}

void calculator() {
  const char* screen1[] = {"1","2","3","4","5","6","7","8","9","0",".","pi"};
  const int screen1_size = sizeof(screen1) / sizeof(screen1[0]);
  const char* screen2[] = {"+","-","*","/","(",")","^","%","!","sin","cos","tan","asn","acs","atn"};
  const int screen2_size = sizeof(screen2) / sizeof(screen2[0]);
  
  bool onScreen1 = true;
  int selected = 0;
  char expr[64] = "";
  int exprLen = 0;
  bool showResult = false;
  double result = 0;
  bool error = false;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    if (showResult) {
      if (error) display.print("Error!");
      else {
        display.print("= ");
        display.print(result);
      }
    }
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print(expr);

    int y;
    int x;
    int spacing = 20;
    const char** activeScreen = onScreen1 ? screen1 : screen2;
    int activeSize = onScreen1 ? screen1_size : screen2_size;
    for (int i = 0; i < activeSize; i++) {
      x = (i % 6) * spacing;
      y = 20 + (i / 6) * 10;
      if (i == selected) {
        display.fillRect(x-1, y-1, spacing, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(x, y);
        display.print(activeScreen[i]);
        display.setTextColor(SSD1306_WHITE);
      } else {
        display.setCursor(x, y);
        display.print(activeScreen[i]);
      }
    }
    display.display();

    if (button_is_pressed(btn2)) {
      selected = (selected+1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn1)) {
      selected = (selected-1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn3)) {
      if (exprLen < 63) {
        if (onScreen1 && strcmp(activeScreen[selected], "pi") == 0) {
          strcat(expr, "pi");
          exprLen += 2;
        } else {
          strcat(expr, activeScreen[selected]);
          exprLen = strlen(expr);
        }
      }
      delay(120);
    }
    else if (button_is_pressed(btn4)) {
      int len = strlen(expr);
      if (len > 0) {
        if (len >= 2 && expr[len-2] == 'p' && expr[len-1] == 'i') {
          expr[len-2] = '\0';
        } else {
          expr[len-1] = '\0';
        }
        exprLen = strlen(expr);
      }
      delay(120);
    }
    else if (button_is_pressed(btn5)) {
      onScreen1 = !onScreen1;
      selected = 0;
      delay(120);
    }
    else if (button_is_pressed(btn6)) {
      if (showResult) return;
      int err;
      te_variable vars[] = {};
      te_expr* te = te_compile(expr, vars, 1, &err);
      if (te) {
        result = te_eval(te);
        te_free(te);
        showResult = true;
        error = false;
      } else {
        showResult = true;
        error = true;
      }
      delay(300);
    }
    delay(40);
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
            selectedType = (selectedType - 1) % numTypes;
            enteringValue = true;
            delay(250);
        } 
        else if (button_is_pressed(btn2)){
            selectedType = (selectedType + 1) % numTypes;
            enteringValue = true;
            delay(250);
        } 
        else if (button_is_pressed(btn4)){
            if (enteringValue) inputValue--;
            delay(150);
        } 
        else if (button_is_pressed(btn5)){
            if (enteringValue) inputValue++;
            delay(150);
        } 
        else if (button_is_pressed(btn6)){
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
            else return;
        }
        delay(50);
    }
}

void randomNum(void) {
  int range = 10;
  bool floatMode = false;
  int decimals = 2;
  while (true) {
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
    display.print("3:tog 4:d.p 5:gen");
    display.display();
    delay(100);
    
    if (button_is_pressed(btn1)) {
      range += 10;
      delay(250);
    } 
    else if (button_is_pressed(btn2)) {
      range = max(1, range - 1);
      delay(250);
    } 
    else if (button_is_pressed(btn5)) {
      if (!floatMode) {
        int r = random(0, range + 1);
        display.clearDisplay();
        display.setTextSize(3);
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
    else if (button_is_pressed(btn3)) {
      floatMode = !floatMode;
      delay(250);
    }
    else if (button_is_pressed(btn4)) {
      decimals = min(6, decimals + 1);
      delay(200);
    }
    else if (button_is_pressed(btn6, true)) {
      return;
    }
  }
}


int score1 = 0;
int score2 = 0;
void counter(void){
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
    else if (button_is_pressed(btn3)){
    score1 = 0;
    score2 = 0;  
    }
    
    else if (button_is_pressed(btn4)){
      --score1;
      delay(150);
    }
    else if (button_is_pressed(btn5)){
      --score2;
      delay(150);
    }
    else if (button_is_pressed(btn6)){
      return;
    }
    delay(50);
  }
}

int bpm = 100;
void metronome(void){
  const int MIN_BPM = 1;
  const unsigned long PULSE_MS = 10;
  unsigned long lastBeat = millis();
  unsigned long ledOffAt = 0;
  volatile byte Func = Func2;
  
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
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Metronome");
  display.setCursor(0,30);
  display.print(bpm);
  display.print("BPM");
  display.display();
  
  digitalWrite(Func, LOW);

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
    if (button_is_pressed(btn6)){
      digitalWrite(Func, LOW);
      return;
    }  
    yield();
  }
}

void shooterGame(void) {
  int playerX = 120;
  int playerY = 55;
  int score = 0;
  int health = 3;
  
  struct Bullet { int x; int y; bool active; };
  Bullet bullets[8];
  for (int i = 0; i < 8; i++) bullets[i].active = false;
  
  struct Enemy { int x; int y; bool active; };
  Enemy enemies[3];
  for (int i = 0; i < 3; i++) {
    enemies[i].x = (i * 40) + 10;
    enemies[i].y = 5;
    enemies[i].active = true;
  }

  unsigned long lastShot = 0;
  unsigned long lastSpawn = 0;

  while (health > 0) {
    unsigned long now = millis();

    if (button_is_pressed(btn1, false) && playerX > 0) {
      playerX -= 3;
    }
    if (button_is_pressed(btn2, false) && playerX < 120) {
      playerX += 3;
    }
    if (button_is_pressed(btn3, false) && now - lastShot > 200) {
      for (int i = 0; i < 8; i++) {
        if (!bullets[i].active) {
          bullets[i].x = playerX + 2;
          bullets[i].y = playerY - 2;
          bullets[i].active = true;
          lastShot = now;
          break;
        }
      }
    }
    if (button_is_pressed(btn6)) return;

    for (int i = 0; i < 8; i++) {
      if (bullets[i].active) {
        bullets[i].y -= 4;
        if (bullets[i].y < 0) bullets[i].active = false;
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        enemies[i].y += 1;
        if (enemies[i].y > 64) {
          enemies[i].active = false;
          health--;
        }
      }
    }

    if (now - lastSpawn > 2500) {
      for (int i = 0; i < 3; i++) {
        if (!enemies[i].active) {
          enemies[i].x = random(10, 118);
          enemies[i].y = 5;
          enemies[i].active = true;
          lastSpawn = now;
          break;
        }
      }
    }

    for (int b = 0; b < 8; b++) {
      if (bullets[b].active) {
        for (int e = 0; e < 3; e++) {
          if (enemies[e].active) {
            if (bullets[b].x >= enemies[e].x - 2 && bullets[b].x <= enemies[e].x + 6 &&
                bullets[b].y >= enemies[e].y - 2 && bullets[b].y <= enemies[e].y + 6) {
              bullets[b].active = false;
              enemies[e].active = false;
              score += 10;
            }
          }
        }
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        if (playerX >= enemies[i].x - 4 && playerX <= enemies[i].x + 6 &&
            playerY >= enemies[i].y - 4 && playerY <= enemies[i].y + 8) {
          enemies[i].active = false;
          health--;
        }
      }
    }

    display.clearDisplay();
    
    for (int dx = 0; dx < 4; dx++) {
      for (int dy = 0; dy < 6; dy++) {
        display.drawPixel(playerX + dx, playerY + dy, 1);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (bullets[i].active) {
        display.drawPixel(bullets[i].x, bullets[i].y, 1);
        display.drawPixel(bullets[i].x + 1, bullets[i].y, 1);
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        for (int dx = 0; dx < 4; dx++) {
          for (int dy = 0; dy < 4; dy++) {
            display.drawPixel(enemies[i].x + dx, enemies[i].y + dy, 1);
          }
        }
      }
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(score);
    display.setCursor(100, 0);
    display.print("H:");
    display.print(health);
    
    display.display();
    delay(30);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(3000);
}

void notesFunction(void) {
  int selectedNote = 0;
  bool viewingMode = true;

  while (true) {
    if (viewingMode) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.print("Notepad");   
      display.setTextSize(1);   
      display.setCursor(0, 18);
      for (int i = 0; i < MAX_NOTES; i++) {
        if (i == selectedNote) {
          display.print(">");
        } else {
          display.print(" ");
        }
        display.print(i + 1);
        display.print(": ");
        
        if (notes[i].used) {
          char preview[13];
          strncpy(preview, notes[i].text, 12);
          preview[12] = '\0';
          display.print(preview);
        } else {
          display.print("[Empty]");
        }
        display.setCursor(0, 18 + ((i + 1) * 10));
      }
     
      display.display();
      
      if (button_is_pressed(btn1)) {
        selectedNote = (selectedNote - 1 + MAX_NOTES) % MAX_NOTES;
        delay(150);
      }
      else if (button_is_pressed(btn2)) {
        selectedNote = (selectedNote + 1) % MAX_NOTES;
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        // Enter edit mode
        viewingMode = false;
        delay(200);
      }
      else if (button_is_pressed(btn6)) {
        saveNotesToNVS();
        return;
      }
    } else {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Note ");
      display.print(selectedNote + 1);
      display.print(" - Edit");
      
      display.setCursor(0, 20);
      display.print("Text:");
      display.setCursor(0, 35);
      display.print(notes[selectedNote].text);
      display.print("_");
      
      display.setCursor(0, 48);
      display.print("1:Del 2:Add 3:Char");
      display.setCursor(0, 56);
      display.print("4:Clr 5:Done 6:Back");
      display.display();
      
      if (button_is_pressed(btn1)) {
        int len = strlen(notes[selectedNote].text);
        if (len > 0) {
          notes[selectedNote].text[len - 1] = '\0';
          if (len - 1 == 0) {
            notes[selectedNote].used = false;
          }
        }
        delay(150);
      }
      else if (button_is_pressed(btn2)) {
        int len = strlen(notes[selectedNote].text);
        if (len < MAX_NOTE_LENGTH - 1) {
          notes[selectedNote].text[len] = ' ';
          notes[selectedNote].text[len + 1] = '\0';
          notes[selectedNote].used = true;
        }
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-:,!?";
        int charIndex = 0;
        bool selectingChar = true;
        
        while (selectingChar) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("Select Char:");
          display.setCursor(0, 20);
          display.setTextSize(2);
          display.print(charset[charIndex]);
          display.setTextSize(1);
          display.setCursor(0, 56);
          display.print("1: < 2: > 3: Select");
          display.display();
          
          if (button_is_pressed(btn1)) {
            charIndex = (charIndex - 1 + (int)strlen(charset)) % (int)strlen(charset);
            delay(100);
          }
          else if (button_is_pressed(btn2)) {
            charIndex = (charIndex + 1) % (int)strlen(charset);
            delay(100);
          }
          else if (button_is_pressed(btn3)) {
            int len = strlen(notes[selectedNote].text);
            if (len < MAX_NOTE_LENGTH - 1) {
              notes[selectedNote].text[len] = charset[charIndex];
              notes[selectedNote].text[len + 1] = '\0';
              notes[selectedNote].used = true;
            }
            selectingChar = false;
            delay(200);
          }
          else if (button_is_pressed(btn6)) {
            selectingChar = false;
            delay(200);
          }
          delay(50);
        }
      }
      else if (button_is_pressed(btn4)) {
        notes[selectedNote].text[0] = '\0';
        notes[selectedNote].used = false;
        delay(200);
      }
      else if (button_is_pressed(btn5)) {
        saveNotesToNVS();
        viewingMode = true;
        delay(200);
      }
      else if (button_is_pressed(btn6)) {
        saveNotesToNVS();
        viewingMode = true;
        delay(200);
      }
    }
    
    delay(50);
  }
}

void debug() {
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Debug Info");
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("ADC: ");
    display.print(analogRead(buttonPin));
    display.setCursor(0, 30);
    display.print("Func1: ");
    display.print(digitalRead(Func1));
    display.setCursor(0, 40);
    display.print("Func2: ");
    display.print(digitalRead(Func2));
    display.setCursor(0, 50);
    display.print("Func3: ");
    display.print(digitalRead(Func3));
    display.display();
    
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(100);
  }
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);

  randomSeed(analogRead(1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) {
      digitalWrite(Func1, HIGH); delay(50);
      digitalWrite(Func1, LOW); delay(200);
    }
  }

  // Initialize NVS-based notes
  initializeNotesNVS();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(7, 0);
  display.print("Welcome to");
  display.setCursor(20, 20);
  display.print("Watch 0");
  display.setCursor(30, 50);
  display.print("Gen 5");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();

  delay(2000);
}

void loop() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Watch 5.0");
  display.setCursor(0, 30);
  display.print(Functions[selectedFunction - 1]);
  display.display();

  delay(100);
  
  if (button_is_pressed(btn2)) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  } 
  else if (button_is_pressed(btn1)) {
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
  } 
  else if (button_is_pressed(btn6)) {
    switch (selectedFunction) {
      case 1:
        watchFuncs();
        break;
      case 2:
        calculator();
        break;
      case 3:
        randomNum();
        break;
      case 4:
        unitConverter();
        break;
      case 5:
        counter();
        break;
      case 6:
        shooterGame();
        break;
      case 7:
        metronome();
        break;
      case 8:
        notesFunction();
        break;
      case 9:
        debug();
        break;
    }
  }
}
