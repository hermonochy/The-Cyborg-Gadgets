#include <TFT_eSPI.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

TFT_eSPI display = TFT_eSPI();
TFT_eSprite canvas = TFT_eSprite(&display);

// pins of buttons.
const int btn1 = 4;
const int btn2 = 5;
const int btn3 = 6;
const int btn4 = 13;
const int btn5 = 8;
const int btn6 = 7;

// when touched, reading rises to low 6 digits. when not touched, remains in low 5 digits.
const int threshold = 100000;

bool button_is_pressed(int btn, bool onlyOnce = false) {
  if(touchRead(btn) >= threshold){
    if (onlyOnce) {
      while(touchRead(btn) >= threshold) delay(50);
    }
    return true;
  }
  return false;
}

void drawMainUI() {
  const int cx = SCREEN_WIDTH / 2;
  const int cy = SCREEN_HEIGHT / 2;
  const int SAFE_MARGIN = 36;

  const uint16_t BG = display.color565(10, 10, 12);
  const uint16_t VIGNETTE = display.color565(14, 14, 16);
  const uint16_t CARD_BG = display.color565(22, 24, 28);
  const uint16_t OUTLINE = display.color565(44, 48, 54);
  const uint16_t ACCENT = display.color565(0, 160, 220);
  const uint16_t LIGHT = display.color565(240, 240, 245);

  const char *title = "Watch 5.0";
  const char *func = "func";

  canvas.fillSprite(BG);
  canvas.fillCircle(cx, cy, SCREEN_WIDTH / 2, VIGNETTE);

  int left = SAFE_MARGIN;
  int right = SCREEN_WIDTH - SAFE_MARGIN;
  canvas.fillRoundRect(left + 6, SAFE_MARGIN - 8, (right - left) - 12, 36, 10, CARD_BG);
  canvas.setTextColor(LIGHT, CARD_BG);
  canvas.setTextSize(1);
  canvas.setTextDatum(TL_DATUM);
  canvas.drawString(title, left + 16, SAFE_MARGIN - 6);

  int cardW = min((right - left) - 20, 200);
  int cardH = 84;
  int cardX = cx - cardW / 2;
  int cardY = cy - cardH / 2 - 6;
  canvas.fillRoundRect(cardX, cardY, cardW, cardH, 14, CARD_BG);
  canvas.drawRoundRect(cardX, cardY, cardW, cardH, 14, OUTLINE);

  canvas.setTextSize(2);
  canvas.setTextColor(LIGHT, CARD_BG);
  canvas.setTextDatum(TL_DATUM);
  canvas.drawString(func, cardX + 14, cardY + 18);

  int pillW = 40, pillH = 28;
  int pillX = cardX + cardW - pillW - 12;
  int pillY = cardY + 10;
  canvas.fillRoundRect(pillX, pillY, pillW, pillH, 8, ACCENT);
  canvas.setTextColor(TFT_WHITE, ACCENT);
  canvas.setTextSize(2);
  canvas.drawString("1", pillX + 10, pillY + 2);

  int barW = (right - left) - 24;
  int barX = left + 12;
  int barY = SCREEN_HEIGHT - SAFE_MARGIN - 36;
  canvas.drawRoundRect(barX, barY, barW, 18, 8, OUTLINE);
  canvas.fillRoundRect(barX + 2, barY + 2, barW / 4, 14, 6, ACCENT);

  canvas.setTextSize(1);
  canvas.setTextColor(display.color565(170, 170, 180), BG);
  canvas.setTextDatum(MC_DATUM);
  canvas.drawString("< SEL >", cx, SCREEN_HEIGHT - SAFE_MARGIN + 6);

  canvas.pushSprite(0, 0);
}

void setup() {
  display.init();
  display.setRotation(2);

  canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  canvas.setTextDatum(TL_DATUM);

  drawMainUI();
}

void loop() {
  delay(1000);
}