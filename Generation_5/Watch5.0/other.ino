// Includes: Watch Funcs, Counter, Random Num, Metronome, Notes storage, Serial Notes Menu

#define MAX_NOTES 5
#define MAX_NOTE_LENGTH 64

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern byte Func1, Func2, Func3;
extern Preferences preferences;

struct Note {
  char text[MAX_NOTE_LENGTH];
  bool used;
};

Note notes[MAX_NOTES];

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

// Professional Metronome Variables
int metronome_bpm = 100;
int metronome_time_sig = 0;  // 0=4/4, 1=3/4, 2=2/4
byte metronome_subdivision = 0; // 0=quarters, 1=eighths, 2=triplets
const int MIN_BPM = 30;
const int MAX_BPM = 300;
const int TEMPO_INCREMENT = 1;
const char* TIME_SIGS[] = {"4/4", "3/4", "2/4"};
const int TIME_SIG_BEATS[] = {4, 3, 2};

void metronome_display_main(int bpm, int time_sig, int beat, int total_beats) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(bpm);
  display.print(" BPM");
  
  display.setTextSize(1);
  display.setCursor(90, 0);
  display.print(TIME_SIGS[time_sig]);
  
  // Display beat indicator
  display.setCursor(0, 20);
  display.print("Beat: ");
  display.print(beat + 1);
  display.print("/");
  display.print(total_beats);
  
  // Visual beat indicator with accent on downbeat
  display.setCursor(0, 32);
  for (int i = 0; i < total_beats; i++) {
    if (i == beat) {
      display.print(i == 0 ? "[*]" : "[ ]");
    } else if (i == 0) {
      display.print("[*]");
    } else {
      display.print("[ ]");
    }
  }
  
  display.setCursor(0, 46);
  display.setTextSize(1);
  display.print("1:< 2:> 3:Time 4:Sub");
  display.setCursor(0, 56);
  display.print("5:Tap 6:Exit");
  
  display.display();
}

void metronome_pulse_beat(int beat, int total_beats) {
  const unsigned long PULSE_STRONG = 30;  // Downbeat pulse
  const unsigned long PULSE_WEAK = 15;    // Regular beat pulse
  
  unsigned long pulse_duration = (beat == 0) ? PULSE_STRONG : PULSE_WEAK;
  
  digitalWrite(Func1, HIGH);
  delay(pulse_duration);
  digitalWrite(Func1, LOW);
}

void metronome_tap_tempo() {
  static unsigned long lastTapTime = 0;
  unsigned long currentTime = millis();
  
  if (lastTapTime == 0) {
    lastTapTime = currentTime;
    return;
  }
  
  unsigned long tapInterval = currentTime - lastTapTime;
  
  // Ignore taps that are too fast or too slow (prevent anomalies)
  if (tapInterval > 200 && tapInterval < 4000) {
    int calculatedBpm = 60000 / tapInterval;
    metronome_bpm = constrain(calculatedBpm, MIN_BPM, MAX_BPM);
  }
  
  lastTapTime = currentTime;
  
  // Visual feedback
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 25);
  display.print("Tempo Set!");
  display.setCursor(20, 45);
  display.print(metronome_bpm);
  display.print(" BPM");
  display.display();
  delay(800);
}

void metronome_time_signature_menu() {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Time Signature");
    
    display.setCursor(10, 20);
    display.setTextSize(2);
    for (int i = 0; i < 3; i++) {
      if (i == metronome_time_sig) {
        display.print(">");
      } else {
        display.print(" ");
      }
      display.println(TIME_SIGS[i]);
    }
    
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("1:< 2:> 3:OK 6:Back");
    display.display();
    
    if (button_is_pressed(btn1)) {
      metronome_time_sig = (metronome_time_sig - 1 + 3) % 3;
      delay(200);
    }
    else if (button_is_pressed(btn2)) {
      metronome_time_sig = (metronome_time_sig + 1) % 3;
      delay(200);
    }
    else if (button_is_pressed(btn3)) {
      return;
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
  }
}

void metronome_subdivision_menu() {
  const char* SUBS[] = {"Quarters", "Eighths", "Triplets"};
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Subdivision");
    
    display.setCursor(10, 20);
    display.setTextSize(1);
    for (int i = 0; i < 3; i++) {
      if (i == metronome_subdivision) {
        display.print(">");
      } else {
        display.print(" ");
      }
      display.println(SUBS[i]);
    }
    
    display.setCursor(0, 56);
    display.print("1:< 2:> 3:OK 6:Back");
    display.display();
    
    if (button_is_pressed(btn1)) {
      metronome_subdivision = (metronome_subdivision - 1 + 3) % 3;
      delay(200);
    }
    else if (button_is_pressed(btn2)) {
      metronome_subdivision = (metronome_subdivision + 1) % 3;
      delay(200);
    }
    else if (button_is_pressed(btn3)) {
      return;
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
  }
}

void metronome(void) {
  int total_beats = TIME_SIG_BEATS[metronome_time_sig];
  int current_beat = 0;
  
  unsigned long lastBeatTime = millis();
  unsigned long beatInterval = 60000UL / (unsigned long)max(1, metronome_bpm);
  
  // Button hold tracking for smooth tempo adjustment
  bool btn1Held = false;
  bool btn2Held = false;
  unsigned long btn1NextRepeat = 0;
  unsigned long btn2NextRepeat = 0;
  const unsigned long HOLD_INITIAL_MS = 500;
  const unsigned long HOLD_MIN_MS = 50;
  const float HOLD_ACCEL_FACTOR = 0.85f;
  unsigned long btn1RepeatDelay = HOLD_INITIAL_MS;
  unsigned long btn2RepeatDelay = HOLD_INITIAL_MS;
  
  digitalWrite(Func1, LOW);

  while (true) {
    unsigned long now = millis();
    
    // Update display only when BPM or time signature changes
    static int lastDisplayedBpm = -1;
    static int lastDisplayedTimeSig = -1;
    if (metronome_bpm != lastDisplayedBpm || metronome_time_sig != lastDisplayedTimeSig) {
      lastDisplayedBpm = metronome_bpm;
      lastDisplayedTimeSig = metronome_time_sig;
      total_beats = TIME_SIG_BEATS[metronome_time_sig];
      beatInterval = 60000UL / (unsigned long)max(1, metronome_bpm);
      metronome_display_main(metronome_bpm, metronome_time_sig, current_beat, total_beats);
    }
    
    // Main metronome beat timing
    if ((unsigned long)(now - lastBeatTime) >= beatInterval) {
      metronome_pulse_beat(current_beat, total_beats);
      current_beat = (current_beat + 1) % total_beats;
      lastBeatTime = now;
      metronome_display_main(metronome_bpm, metronome_time_sig, current_beat, total_beats);
    }
    
    // Button 1: Decrease tempo
    if (button_is_pressed(btn1, false)) {
      if (!btn1Held) {
        btn1Held = true;
        btn1RepeatDelay = HOLD_INITIAL_MS;
        btn1NextRepeat = now + btn1RepeatDelay;
        if (metronome_bpm > MIN_BPM) metronome_bpm -= TEMPO_INCREMENT;
      } else {
        if (now >= btn1NextRepeat) {
          if (metronome_bpm > MIN_BPM) metronome_bpm -= TEMPO_INCREMENT;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn1RepeatDelay * HOLD_ACCEL_FACTOR);
          btn1RepeatDelay = newDelay;
          btn1NextRepeat = now + btn1RepeatDelay;
        }
      }
    } else {
      btn1Held = false;
    }
    
    // Button 2: Increase tempo
    if (button_is_pressed(btn2, false)) {
      if (!btn2Held) {
        btn2Held = true;
        btn2RepeatDelay = HOLD_INITIAL_MS;
        btn2NextRepeat = now + btn2RepeatDelay;
        if (metronome_bpm < MAX_BPM) metronome_bpm += TEMPO_INCREMENT;
      } else {
        if (now >= btn2NextRepeat) {
          if (metronome_bpm < MAX_BPM) metronome_bpm += TEMPO_INCREMENT;
          unsigned long newDelay = (unsigned long)max((float)HOLD_MIN_MS, btn2RepeatDelay * HOLD_ACCEL_FACTOR);
          btn2RepeatDelay = newDelay;
          btn2NextRepeat = now + btn2RepeatDelay;
        }
      }
    } else {
      btn2Held = false;
    }
    
    // Button 3: Time signature menu
    if (button_is_pressed(btn3, true)) {
      metronome_time_signature_menu();
      delay(200);
    }
    
    // Button 4: Subdivision menu
    if (button_is_pressed(btn4, true)) {
      metronome_subdivision_menu();
      delay(200);
    }
    
    // Button 5: Tap tempo
    if (button_is_pressed(btn5, true)) {
      metronome_tap_tempo();
      delay(200);
    }
    
    // Button 6: Exit metronome
    if (button_is_pressed(btn6, true)) {
      digitalWrite(Func1, LOW);
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
      display.print("4:Clr 5:Save 6:Back");
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
          display.setCursor(0, 56);
          display.print("1:< 2:> 3:Select");
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
