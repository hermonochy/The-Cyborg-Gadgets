// Watch 5.0: Initial OLED display watch

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "tinyexpr.h"
#include <Wire.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define totalFunctions 8
#define totalParts 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int selectedFunction = 1;

const char *Functions[] = {"Outputs", "Maths", "Random"};

const byte buttonPin = 2;

// Ordered so the most frequently used buttons have the highest resistance.
const byte btn1 = 1433; // 4.7K
const byte btn2 = 812;  // 2.2K
const byte btn3 = 202;  // 470
const byte btn4 = 409;  // 1K
const byte btn5 = 95;   // 220
const byte btn6 = 2304; // 10K

const byte Func1 = 1;
const byte Func2 = 0;
const byte Func3 = 3;

bool button_is_pressed(int btnVal, bool onlyOnce = true) {
  int pinVal = analogRead(buttonPin);
  if (abs(pinVal - btnVal) <= 10) {
    delay(50);
    pinVal = analogRead(buttonPin);
    if (abs(pinVal - btnVal) <= 10) {
      if (onlyOnce) {
        while (abs(analogRead(buttonPin) - btnVal) <= 10) {
          delay(10);
        }
      }
      return true;
    }
  }
  return false;
}

void activateFunc(const byte func, int blinkTime = 500){
  bool blink = false;
  bool keepOn = false;

  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("1. Quick Flash");
    display.setCursor(0, 30);
    display.println("2. Always On");
    display.setCursor(0, 40);
    display.println("3. Blink");
    display.setCursor(0, 50);
    display.println("4. Return");
    
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.print("State: ");
    display.print(digitalRead(func));
    display.display();
    
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime / 2);
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
    delay(200);
    }
  }
}

void watchFuncs(void){
  while (true){
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
  const char* screen2[] = {"+","-","*","/","(",")","^","sqrt","%","!","sin","cos","tan","asin","acos","atan"};
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
        display.print(result, 8);
      }
    }
    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print(expr);

    int y = 44;
    int x = 0;
    int spacing = 20;
    const char** activeScreen = onScreen1 ? screen1 : screen2;
    int activeSize = onScreen1 ? screen1_size : screen2_size;
    for (int i = 0; i < activeSize; i++) {
      x = (i % 6) * spacing;
      y = 44 + (i / 6) * 10;
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

    if (button_is_pressed(btn1)) {
      selected = (selected+1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn2)) {
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
    else if (button_is_pressed(btn3)) {
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
    else if (button_is_pressed(btn4)) {
      onScreen1 = !onScreen1;
      selected = 0;
      delay(120);
    }
    else if (button_is_pressed(btn5)) {
      if (showResult) return;
      int err;
      te_variable vars[] = {
      };
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

void setup(){
  pinMode(buttonPin, INPUT);
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
  display.setCursor(7, 0);
  display.print("Wecome to");
  display.setCursor(20, 20);
  display.print("Watch 0");
  display.setCursor(30, 50);
  display.print("Gen 5");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();

  delay(100);

  for (int i = 0; i < 200; i++){
    delay(20);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    if (button_is_pressed(btn1)){
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
    if (button_is_pressed(btn4)){
      digitalWrite(Func1, HIGH);
    } 
    else if (button_is_pressed(btn2)){
      digitalWrite(Func3, HIGH);
    }
    else if (button_is_pressed(btn3)){
      digitalWrite(LED_BUILTIN, LOW);
      calculator();
      break;
    }
  }
}

void loop(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.print("Watch 5.0");
  display.setCursor(0, 20);
  display.print(Functions[selectedFunction-1]);
  display.display();
  delay(200);

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
        break;
      case 2:
        break;
      case 3:
        break;
      case 4:
        break;
      case 5:
        break;
      case 6:
        break;
    }
  }
}
