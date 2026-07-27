#include "tinyexpr.h"

void calculator() {
  const int BTN_PREV   = buttons[0];
  const int BTN_BS     = buttons[1];
  const int BTN_TOGGLE = buttons[2];
  const int BTN_NEXT   = buttons[3];
  const int BTN_SELECT = buttons[4];
  const int BTN_SOLVE  = buttons[5];

  const char *panelNums[] = {"1","2","3","4","5","6","7","8","9","0","."};
  const int panelNumsN = sizeof(panelNums)/sizeof(panelNums[0]);

  const char *panelOps[] = { "+", "-", "*", "/", "^", "%", "(",")", "," };
  const int panelOpsN = sizeof(panelOps)/sizeof(panelOps[0]);

  const char *panelFns[] = {
    "sin(", "cos(", "tan(", "asin(", "acos()", "atan(",
    "sqrt(", "fac(", "log(", "ln()", "pi", "e"
  };
  const int panelFnsN = sizeof(panelFns)/sizeof(panelFns[0]);

  int panel = 0;
  int cursor = 0;

  char expr[256] = "";
  double lastAns = 0.0;
  bool evaluated = false; // true when the current expr is the result of evaluation and not yet modified

  auto appendToken = [&](const char* tok){
    if (!tok) return;
    int cur = strlen(expr);
    int addLen = strlen(tok);
    if (cur + addLen < (int)sizeof(expr) - 1) {
      strncat(expr, tok, addLen);
    }
  };

  auto backspaceChar = [&](){
    int len = strlen(expr);
    if (len > 0) expr[len - 1] = '\0';
  };

  while (a_button_is_pressed()) delay(10);

  unsigned long lastDraw = 0;
  const unsigned long DRAW_MS = 80;

  while (true) {
    unsigned long now = millis();

    if (button_is_pressed(BTN_TOGGLE, true)) {
      panel = (panel + 1) % 3;
    } else if (button_is_pressed(BTN_NEXT, true)) {
      int maxc = (panel==0)?panelNumsN:(panel==1)?panelOpsN:panelFnsN;
      cursor = (cursor + 1) % maxc;
    } else if (button_is_pressed(BTN_PREV, true)) {
      int maxc = (panel==0)?panelNumsN:(panel==1)?panelOpsN:panelFnsN;
      cursor = (cursor - 1 + maxc) % maxc;
    } else if (button_is_pressed(BTN_BS, true)) {
      backspaceChar();
      evaluated = false;
      delay(120);
    }

    if (button_is_pressed(BTN_SELECT, true)) {
      const char *tok = NULL;
      if (panel == 0) tok = panelNums[cursor % panelNumsN];
      else if (panel == 1) tok = panelOps[cursor % panelOpsN];
      else tok = panelFns[cursor % panelFnsN];

      if (tok) {
        if (strcmp(tok, "acos()") == 0) appendToken("acos(");
        else if (strcmp(tok, "ln()") == 0) appendToken("ln(");
        else appendToken(tok);
        evaluated = false;
      }
      delay(120);
    }

    if (button_is_pressed(BTN_SOLVE, true)) {
      if (evaluated) {
        return;
      }
      if (strlen(expr) > 0) {
        int err = 0;
        double val = te_interp(expr, &err);
        if (err == 0 && !isnan(val)) {
          lastAns = val;
          char buf[128];
          if (fabs(val) < 1e12 && fabs(val - round(val)) < 1e-9) {
            snprintf(buf, sizeof(buf), "%.0f", val);
          } else {
            snprintf(buf, sizeof(buf), "%.10g", val);
          }
          strncpy(expr, buf, sizeof(expr)-1);
          expr[sizeof(expr)-1] = '\0';
          evaluated = true;
        } else {
          const int cx = SCREEN_WIDTH/2;
          const int cy = SCREEN_HEIGHT/2;
          canvas.fillSprite(display.color565(10,10,12));
          canvas.fillCircle(cx, cy, SCREEN_WIDTH/2, display.color565(14,14,16));
          canvas.setTextDatum(MC_DATUM);
          canvas.setTextSize(2);
          canvas.setTextColor(display.color565(255,10,10), display.color565(14,14,16));
          canvas.drawString("ERROR", cx, cy - 8);
          canvas.setTextSize(1);
          canvas.setTextColor(display.color565(200,200,200), display.color565(14,14,16));
          canvas.drawString("Parse error", cx, cy + 18);
          canvas.pushSprite(0,0);
          delay(800);
          evaluated = false;
        }
      }
      delay(150);
    }

    if (touchHeld(BTN_BS, 900)) {
      return;
    }

    if (now - lastDraw >= DRAW_MS) {
      lastDraw = now;

      const int cx = SCREEN_WIDTH/2;
      const int cy = SCREEN_HEIGHT/2;
      canvas.fillSprite(display.color565(8,8,10));
      canvas.fillCircle(cx, cy, SCREEN_WIDTH/2, display.color565(14,14,16));
      canvas.fillCircle(cx, cy, SCREEN_WIDTH/2 - 6, display.color565(10,10,12));

      canvas.setTextDatum(TL_DATUM);
      canvas.setTextSize(1);
      canvas.setTextColor(display.color565(200,200,200), display.color565(10,10,12));
      char exprShow[128];
      int elen = strlen(expr);
      if (elen <= 18) {
        strncpy(exprShow, expr, sizeof(exprShow)-1);
        exprShow[sizeof(exprShow)-1] = '\0';
      } else {
        strncpy(exprShow, expr + max(0, elen - 18), sizeof(exprShow)-1);
        exprShow[0] = '.'; exprShow[1] = '.'; exprShow[2] = '.';
        exprShow[sizeof(exprShow)-1] = '\0';
      }
      canvas.drawString(exprShow, 12, 12);

      canvas.fillCircle(cx, cy - 6, 68, display.color565(22,24,28));
      canvas.drawCircle(cx, cy - 6, 68, display.color565(46,48,52));

      canvas.setTextDatum(MC_DATUM);
      char centerBuf[32];
      int centerLen = strlen(expr);
      if (centerLen == 0) strcpy(centerBuf, "0");
      else {
        const char *tail = (centerLen > 10) ? expr + (centerLen - 10) : expr;
        strncpy(centerBuf, tail, sizeof(centerBuf)-1);
        centerBuf[sizeof(centerBuf)-1] = '\0';
      }

      int plateR = 68;
      int maxTextW = plateR * 2 - 18;
      int textSize = 2;
      canvas.setTextSize(textSize);
      int w = canvas.textWidth(centerBuf);
      while (w > maxTextW && textSize > 1) {
        textSize--;
        canvas.setTextSize(textSize);
        w = canvas.textWidth(centerBuf);
      }
      canvas.setTextColor(display.color565(235,235,240), display.color565(22,24,28));
      canvas.drawString(centerBuf, cx, cy - 6);

      const char **panelItems;
      int panelN;
      if (panel == 0) { panelItems = panelNums; panelN = panelNumsN; }
      else if (panel == 1) { panelItems = panelOps; panelN = panelOpsN; }
      else { panelItems = panelFns; panelN = panelFnsN; }

      int displayCount = 5;
      int start = cursor - displayCount/2;
      if (displayCount > panelN) displayCount = panelN;
      int pillW = 40;
      int gap = 6;
      int totalW = displayCount * pillW + (displayCount - 1) * gap;
      int startX = cx - totalW/2;
      int y = SCREEN_HEIGHT - 58;

      for (int i = 0; i < displayCount; i++) {
        int pos = (start + i) % panelN;
        if (pos < 0) pos += panelN;
        int x = startX + i * (pillW + gap);
        bool isCursor = (pos == cursor);
        uint16_t bg = isCursor ? display.color565(0,160,220) : display.color565(28,30,34);
        uint16_t fg = isCursor ? display.color565(10,10,12) : display.color565(200,200,205);
        canvas.fillRoundRect(x, y, pillW, 30, 8, bg);
        canvas.drawRoundRect(x, y, pillW, 30, 8, display.color565(56,60,66));
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextSize(1);
        char shortLab[12];
        strncpy(shortLab, panelItems[pos], sizeof(shortLab)-1);
        shortLab[sizeof(shortLab)-1]='\0';
        canvas.setTextColor(fg, bg);
        canvas.drawString(shortLab, x + pillW/2, y + 15);
      }

      canvas.setTextSize(1);
      canvas.setTextDatum(MC_DATUM);
      const char *panelNames[3] = { "NUM", "OP", "FNC" };
      canvas.setTextColor(display.color565(170,170,180), display.color565(10,10,12));
      canvas.drawString(panelNames[panel], cx, SCREEN_HEIGHT - 12);

      canvas.pushSprite(0,0);
    }

    delay(8);
  }
}