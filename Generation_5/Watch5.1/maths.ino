#include "tinyexpr.h"

extern TFT_eSPI display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG,colourText,colour1,colour2,colour3,colour4,colour5,colour6;
extern const byte buttonPin;

#define MAX_NUMBER_LENGTH 32
#define totalMathsFunctions 5
const char* mathsFuncs[] = { "Calculator", "Unit Conv", "Base Conv", "Graph Plot", "Prime Fact" };
int selectedMathsFunction = 1;

void calculator();
void unitConverter();
void baseConverter();
void graphPlotter();
void primeFactorisation();

void maths(void) {
  int lastSel = -1;
  while (true) {
    if (lastSel != selectedMathsFunction) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(50, 18);
      display.print("MATHS");
      int y0 = 54, dy = 28;
      for (int i = 0; i < totalMathsFunctions; ++i) {
        display.setTextSize(i + 1 == selectedMathsFunction ? 2 : 1);
        display.setTextColor(i + 1 == selectedMathsFunction ? colour6 : colour1);
        display.setCursor(32, y0 + i * dy);
        display.print(mathsFuncs[i]);
      }
      lastSel = selectedMathsFunction;
    }
    if (button_is_pressed(btn2, true)) {
      selectedMathsFunction++;
      if (selectedMathsFunction > totalMathsFunctions) selectedMathsFunction = 1;
    } else if (button_is_pressed(btn1, true)) {
      selectedMathsFunction--;
      if (selectedMathsFunction < 1) selectedMathsFunction = totalMathsFunctions;
    } else if (button_is_pressed(btn6, true)) {
      display.fillScreen(colourBG);
      return;
    } else if (button_is_pressed(btn3, true)) {
      switch (selectedMathsFunction) {
        case 1: calculator(); break;
        case 2: unitConverter(); break;
        case 3: baseConverter(); break;
        case 4: graphPlotter(); break;
        case 5: primeFactorisation(); break;
      }
      lastSel = -1;
      continue;
    }
    delay(40);
  }
}

int prevMenuDrawn = 0;
void calculator() {
  const char* keys[] = {
    "7", "8", "9", "/", "(", ")",
    "4", "5", "6", "*", "^", "%",
    "1", "2", "3", "-", "sin", "cos",
    "0", ".", "pi", "+", "tan", "sqrt"
  };
  int sz = 24;
  int sel = 0, lastSel = -1;
  char expr[64] = "";
  int exprLen = 0;
  bool showRes = false, error = false;
  double result = 0;
  while (true) {
    static char prevExp[64] = "";
    static bool prev_showRes = false;
    static int lastMenuDrawn = -1;
    if (lastSel != sel || strcmp(expr, prevExp) || showRes != prev_showRes || lastMenuDrawn != 0) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(22, 10);
      display.print("CALCULATOR");
      display.setTextSize(1);
      display.setTextColor(colourText);
      display.fillRect(18, 34, 186, 16, colourBG);
      display.setCursor(20, 38);
      display.print(expr);
      prevMenuDrawn = 0;
      strcpy(prevExp, expr);
      prev_showRes = showRes;
    }

    int baseX = 22, baseY = 58, w = 36, h = 18;
    for (int i = 0; i < sz; ++i) {
      int x = baseX + (i % 6) * w;
      int y = baseY + (i / 6) * h;
      display.setTextSize(1);
      if (i == sel) {
        display.fillRect(x - 2, y - 2, w - 1, h - 2, colourText);
        display.setTextColor(colourBG);
      } else display.setTextColor(colour6);
      display.setCursor(x, y);
      display.print(keys[i]);
      if (i == sel) display.setTextColor(colour6);
    }
    if (showRes) {
      display.fillRect(20, 190, 180, 20, colourBG);
      display.setCursor(26, 194);
      display.setTextSize(2);
      if (error) display.setTextColor(colour2), display.print(" Error!");
      else display.setTextColor(colour3), display.print("= "), display.print(result, 8);
      display.setTextSize(1);
    }

    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % sz;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel - 1 + sz) % sz;
    } else if (button_is_pressed(btn3, true)) {
      if (exprLen < 63) {
        if (strcmp(keys[sel], "pi") == 0) {
          strcat(expr, "pi");
          exprLen += 2;
        } else if (strcmp(keys[sel], "sin") == 0) {
          strcat(expr, "sin(");
          exprLen += 4;
        } else if (strcmp(keys[sel], "cos") == 0) {
          strcat(expr, "cos(");
          exprLen += 4;
        } else if (strcmp(keys[sel], "tan") == 0) {
          strcat(expr, "tan(");
          exprLen += 4;
        } else if (strcmp(keys[sel], "sqrt") == 0) {
          strcat(expr, "sqrt(");
          exprLen += 5;
        } else strcat(expr, keys[sel]), exprLen = strlen(expr);
      }
    } else if (button_is_pressed(btn4, true)) {
      int l = strlen(expr);
      if (l > 0) {
        if (l >= 3 && !strncmp(expr + l - 4, "sin(", 4)) expr[l - 4] = '\0';
        else if (l >= 4 && !strncmp(expr + l - 5, "sqrt(", 5)) expr[l - 5] = '\0';
        else if (l >= 4 && !strncmp(expr + l - 4, "cos(", 4)) expr[l - 4] = '\0';
        else if (l >= 4 && !strncmp(expr + l - 4, "tan(", 4)) expr[l - 4] = '\0';
        else if (l >= 2 && !strncmp(expr + l - 2, "pi", 2)) expr[l - 2] = '\0';
        else expr[l - 1] = '\0';
        exprLen = strlen(expr);
      }
    } else if (button_is_pressed(btn5, true)) {
      expr[0] = '\0';
      exprLen = 0;
      showRes = false;
    } else if (button_is_pressed(btn6, true)) {
      int err;
      te_variable vars[] = {};
      te_expr* te = te_compile(expr, vars, 0, &err);
      if (te) {
        result = te_eval(te);
        te_free(te);
        showRes = true;
        error = false;
      } else {
        showRes = true;
        error = true;
      }
    } else if (button_is_pressed(btn6, false) && showRes) return;
    lastSel = sel;
    delay(40);
  }
}

void unitConverter() {
  const char* types[] = {
    "cm->in", "in->cm", "C->F", "F->C", "kg->lb", "lb->kg",
    "km->mi", "mi->km", "g->oz", "oz->g", "L->gal", "gal->L"
  };
  int typeCount = 12, sel = 0;
  float value = 0, result = 0, lastVal = 99999;
  bool entering = true, first = true;
  while (true) {
    if (first || value != lastVal || entering) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(54, 18);
      display.print("UNITS");
      display.setTextSize(1);
      for (int i = 0; i < typeCount; ++i) {
        display.setTextColor(i == sel ? colour4 : colour1);
        display.setCursor(30, 48 + i * 15);
        display.print(types[i]);
      }
      display.setTextColor(colourText);
      display.setCursor(34, 48 + typeCount * 15);
      display.print("Value: ");
      display.setTextSize(2);
      display.setCursor(134, 48 + typeCount * 15 - 4);
      display.print(value, 2);
      if (!entering) {
        display.setTextColor(colour3);
        display.setCursor(34, 48 + typeCount * 15 + 22);
        display.print("= ");
        display.print(result, 4);
      }
      first = false;
      lastVal = value;
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % typeCount;
      entering = true;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel - 1 + typeCount) % typeCount;
      entering = true;
    } else if (button_is_pressed(btn4, true)) {
      if (entering) value -= 1;
    } else if (button_is_pressed(btn5, true)) {
      if (entering) value += 1;
    } else if (button_is_pressed(btn3, true)) {
      if (entering) value += 0.1;
    } else if (button_is_pressed(btn6, true)) {
      if (entering) {
        switch (sel) {
          case 0: result = value / 2.54; break;
          case 1: result = value * 2.54; break;
          case 2: result = value * 9.0 / 5.0 + 32.0; break;
          case 3: result = (value - 32.0) * 5.0 / 9.0; break;
          case 4: result = value * 2.20462; break;
          case 5: result = value / 2.20462; break;
          case 6: result = value * 0.621371; break;
          case 7: result = value / 0.621371; break;
          case 8: result = value * 0.035274; break;
          case 9: result = value / 0.035274; break;
          case 10: result = value * 0.264172; break;
          case 11: result = value / 0.264172; break;
        }
        entering = false;
        first = true;
      } else return;
    }
    delay(40);
  }
}

const char* baseCharsets[] = {
  "", "", "01", "012", "0123", "01234", "012345", "0123456", "01234567", "012345678",
  "0123456789", "0123456789A", "0123456789AB", "0123456789ABC", "0123456789ABCD",
  "0123456789ABCDE", "0123456789ABCDEF"
};

void baseConverter() {
  int sourceBase = 10, targetBase = 16, sIdx = 10, tIdx = 16, sel = 0;
  char inputNum[MAX_NUMBER_LENGTH] = "";
  int inputLen = 0;
  bool inputting = true;
  int lastSource = -1, lastTarget = -1, redrawAll = 1;

  while (true) {
    if (redrawAll || sourceBase != lastSource || targetBase != lastTarget) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(48, 14);
      display.print("BASE CONV");
      display.setTextSize(1);
      display.setTextColor(colourText);
      display.setCursor(28, 46);
      display.print("From:");
      display.setTextColor(colour4);
      display.print(sourceBase);
      display.setTextColor(colourText);
      display.print(" To:");
      display.setTextColor(colour4);
      display.print(targetBase);
      display.setTextColor(colourText);
      display.setCursor(28, 64);
      display.print("Input: ");
      display.print(inputNum);
      if (inputting) display.print("_");
      display.setCursor(28, 80);
      display.print("3:Base  4:Enter/Conv   5:Clear  6:Back");

      lastSource = sourceBase;
      lastTarget = targetBase;
      redrawAll = 0;
    }

    if (button_is_pressed(btn3, true)) {
      int idx = 0, choosing = 0;
      int tempSource = sourceBase, tempTarget = targetBase;
      while (1) {
        display.fillRect(0, 110, 240, 60, colourBG);
        display.setTextSize(1);
        display.setCursor(38, 115);
        display.setTextColor(choosing ? colourText : colour6);
        display.print("Source Base: ");
        display.setTextColor(colour4);
        display.print(tempSource);
        display.setTextColor(colourText);
        display.setCursor(38, 135);
        display.setTextColor(choosing ? colour6 : colourText);
        display.print("Target Base: ");
        display.setTextColor(!choosing ? colour4 : colourText);
        display.print(tempTarget);
        display.setTextColor(colourText);
        display.setCursor(38, 160);
        display.print("btn1/2:-/+  btn4:Switch  btn6:OK");
        if (button_is_pressed(btn1, true)) {
          if (choosing) tempTarget--;
          else tempSource--;
          if (tempSource < 2) tempSource = 2;
          if (tempTarget < 2) tempTarget = 2;
        } else if (button_is_pressed(btn2, true)) {
          if (choosing) tempTarget++;
          else tempSource++;
          if (tempSource > 16) tempSource = 16;
          if (tempTarget > 16) tempTarget = 16;
        } else if (button_is_pressed(btn4, true)) {
          choosing = !choosing;
        } else if (button_is_pressed(btn6, true)) {
          sourceBase = tempSource;
          targetBase = tempTarget;
          redrawAll = 1;
          break;
        }
        delay(80);
      }
    } else if (button_is_pressed(btn4, true)) {
      const char* charset = baseCharsets[sourceBase];
      int clen = strlen(charset);
      int idx = 0;
      while (1) {
        display.fillRect(24, 105, 192, 24, colourBG);
        display.setTextSize(2);
        display.setTextColor(colour4);
        display.setCursor(120 - 8, 112);
        display.print(charset[idx]);
        display.setTextSize(1);
        display.setCursor(60, 134);
        display.setTextColor(colourText);
        display.print("1/2:Prev/Next 3:Add 6:Done");
        if (button_is_pressed(btn1, true)) {
          idx = (idx - 1 + clen) % clen;
        } else if (button_is_pressed(btn2, true)) {
          idx = (idx + 1) % clen;
        } else if (button_is_pressed(btn3, true)) {
          if (inputLen < MAX_NUMBER_LENGTH - 1) {
            inputNum[inputLen++] = charset[idx];
            inputNum[inputLen] = '\0';
          }
        } else if (button_is_pressed(btn6, true)) break;
        delay(90);
      }
      redrawAll = 1;
    } else if (button_is_pressed(btn5, true)) {
      inputLen = 0;
      inputNum[0] = '\0';
      redrawAll = 1;
    } else if (button_is_pressed(btn6, true)) return;
    else if (!inputting && button_is_pressed(btn4)) {
      inputting = true;
      redrawAll = 1;
    } else if (button_is_pressed(btn4, true) || button_is_pressed(btn6, true)) break;
    
    if (inputLen > 0 && button_is_pressed(btn4, true)) {
      unsigned long val = 0;
      for (int i = 0; i < inputLen; ++i) {
        char c = toupper(inputNum[i]);
        val *= sourceBase;
        if (c >= '0' && c <= '9') val += c - '0';
        else if (c >= 'A' && c <= 'F') val += 10 + c - 'A';
      }
      char buf[40] = "";
      int idx = 0;
      unsigned long v = val;
      if (v == 0) buf[idx++] = '0';
      else
        while (v > 0 && idx < MAX_NUMBER_LENGTH - 1) {
          int rem = v % targetBase;
          buf[idx++] = baseCharsets[targetBase][rem];
          v /= targetBase;
        }
      for (int i = 0; i < idx / 2; i++) {
        char t = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = t;
      }
      buf[idx] = '\0';
      display.fillRect(34, 104, 160, 32, colourBG);
      display.setTextColor(colour3);
      display.setCursor(34, 113);
      display.print("Result: ");
      display.print(buf);
      inputting = false;
      delay(1000);
    }
    delay(50);
  }
}

void graphPlotter() {
  char eqn[32] = "";
  int sel = 0;
  const char* keys[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "x", "+", "-", "*", "/", "^", "(", ")", "sin", "cos", "tan", "sqrt", "mod", "%" };
  int sz = 25;
  int lastSel = -1, eqLast = -1;
  while (true) {
    if (lastSel != sel || eqLast != strlen(eqn)) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(85, 17);
      display.print("GRAPH");
      display.setTextSize(1);
      display.setTextColor(colourText);
      display.setCursor(25, 45);
      display.print("y=");
      display.print(eqn);
      int bx = 32, by = 65, bw = 26, bh = 16;
      for (int i = 0; i < sz; ++i) {
        int x = bx + (i % 6) * bw, y = by + (i / 6) * bh;
        display.setTextSize(1);
        if (i == sel) {
          display.setTextColor(colourBG);
          display.fillRect(x - 1, y - 2, bw - 1, bh - 2, colourText);
        } else display.setTextColor(colour6);
        display.setCursor(x, y);
        display.print(keys[i]);
        if (i == sel) display.setTextColor(colour6);
      }
      lastSel = sel;
      eqLast = strlen(eqn);
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % sz;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel - 1 + sz) % sz;
    } else if (button_is_pressed(btn3, true)) {
      if (strlen(eqn) < 30) {
        strcat(eqn, keys[sel]);
      }
    } else if (button_is_pressed(btn4, true)) {
      int l = strlen(eqn);
      if (l > 0) eqn[l - 1] = '\0';
    } else if (button_is_pressed(btn5, true)) {
      eqn[0] = '\0';
    } else if (button_is_pressed(btn6, true)) {
      display.fillCircle(120, 120, 119, colourBG);
      display.setTextColor(colour1);
      int16_t ymid = 120, xmid = 120;
      display.drawLine(24, ymid, 216, ymid, colour1);
      display.drawLine(xmid, 24, xmid, 216, colour1);
      for (int px = 24; px < 216; px++) {
        double x = -10 + 20.0 * (px - 24) / (216 - 24);
        char buf[128] = "";
        int p = 0;
        for (unsigned j = 0; eqn[j] != '\0' && p < 120; j++)
          if (eqn[j] == 'x') p += sprintf(buf + p, "(%.4f)", x);
          else buf[p++] = eqn[j], buf[p] = '\0';
        int err;
        te_variable v[] = {};
        te_expr* te = te_compile(buf, v, 0, &err);
        if (!te) continue;
        double y = te_eval(te);
        te_free(te);
        if (isnan(y) || isinf(y)) continue;
        int py = ymid - (int)(y * 9.0);
        if (py < 24 || py > 216) continue;
        display.drawPixel(px, py, colour6);
      }
      delay(1500);
      return;
    }
    delay(40);
  }
}

void primeFactorisation() {
  char inputBuffer[32] = "";
  int inputLen = 0, sel = 0, lastLen = -1;
  char resultBuffer[64] = "";
  bool showRes = false;
  const char* digits[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  int digitCount = 10;
  while (true) {
    if (inputLen != lastLen || !showRes) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(62, 18);
      display.print("PRIME");
      display.setTextSize(1);
      display.setTextColor(colour6);
      display.setCursor(38, 60);
      display.print("Input: ");
      display.print(inputBuffer);
      if (showRes) {
        display.setCursor(38, 85);
        display.setTextColor(colour3);
        display.print(resultBuffer);
      }
      lastLen = inputLen;
    }
    int bx = 38, by = 110, bw = 23, bh = 16;
    for (int i = 0; i < digitCount; i++) {
      display.setTextSize(1);
      if (i == sel) {
        display.setTextColor(colourBG);
        display.fillRect(bx + (i % 5) * bw - 2, by + (i / 5) * bh - 2, bw - 1, bh - 2, colourText);
      } else display.setTextColor(colour6);
      display.setCursor(bx + (i % 5) * bw, by + (i / 5) * bh);
      display.print(digits[i]);
      if (i == sel) display.setTextColor(colour6);
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % digitCount;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel - 1 + digitCount) % digitCount;
    } else if (button_is_pressed(btn3, true)) {
      if (inputLen < 31) {
        inputBuffer[inputLen++] = digits[sel][0];
        inputBuffer[inputLen] = '\0';
        showRes = false;
      }
    } else if (button_is_pressed(btn4, true)) {
      if (inputLen > 0) {
        inputBuffer[--inputLen] = '\0';
        showRes = false;
      }
    } else if (button_is_pressed(btn5, true)) {
      if (inputLen > 0) {
        unsigned long n = atol(inputBuffer);
        unsigned long temp = n;
        int rlen = 0;
        resultBuffer[0] = '\0';
        bool first = true;
        for (unsigned long f = 2; f <= temp && temp > 1; ++f)
          if (temp % f == 0) {
            int c = 0;
            while (temp % f == 0) c++, temp /= f;
            if (!first) resultBuffer[rlen++] = '*';
            char buf[10];
            sprintf(buf, "%lu", f);
            for (int i = 0; buf[i] != '\0'; ++i) resultBuffer[rlen++] = buf[i];
            if (c > 1) {
              resultBuffer[rlen++] = '^';
              sprintf(buf, "%d", c);
              for (int i = 0; buf[i] != '\0'; ++i) resultBuffer[rlen++] = buf[i];
            }
            first = false;
          }
        resultBuffer[rlen] = '\0';
        showRes = true;
      }
    } else if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}
