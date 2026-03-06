// Includes: Watch Funcs, Counter, Random Num, Metronome, Notes storage, Serial Notes Menu

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const int btn1, btn2, btn3, btn4, btn5, btn6;
extern byte Func1, Func2, Func3;
extern int bpm;
extern Note notes[5];
extern Preferences preferences;

void activateFunc(byte func, int blinkTime = 500){
  bool blink = false;
  bool keepOn = false;

  while (true){
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    } 
    else {
      if (button_is_pressed(btn1, false)) digitalWrite(func, HIGH);
      else digitalWrite(func, LOW);
    }

    if (button_is_pressed(btn2, true)){
      keepOn = !keepOn;
      if (keepOn) blink = false;
    } 
    else if (button_is_pressed(btn3, true)){
      blink = !blink;
      if (blink) keepOn = false;
    }
    else if (button_is_pressed(btn6)){
      return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Quick Flash");
    display.setCursor(0, 30);
    display.println("2. Always On");
    display.setCursor(0, 40);
    display.println("3. Blink");
    display.setCursor(0, 50);
    display.println("6. Return");
    
    display.setTextSize(2);
    display.setCursor(5, 0);
    display.print(digitalRead(func) ? "On" : "Off");
    display.display();
  }
}

void watchFuncs(void) {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. White LED");
    display.setCursor(0, 30);
    display.println("2. Laser");
    display.setCursor(0, 40);
    display.println("3. UV LED");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) activateFunc(Func1);
    else if (button_is_pressed(btn2)) activateFunc(Func2);
    else if (button_is_pressed(btn3)) activateFunc(Func3);
    else if (button_is_pressed(btn6)) return;
  }
}

int score1 = 0;
int score2 = 0;
void counter(void){
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 5);
    display.print("Score Counter");

    display.setTextSize(3);
    display.setCursor(0, 30);
    display.print(score1);
    display.print(" : ");
    display.print(score2);
    display.display();

    if (button_is_pressed(btn1)){
      ++score1;
      delay(150);
    }
    else if (button_is_pressed(btn2)){
      ++score2;
      delay(150);
    }
    else if (button_is_pressed(btn3)){
      score1 = 0;
      score2 = 0;  
    }
    else if (button_is_pressed(btn4)){
      --score1;
      delay(150);
    }
    else if (button_is_pressed(btn5)){
      --score2;
      delay(150);
    }
    else if (button_is_pressed(btn6)){
      return;
    }
    delay(50);
  }
}

void randomNum(void) {
  int range = 10;
  bool floatMode = false;
  int decimals = 2;
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Random");
    display.setCursor(0, 22);
    display.setTextSize(1);
    display.print("Max:");
    display.print(range);
    display.setCursor(64, 22);
    display.print(floatMode ? "Float" : "Int");
    display.setCursor(0, 44);
    display.print("Dec:");
    display.print(decimals);
    display.setCursor(0, 56);
    display.print("3:tog 4:d.p 5:gen");
    display.display();
    delay(100);
    
    if (button_is_pressed(btn1)) {
      range += 10;
      delay(250);
    } 
    else if (button_is_pressed(btn2)) {
      range = max(1, range - 1);
      delay(250);
    } 
    else if (button_is_pressed(btn5)) {
      if (!floatMode) {
        int r = random(0, range + 1);
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(10, 25);
        display.print(r);
        display.display();
        delay(2000);
      } else {
        double u = (random(0, 32767) / 32767.0);
        double val = u * range;
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(0, 18);
        display.print(val, decimals);
        display.display();
        delay(2000);
      }
    } 
    else if (button_is_pressed(btn3)) {
      floatMode = !floatMode;
      delay(250);
    }
    else if (button_is_pressed(btn4)) {
      decimals = min(6, decimals + 1);
      delay(200);
    }
    else if (button_is_pressed(btn6, true)) {
      return;
    }
  }
}


void metronome(void){
  const int MIN_BPM = 1;
  const unsigned long PULSE_MS = 10;
  unsigned long lastBeat = millis();
  unsigned long ledOffAt = 0;
  volatile byte Func = Func1;
  
  const unsigned long HOLD_INITIAL_MS = 400; 
  const unsigned long HOLD_MIN_MS = 1; 
  const float HOLD_ACCEL_FACTOR = 0.75f;     

  unsigned long now = 0;
  unsigned long interval = 60000UL / (unsigned long)max(1, bpm);
  
  bool btn1Held = false;
  bool btn2Held = false;
  unsigned long btn1NextRepeat = 0;
  unsigned long btn2NextRepeat = 0;
  unsigned long btn1RepeatDelay = HOLD_INITIAL_MS;
  unsigned long btn2RepeatDelay = HOLD_INITIAL_MS;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Metronome");
  display.setCursor(0,30);
  display.print(bpm);
  display.print("BPM");
  display.display();
  
  digitalWrite(Func, LOW);

  while (true){
    now = millis();
    static int lastShownBpm = -1;
    if (bpm != lastShownBpm){
      lastShownBpm = bpm;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.print("Metronome");
      display.setCursor(0,30);
      display.print(bpm);
      display.print("BPM");
      display.display();
      
      interval = 60000UL / (unsigned long)max(1, bpm);
    }
    
    if ((unsigned long)(now - lastBeat) >= interval){
      digitalWrite(Func, HIGH);
      ledOffAt = now + PULSE_MS;
      lastBeat = now;
    }

    if (ledOffAt && now >= ledOffAt){
      digitalWrite(Func, LOW);
      ledOffAt = 0;
    }
    
    if (button_is_pressed(btn1, false)){
      if (!btn1Held){
        btn1Held = true;
        btn1RepeatDelay = HOLD_INITIAL_MS;
        btn1NextRepeat = now + btn1RepeatDelay;
        if (bpm > MIN_BPM) bpm--;
      } else {
        if (now >= btn1NextRepeat){
          if (bpm > MIN_BPM) bpm--;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn1RepeatDelay * HOLD_ACCEL_FACTOR);
          btn1RepeatDelay = newDelay;
          btn1NextRepeat = now + btn1RepeatDelay;
        }
      }
    } else {
      btn1Held = false;
    }

    if (button_is_pressed(btn2, false)){
      if (!btn2Held){
        btn2Held = true;
        btn2RepeatDelay = HOLD_INITIAL_MS;
        btn2NextRepeat = now + btn2RepeatDelay;
        bpm++;
      } else {
        if (now >= btn2NextRepeat){
          bpm++;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn2RepeatDelay * HOLD_ACCEL_FACTOR);
          btn2RepeatDelay = newDelay;
          btn2NextRepeat = now + btn2RepeatDelay;
        }
      }
    } else {
      btn2Held = false;
    }
    if (button_is_pressed(btn6)){
      digitalWrite(Func, LOW);
      return;
    }  
    yield();
  }
}

void saveNotesToNVS(void) {
  preferences.begin("notes", false);
  
  for (int i = 0; i < MAX_NOTES; i++) {
    char keyText[20];
    char keyUsed[20];
    
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
    char keyText[20];
    char keyUsed[20];
    
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
  int selectedNote = 0;
  bool viewingMode = true;

  while (true) {
    if (viewingMode) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Notes");
      
      display.setCursor(0, 18);
      for (int i = 0; i < MAX_NOTES; i++) {
        if (i == selectedNote) {
          display.print(">");
        } else {
          display.print(" ");
        }
        display.print(i + 1);
        display.print(": ");
        
        if (notes[i].used) {
          char preview[13];
          strncpy(preview, notes[i].text, 12);
          preview[12] = '\0';
          display.print(preview);
        } else {
          display.print("[Empty]");
        }
        display.setCursor(0, 18 + ((i + 1) * 10));
      }
     
      display.display();
      
      if (button_is_pressed(btn1)) {
        selectedNote = (selectedNote - 1 + MAX_NOTES) % MAX_NOTES;
        delay(150);
      }
      else if (button_is_pressed(btn2)) {
        selectedNote = (selectedNote + 1) % MAX_NOTES;
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        viewingMode = false;
        delay(200);
      }
      else if (button_is_pressed(btn6)) {
        saveNotesToNVS();
        return;
      }
    } else {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Note ");
      display.print(selectedNote + 1);
      display.print(" - Edit");
      
      display.setCursor(0, 18);
      display.print("Text:");
      display.setCursor(0, 30);
      display.print(notes[selectedNote].text);
      display.print("_");
      
      display.setCursor(0, 40);
      display.print("1:Del 2:Add 3:Char");
      display.setCursor(0, 50);
      display.print("4:Clr 5:Done 6:Back");
      display.display();
      
      if (button_is_pressed(btn1, false)) {
        int len = strlen(notes[selectedNote].text);
        if (len > 0) {
          notes[selectedNote].text[len - 1] = '\0';
          if (len - 1 == 0) {
            notes[selectedNote].used = false;
          }
        }
        delay(150);
      }
      else if (button_is_pressed(btn2, false)) {
        int len = strlen(notes[selectedNote].text);
        if (len < MAX_NOTE_LENGTH - 1) {
          notes[selectedNote].text[len] = ' ';
          notes[selectedNote].text[len + 1] = '\0';
          notes[selectedNote].used = true;
        }
        delay(150);
      }
      else if (button_is_pressed(btn3)) {
        char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-:,!?";
        int charIndex = 0;
        bool selectingChar = true;
        
        while (selectingChar) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.print("Select Char:");
          display.setCursor(0, 18);
          display.setTextSize(2);
          display.print(charset[charIndex]);
          display.setTextSize(1);
          display.setCursor(0, 30);
          display.print("1:< 2:> 3:Select");
          display.setCursor(0, 40);
          display.print("6:Cancel");
          display.display();
          
          if (button_is_pressed(btn1)) {
            charIndex = (charIndex - 1 + (int)strlen(charset)) % (int)strlen(charset);
            delay(100);
          }
          else if (button_is_pressed(btn2)) {
            charIndex = (charIndex + 1) % (int)strlen(charset);
            delay(100);
          }
          else if (button_is_pressed(btn3)) {
            int len = strlen(notes[selectedNote].text);
            if (len < MAX_NOTE_LENGTH - 1) {
              notes[selectedNote].text[len] = charset[charIndex];
              notes[selectedNote].text[len + 1] = '\0';
              notes[selectedNote].used = true;
            }
            selectingChar = false;
            delay(200);
          }
          else if (button_is_pressed(btn6)) {
            selectingChar = false;
            delay(200);
          }
          delay(50);
        }
      }
      else if (button_is_pressed(btn4)) {
        notes[selectedNote].text[0] = '\0';
        notes[selectedNote].used = false;
        delay(200);
      }
      else if (button_is_pressed(btn5)) {
        saveNotesToNVS();
        viewingMode = true;
        delay(200);
      }
      else if (button_is_pressed(btn6)) {
        saveNotesToNVS();
        viewingMode = true;
        delay(200);
      }
    }
    
    delay(50);
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
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Note ");
  display.print(noteNum);
  display.println(" saved via");
  display.println("Serial!");
  display.display();
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
