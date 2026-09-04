// Watch 5.2: first watch with casing. uses touch.
// NOTE: will NOT work with a C3. Requires an S3.

#include <TFT_eSPI.h>
#include <MacRandomizer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <esp_wifi.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

TFT_eSPI display = TFT_eSPI();
TFT_eSprite canvas = TFT_eSprite(&display);

MacRandomizer macRandom;

Preferences preferences;

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};
WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;
int currentWiFiIndex = 0;

const char *Functions[] = {"Time", "GPIO","Maths", "Random", "Score", "Games", "Metronome", "Notes", "Calendar", "WiFi Setup", "WiFi Funcs","Shell", "Settings", "Sleep"};
const int totalFunctions = sizeof(Functions) / sizeof(Functions[0]);

int selectedFunction = 0;

const int buttons[] = {7, 8, 13, 6, 5, 4};

const int threshold = 50000;

byte Func1 = 2;
byte Func2 = 1;
int blinkTime1 = 1000;
int blinkTime2 = 500;

bool button_is_pressed(int btn, bool onlyOnce = false) {
  if (touchRead(btn) >= threshold) {
    if (onlyOnce) {
      while (touchRead(btn) >= threshold) delay(50);
    }
    return true;
  }
  return false;
}

bool a_button_is_pressed() {
  for (int i = 0; i < 6; i++) {
    if (button_is_pressed(buttons[i])) return true;
  }
  return false;
}

static bool touchHeld(int pin, unsigned long holdMs) {
  if (touchRead(pin) < threshold) return false;
  unsigned long start = millis();
  while (touchRead(pin) >= threshold) {
    if (millis() - start >= holdMs) {
      while (touchRead(pin) >= threshold) delay(10);
      return true;
    }
    delay(10);
  }
  return false;
}

int drawMenu(const char* items[], int itemCount, int startIndex = 0) {
  if (itemCount <= 0) return -1;

  const int BTN_PREV = buttons[3];
  const int BTN_NEXT = buttons[5];
  const int BTN_SELECT = buttons[4];

  int idx = startIndex;
  if (idx < 0) idx = 0;
  if (idx >= itemCount) idx = itemCount - 1;

  unsigned long lastDraw = 0;
  const unsigned long DRAW_MS = 80;

  auto shortLabel = [](const char* s, int maxChars, char* out, int outLen) {
    if (!s) { out[0] = '\0'; return; }
    int len = strlen(s);
    if (len <= maxChars) {
      strncpy(out, s, outLen-1);
      out[outLen-1] = '\0';
    } else if (maxChars > 3) {
      int take = maxChars - 3;
      strncpy(out, s, min(take, outLen-1));
      int n = min(take, outLen-1);
      out[n] = '\0';
      strncat(out, "...", outLen - strlen(out) - 1);
    } else {
      strncpy(out, s, min(maxChars, outLen-1));
      out[outLen-1] = '\0';
    }
  };

  while (a_button_is_pressed()) delay(10);

  while (true) {
    unsigned long now = millis();

    if (button_is_pressed(BTN_NEXT, true)) {
      idx = (idx + 1) % itemCount;
    } else if (button_is_pressed(BTN_PREV, true)) {
      idx = (idx - 1 + itemCount) % itemCount;
    } else if (button_is_pressed(BTN_SELECT, true)) {
      return idx;
    }

    if (now - lastDraw >= DRAW_MS) {
      lastDraw = now;

      const int cx = SCREEN_WIDTH / 2;
      const int cy = SCREEN_HEIGHT / 2;
      const int outerR = SCREEN_WIDTH / 2;

      const uint16_t BG = display.color565(8,8,10);
      const uint16_t RIM = display.color565(14,16,18);
      const uint16_t CARD = display.color565(22,24,28);
      const uint16_t ACCENT = display.color565(0,160,220);
      const uint16_t LT = display.color565(235,235,240);
      const uint16_t MUTED = display.color565(130,134,140);

      canvas.fillSprite(BG);
      canvas.fillCircle(cx, cy, outerR, RIM);
      canvas.fillCircle(cx, cy, outerR - 6, BG);

      int prev = (idx - 1 + itemCount) % itemCount;
      int next = (idx + 1) % itemCount;

      char bufPrev[32], bufCurr[32], bufNext[32];
      shortLabel(items[prev], 18, bufPrev, sizeof(bufPrev));
      shortLabel(items[idx], 18, bufCurr, sizeof(bufCurr));
      shortLabel(items[next], 18, bufNext, sizeof(bufNext));

      const int textRadius = outerR - 60;

      canvas.setTextDatum(MC_DATUM);
      canvas.setTextSize(1);
      canvas.setTextColor(MUTED, BG);
      canvas.drawString(bufPrev, cx, cy - textRadius);

      canvas.setTextSize(3);
      canvas.setTextColor(ACCENT, CARD);
      canvas.drawString(bufCurr, cx, cy);

      canvas.setTextSize(1);
      canvas.setTextColor(MUTED, BG);
      canvas.drawString(bufNext, cx, cy + textRadius);

      canvas.drawCircle(cx, cy, textRadius - 20, CARD);
      canvas.drawCircle(cx, cy, textRadius - 20 + 6, CARD);

      canvas.setTextSize(1);
      canvas.setTextDatum(TL_DATUM);
      char idxBuf[16];
      snprintf(idxBuf, sizeof(idxBuf), "%d/%d", idx + 1, itemCount);
      canvas.setTextColor(LT, BG);
      canvas.drawString(idxBuf, 12, 12);

      canvas.pushSprite(0,0);
    }

    delay(10);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);

  macRandom.begin();

  setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
  tzset();

  display.init();
  display.setRotation(2);

  canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  canvas.setTextDatum(TL_DATUM);

  canvas.fillSprite(display.color565(10, 10, 12));
  canvas.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, display.color565(14, 14, 16));
  canvas.setTextSize(2);
  canvas.setTextColor(display.color565(240, 240, 245), display.color565(14, 14, 16));
  canvas.setTextDatum(MC_DATUM);
  canvas.drawString("Watch 5.2", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 6);
  canvas.pushSprite(0, 0);
  delay(1000);
  
  timeMenu();
}

void loop() {
  selectedFunction = drawMenu(Functions, totalFunctions, selectedFunction);
  switch (selectedFunction) {
    case 0: timeMenu();    break;
    case 1: watchFuncs();  break;
    case 2: calculator();  break;
    case 4: counterMenu(); break;
  }
  delay(120);
}