// Includes: Calculator, Unit Converter, Graph Plotter

#include "tinyexpr.h"

extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern const byte buttonPin;
extern byte Func1;

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SCREEN_CENTER_X (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT / 2)
#define SCREEN_RADIUS 120

#define MAX_NUMBER_LENGTH 64
#define totalMathsFunctions 6

#define MAX_MATRIX_SIZE 3
#define MAT_YY_STEP 28
#define MAT_XX_STEP 48

const char* mathsFuncs[] = { "Calculator", "Unit Conv.", "Base Conv.", "Graph", "Matrix", "PrimeFact" };
int selectedMathsFunction = 1;

#define CHAR_W(sz) (6 * (sz))
#define CHAR_H(sz) (8 * (sz))
#define STR_W(text, sz) (strlen(text) * CHAR_W(sz))

// ---------------------------------------- MAIN MENU ----------------------------------------
void maths(void) {
  int lastSelected = -999;
  while (true) {
    if (selectedMathsFunction != lastSelected) {
      display.fillScreen(COLOR_BG);
      display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, COLOR_ACCENT);
      display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 6, COLOR_FG);

      for (int i = 0; i < totalMathsFunctions; i++) {
        float angle = (2 * PI * i / totalMathsFunctions) - PI / 2;
        int rDots = SCREEN_RADIUS - 26;
        int dotX = SCREEN_CENTER_X + (int)(rDots * cos(angle));
        int dotY = SCREEN_CENTER_Y + (int)(rDots * sin(angle));
        display.fillCircle(dotX, dotY, ((i + 1) == selectedMathsFunction) ? 9 : 6, ((i + 1) == selectedMathsFunction) ? COLOR_SELECTED : COLOR_UNSELECTED);
      }

      display.setTextColor(COLOR_FG);
      display.setTextSize(2);
      const char* funcName = mathsFuncs[selectedMathsFunction - 1];
      display.setCursor(SCREEN_CENTER_X - STR_W(funcName, 2) / 2, SCREEN_CENTER_Y - 28);
      display.print(funcName);

      char numBuf[8];
      sprintf(numBuf, "[%d/%d]", selectedMathsFunction, totalMathsFunctions);
      display.setTextColor(COLOR_ACCENT);
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - STR_W(numBuf, 1) / 2, SCREEN_CENTER_Y + 8);
      display.print(numBuf);

      display.setTextColor(COLOR_FG);
      const char* navHint = "< NAV >    SEL    EXIT";
      display.setCursor(SCREEN_CENTER_X - STR_W(navHint, 1) / 2, SCREEN_HEIGHT - 28);
      display.print(navHint);

      lastSelected = selectedMathsFunction;
    }

    delay(30);
    if (button_is_pressed(btn2)) {
      selectedMathsFunction++;
      if (selectedMathsFunction > totalMathsFunctions) selectedMathsFunction = 1;
      lastSelected = -999;
    } else if (button_is_pressed(btn1)) {
      selectedMathsFunction--;
      if (selectedMathsFunction < 1) selectedMathsFunction = totalMathsFunctions;
      lastSelected = -999;
    } else if (button_is_pressed(btn6)) return;
    else if (button_is_pressed(btn3)) {
      display.fillScreen(COLOR_BG);
      switch (selectedMathsFunction) {
        case 1: calculator(); break;
        case 2: unitConverter(); break;
        case 3: baseConverter(); break;
        case 4: graphPlotter(); break;
        case 5: matrixCalculator(); break;
        case 6: primeFactorisation(); break;
      }
      lastSelected = -999;
    }
  }
}

// ---------------------------------------- CALCULATOR ----------------------------------------
void calculator() {
  const char* screen1[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", ".", "pi" };
  const char* screen2[] = { "+", "-", "*", "/", "(", ")", "^", "%", "sin", "cos", "tan" };
  const int screen1_size = sizeof(screen1) / sizeof(screen1[0]);
  const int screen2_size = sizeof(screen2) / sizeof(screen2[0]);

  bool onScreen1 = true;
  int selected = 0;
  char expr[64] = "";
  int exprLen = 0;
  bool showResult = false;
  double result = 0;
  bool error = false;
  int redraw = 1;

  while (true) {
    if (redraw) {
      display.fillScreen(COLOR_BG);
      display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 8, COLOR_ACCENT);

      display.setTextSize(1);
      display.setTextColor(COLOR_ACCENT);
      display.setCursor(SCREEN_CENTER_X - STR_W("Calculator", 1) / 2, 14);
      display.print("Calculator");

      display.setTextSize(2);
      display.setTextColor(COLOR_FG);
      display.setCursor(SCREEN_CENTER_X - STR_W(expr, 2) / 2, 44);
      display.print(expr);

      // Result or Error
      display.setTextSize(2);
      if (showResult) {
        display.setTextColor(error ? COLOR_ERROR : COLOR_SELECTED);
        if (error)
          display.setCursor(SCREEN_CENTER_X - STR_W("Error!", 2) / 2, 72), display.print("Error!");
        else {
          char buf[24];
          snprintf(buf, sizeof(buf), "= %.6g", result);
          display.setCursor(SCREEN_CENTER_X - STR_W(buf, 2) / 2, 72);
          display.print(buf);
        }
      }

      // Input grid
      const char** activeScreen = onScreen1 ? screen1 : screen2;
      int activeSize = onScreen1 ? screen1_size : screen2_size;
      int cols = 4, rows = (activeSize + cols - 1) / cols;
      int gridBoxW = 44, gridBoxH = 28;
      int gridStartX = SCREEN_CENTER_X - (cols * gridBoxW) / 2 + 2;
      int gridStartY = 112;
      for (int i = 0; i < activeSize; i++) {
        int gx = gridStartX + (i % cols) * gridBoxW;
        int gy = gridStartY + (i / cols) * gridBoxH;
        if (i == selected) {
          display.fillRoundRect(gx - 2, gy - 2, gridBoxW, gridBoxH, 7, COLOR_SELECTED);
          display.setTextColor(COLOR_BG);
        } else {
          display.setTextColor(COLOR_ACCENT);
        }
        display.setTextSize(2);
        display.setCursor(gx + 8, gy + 2);
        display.print(activeScreen[i]);
      }
      display.setTextColor(COLOR_ACCENT);
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - STR_W("<L/R Sel>[OK]=Enter[5]=Alt[6]=Eval [B]=Back", 1) / 2, SCREEN_HEIGHT - 20);
      display.print("<L/R Sel>[OK]=Enter[5]=Alt[6]=Eval [B]=Back");

      redraw = 0;
    }

    if (button_is_pressed(btn2)) {
      selected = (selected + 1) % ((onScreen1) ? screen1_size : screen2_size);
      redraw = 1;
      delay(120);
    } else if (button_is_pressed(btn1)) {
      selected = (selected - 1 + (onScreen1 ? screen1_size : screen2_size)) % (onScreen1 ? screen1_size : screen2_size);
      redraw = 1;
      delay(120);
    } else if (button_is_pressed(btn5)) {
      onScreen1 = !onScreen1;
      selected = 0;
      redraw = 1;
      delay(120);
    } else if (button_is_pressed(btn3)) {
      if (exprLen < 63) {
        strcat(expr, (onScreen1 ? screen1[selected] : screen2[selected]));
        exprLen = strlen(expr);
      }
      redraw = 1;
      delay(120);
    } else if (button_is_pressed(btn4)) {
      int l = strlen(expr);
      if (l > 0) expr[l - 1] = '\0';
      exprLen = strlen(expr);
      redraw = 1;
      delay(120);
    } else if (button_is_pressed(btn6)) {
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
      redraw = 1;
      delay(300);
    } else if (button_is_pressed(btn1) && showResult) {
      return;
    }
    delay(40);
  }
}

// -------------------------------------- UNIT CONVERTER --------------------------------------
void unitConverter(void) {
  const char* types[] = {
    "cm->in", "in->cm", "C->F", "F->C", "kg->lb", "lb->kg", "km->mi", "mi->km", "g->oz", "oz->g", "L->gal", "gal->L"
  };
  enum { LEN = 0,
         LEN2,
         TEMP,
         TEMP2,
         WT,
         WT2,
         KM_MI,
         MI_KM,
         G_OZ,
         OZ_G,
         L_GAL,
         GAL_L };
  const int numTypes = sizeof(types) / sizeof(types[0]);
  int selectedType = 0;
  float inputValue = 0;
  bool enteringValue = true;
  float result = 0;

  while (true) {
    display.fillScreen(COLOR_BG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 8, COLOR_ACCENT);

    // Type selector as radial dots
    for (int i = 0; i < numTypes; i++) {
      float angle = (2 * PI * i / numTypes) - PI / 2;
      int rDots = SCREEN_RADIUS - 32;
      int dotX = SCREEN_CENTER_X + (int)(rDots * cos(angle));
      int dotY = SCREEN_CENTER_Y + (int)(rDots * sin(angle));
      display.fillCircle(dotX, dotY, i == selectedType ? 7 : 4, i == selectedType ? COLOR_SELECTED : COLOR_UNSELECTED);
    }

    display.setTextColor(COLOR_FG);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - STR_W("UnitConv", 2) / 2, 40);
    display.print("UnitConv");

    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X - STR_W(types[selectedType], 1) / 2, 72);
    display.print(types[selectedType]);

    display.setTextColor(COLOR_FG);
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X - 40, SCREEN_CENTER_Y + 4);
    display.print("In: ");
    display.setTextSize(2);
    display.print(inputValue, 2);

    if (!enteringValue) {
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - 40, SCREEN_CENTER_Y + 24);
      display.print("= ");
      display.setTextSize(2);
      display.print(result, 4);
    }

    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    const char* hint = "1/2:Type +/- 3:Conv 6:Back";
    display.setCursor(SCREEN_CENTER_X - STR_W(hint, 1) / 2, SCREEN_HEIGHT - 28);
    display.print(hint);

    if (button_is_pressed(btn1)) {
      selectedType = (selectedType - 1 + numTypes) % numTypes;
      enteringValue = true;
      delay(200);
    } else if (button_is_pressed(btn2)) {
      selectedType = (selectedType + 1) % numTypes;
      enteringValue = true;
      delay(200);
    } else if (button_is_pressed(btn4)) {
      if (enteringValue) inputValue--;
      delay(100);
    } else if (button_is_pressed(btn5)) {
      if (enteringValue) inputValue++;
      delay(100);
    } else if (button_is_pressed(btn3)) {
      switch (selectedType) {
        case LEN: result = inputValue / 2.54; break;
        case LEN2: result = inputValue * 2.54; break;
        case TEMP: result = inputValue * 9.0 / 5.0 + 32.0; break;
        case TEMP2: result = (inputValue - 32.0) * 5.0 / 9.0; break;
        case WT: result = inputValue * 2.20462; break;
        case WT2: result = inputValue / 2.20462; break;
        case KM_MI: result = inputValue * 0.621371; break;
        case MI_KM: result = inputValue / 0.621371; break;
        case G_OZ: result = inputValue * 0.035274; break;
        case OZ_G: result = inputValue / 0.035274; break;
        case L_GAL: result = inputValue * 0.264172; break;
        case GAL_L: result = inputValue / 0.264172; break;
      }
      enteringValue = false;
      delay(300);
    } else if (button_is_pressed(btn6)) {
      return;
    }
    delay(30);
  }
}

// -------------------------------------- BASE CONVERTER --------------------------------------
const char* baseCharsets[] = {
  "", "", "01", "012", "0123", "01234", "012345", "0123456", "01234567", "012345678", "0123456789", "0123456789A", "0123456789AB", "0123456789ABC", "0123456789ABCD", "0123456789ABCDE", "0123456789ABCDEF"
};
void inputNum(char* buffer, int maxLen, int base);
void convertAndDisplay(const char* number, int sourceBase, int targetBase);

void baseConverter(void) {
  int sourceBase = 10;
  int targetBase = 16;
  char inputNumber[MAX_NUMBER_LENGTH] = "";

  while (true) {
    display.fillScreen(COLOR_BG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 8, COLOR_ACCENT);

    display.setTextColor(COLOR_FG);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - STR_W("BaseConv", 2) / 2, 32);
    display.print("BaseConv");

    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    char buf[20];
    sprintf(buf, "%d -> %d", sourceBase, targetBase);
    display.setCursor(SCREEN_CENTER_X - STR_W(buf, 1) / 2, 62);
    display.print(buf);

    display.setTextSize(1);
    display.setTextColor(COLOR_FG);
    display.setCursor(SCREEN_CENTER_X - 60, 90);
    display.print("Num: ");
    display.print(inputNumber[0] ? inputNumber : "_");

    const char* hint = "3:SetBase 4:Conv 5:Input 6:Back";
    display.setTextColor(COLOR_ACCENT);
    display.setCursor(SCREEN_CENTER_X - STR_W(hint, 1) / 2, SCREEN_HEIGHT - 28);
    display.print(hint);

    if (button_is_pressed(btn3)) {
      // Select base
      int sel = 0, tmpSrc = sourceBase, tmpTgt = targetBase;
      while (true) {
        display.fillScreen(COLOR_BG);
        display.setTextColor(COLOR_ACCENT);
        display.setTextSize(2);
        display.setCursor(SCREEN_CENTER_X - 52, 80);
        display.print("SRC:");
        display.print(tmpSrc);
        display.setCursor(SCREEN_CENTER_X + 12, 80);
        display.print("DST:");
        display.print(tmpTgt);

        display.setTextSize(1);
        display.setCursor(SCREEN_CENTER_X - 70, SCREEN_CENTER_Y + 28);
        display.print(sel == 0 ? "^      " : "       ");
        display.setCursor(SCREEN_CENTER_X + 38, SCREEN_CENTER_Y + 28);
        display.print(sel == 1 ? "^" : " ");
        display.setCursor(SCREEN_CENTER_X - 64, SCREEN_HEIGHT - 28);
        display.print("2/1:Sel 4/5:+- 3:Ok 6:Back");

        if (button_is_pressed(btn2)) {
          sel = (sel + 1) % 2;
          delay(120);
        } else if (button_is_pressed(btn1)) {
          sel = (sel - 1 + 2) % 2;
          delay(120);
        } else if (button_is_pressed(btn4)) {
          if (sel == 0) tmpSrc = tmpSrc > 2 ? tmpSrc - 1 : 16;
          else tmpTgt = tmpTgt > 2 ? tmpTgt - 1 : 16;
          delay(100);
        } else if (button_is_pressed(btn5)) {
          if (sel == 0) tmpSrc = tmpSrc < 16 ? tmpSrc + 1 : 2;
          else tmpTgt = tmpTgt < 16 ? tmpTgt + 1 : 2;
          delay(100);
        } else if (button_is_pressed(btn3)) {
          sourceBase = tmpSrc;
          targetBase = tmpTgt;
          break;
        } else if (button_is_pressed(btn6)) break;
        delay(20);
      }
    } else if (button_is_pressed(btn4)) {  // Convert!
      if (strlen(inputNumber) > 0) convertAndDisplay(inputNumber, sourceBase, targetBase);
      delay(200);
    } else if (button_is_pressed(btn5)) {
      inputNum(inputNumber, MAX_NUMBER_LENGTH, sourceBase);
      delay(200);
    } else if (button_is_pressed(btn6)) return;
    delay(30);
  }
}

void inputNum(char* buffer, int maxLen, int base) {
  buffer[0] = '\0';
  int charIndex = 0;
  const char* charset = baseCharsets[base];
  int charsetSize = strlen(charset);
  while (true) {
    display.fillScreen(COLOR_BG);
    display.setTextColor(COLOR_FG);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - 44, 50);
    display.print(">");
    display.print(buffer[0] ? buffer : "_");
    display.print("<");
    display.setTextSize(1);
    display.setTextColor(COLOR_ACCENT);
    display.setCursor(SCREEN_CENTER_X - 34, 90);
    display.printf("Add:%c", charset[charIndex]);
    display.setCursor(SCREEN_CENTER_X - 52, SCREEN_HEIGHT - 32);
    display.print("1/2:Digit 3:OK 4:DEL 5:CLR 6:Back");
    if (button_is_pressed(btn1)) {
      charIndex = (charIndex - 1 + charsetSize) % charsetSize;
      delay(70);
    } else if (button_is_pressed(btn2)) {
      charIndex = (charIndex + 1) % charsetSize;
      delay(70);
    } else if (button_is_pressed(btn3)) {
      int l = strlen(buffer);
      if (l < maxLen - 1) {
        buffer[l] = charset[charIndex];
        buffer[l + 1] = '\0';
      }
      delay(100);
    } else if (button_is_pressed(btn4)) {
      int l = strlen(buffer);
      if (l) buffer[l - 1] = '\0';
      delay(90);
    } else if (button_is_pressed(btn5)) {
      buffer[0] = '\0';
      delay(80);
    } else if (button_is_pressed(btn6)) return;
    delay(20);
  }
}

void convertAndDisplay(const char* number, int sourceBase, int targetBase) {
  unsigned long decimalValue = 0;
  const char* charset = baseCharsets[sourceBase];
  for (int i = 0; i < strlen(number); i++) {
    char c = toupper(number[i]);
    if (!strchr(charset, c)) {
      display.fillScreen(COLOR_ERROR);
      display.setTextColor(COLOR_FG);
      display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X - 64, SCREEN_CENTER_Y - 6);
      display.print("Invalid");
      delay(1200);
      return;
    }
  }
  for (int i = 0; i < strlen(number); i++) {
    char c = toupper(number[i]);
    int digit = (c >= '0' && c <= '9') ? (c - '0') : (c - 'A' + 10);
    decimalValue = decimalValue * sourceBase + digit;
  }
  char result[MAX_NUMBER_LENGTH] = "";
  if (decimalValue == 0) strcpy(result, "0");
  else {
    char temp[MAX_NUMBER_LENGTH] = "";
    int idx = 0;
    unsigned long val = decimalValue;
    while (val) {
      int rem = val % targetBase;
      temp[idx++] = baseCharsets[targetBase][rem];
      val /= targetBase;
    }
    for (int i = 0; i < idx; i++) result[i] = temp[idx - 1 - i];
    result[idx] = '\0';
  }
  while (1) {
    display.fillScreen(COLOR_BG);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X - 64, 50);
    display.print("Result: ");
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - STR_W(result, 2) / 2, 70);
    display.print(result);
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X - 42, SCREEN_HEIGHT - 32);
    display.print("Any Btn:Back");
    if (button_is_pressed(btn1) || button_is_pressed(btn2) || button_is_pressed(btn3) || button_is_pressed(btn4) || button_is_pressed(btn5) || button_is_pressed(btn6))
      return;
    delay(25);
  }
}

// ---------------------- Graph Plotter UI: (Quick, centered, not touch) ----------------------
double evaluateEquation(char* equation, double x);

void graphPlotter(void) {
  char equation[64] = "";
  int exprLen = 0;
  double xMin = -10, xMax = 10, yMin = -10, yMax = 10;
  int sel = 0, redraw = 1;
  while (1) {
    if (redraw) {
      display.fillScreen(COLOR_BG);
      display.setTextColor(COLOR_ACCENT);
      display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X - 36, 18);
      display.print("Graph");

      display.setTextColor(COLOR_FG);
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - 58, 50);
      display.print("y = ");
      display.print(equation);
      display.print("_");

      display.setTextColor(COLOR_ACCENT);
      display.setCursor(SCREEN_CENTER_X - 68, 74);
      display.print("1/2:LR 3:Add 4:Del 5:Plot 6:Back");
      redraw = 0;
    }
    if (button_is_pressed(btn2)) {
      sel = (sel + 1) % 11;
      redraw = 1;
      delay(70);
    } else if (button_is_pressed(btn1)) {
      sel = (sel - 1 + 11) % 11;
      redraw = 1;
      delay(70);
    } else if (button_is_pressed(btn3)) {
      if (exprLen < 63) {
        char* ky[] = { "x", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "." };
        if (sel < sizeof(ky) / sizeof(ky[0])) strcat(equation, ky[sel]);
        exprLen = strlen(equation);
      }
      redraw = 1;
      delay(80);
    } else if (button_is_pressed(btn4)) {
      int l = strlen(equation);
      if (l) {
        equation[l - 1] = '\0';
        exprLen = strlen(equation);
      }
      redraw = 1;
      delay(90);
    } else if (button_is_pressed(btn5) && (exprLen > 0)) {
      plotGraph(equation, xMin, xMax, yMin, yMax);
      redraw = 1;
      delay(200);
    } else if (button_is_pressed(btn6)) {
      return;
    }
    delay(20);
  }
}

// Fast, simple, just plot lines
bool plotGraph(char* equation, double xMin, double xMax, double yMin, double yMax) {
  display.fillScreen(COLOR_BG);
  display.setTextColor(COLOR_FG);
  display.setTextSize(1);
  display.setCursor(10, 16);
  display.print("Plotting...");
  delay(300);

  display.fillScreen(COLOR_BG);

  int centerX = SCREEN_CENTER_X;
  int centerY = SCREEN_CENTER_Y;

  display.drawLine(0, centerY, SCREEN_WIDTH - 1, centerY, COLOR_ACCENT);
  display.drawLine(centerX, 0, centerX, SCREEN_HEIGHT - 1, COLOR_ACCENT);

  double pxUnitX = (SCREEN_WIDTH - 21) / (xMax - xMin);
  double pxUnitY = (SCREEN_HEIGHT - 21) / (yMax - yMin);
  double prevY = INFINITY;
  int prevPixelX = -1, prevPixelY = -1;
  for (int px = 10; px < SCREEN_WIDTH - 10; px++) {
    double x = xMin + (px - 10) / pxUnitX;
    double y = evaluateEquation(equation, x);
    if (!isnan(y) && !isinf(y) && y >= yMin && y <= yMax) {
      int py = SCREEN_HEIGHT - 11 - (int)((y - yMin) * pxUnitY);
      py = (py < 10) ? 10 : ((py > SCREEN_HEIGHT - 11) ? SCREEN_HEIGHT - 11 : py);
      if (prevPixelX >= 0 && prevPixelY >= 0 && !isinf(prevY))
        if (abs(py - prevPixelY) < SCREEN_HEIGHT / 2) display.drawLine(prevPixelX, prevPixelY, px, py, COLOR_SELECTED);
      prevPixelX = px;
      prevPixelY = py;
      prevY = y;
    } else {
      prevY = INFINITY;
      prevPixelX = -1;
      prevPixelY = -1;
    }
  }
  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(1);
  display.setCursor(10, SCREEN_HEIGHT - 22);
  display.print("6:Back");
  while (!button_is_pressed(btn6)) delay(50);
  return 1;
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
  double result = NAN;
  if (te) {
    result = te_eval(te);
    te_free(te);
  }
  return result;
}
// ------ MATRIX CALCULATOR: Round, Modern, Fully Standalone ------

#define MAX_MATRIX_SIZE 3
#define MAT_YY_STEP 28
#define MAT_XX_STEP 48

float matrixA[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], matrixB[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], matrixRes[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];
int rowsA = 2, colsA = 2, rowsB = 2, colsB = 2, rowsRes = 0, colsRes = 0;
float scalarVal = 1;

void setIdentity(float m[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) m[i][j] = i == j ? 1 : 0;
}
void copyMatrix(float src[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], float dest[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols) {
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) dest[i][j] = src[i][j];
}
void matrixAdd(float a[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], float b[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], float out[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols) {
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) out[i][j] = a[i][j] + b[i][j];
}
void matrixSub(float a[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], float b[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], float out[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols) {
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) out[i][j] = a[i][j] - b[i][j];
}
void matrixMul(float a[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int aR, int aC, float b[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int bR, int bC, float out[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int& outR, int& outC) {
  outR = aR;
  outC = bC;
  for (int i = 0; i < aR; i++)
    for (int j = 0; j < bC; j++) {
      float sum = 0;
      for (int k = 0; k < aC; k++) sum += a[i][k] * b[k][j];
      out[i][j] = sum;
    }
}
void matrixScalar(float a[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols, float factor, float out[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE]) {
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) out[i][j] = a[i][j] * factor;
}
void matrixTranspose(float in[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols, float out[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE]) {
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) out[j][i] = in[i][j];
}

float matrixDet(float m[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int n) {
  if (n == 1) return m[0][0];
  if (n == 2) return m[0][0] * m[1][1] - m[0][1] * m[1][0];
  if (n == 3)
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
           - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
           + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
  return 0;
}
bool matrixInverse(float m[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int n, float out[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE]) {
  float det = matrixDet(m, n);
  if (fabs(det) < 1e-6) return false;
  if (n == 2) {
    out[0][0] = m[1][1] / det;
    out[0][1] = -m[0][1] / det;
    out[1][0] = -m[1][0] / det;
    out[1][1] = m[0][0] / det;
    return true;
  }
  if (n == 3) {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++) {
        float minorWork[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];
        int mi = 0;
        for (int ii = 0; ii < 3; ii++)
          if (ii != i) {
            int mj = 0;
            for (int jj = 0; jj < 3; jj++)
              if (jj != j)
                minorWork[mi][mj++] = m[ii][jj];
            mi++;
          }
        float cofactor = ((i + j) % 2 == 0 ? 1 : -1) * matrixDet(minorWork, 2);
        out[j][i] = cofactor / det;
      }
    return true;
  }
  return false;
}

void drawMatrix(float m[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols, int baseY = 74) {
  int boxW = 44, boxH = 22;
  int startX = SCREEN_CENTER_X - (cols * boxW) / 2;
  int startY = baseY;
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) {
      int xx = startX + j * boxW;
      int yy = startY + i * boxH;
      display.setTextColor(COLOR_ACCENT);
      display.setTextSize(1);
      display.drawRoundRect(xx - 2, yy - 2, boxW, boxH, 4, COLOR_ACCENT);
      display.setTextColor(COLOR_FG);
      display.setTextSize(2);
      char buf[10];
      snprintf(buf, sizeof(buf), "%.1f", m[i][j]);
      display.setCursor(xx + boxW / 2 - STR_W(buf, 2) / 2, yy + 3);
      display.print(buf);
    }
}

void matrixCalculator() {
  setIdentity(matrixA, MAX_MATRIX_SIZE);
  setIdentity(matrixB, MAX_MATRIX_SIZE);
  rowsA = 2;
  colsA = 2;
  rowsB = 2;
  colsB = 2;
  scalarVal = 1;
  const char* ops[] = { "Edit A", "Edit B", "A+B", "A-B", "A*B", "A*x", "TA", "detA", "invA", "Clear", "Back" };
  const int nops = sizeof(ops) / sizeof(ops[0]);
  int sel = 0;
  int redraw = 1;

  while (true) {
    if (redraw) {
      display.fillScreen(COLOR_BG);
      display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 8, COLOR_ACCENT);
      // Menu ring selectors
      for (int i = 0; i < nops - 1; i++) {
        float angle = (2 * PI * i / (nops - 1)) - PI / 2;
        int r = SCREEN_RADIUS - 32;
        int dx = SCREEN_CENTER_X + (int)(r * cos(angle));
        int dy = SCREEN_CENTER_Y + (int)(r * sin(angle));
        display.fillCircle(dx, dy, i == sel ? 8 : 4, i == sel ? COLOR_SELECTED : COLOR_UNSELECTED);
        display.setTextColor(COLOR_UNSELECTED);
        display.setTextSize(1);
        display.setCursor(dx - 10, dy - 16);
        display.print(ops[i]);
      }
      // Title
      display.setTextColor(COLOR_FG);
      display.setTextSize(2);
      display.setCursor(SCREEN_CENTER_X - STR_W("Matrix", 2) / 2, 26);
      display.print("Matrix");

      // Preview A (center left), B (center right)
      drawMatrix(matrixA, rowsA, colsA, 76);
      drawMatrix(matrixB, rowsB, colsB, 76);

      display.setTextColor(COLOR_ACCENT);
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - 54, SCREEN_HEIGHT - 30);
      display.print("L/R:Move 3:Sel 6:Back");
      redraw = 0;
    }
    if (button_is_pressed(btn2)) {
      sel = (sel + 1) % (nops);
      redraw = 1;
      delay(90);
    } else if (button_is_pressed(btn1)) {
      sel = (sel - 1 + nops) % (nops);
      redraw = 1;
      delay(90);
    } else if (button_is_pressed(btn3)) {
      switch (sel) {
        case 0: editMatrix(matrixA, rowsA, colsA, 'A'); break;
        case 1: editMatrix(matrixB, rowsB, colsB, 'B'); break;
        case 2:
          if (rowsA == rowsB && colsA == colsB) {
            matrixAdd(matrixA, matrixB, matrixRes, rowsA, colsA);
            showResultMatrix("A+B", matrixRes, rowsA, colsA);
          }
          break;
        case 3:
          if (rowsA == rowsB && colsA == colsB) {
            matrixSub(matrixA, matrixB, matrixRes, rowsA, colsA);
            showResultMatrix("A-B", matrixRes, rowsA, colsA);
          }
          break;
        case 4:
          if (colsA == rowsB) {
            matrixMul(matrixA, rowsA, colsA, matrixB, rowsB, colsB, matrixRes, rowsRes, colsRes);
            showResultMatrix("A*B", matrixRes, rowsRes, colsRes);
          }
          break;
        case 5:
          showScalarMenu(scalarVal);
          matrixScalar(matrixA, rowsA, colsA, scalarVal, matrixRes);
          showResultMatrix("A*x", matrixRes, rowsA, colsA);
      }
      break;
      case 6:
        matrixTranspose(matrixA, rowsA, colsA, matrixRes);
        showResultMatrix("TA", matrixRes, colsA, rowsA);
        break;
      case 7:
        if (rowsA == colsA) {
          float d = matrixDet(matrixA, rowsA);
          showMatrixDet(d);
        }
        break;
      case 8:
        if (rowsA == colsA && rowsA >= 2 && rowsA <= 3) {
          if (matrixInverse(matrixA, rowsA, matrixRes)) showResultMatrix("invA", matrixRes, rowsA, colsA);
          else showMatrixError("Not invertible");
        }
        break;
      case 9:
        setIdentity(matrixA, MAX_MATRIX_SIZE);
        setIdentity(matrixB, MAX_MATRIX_SIZE);
        rowsA = colsA = rowsB = colsB = 2;
        break;
      case 10: return;
    }
    redraw = 1;
    delay(120);
  }
  else if (button_is_pressed(btn6)) {
    return;
  }
  delay(25);
}
}

void showResultMatrix(const char* op, float m[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int rows, int cols, float scalar = 0) {
  display.fillScreen(COLOR_BG);
  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(2);
  display.setCursor(SCREEN_CENTER_X - STR_W(op, 2) / 2, 24);
  display.print(op);
  drawMatrix(m, rows, cols, 76);
  if (scalar != 0) {
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    char buf[24];
    snprintf(buf, sizeof(buf), "Scalar: %.2f", scalar);
    display.setCursor(SCREEN_CENTER_X - STR_W(buf, 1) / 2, 52);
    display.print(buf);
  }
  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(1);
  display.setCursor(SCREEN_CENTER_X - 32, SCREEN_HEIGHT - 26);
  display.print("Btn:Back");
  while (!button_is_pressed(btn6)) delay(40);
}

void editMatrix(float m[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE], int& rows, int& cols, char name) {
  rows = 2;
  cols = 2;
  int sel = 0;
  static const char* sizes[] = { "2x2", "2x3", "3x2", "3x3" };
  static int sizeOpts[4][2] = { { 2, 2 }, { 2, 3 }, { 3, 2 }, { 3, 3 } };
  while (1) {
    display.fillScreen(COLOR_BG);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - 64, 26);
    display.print("Edit Mat ");
    display.print(name);
    for (int i = 0; i < 4; i++) {
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - (4 * 18) + i * 36, 70);
      if (i == sel) display.setTextColor(COLOR_SELECTED);
      else display.setTextColor(COLOR_FG);
      display.print(sizes[i]);
    }
    display.setTextColor(COLOR_ACCENT);
    display.setCursor(SCREEN_CENTER_X - 54, SCREEN_HEIGHT - 30);
    display.print("L/R:+- 3:OK 6:Back");
    if (button_is_pressed(btn2)) {
      sel = (sel + 1) % 4;
      delay(80);
    } else if (button_is_pressed(btn1)) {
      sel = (sel - 1 + 4) % 4;
      delay(80);
    } else if (button_is_pressed(btn3)) {
      rows = sizeOpts[sel][0];
      cols = sizeOpts[sel][1];
      break;
    } else if (button_is_pressed(btn6)) return;
    delay(18);
  }
  // Now edit entry-by-entry
  int i = 0, j = 0;
  while (1) {
    display.fillScreen(COLOR_BG);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(2);
    char buf[12];
    sprintf(buf, "%c[%d,%d]", name, i + 1, j + 1);
    display.setCursor(SCREEN_CENTER_X - STR_W(buf, 2) / 2, 40);
    display.print(buf);
    drawMatrix(m, rows, cols, 90);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X - 38, SCREEN_HEIGHT - 26);
    display.print("1/2:UP/DN 4/5:LF/RT 3:EDIT 6:Done");
    if (button_is_pressed(btn1)) {
      i = (i - 1 + rows) % rows;
      delay(80);
    } else if (button_is_pressed(btn2)) {
      i = (i + 1) % rows;
      delay(80);
    } else if (button_is_pressed(btn4)) {
      j = (j - 1 + cols) % cols;
      delay(80);
    } else if (button_is_pressed(btn5)) {
      j = (j + 1) % cols;
      delay(80);
    } else if (button_is_pressed(btn3)) {
      float val = m[i][j];
      while (1) {
        display.fillScreen(COLOR_BG);
        display.setTextColor(COLOR_ACCENT);
        display.setTextSize(2);
        char bufval[20];
        snprintf(bufval, sizeof(bufval), "%c[%d,%d]=%.2f", name, i + 1, j + 1, val);
        display.setCursor(SCREEN_CENTER_X - STR_W(bufval, 2) / 2, SCREEN_CENTER_Y - 4);
        display.print(bufval);
        display.setTextSize(1);
        display.setCursor(SCREEN_CENTER_X - 28, SCREEN_HEIGHT - 34);
        display.print("4/5: -/+ 3:OK 6:Cancel");
        if (button_is_pressed(btn4)) val -= 0.1;
        else if (button_is_pressed(btn5)) val += 0.1;
        else if (button_is_pressed(btn3)) {
          m[i][j] = val;
          break;
        } else if (button_is_pressed(btn6)) break;
        delay(15);
      }
    } else if (button_is_pressed(btn6)) break;
    delay(18);
  }
}

void showScalarMenu(float& scalar) {
  while (1) {
    display.fillScreen(COLOR_BG);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - 56, SCREEN_CENTER_Y - 16);
    display.print("Scalar: ");
    display.print(scalar, 2);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    display.setCursor(SCREEN_CENTER_X - 34, SCREEN_HEIGHT - 28);
    display.print("4/5:-/+ 3:OK 6:Back");
    if (button_is_pressed(btn4)) scalar -= 0.1;
    else if (button_is_pressed(btn5)) scalar += 0.1;
    else if (button_is_pressed(btn3)) return;
    else if (button_is_pressed(btn6)) break;
    delay(12);
  }
}

void showMatrixDet(float det) {
  display.fillScreen(COLOR_BG);
  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(2);
  char buf[30];
  snprintf(buf, sizeof(buf), "Det=%.3f", det);
  display.setCursor(SCREEN_CENTER_X - STR_W(buf, 2) / 2, SCREEN_CENTER_Y - 8);
  display.print(buf);
  display.setTextSize(1);
  display.setCursor(SCREEN_CENTER_X - 32, SCREEN_HEIGHT - 28);
  display.print("Any Btn:Back");
  while (!button_is_pressed(btn6)) delay(40);
}
void showMatrixError(const char* msg) {
  display.fillScreen(COLOR_ERROR);
  display.setTextColor(COLOR_FG);
  display.setTextSize(2);
  display.setCursor(SCREEN_CENTER_X - STR_W(msg, 2) / 2, SCREEN_CENTER_Y - 8);
  display.print(msg);
  while (!button_is_pressed(btn6)) delay(35);
}

void primeFactorisation() {
  char inputBuffer[14] = "";
  int inputLen = 0;
  bool showingResult = false;
  char resultBuffer[64] = "";
  int resultLen = 0, selectedDigit = 0;
  const char* digits[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

  while (1) {
    display.fillScreen(COLOR_BG);
    display.drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 8, COLOR_ACCENT);
    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - 52, 24);
    display.print("PrimeFac");

    display.setTextColor(COLOR_FG);
    display.setTextSize(2);
    display.setCursor(SCREEN_CENTER_X - 44, 68);
    display.print(inputBuffer[0] ? inputBuffer : "_");

    // Draw digit ring
    for (int i = 0; i < 10; i++) {
      float angle = (2 * PI * i / 10.0) - PI / 2;
      int r = SCREEN_RADIUS - 45;
      int dx = SCREEN_CENTER_X + (int)(r * cos(angle));
      int dy = SCREEN_CENTER_Y + (int)(r * sin(angle));
      display.fillCircle(dx, dy, i == selectedDigit ? 8 : 4, i == selectedDigit ? COLOR_SELECTED : COLOR_UNSELECTED);
      display.setTextColor(COLOR_FG);
      display.setTextSize(1);
      display.setCursor(dx - 2, dy - 6);
      display.print(digits[i]);
    }

    if (showingResult) {
      display.setTextColor(COLOR_ACCENT);
      display.setTextSize(1);
      display.setCursor(SCREEN_CENTER_X - 48, SCREEN_HEIGHT - 46);
      display.print("Result:");
      display.setTextSize(2);
      display.setTextColor(COLOR_FG);
      display.setCursor(SCREEN_CENTER_X - STR_W(resultBuffer, 2) / 2, SCREEN_HEIGHT - 34);
      display.print(resultBuffer);
    }

    display.setTextColor(COLOR_ACCENT);
    display.setTextSize(1);
    const char* hint = "1/2:Num 3:Add 4:Del 5:Fact 6:Back";
    display.setCursor(SCREEN_CENTER_X - STR_W(hint, 1) / 2, SCREEN_HEIGHT - 20);
    display.print(hint);

    if (button_is_pressed(btn1)) {
      selectedDigit = (selectedDigit - 1 + 10) % 10;
      delay(90);
    } else if (button_is_pressed(btn2)) {
      selectedDigit = (selectedDigit + 1) % 10;
      delay(90);
    } else if (button_is_pressed(btn3)) {
      if (inputLen < 12) {
        inputBuffer[inputLen++] = digits[selectedDigit][0];
        inputBuffer[inputLen] = '\0';
        showingResult = false;
      }
      delay(80);
    } else if (button_is_pressed(btn4)) {
      if (inputLen > 0) inputBuffer[--inputLen] = '\0';
      showingResult = false;
      delay(80);
    } else if (button_is_pressed(btn5)) {
      unsigned long n = 0;
      for (int i = 0; i < inputLen; i++) n = n * 10 + (inputBuffer[i] - '0');
      if (n < 2) {
        strcpy(resultBuffer, "Err!");
      } else {
        int p = 0;
        unsigned long t = n;
        for (unsigned long d = 2; d <= t; d++) {
          int c = 0;
          while ((t % d) == 0) {
            c++;
            t /= d;
          }
          if (c) {
            if (p) strcat(resultBuffer, "*");
            char xx[8];
            sprintf(xx, "%lu", d);
            strcat(resultBuffer, xx);
            if (c > 1) {
              strcat(resultBuffer, "^");
              char yy[4];
              sprintf(yy, "%d", c);
              strcat(resultBuffer, yy);
            }
            p = 1;
          }
          if (t == 1) break;
        }
      }
      showingResult = true;
      delay(180);
    } else if (button_is_pressed(btn6)) return;
    delay(16);
  }
}