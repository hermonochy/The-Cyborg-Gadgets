#define MAX_NOTES 5
#define MAX_NOTE_LENGTH 64

extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG, colourText, colour1, colour2, colour3, colour4, colour5, colour6;
extern byte Func1, Func2, Func3;
extern int blinkTime1, blinkTime2, blinkTime3;
extern Preferences preferences;

struct Note {
  char text[MAX_NOTE_LENGTH];
  bool used;
};
Note notes[MAX_NOTES];

///////////////// WATCH FUNCTIONS (GPIO) MENU ///////////////

void activateFunc(byte func, int blinkTime = 500) {
  bool blink = false, keepOn = false, showMode = true, lastState = false;
  int oldMode = -1;
  while (true) {
    // UI update only when mode changes (avoid flicker)
    if (showMode) {
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour4);
      display.setCursor(62, 25);
      display.print("ACTUATOR");
      display.setTextSize(1);
      display.setTextColor(colour6);
      display.setCursor(54, 70);
      display.print("1:Flash 2:HOLD 3:Blink");
      display.setCursor(72, 96);
      display.print("6:Back");
      showMode = false;
    }
    if (button_is_pressed(btn2, true)) {
      keepOn = !keepOn;
      blink = false;
      lastState = false;
      showMode = true;
    } else if (button_is_pressed(btn3, true)) {
      blink = !blink;
      keepOn = false;
      lastState = false;
      showMode = true;
    } else if (button_is_pressed(btn6, true)) {
      digitalWrite(func, LOW);
      return;
    }
    if (blink) {
      digitalWrite(func, HIGH);
      delay(blinkTime);
      digitalWrite(func, LOW);
      delay(blinkTime);
    } else if (keepOn) digitalWrite(func, HIGH);
    else if (button_is_pressed(btn1, false)) {
      digitalWrite(func, HIGH);
      lastState = true;
    } else if (lastState) {
      digitalWrite(func, LOW);
      lastState = false;
    }
    delay(10);
  }
}

void watchFuncs(void) {
  int sel = 0, oldSel = -1;
  const char* funcs[] = { "White LED", "Laser", "UV LED", "Back" };
  while (1) {
    if (sel != oldSel) {
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(74, 28);
      display.print("OUTPUTS");
      int y0 = 70;
      for (int i = 0; i < 4; ++i) {
        display.setTextSize(i == sel ? 2 : 1);
        display.setTextColor(i == sel ? colour6 : colour3);
        display.setCursor(60, y0 + i * 34);
        display.print(funcs[i]);
      }
      oldSel = sel;
    }
    if (button_is_pressed(btn2, true)) {
      sel = (sel + 1) % 4;
    } else if (button_is_pressed(btn1, true)) {
      sel = (sel + 3) % 4;
    } else if (button_is_pressed(btn3, true) || button_is_pressed(btn5, true)) {
      if (sel == 0) activateFunc(Func1, blinkTime1);
      else if (sel == 1) activateFunc(Func2, blinkTime2);
      else if (sel == 2) activateFunc(Func3, blinkTime3);
      else if (sel == 3) return;
      oldSel = -1;
    } else if (button_is_pressed(btn6, true)) return;
    delay(50);
  }
}

///////////////// SCORE COUNTER //////////////////////

int score1 = 0, score2 = 0;
void counter(void) {
  int lastS1 = -1000, lastS2 = -1000;
  while (true) {
    if (score1 != lastS1 || score2 != lastS2) {
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour2);
      display.setCursor(87, 26);
      display.print("COUNTER");
      display.setTextSize(3);
      display.setTextColor(colour3);
      display.setCursor(54, 90);
      display.printf("%d : %d", score1, score2);
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(34, 182);
      display.print("1/4/5:+/-  2:+ 3:Zr  6:Bk");
      lastS1 = score1;
      lastS2 = score2;
    }
    if (button_is_pressed(btn1)) {
      ++score1;
      delay(110);
    } else if (button_is_pressed(btn2)) {
      ++score2;
      delay(110);
    } else if (button_is_pressed(btn4)) {
      --score1;
      delay(110);
    } else if (button_is_pressed(btn5)) {
      --score2;
      delay(110);
    } else if (button_is_pressed(btn3)) {
      score1 = score2 = 0;
      delay(110);
    } else if (button_is_pressed(btn6, true)) return;
    delay(10);
  }
}

///////////////// RANDOM NUMBER //////////////////////

void randomNum(void) {
  int range = 10, decimals = 2, lastRange = -1, lastDec = -1, showNum = 0, floatMode = false;
  double lastVal = -1;
  char numStr[18] = "";
  while (true) {
    if (range != lastRange || decimals != lastDec || floatMode || showNum) {
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.setCursor(80, 25);
      display.print("RAND");
      display.setTextSize(1);
      display.setTextColor(colourText);
      display.setCursor(40, 66);
      display.print("Max:");
      display.print(range);
      display.setCursor(124, 66);
      display.print(floatMode ? "Float" : "Int");
      display.setCursor(40, 90);
      display.print("Dec :");
      display.print(decimals);
      display.setCursor(40, 120);
      display.print("btn1:+10 btn2:-1 btn3:Mode");
      display.setCursor(40, 140);
      display.print("btn4:Dp+ btn5:Go  btn6:Return");

      if (showNum) {
        display.setTextSize(floatMode ? 2 : 3);
        display.setTextColor(colour3);
        display.setCursor(82, 176);
        display.print(numStr);
        showNum = 0;  // Only draws the result once
      }
      lastRange = range;
      lastDec = decimals;
      lastVal = -1;
    }
    if (button_is_pressed(btn1, true)) {
      range += 10;
    } else if (button_is_pressed(btn2, true)) {
      range = max(1, range - 1);
    } else if (button_is_pressed(btn4, true)) {
      decimals = min(6, decimals + 1);
    } else if (button_is_pressed(btn3, true)) {
      floatMode = !floatMode;
    } else if (button_is_pressed(btn5, true)) {
      if (!floatMode) {
        int r = random(0, range + 1);
        snprintf(numStr, sizeof(numStr), "%d", r);
        showNum = 1;
      } else {
        double u = (random(0, 32767) / 32767.0);
        double val = u * range;
        dtostrf(val, 0, decimals, numStr);
        showNum = 1;
      }
    } else if (button_is_pressed(btn6, true)) return;
    delay(55);
  }
}

///////////////// METRONOME (NO FLICKERING UI ON BEAT) /////////////////

int metronome_bpm = 100;
int metronome_time_sig = 0;
const char* TIME_SIGS[] = { "4/4", "3/4", "2/4" };
const int TIME_SIG_BEATS[] = { 4, 3, 2 };

void metronome(void) {
  int selScreen = 0, oldScreen = -1;
  int cur_bpm = metronome_bpm, cur_sig = metronome_time_sig, beat = 0;
  unsigned long lastBeat = millis(), interval = 60000UL / cur_bpm;
  while (1) {
    if (oldScreen != selScreen || cur_bpm != metronome_bpm || cur_sig != metronome_time_sig) {
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colour3);
      display.setCursor(79, 24);
      display.print("METRO");
      display.setTextSize(3);
      display.setTextColor(colourText);
      display.setCursor(80, 70);
      display.print(cur_bpm);
      display.setTextSize(1);
      display.setCursor(172, 72);
      display.print("BPM");
      display.setTextSize(1);
      display.setTextColor(colour6);
      display.setCursor(80, 98);
      display.print("Sig: ");
      display.print(TIME_SIGS[cur_sig]);
      display.setCursor(40, 166);
      display.print("1/2:-/+  3:Sig 4:GO 6:Back");
      oldScreen = selScreen;
      cur_bpm = metronome_bpm;
      cur_sig = metronome_time_sig;
    }
    if (button_is_pressed(btn1, true)) {
      metronome_bpm = max(30, metronome_bpm - 1);
    } else if (button_is_pressed(btn2, true)) {
      metronome_bpm = min(250, metronome_bpm + 1);
    } else if (button_is_pressed(btn3, true)) {
      metronome_time_sig = (metronome_time_sig + 1) % 3;
    } else if (button_is_pressed(btn4, true)) {
      int total_beats = TIME_SIG_BEATS[metronome_time_sig];
      beat = 0;
      lastBeat = millis();
      while (!button_is_pressed(btn6, false)) {
        unsigned long now = millis();
        if (now - lastBeat >= 60000UL / metronome_bpm) {
          (beat == 0 ? digitalWrite(Func1, HIGH) : digitalWrite(Func2, HIGH));
          delay(75);
          digitalWrite(Func1, LOW);
          digitalWrite(Func2, LOW);
          beat = (beat + 1) % total_beats;
          lastBeat = now;
        }
        // no UI redraw while metronome running!
        if (button_is_pressed(btn6, true)) break;
      }
    } else if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}

////////////////// NOTES ///////////////////////

void saveNotesToNVS(void) {
  preferences.begin("notes", false);
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20], keyUsed[20];
    sprintf(keyText, "note%d_text", i);
    sprintf(keyUsed, "note%d_used", i);
    preferences.putString(keyText, notes[i].text);
    preferences.putBool(keyUsed, notes[i].used);
  }
  preferences.end();
}

void loadNotesFromNVS(void) {
  preferences.begin("notes", true);
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20], keyUsed[20];
    sprintf(keyText, "note%d_text", i);
    sprintf(keyUsed, "note%d_used", i);
    String noteText = preferences.getString(keyText, "");
    notes[i].used = preferences.getBool(keyUsed, false);
    strncpy(notes[i].text, noteText.c_str(), MAX_NOTE_LENGTH - 1);
    notes[i].text[MAX_NOTE_LENGTH - 1] = '\0';
  }
  preferences.end();
}

void initializeNotesNVS(void) {
  loadNotesFromNVS();
}

void notesFunction(void) {
  int sel = 0, oldSel = -1, mode = 0, charSel = 0;
  while (1) {
    if (mode == 0 && sel != oldSel) {  // List
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(80, 24);
      display.print("NOTES");
      for (int i = 0; i < MAX_NOTES; i++) {
        display.setTextSize(i == sel ? 2 : 1);
        display.setTextColor(i == sel ? colour6 : colour3);
        display.setCursor(38, 70 + i * 30);
        if (notes[i].used) {
          char prev[14];
          strncpy(prev, notes[i].text, 13);
          prev[13] = '\0';
          display.print(prev);
        } else display.print("[Empty]");
      }
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.setCursor(36, 190);
      display.print("3:Edit/Add 4:Del 5:Sve 6:Back");
      oldSel = sel;
    }

    if (mode == 0 && button_is_pressed(btn2, true)) {
      sel = (sel + 1) % MAX_NOTES;
    } else if (mode == 0 && button_is_pressed(btn1, true)) {
      sel = (sel + MAX_NOTES - 1) % MAX_NOTES;
    } else if (mode == 0 && button_is_pressed(btn3, true)) {
      mode = 1;
      oldSel = -1;
    } else if (mode == 0 && button_is_pressed(btn4, true)) {
      notes[sel].text[0] = 0;
      notes[sel].used = 0;
      oldSel = -1;
    } else if (mode == 0 && button_is_pressed(btn5, true)) {
      saveNotesToNVS();
    } else if (mode == 0 && button_is_pressed(btn6, true)) return;
    else if (mode == 1) {  // Edit
      display.fillCircle(120, 120, 120, colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      display.setCursor(60, 24);
      display.print("EDIT NOTE");
      display.setTextSize(1);
      display.setCursor(30, 64);
      display.print(notes[sel].text);
      display.print("_");
      display.setTextColor(colour6);
      display.setCursor(44, 142);
      display.print("1:< 2:> 3:Char 4:Del 5:Sve 6:Back");
      if (button_is_pressed(btn1, true)) {
        charSel = (charSel + 94) % 95;
      } else if (button_is_pressed(btn2, true)) {
        charSel = (charSel + 1) % 95;
      } else if (button_is_pressed(btn3, true)) {
        int l = strlen(notes[sel].text);
        if (l < MAX_NOTE_LENGTH - 2) {
          notes[sel].text[l] = 32 + charSel;
          notes[sel].text[l + 1] = 0;
          notes[sel].used = 1;
        }
      } else if (button_is_pressed(btn4, true)) {
        int l = strlen(notes[sel].text);
        if (l > 0) { notes[sel].text[l - 1] = 0; }
      } else if (button_is_pressed(btn5, true)) {
        saveNotesToNVS();
        mode = 0;
        oldSel = -1;
      } else if (button_is_pressed(btn6, true)) {
        mode = 0;
        oldSel = -1;
      }
      display.setCursor(58, 106);
      display.setTextColor(colour2);
      display.setTextSize(3);
      display.print((char)(32 + charSel));
    }
    delay(44);
  }
}

void serialCreateEditNote(void) {
  Serial.println("\n--- Create/Edit Note ---");
  
  Serial.println("\nCurrent Notes:");
  for (int i = 0; i < MAX_NOTES; i++) {
    Serial.print("  ");
    Serial.print(i + 1);
    Serial.print(": ");
    if (notes[i].used) {
      Serial.println(notes[i].text);
    } else {
      Serial.println("[Empty]");
    }
  }
  
  Serial.print("\nEnter note number (1-");
  Serial.print(MAX_NOTES);
  Serial.print("): ");
  while (!Serial.available()) delay(10);
  int noteNum = Serial.parseInt();
  Serial.println(noteNum);
  
  if (noteNum < 1 || noteNum > MAX_NOTES) {
    Serial.println("✗ Invalid note number!");
    return;
  }
  
  int noteIndex = noteNum - 1;
  
  Serial.print("Enter note text (max ");
  Serial.print(MAX_NOTE_LENGTH - 1);
  Serial.print(" chars): ");
  while (!Serial.available()) delay(10);
  String inputNote = Serial.readStringUntil('\n');
  inputNote.trim();
  
  if (inputNote.length() == 0) {
    Serial.println("✗ Note cannot be empty!");
    return;
  }
  
  if (inputNote.length() > MAX_NOTE_LENGTH - 1) {
    Serial.println("✗ Note too long!");
    return;
  }
  
  strncpy(notes[noteIndex].text, inputNote.c_str(), MAX_NOTE_LENGTH - 1);
  notes[noteIndex].text[MAX_NOTE_LENGTH - 1] = '\0';
  notes[noteIndex].used = true;
  
  saveNotesToNVS();
  
  Serial.println("\n✓ Note saved successfully!");
  Serial.print("  Note ");
  Serial.print(noteNum);
  Serial.print(": ");
  Serial.println(notes[noteIndex].text);
  
  display.fillScreen(colourBG);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Note ");
  display.print(noteNum);
  display.println(" saved via");
  display.println("Serial!");
  
  delay(1500);
}

void serialViewAllNotes(void) {
  Serial.println("\n--- All Notes ---");
  
  bool hasNotes = false;
  for (int i = 0; i < MAX_NOTES; i++) {
    if (notes[i].used) {
      hasNotes = true;
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(notes[i].text);
    }
  }
  
  if (!hasNotes) {
    Serial.println("  (No notes yet)");
  }
}

void serialDeleteNote(void) {
  Serial.println("\n--- Delete Note ---");
  
  Serial.println("Current Notes:");
  for (int i = 0; i < MAX_NOTES; i++) {
    Serial.print("  ");
    Serial.print(i + 1);
    Serial.print(": ");
    if (notes[i].used) {
      Serial.println(notes[i].text);
    } else {
      Serial.println("[Empty]");
    }
  }
  
  Serial.print("\nEnter note number to delete (1-");
  Serial.print(MAX_NOTES);
  Serial.print("): ");
  while (!Serial.available()) delay(10);
  int noteNum = Serial.parseInt();
  Serial.println(noteNum);
  
  if (noteNum < 1 || noteNum > MAX_NOTES) {
    Serial.println("✗ Invalid note number!");
    return;
  }
  
  int noteIndex = noteNum - 1;
  
  if (!notes[noteIndex].used) {
    Serial.println("✗ Note is already empty!");
    return;
  }
  
  Serial.print("Are you sure? (y/n): ");
  while (!Serial.available()) delay(10);
  char response = Serial.read();
  Serial.println(response);
  
  if (response == 'y' || response == 'Y') {
    notes[noteIndex].text[0] = '\0';
    notes[noteIndex].used = false;
    saveNotesToNVS();
    Serial.print("✓ Note ");
    Serial.print(noteNum);
    Serial.println(" deleted!");
  } else {
    Serial.println("Cancelled");
  }
}

void serialClearAllNotes(void) {
  Serial.println("\n--- Clear All Notes ---");
  Serial.print("This will delete ALL notes! Are you sure? (y/n): ");
  while (!Serial.available()) delay(10);
  char response = Serial.read();
  Serial.println(response);
  
  if (response == 'y' || response == 'Y') {
    for (int i = 0; i < MAX_NOTES; i++) {
      notes[i].text[0] = '\0';
      notes[i].used = false;
    }
    saveNotesToNVS();
    Serial.println("✓ All notes cleared!");
  } else {
    Serial.println("Cancelled");
  }
}

void serialNotesMenu(void) {
  while (true) {
    Serial.println("\n========== WATCH 5.0 NOTES MENU ==========");
    Serial.println("1. Create/Edit Note");
    Serial.println("2. View All Notes");
    Serial.println("3. Delete Note");
    Serial.println("4. Clear All Notes");
    Serial.println("5. Exit Menu");
    Serial.println("=========================================");
    Serial.print("Enter option (1-5): ");
    
    while (!Serial.available()) delay(10);
    char option = Serial.read();
    Serial.println(option);
    
    while (Serial.available()) Serial.read();
    
    switch (option) {
      case '1':
        serialCreateEditNote();
        break;
      case '2':
        serialViewAllNotes();
        break;
      case '3':
        serialDeleteNote();
        break;
      case '4':
        serialClearAllNotes();
        break;
      case '5':
        Serial.println("\nExiting menu...");
        return;
      default:
        Serial.println("✗ Invalid option");
    }
    
    delay(500);
  }
}
