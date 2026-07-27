void timeMenu() {
  while (a_button_is_pressed()) delay(10);

  const int cx = SCREEN_WIDTH / 2;
  const int cy = SCREEN_HEIGHT / 2;

  unsigned long lastDraw = 0;
  int lastSec = -1;

  while (true) {
    unsigned long nowMs = millis();
    if (nowMs - lastDraw >= 150) {
      time_t t = time(nullptr);
      struct tm *tm = localtime(&t);

      int hh = tm ? tm->tm_hour : 0;
      int mm = tm ? tm->tm_min : 0;
      int ss = tm ? tm->tm_sec : 0;

      if (ss != lastSec || (nowMs - lastDraw) > 1000) {
        lastSec = ss;

        canvas.fillSprite(display.color565(10, 10, 12));
        canvas.fillCircle(cx, cy, SCREEN_WIDTH / 2, display.color565(14, 14, 16));
        canvas.fillCircle(cx, cy, SCREEN_WIDTH / 2 - 6, display.color565(12, 12, 14));

        // Increased central plate radius so seconds can fit without overlapping
        canvas.fillCircle(cx, cy, 90, display.color565(22, 24, 28));
        canvas.drawCircle(cx, cy, 90, display.color565(56, 60, 66));

        char timeBuf[16];
        if ((ss & 1) == 0) 
          snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hh, mm);
        else
          snprintf(timeBuf, sizeof(timeBuf), "%02d %02d", hh, mm);

        const int mainTextSize = 5;
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextSize(mainTextSize);
        canvas.setTextColor(display.color565(240, 240, 245), display.color565(22, 24, 28));
        canvas.drawString(timeBuf, cx, cy - 6);

        int chars = (int)strlen(timeBuf);
        int approxCharW = 6 * mainTextSize;
        int mainWidth = approxCharW * chars;
        int secX = cx + (mainWidth / 2) + 8;

        char secBuf[8];
        snprintf(secBuf, sizeof(secBuf), "%02d", ss);
        canvas.setTextSize(2);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(display.color565(180, 180, 190), display.color565(22, 24, 28));
        canvas.drawString(secBuf, secX, cy - 6);

        char dateBuf[32];
        if (tm) strftime(dateBuf, sizeof(dateBuf), "%a %02d %b %Y", tm);
        else strcpy(dateBuf, "No RTC");
        canvas.setTextSize(1);
        canvas.setTextColor(display.color565(170, 170, 180), display.color565(22, 24, 28));
        canvas.setTextDatum(MC_DATUM);
        canvas.drawString(dateBuf, cx, cy + 48);

        int hintW = 160;
        int hintH = 28;
        int hintX = cx - hintW / 2;
        int hintY = SCREEN_HEIGHT - 34;
        canvas.fillRoundRect(hintX, hintY, hintW, hintH, 14, display.color565(28, 30, 34));
        canvas.drawRoundRect(hintX, hintY, hintW, hintH, 14, display.color565(56, 60, 66));
        canvas.setTextSize(1);
        canvas.setTextColor(display.color565(200,200,205), display.color565(28,30,34));
        canvas.setTextDatum(MC_DATUM);
        canvas.drawString("Back ◀   •   Set Time   •   Menu ▶", cx, hintY + hintH/2);

        canvas.pushSprite(0, 0);
        lastDraw = nowMs;
      }
    }

    if (button_is_pressed(buttons[5], true)) {
      return;
    }
    if (button_is_pressed(buttons[3], true)) {
      while (a_button_is_pressed()) delay(10);

      time_t tnow = time(nullptr);
      struct tm tmval;
      if (localtime_r(&tnow, &tmval) == NULL) {
        memset(&tmval, 0, sizeof(tmval));
        tmval.tm_year = 126; // 2026
        tmval.tm_mon = 0;
        tmval.tm_mday = 1;
      }

      int field = 0;
      bool done = false;
      unsigned long lastSetDraw = 0;

      while (!done) {
        unsigned long m = millis();
        if (m - lastSetDraw > 100) {
          // Match editor to the larger central plate
          canvas.fillCircle(cx, cy, 90, display.color565(20,22,24));
          canvas.drawCircle(cx, cy, 90, display.color565(56, 60, 66));
          canvas.setTextDatum(MC_DATUM);

          char editBuf[16];
          snprintf(editBuf, sizeof(editBuf), "%02d:%02d", tmval.tm_hour, tmval.tm_min);
          canvas.setTextSize(4);
          canvas.setTextColor(display.color565(0,160,220), display.color565(20,22,24));
          canvas.drawString(editBuf, cx, cy - 6);

          canvas.setTextSize(1);
          if (field == 0) {
            canvas.drawString("Edit: HOURS", cx, cy + 44);
          } else {
            canvas.drawString("Edit: MINUTES", cx, cy + 44);
          }

          canvas.pushSprite(0,0);
          lastSetDraw = m;
        }

        if (button_is_pressed(buttons[3])) { 
          if (field == 0) tmval.tm_hour = (tmval.tm_hour + 1) % 24;
          else tmval.tm_min = (tmval.tm_min + 1) % 60;
          delay(140);
        } else if (button_is_pressed(buttons[5])) {
          if (field == 0) tmval.tm_hour = (tmval.tm_hour + 23) % 24;
          else tmval.tm_min = (tmval.tm_min + 59) % 60;
          delay(140);
        } else if (button_is_pressed(buttons[0], true)) {
          field = (field + 1) % 2;
        } else if (button_is_pressed(buttons[4], true)) {
          struct tm tset = tmval;
          tset.tm_sec = 0;
          time_t newt = mktime(&tset);
          if (newt != (time_t)-1) {
            struct timeval tv = { .tv_sec = newt, .tv_usec = 0 };
            settimeofday(&tv, NULL);
          }
          done = true;
          while (a_button_is_pressed()) delay(10);
        }
        delay(20);
      }
    }

    delay(40);
  }
}