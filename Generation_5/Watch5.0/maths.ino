// Includes: Calculator, Unit Converter, Graph Plotter

#include "tinyexpr.h"

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const int btn1, btn2, btn3, btn4, btn5, btn6;
extern const byte buttonPin;
extern byte Func1;
extern int buttonOffset;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

void maths(void) {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Calculator");
    display.println("2. Unit Converter");
    display.println("3. Graph Plotter");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) calculator();
    else if (button_is_pressed(btn2)) unitConverter();
    else if (button_is_pressed(btn3)) graphPlotter();
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

    if (button_is_pressed(btn2, false)) {
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
        else if (button_is_pressed(btn4, false)){
            if (enteringValue) inputValue--;
            delay(150);
        } 
        else if (button_is_pressed(btn5, false)){
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

void graphPlotter(void) {
  const char* charScreen1[] = {"x","0","1","2","3","4","5","6","7","8","9","."};
  const int charScreen1_size = sizeof(charScreen1) / sizeof(charScreen1[0]);
  const char* charScreen2[] = {"+","-","*","/","(",")","^","%","sin","cos","tan","sqrt"};
  const int charScreen2_size = sizeof(charScreen2) / sizeof(charScreen2[0]);
  
  char equation[64] = "";
  int exprLen = 0;
  double xMin = -10, xMax = 10, yMin = -10, yMax = 10;
  
  bool onScreen1 = true;
  int selected = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Graph");
    
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("y = ");
    display.print(equation);
    display.print("_");

    int y;
    int x;
    int spacing = 20;
    const char** activeScreen = onScreen1 ? charScreen1 : charScreen2;
    int activeSize = onScreen1 ? charScreen1_size : charScreen2_size;
    
    for (int i = 0; i < activeSize; i++) {
      x = (i % 6) * spacing;
      y = 35 + (i / 6) * 10;
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

    if (button_is_pressed(btn2, false)) {
      selected = (selected+1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn1, false)) {
      selected = (selected-1) % activeSize;
      delay(120);
    }
    else if (button_is_pressed(btn3)) {
      if (exprLen < 63) {
        const char** activeScreen = onScreen1 ? charScreen1 : charScreen2;
        const char* selectedChar = activeScreen[selected];
        
        if (strcmp(selectedChar, "sin") == 0) {
          strcat(equation, "sin(");
          exprLen += 4;
        } else if (strcmp(selectedChar, "cos") == 0) {
          strcat(equation, "cos(");
          exprLen += 4;
        } else if (strcmp(selectedChar, "tan") == 0) {
          strcat(equation, "tan(");
          exprLen += 4;
        } else if (strcmp(selectedChar, "sqrt") == 0) {
          strcat(equation, "sqrt(");
          exprLen += 5;
        } else {
          strcat(equation, selectedChar);
          exprLen = strlen(equation);
        }
      }
      delay(120);
    }
    else if (button_is_pressed(btn4)) {
      int len = strlen(equation);
      if (len > 0) {
        equation[len-1] = '\0';
        exprLen = strlen(equation);
      }
      delay(120);
    }
    else if (button_is_pressed(btn5)) {
      onScreen1 = !onScreen1;
      selected = 0;
      delay(120);
    }
    else if (button_is_pressed(btn6)) {
      if (exprLen > 0) {
        if (plotGraph(equation, xMin, xMax, yMin, yMax)) return;
      }
      delay(200);
    }
    delay(40);
  }
}

bool plotGraph(char* equation, double xMin, double xMax, double yMin, double yMax) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Plotting...");
  display.display();
  
  delay(500);
  
  display.clearDisplay();
  
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  
  display.drawLine(0, centerY, SCREEN_WIDTH - 1, centerY, SSD1306_WHITE);
  display.drawLine(centerX, 0, centerX, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  
  double pixelsPerUnitX = (SCREEN_WIDTH - 1) / (xMax - xMin);
  double pixelsPerUnitY = (SCREEN_HEIGHT - 1) / (yMax - yMin);
  
  double prevY = INFINITY;
  int prevPixelX = -1;
  int prevPixelY = -1;
  
  for (int pixelX = 0; pixelX < SCREEN_WIDTH; pixelX++) {
    double x = xMin + (pixelX / pixelsPerUnitX);
    double y = evaluateEquation(equation, x);
    
    if (!isnan(y) && !isinf(y) && y >= yMin && y <= yMax) {
      int pixelY = SCREEN_HEIGHT - 1 - (int)((y - yMin) * pixelsPerUnitY);
      pixelY = constrain(pixelY, 0, SCREEN_HEIGHT - 1);
      
      if (prevPixelX >= 0 && prevPixelY >= 0 && !isinf(prevY)) {
        if (abs(pixelY - prevPixelY) < SCREEN_HEIGHT / 2) {
          display.drawLine(prevPixelX, prevPixelY, pixelX, pixelY, SSD1306_WHITE);
        }
      }
      
      prevPixelX = pixelX;
      prevPixelY = pixelY;
      prevY = y;
    } else {
      prevY = INFINITY;
      prevPixelX = -1;
      prevPixelY = -1;
    }
  }
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("X:");
  display.print((int)xMin);
  display.print("-");
  display.print((int)xMax);
  
  display.display();
  
  bool configuring = true;
  int settingIndex = 0;
  
  while (configuring) {
    if (button_is_pressed(btn1)) {
      settingIndex = 0;
      delay(150);
    }
    else if (button_is_pressed(btn2)) {
      settingIndex = 1;
      delay(150);
    }
    else if (button_is_pressed(btn3)) {
      if (settingIndex == 0) {
        Serial.println("\n--- Set X Range ---");
        Serial.print("X Min: ");
        while (!Serial.available()) delay(10);
        xMin = Serial.parseFloat();
        Serial.println(xMin);
        
        Serial.print("X Max: ");
        while (!Serial.available()) delay(10);
        xMax = Serial.parseFloat();
        Serial.println(xMax);
        
        plotGraph(equation, xMin, xMax, yMin, yMax);
        return false;
      } else {
        Serial.println("\n--- Set Y Range ---");
        Serial.print("Y Min: ");
        while (!Serial.available()) delay(10);
        yMin = Serial.parseFloat();
        Serial.println(yMin);
        
        Serial.print("Y Max: ");
        while (!Serial.available()) delay(10);
        yMax = Serial.parseFloat();
        Serial.println(yMax);
        
        plotGraph(equation, xMin, xMax, yMin, yMax);
        return false;
      }
      delay(200);
    }
    else if (button_is_pressed(btn6)) {
      return true;
    }
    else if (button_is_pressed(btn1)) {
      return false;
    }
    delay(50);
  }
}

double evaluateEquation(char* equation, double x) {
  char fullExpr[128] = "";
  
  for (int i = 0; equation[i] != '\0' && strlen(fullExpr) < 120; i++) {
    char c = equation[i];
    
    if (c == 'x' || c == 'X') {
      char xStr[20];
      snprintf(xStr, sizeof(xStr), "(%g)", x);
      strcat(fullExpr, xStr);
    } else {
      strncat(fullExpr, &c, 1);
    }
  }
  
  int err;
  te_variable vars[] = {};
  te_expr* te = te_compile(fullExpr, vars, 0, &err);
  
  if (te) {
    double result = te_eval(te);
    te_free(te);
    return result;
  } else {
    return NAN;
  }
}
