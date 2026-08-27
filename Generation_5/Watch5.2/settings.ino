void touchDebug() {
  while (a_button_is_pressed()) delay(10);

  const int cx = SCREEN_WIDTH / 2;
  const int cy = SCREEN_HEIGHT / 2;
  const int outerR = SCREEN_WIDTH / 2;

  const uint16_t BG = display.color565(8, 8, 10);
  const uint16_t RIM = display.color565(14, 14, 16);
  const uint16_t CARD = display.color565(10, 10, 12);
  const uint16_t TIT = display.color565(235, 235, 240);
  const uint16_t VAL = display.color565(200, 200, 200);

  while (true) {
    canvas.fillSprite(BG);
    canvas.fillCircle(cx, cy, outerR, RIM);
    canvas.fillCircle(cx, cy, outerR - 6, CARD);

    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(2);
    canvas.setTextColor(TIT, CARD);
    canvas.drawString("TOUCH DEBUG", cx, 24);

    canvas.setTextSize(1);
    //canvas.setTextDatum(LC_DATUM);
    for (int i = 0; i < 6; ++i) {
      int tr = touchRead(buttons[i]);
      char buf[32];
      snprintf(buf, sizeof(buf), "Btn%d (pin %d): %d", i + 1, buttons[i], tr);
      int y = 48 + i * 18;
      canvas.drawString(buf, 18, y);
    }

    char tbuf[32];
    snprintf(tbuf, sizeof(tbuf), "Threshold: %d", threshold);
    //canvas.setTextDatum(RC_DATUM);
    canvas.drawString(tbuf, SCREEN_WIDTH - 18, SCREEN_HEIGHT - 22);

    canvas.pushSprite(0, 0);

    if (button_is_pressed(buttons[5], true)) return;
    delay(5);
  }
}