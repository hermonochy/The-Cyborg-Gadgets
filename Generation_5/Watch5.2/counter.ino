#define MAX_SLOTS 5

struct SavedSlot {
  bool used;
  uint8_t players;
  int scores[4];
} savedSlots[MAX_SLOTS];

void loadAllScoreSlots() {
  preferences.begin("scores", true);
  for (int i = 0; i < MAX_SLOTS; ++i) {
    String keyUsed = String("s") + i + "_u";
    String keyPlayers = String("s") + i + "_p";
    savedSlots[i].used = preferences.getBool(keyUsed.c_str(), false);
    savedSlots[i].players = (uint8_t)preferences.getInt(keyPlayers.c_str(), 0);
    for (int j = 0; j < 4; ++j) {
      String keyScore = String("s") + i + "_v" + j;
      savedSlots[i].scores[j] = preferences.getInt(keyScore.c_str(), 0);
    }
  }
  preferences.end();
}

void saveScoreSlotToNVS(int slot) {
  if (slot < 0 || slot >= MAX_SLOTS) return;
  preferences.begin("scores", false);
  String keyUsed = String("s") + slot + "_u";
  String keyPlayers = String("s") + slot + "_p";
  preferences.putBool(keyUsed.c_str(), savedSlots[slot].used);
  preferences.putInt(keyPlayers.c_str(), savedSlots[slot].players);
  for (int j = 0; j < 4; ++j) {
    String keyScore = String("s") + slot + "_v" + j;
    preferences.putInt(keyScore.c_str(), savedSlots[slot].scores[j]);
  }
  preferences.end();
}

void clearScoreSlotNVS(int slot) {
  if (slot < 0 || slot >= MAX_SLOTS) return;
  savedSlots[slot].used = false;
  savedSlots[slot].players = 0;
  for (int j = 0; j < 4; ++j) savedSlots[slot].scores[j] = 0;
  saveScoreSlotToNVS(slot);
}

int findFirstEmptySlot() {
  for (int i = 0; i < MAX_SLOTS; ++i) if (!savedSlots[i].used) return i;
  return -1;
}

void drawCounterCountingUI(int players, int scores[4]) {
  const int cx = SCREEN_WIDTH / 2;
  const int cy = SCREEN_HEIGHT / 2;
  const int outerR = SCREEN_WIDTH/2;
  const int SAFE = 36;
  canvas.fillSprite(display.color565(10,10,12));
  canvas.fillCircle(cx, cy, outerR, display.color565(14,14,16));
  canvas.fillCircle(cx, cy, outerR - 6, display.color565(12,12,14));

  const uint16_t PLATE = display.color565(22,24,28);
  const uint16_t ACCENT = display.color565(0,160,220);
  const uint16_t TEXT = display.color565(235,235,240);

  canvas.fillCircle(cx, cy, outerR - SAFE, PLATE);
  canvas.drawCircle(cx, cy, outerR - SAFE, display.color565(40,44,48));

  int ringR = outerR - SAFE - 36;
  if (ringR < 40) ringR = outerR - SAFE - 24;
  for (int i = 0; i < players; ++i) {
    float ang = -M_PI/2 + i * (2.0 * M_PI / players);
    int px = cx + int(cos(ang) * ringR);
    int py = cy + int(sin(ang) * ringR) - 6;
    int r = 36;
    canvas.fillCircle(px, py, r, display.color565(28,30,34));
    canvas.drawCircle(px, py, r, display.color565(56,60,66));
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextColor(TEXT, display.color565(28,30,34));
    canvas.setTextSize(3);
    char sbuf[16];
    snprintf(sbuf, sizeof(sbuf), "%d", scores[i]);
    int textSize = 3;
    canvas.setTextSize(textSize);
    int w = canvas.textWidth(sbuf);
    while (w > r*2 - 8 && textSize > 1) {
      textSize--;
      canvas.setTextSize(textSize);
      w = canvas.textWidth(sbuf);
    }
    canvas.drawString(sbuf, px, py - 6);
    canvas.setTextSize(1);
    char label[8];
    snprintf(label, sizeof(label), "P%d", i+1);
    canvas.drawString(label, px, py + r - 8);
  }

  canvas.setTextSize(1);
  canvas.setTextDatum(TL_DATUM);
  canvas.setTextColor(display.color565(200,200,200), display.color565(12,12,14));
  canvas.drawString("Btn2:Save  Btn5:Menu", 12, 12);

  canvas.pushSprite(0,0);
}

void counterCounting(int initialPlayers, int initialScores[4], int preferredSlot = -1) {
  int players = constrain(initialPlayers, 1, 4);
  int scores[4] = {0,0,0,0};
  for (int i=0;i<4;i++) scores[i] = initialScores ? initialScores[i] : 0;
  int slotToSave = preferredSlot; 

  const int incButtons[4] = { buttons[0], buttons[3], buttons[1], buttons[4] };
  while (true) {
    drawCounterCountingUI(players, scores);

    for (int p=0;p<players;p++) {
      if (button_is_pressed(incButtons[p])) {
        scores[p]++;
        delay(140);
      }
    }

    if (button_is_pressed(buttons[2], true)) {
      int slot = slotToSave;
      if (slot == -1) slot = findFirstEmptySlot();
      if (slot == -1) {
        slot = 0; 
      }
      savedSlots[slot].used = true;
      savedSlots[slot].players = players;
      for (int i=0;i<4;i++) savedSlots[slot].scores[i] = scores[i];
      saveScoreSlotToNVS(slot);
      const int cx = SCREEN_WIDTH/2;
      const int cy = SCREEN_HEIGHT/2;
      canvas.fillSprite(display.color565(10,10,12));
      canvas.fillCircle(cx, cy, SCREEN_WIDTH/2, display.color565(14,14,16));
      canvas.setTextDatum(MC_DATUM);
      canvas.setTextSize(2);
      canvas.setTextColor(display.color565(220,220,220), display.color565(14,14,16));
      canvas.drawString("SAVED", cx, cy - 8);
      canvas.setTextSize(1);
      char buf[32];
      snprintf(buf, sizeof(buf), "Slot %d", slot + 1);
      canvas.drawString(buf, cx, cy + 18);
      canvas.pushSprite(0,0);
      delay(700);
      slotToSave = slot; 
    }

    if (button_is_pressed(buttons[5], true)) {
      delay(200);
      return;
    }
    delay(20);
  }
}

void drawCounterMenu(int selectedSlot, int selectedActionIndex) {
  const int cx = SCREEN_WIDTH/2;
  const int cy = SCREEN_HEIGHT/2;
  const int outerR = SCREEN_WIDTH/2;
  canvas.fillSprite(display.color565(8,8,10));
  canvas.fillCircle(cx, cy, outerR, display.color565(14,14,16));
  canvas.fillCircle(cx, cy, outerR - 6, display.color565(10,10,12));
  canvas.setTextDatum(TL_DATUM);

  const uint16_t TITLE = display.color565(235,235,240);
  const uint16_t MUTED = display.color565(170,170,180);
  canvas.setTextSize(2);
  canvas.setTextColor(TITLE, display.color565(14,14,16));
  canvas.drawString("Score Slots", 18, 12);

  int safeTop = 36;
  int slotH = 36;
  int startY = safeTop + 6;
  for (int i = 0; i < MAX_SLOTS; ++i) {
    int y = startY + i * (slotH + 6);
    int left = 18;
    int w = SCREEN_WIDTH - 36;
    canvas.fillRoundRect(left, y, w, slotH, 8, display.color565(22,24,28));
    if (i == selectedSlot) {
      canvas.drawRoundRect(left, y, w, slotH, 8, display.color565(0,160,220));
    } else {
      canvas.drawRoundRect(left, y, w, slotH, 8, display.color565(46,48,52));
    }
    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200,200,200), display.color565(22,24,28));
    if (!savedSlots[i].used) {
      canvas.drawString(String(i+1) + ": [Empty]", left + 8, y + 10);
    } else {
      char buf[64];
      if (savedSlots[i].players == 1) {
        snprintf(buf, sizeof(buf), "%d) P1:%d", i+1, savedSlots[i].scores[0]);
      } else if (savedSlots[i].players == 2) {
        snprintf(buf, sizeof(buf), "%d) P1:%d P2:%d", i+1, savedSlots[i].scores[0], savedSlots[i].scores[1]);
      } else if (savedSlots[i].players == 3) {
        snprintf(buf, sizeof(buf), "%d) %d/%d/%d", i+1, savedSlots[i].scores[0], savedSlots[i].scores[1], savedSlots[i].scores[2]);
      } else {
        snprintf(buf, sizeof(buf), "%d) %d/%d/%d/%d", i+1, savedSlots[i].scores[0], savedSlots[i].scores[1], savedSlots[i].scores[2], savedSlots[i].scores[3]);
      }
      canvas.drawString(buf, left + 8, y + 10);
    }
  }

  canvas.setTextSize(1);
  canvas.setTextColor(MUTED, display.color565(10,10,12));
  canvas.setTextDatum(MC_DATUM);
  canvas.drawString("Prev  Select  Next", cx, SCREEN_HEIGHT - 24);
  canvas.pushSprite(0,0);
}

void counterMenu() {
  loadAllScoreSlots();
  int sel = 0;
  while (true) {
    drawCounterMenu(sel, 0);
    if (button_is_pressed(buttons[5])) { 
      sel = (sel + 1) % MAX_SLOTS;
      delay(160);
    } else if (button_is_pressed(buttons[3])) { 
      sel = (sel - 1 + MAX_SLOTS) % MAX_SLOTS;
      delay(160);
    } else if (button_is_pressed(buttons[4], true)) { 
      if (savedSlots[sel].used) {
        int tmpScores[4];
        for (int i=0;i<4;i++) tmpScores[i] = savedSlots[sel].scores[i];
        counterCounting(savedSlots[sel].players, tmpScores, sel);
        loadAllScoreSlots();
      } else {
        int p = 1;
        unsigned long ld = 0;
        while (true) {
          if (millis() - ld > 60) {
            ld = millis();
            const int cx = SCREEN_WIDTH/2;
            canvas.fillSprite(display.color565(8,8,10));
            canvas.fillCircle(cx, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, display.color565(14,14,16));
            canvas.fillCircle(cx, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 - 6, display.color565(10,10,12));
            canvas.setTextDatum(MC_DATUM);
            canvas.setTextSize(2);
            canvas.setTextColor(display.color565(235,235,240), display.color565(14,14,16));
            canvas.drawString("New Slot", cx, 40);
            canvas.setTextSize(3);
            canvas.setTextColor(display.color565(0,160,220), display.color565(10,10,12));
            char tb[8]; snprintf(tb, sizeof(tb), "%d Players", p);
            canvas.drawString(tb, cx, SCREEN_HEIGHT/2 - 8);
            canvas.setTextSize(1);
            canvas.setTextColor(display.color565(200,200,200), display.color565(10,10,12));
            canvas.drawString("Btn3/- Btn5/+  Sel:create  Btn0:back", cx, SCREEN_HEIGHT - 20);
            canvas.pushSprite(0,0);
          }
          if (button_is_pressed(buttons[5])) { p = min(4, p+1); delay(140); }
          if (button_is_pressed(buttons[3])) { p = max(1, p-1); delay(140); }
          if (button_is_pressed(buttons[4], true)) {
            savedSlots[sel].used = true;
            savedSlots[sel].players = p;
            for (int i=0;i<4;i++) savedSlots[sel].scores[i]=0;
            saveScoreSlotToNVS(sel);
            int tmpScores[4] = {0,0,0,0};
            counterCounting(p, tmpScores, sel);
            loadAllScoreSlots();
            break;
          }
          if (button_is_pressed(buttons[2], true)) { break; }
          delay(20);
        }
      }
    } else if (touchHeld(buttons[4], 700)) {
      if (savedSlots[sel].used) {
        clearScoreSlotNVS(sel);
        loadAllScoreSlots();
        const int cx = SCREEN_WIDTH/2;
        canvas.fillSprite(display.color565(8,8,10));
        canvas.fillCircle(cx, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, display.color565(14,14,16));
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextSize(2);
        canvas.setTextColor(display.color565(220,220,220), display.color565(14,14,16));
        canvas.drawString("Deleted", cx, SCREEN_HEIGHT/2 - 8);
        canvas.pushSprite(0,0);
        delay(600);
      }
    } else if (button_is_pressed(buttons[2], true)) { 
      return;
    }
    delay(20);
  }
}