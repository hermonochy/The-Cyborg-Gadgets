void activateFunc(byte func, unsigned long blinkTime = 500000UL) {
  bool blink = false;
  bool keepOn = false;
  bool outputOn = digitalRead(func);
  unsigned long lastToggle = micros();

  const unsigned long MIN_US = 1UL;
  const unsigned long MAX_US = 10000000UL;
  if (blinkTime < MIN_US) blinkTime = MIN_US;
  if (blinkTime > MAX_US) blinkTime = MAX_US;

  auto formatBlink = [&](char *buf, size_t len, unsigned long us) {
    if (us < 1000) {
      snprintf(buf, len, "%lu\xC2\xB5s", us);  
    } else if (us < 1000000UL) {
      snprintf(buf, len, "%lums", us / 1000UL);
    } else {
      unsigned long tenths = (us + 50000UL) / 100000UL;  
      snprintf(buf, len, "%lu.%lus", tenths / 10, tenths % 10);
    }
  };

  while (a_button_is_pressed()) delay(10);

  unsigned long lastDraw = 0;
  while (true) {
    unsigned long nowMs = millis();
    unsigned long nowUs = micros();

    if (keepOn) {
      outputOn = true;
      digitalWrite(func, HIGH);
    } else if (blink) {
      if ((nowUs - lastToggle) >= blinkTime) {
        outputOn = !outputOn;
        digitalWrite(func, outputOn ? HIGH : LOW);
        lastToggle = nowUs;
      }
    } else {
      if (button_is_pressed(buttons[0])) {
        digitalWrite(func, HIGH);
        outputOn = true;
      } else {
        digitalWrite(func, LOW);
        outputOn = false;
      }
    }
    if (button_is_pressed(buttons[1], true)) {
      keepOn = !keepOn;
      if (keepOn) blink = false;
    }
    if (button_is_pressed(buttons[2], true)) {
      blink = !blink;
      if (blink) keepOn = false;
      lastToggle = micros();
    }
    if (button_is_pressed(buttons[3], true)) {  
      if (blinkTime > MIN_US) {
        blinkTime = max(MIN_US, blinkTime / 2UL);
      }
    }
    if (button_is_pressed(buttons[4], true)) {  
      if (blinkTime < MAX_US) {
        unsigned long next = blinkTime * 2UL;
        blinkTime = next > MAX_US ? MAX_US : next;
      }
    }
    if (button_is_pressed(buttons[5], true)) {  
      return;
    }

    if ((nowMs - lastDraw) > 60) {
      lastDraw = nowMs;

      const int cx = SCREEN_WIDTH / 2;
      const int cy = SCREEN_HEIGHT / 2;
      const uint16_t BG = display.color565(12, 12, 14);
      const uint16_t PLATE = display.color565(24, 26, 28);
      const uint16_t ACCENT = display.color565(0, 160, 220);
      const uint16_t ONCOL = display.color565(80, 220, 140);
      const uint16_t OFFCOL = display.color565(90, 92, 96);
      const uint16_t TXT = display.color565(235, 235, 240);

      canvas.fillSprite(BG);
      canvas.fillCircle(cx, cy, SCREEN_WIDTH / 2, display.color565(14, 14, 16));
      canvas.fillCircle(cx, cy, SCREEN_WIDTH / 2 - 6, BG);

      canvas.fillCircle(cx, cy - 6, 86, PLATE);
      canvas.drawCircle(cx, cy - 6, 86, display.color565(46, 48, 52));

      canvas.setTextDatum(MC_DATUM);
      canvas.setTextSize(4);
      canvas.setTextColor(TXT, PLATE);
      canvas.drawString(outputOn ? "ON" : "OFF", cx, cy - 18);

      canvas.setTextSize(1);
      canvas.setTextColor(outputOn ? ONCOL : OFFCOL, PLATE);
      if (keepOn) canvas.drawString("ALWAYS", cx, cy + 18);
      else if (blink) canvas.drawString("BLINK", cx, cy + 18);
      else canvas.drawString("MANUAL", cx, cy + 18);

      if (blink) {
        char tb[24];
        formatBlink(tb, sizeof(tb), blinkTime);
        canvas.setTextSize(1);
        canvas.setTextColor(TXT, PLATE);
        canvas.drawString(tb, cx, cy + 36);
      }

      canvas.setTextSize(1);
      canvas.setTextDatum(MC_DATUM);
      canvas.setTextColor(display.color565(160, 160, 170), BG);
      canvas.drawString("< Bck  Kp  Blk  <<  >> >", cx, SCREEN_HEIGHT - 12);

      canvas.pushSprite(0, 0);
    }
  }
}

void watchFuncs(void) {
  const byte pins[] = { Func1, Func2 };
  const char *labels[] = { "LED", "Laser" };  
  const int count = sizeof(pins) / sizeof(pins[0]);

  int idx = 0;
  unsigned long lastDraw = 0;

  while (true) {
    unsigned long now = millis();

    if (now - lastDraw > 60) {
      lastDraw = now;

      const int cx = SCREEN_WIDTH / 2;
      const uint16_t BG = display.color565(12, 12, 14);
      const uint16_t CARD = display.color565(22, 24, 28);
      const uint16_t ACCENT = display.color565(0, 160, 220);
      const uint16_t TXT = display.color565(235, 235, 240);
      const uint16_t MUTED = display.color565(140, 144, 150);

      canvas.fillSprite(BG);
      canvas.fillCircle(cx, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, display.color565(14, 14, 16));
      canvas.fillCircle(cx, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 6, BG);

      canvas.setTextDatum(TC_DATUM);
      canvas.setTextSize(2);
      canvas.setTextColor(TXT, CARD);
      canvas.drawString("Outputs", cx, 18);

      canvas.setTextDatum(MC_DATUM);
      canvas.setTextSize(3);
      canvas.setTextColor(ACCENT, CARD);
      canvas.drawString(labels[idx], cx, SCREEN_HEIGHT / 2 - 6);

      bool on = digitalRead(pins[idx]);
      canvas.setTextSize(1);
      canvas.setTextColor(on ? display.color565(80, 220, 140) : MUTED, CARD);
      canvas.drawString(on ? "ON" : "OFF", cx, SCREEN_HEIGHT / 2 + 28);

      canvas.setTextSize(1);
      canvas.setTextColor(TXT, BG);
      canvas.drawString(labels[(idx - 1 + count) % count], cx - 64, SCREEN_HEIGHT / 2 - 6);
      canvas.drawString(labels[(idx + 1) % count], cx + 64, SCREEN_HEIGHT / 2 - 6);

      canvas.setTextSize(1);
      canvas.setTextDatum(MC_DATUM);
      canvas.setTextColor(display.color565(160, 160, 170), BG);
      canvas.drawString("< Prev   Sel   Next >", cx, SCREEN_HEIGHT - 12);

      canvas.pushSprite(0, 0);
    }

    if (button_is_pressed(buttons[4])) {
      idx = (idx + 1) % count;
      delay(160);
    } else if (button_is_pressed(buttons[3])) {  
      idx = (idx - 1 + count) % count;
      delay(160);
    } else if (button_is_pressed(buttons[5], true)) {  
      unsigned long bt = 500000UL;
      if (pins[idx] == Func1) bt = (unsigned long)blinkTime1;
      else if (pins[idx] == Func2) bt = (unsigned long)blinkTime2;
      activateFunc(pins[idx], bt);
      if (pins[idx] == Func1) blinkTime1 = (int)min((unsigned long)INT32_MAX, bt);
      else if (pins[idx] == Func2) blinkTime2 = (int)min((unsigned long)INT32_MAX, bt);
    } else if (button_is_pressed(buttons[2], true)) {  
      return;
    }

    delay(20);
  }
}