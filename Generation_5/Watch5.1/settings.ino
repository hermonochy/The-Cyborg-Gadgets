extern Adafruit_GC9A01A display;
extern void loadBtnVals();
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern bool a_button_is_pressed();
extern void saveBtnVals();
extern void initializeNotesNVS();
extern const char* settingFuncs[];
extern int buttonOffset, buttonValRange, blinkTime1, blinkTime2, blinkTime3;
extern const byte buttonPin;
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG,colourText,colour1,colour2,colour3,colour4,colour5,colour6;
extern bool inverted;
extern const int defBtn1, defBtn2, defBtn3, defBtn4, defBtn5, defBtn6;
extern byte Func1, Func2, Func3;

#define numSettings 5

int rotation = 0;

int *btnRefs[] = {&btn1, &btn2, &btn3, &btn4, &btn5, &btn6};
const int *defBtnRefs[] = {&defBtn1, &defBtn2, &defBtn3, &defBtn4, &defBtn5, &defBtn6};
const char *labels[] = {"Btn 1", "Btn 2", "Btn 3", "Btn 4", "Btn 5", "Btn 6"};

uint16_t *colours[] = {&colourBG,&colourText,&colour1,&colour2,&colour3,&colour4,&colour5,&colour6};

void settings() {
  int sel = 0;
  int menuCount = 5;
  int prevSel = -1;
  bool first = true;
  while (true) {
    // Redraw screen only if selection changes or on first entry
    if (first || sel != prevSel) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);

      display.setTextSize(2);
      display.setTextColor(colourText);
      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds("SETTINGS", 0, 0, &x1, &y1, &w, &h);
      display.setCursor(120 - w / 2, 22);
      display.print("SETTINGS");

      int Y = 70, spacing = 30;
      for (int i = 0; i < menuCount; i++) {
        display.setTextSize(i == sel ? 2 : 1);
        display.setTextColor(i == sel ? colour6 : colour1);
        display.setCursor(20, Y + i * spacing);
        switch(i) {
          case 0: display.print("Tune Btns"); break;
          case 1: display.print("Preferences"); break;
          case 2: display.print("Debug"); break;
          case 3: display.print("Btn Settings"); break;
          case 4: display.print("LCD Settings"); break;
        }
      }
      prevSel = sel;
      first = false;
    }

    if (button_is_pressed(btn2, true)) { sel = (sel + 1) % menuCount; }
    else if (button_is_pressed(btn1, true)) { sel = (sel + menuCount - 1) % menuCount; }
    else if (button_is_pressed(btn3, true)) {
      switch(sel) {
        case 0: tuneButtonVals(); break;
        case 1: prefs(); break;
        case 2: debug(); break;
        case 3: btnSettings(); break;
        case 4: LCDSettings(); break;
      }
      prevSel = -1; first = true;
      continue;
    }
    else if (button_is_pressed(btn6, true)) {
      display.fillScreen(colourBG);
      return;
    }
    delay(40);
  }
}

void tuneButtonVals() {
  int16_t x1, y1; uint16_t w, h;
  const int sampleCount = 75;
  int samples[sampleCount];
  for (int i = 0; i < 6; i++) {
    bool first = true; bool lastPromptDrawn = true;
    while (!a_button_is_pressed()) {
      if (first) {
        display.fillScreen(colourBG);
        display.setTextSize(2);
        String prompt = String("Push ") + labels[i];
        display.getTextBounds(prompt.c_str(), 0, 0, &x1, &y1, &w, &h);
        display.setCursor(120 - w / 2, 90);
        display.setTextColor(colour4);
        display.print(prompt);
        first = false;
      }
      delay(13);
    }
    for (int s = 0; s < sampleCount; s++) {
      samples[s] = analogRead(buttonPin); 
      delay(1); 
    }
    for (int x = 0; x < sampleCount - 1; x++) {
      for (int y = 0; y < sampleCount - x - 1; y++) {
        if (samples[y] > samples[y + 1]) {
          int temp = samples[y];
          samples[y] = samples[y + 1];
          samples[y + 1] = temp;
        }
      }
    }
    *btnRefs[i] = samples[sampleCount/2];

    display.fillRect(0, 90, 240, 40, colourBG);
    String setMsg = String(labels[i]) + " Set";
    display.setTextSize(2);
    display.getTextBounds(setMsg.c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor(120 - w / 2, 90);
    display.setTextColor(colour3);
    display.print(setMsg);

    while(a_button_is_pressed()) delay(18);
  }
}

void prefs() {
  int settingIndex = 0, prevIndex = -1;
  int prevOffset = -9999, prevRange = -9999, prevBlink1 = -2, prevFunc1=-2, prevBlink2=-2, prevFunc2=-2, prevFunc3=-2, prevRot=-2;
  bool displayOff = false;
  int16_t x1, y1; uint16_t w, h;
  while(!button_is_pressed(btn6, true)){
    if(settingIndex != prevIndex){
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);

      display.setTextSize(2);
      display.setTextColor(colourText);
      display.getTextBounds("PREFERENCES", 0, 0, &x1, &y1, &w, &h);
      display.setCursor(120 - w / 2, 20);
      display.print("PREFERENCES");

      display.setTextSize(2);
      display.setTextColor(colour6);
      String thisSetting = settingFuncs[settingIndex];
      display.getTextBounds(thisSetting, 0,0, &x1, &y1, &w, &h);
      display.setCursor(120-w/2, 65);
      display.print(thisSetting);
      prevIndex = settingIndex;
      prevOffset = prevRange = prevBlink1 = prevFunc1 = prevBlink2 = prevFunc2 = prevFunc3 = prevRot = -9999;
    }
    switch(settingIndex) {
      case 0:
        if (buttonOffset != prevOffset || buttonValRange != prevRange) {
          display.fillRect(40, 105, 180, 50, colourBG);
          display.setTextSize(1);
          display.setTextColor(colourText);
          display.setCursor(50, 105);
          display.print("ADC: "); display.print(analogRead(buttonPin));
          display.setCursor(50, 125);
          display.print("Offset: "); display.print(buttonOffset);
          display.setCursor(50, 145);
          display.print("Range: "); display.print(buttonValRange);
          prevOffset = buttonOffset; prevRange = buttonValRange;
        }

        if(button_is_pressed(btn2, false)) buttonOffset++;
        else if(button_is_pressed(btn1, false)) buttonOffset--;
        if(button_is_pressed(btn5, false)) buttonValRange++;
        else if(button_is_pressed(btn4, false)) buttonValRange--;
        else if(button_is_pressed(btn3, true)) buttonOffset = 0;
        break;
      case 1:
        if(blinkTime1 != prevBlink1 || Func1 != prevFunc1) {
          display.fillRect(40, 110, 180, 40, colourBG);
          display.setTextSize(1);
          display.setTextColor(colourText);
          display.setCursor(40, 110);
          display.print("Blk1: "); display.print(blinkTime1); display.print("us");
          display.setCursor(40, 135);
          display.print("Func1: "); display.print(Func1);
          prevBlink1 = blinkTime1; prevFunc1 = Func1;
        }

        if(button_is_pressed(btn1, true)) { blinkTime1 *= 2; if(blinkTime1 > 5000000) blinkTime1 = 5000000; }
        if(button_is_pressed(btn2, true)) { blinkTime1 /= 2; if(blinkTime1 < 1) blinkTime1 = 1; }
        if(button_is_pressed(btn3, true)) { Func1--; if(Func1 < 0) Func1 = 10; }
        if(button_is_pressed(btn4, true)) { Func1++; if(Func1 > 10) Func1 = 0; }
        break;
      case 2:
        if(blinkTime2 != prevBlink2 || Func2 != prevFunc2) {
          display.fillRect(40, 110, 180, 40, colourBG);
          display.setTextSize(1);
          display.setTextColor(colourText);
          display.setCursor(40, 110);
          display.print("Blk2: "); display.print(blinkTime2); display.print("us");
          display.setCursor(40, 135);
          display.print("Func2: "); display.print(Func2);
          prevBlink2 = blinkTime2; prevFunc2 = Func2;
        }

        if(button_is_pressed(btn1, true)) { blinkTime2 *= 2; if(blinkTime2 > 5000000) blinkTime2 = 5000000; }
        if(button_is_pressed(btn2, true)) { blinkTime2 /= 2; if(blinkTime2 < 1) blinkTime2 = 1; }
        if(button_is_pressed(btn3, true)) { Func2--; if(Func2 < 0) Func2 = 10; }
        if(button_is_pressed(btn4, true)) { Func2++; if(Func2 > 10) Func2 = 0; }
        break;
      case 3:
        if(Func3 != prevFunc3) {
          display.fillRect(80, 110, 100, 20, colourBG);
          display.setTextSize(1);
          display.setTextColor(colourText);
          display.setCursor(95, 110);
          display.print("Func3: "); display.print(Func3);
          prevFunc3 = Func3;
        }

        if(button_is_pressed(btn1, true)) { Func3--; if(Func3 < 0) Func3 = 10; }
        if(button_is_pressed(btn2, true)) { Func3++; if(Func3 > 10) Func3 = 0; }
        break;
      case 4:
        if(rotation != prevRot) {
          display.fillRect(95, 110, 90, 20, colourBG);
          display.setTextSize(1);
          display.setTextColor(colourText);
          display.setCursor(95, 110);
          display.print("Rotation: ");
          display.print(rotation%4);
          prevRot = rotation;
        }
        if(button_is_pressed(btn1, true)) rotation++;
        if(button_is_pressed(btn2, true)) {
          if(displayOff) displayOff = false;
          else displayOff = true;
        }
        display.setRotation(rotation % 4);
        delay(80);
        break;
    }
    delay(80);
    if(button_is_pressed(btn3, true)) settingIndex = (settingIndex + 1) % numSettings;
  }
}

void debug() {
  int16_t x1, y1; uint16_t w, h;
  int posY;
  int prevadc = -1, prevbtns[6] = {-1, -1, -1, -1, -1, -1};
  bool first = true;
  while (true) {
    int adc = analogRead(buttonPin);
    bool changed = first || (adc != prevadc);
    for(int i=0;i<6;i++) if (*btnRefs[i] != prevbtns[i]) changed = true;

    if (changed) {
      display.fillScreen(colourBG);
      display.fillCircle(120, 120, 120, colourBG);

      display.setTextSize(2);
      display.setTextColor(colourText);
      display.getTextBounds("DEBUG", 0,0, &x1,&y1,&w,&h);
      display.setCursor(120-w/2, 12);
      display.print("DEBUG");

      display.setTextSize(1);
      display.setCursor(62, 42);
      display.setTextColor(colour6);
      display.print("ADC: ");
      display.setTextColor(colourText);
      display.print(adc);
      for (int i = 0; i < 6;i++) {
        posY = 62 + i*20;
        display.setCursor(34, posY);
        display.setTextColor(colour4);
        display.print(labels[i]);
        display.setTextColor(colourText);
        display.print(": ");
        display.print(*btnRefs[i]);
        display.setCursor(154, posY);
        display.setTextColor(colour1);
        int diff = *defBtnRefs[i]-*btnRefs[i];
        display.print("(");
        if (diff > 0) display.print("+"); 
        display.print(diff);
        display.print(")");
        prevbtns[i] = *btnRefs[i];
      }
      prevadc = adc;
      first = false;
    }
    if (button_is_pressed(btn6)) return;
    delay(100);
  }
}

void btnSettings(){
  int sel = 0, prevSel = -1;
  while(true){
    if (sel != prevSel) {
      display.fillScreen(colourBG);
      display.fillCircle(120,120,120,colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds("BTN SETTINGS", 0,0, &x1, &y1, &w, &h);
      display.setCursor(120-w/2, 24);
      display.print("BTN SETTINGS");

      display.setTextSize(sel == 0 ? 2 : 1);
      display.setTextColor(sel == 0 ? colour6 : colour1);
      display.setCursor(40,90);
      display.print("Save Btn Vals");
      display.setTextSize(sel == 1 ? 2 : 1);
      display.setTextColor(sel == 1 ? colour6 : colour1);
      display.setCursor(40,140);
      display.print("Revert Btn Vals");

      prevSel = sel;
    }
    if (button_is_pressed(btn4, true)) { sel = (sel + 1) % 2;}
    else if (button_is_pressed(btn1, true)) { sel = (sel == 0 ? 1 : 0);}
    else if (button_is_pressed(btn3, true)) {
      if (sel == 0) { saveBtnVals(); }
      else {
        for (int i = 0; i < 6; i++) *btnRefs[i] = *defBtnRefs[i];
      }
    }
    else if (button_is_pressed(btn6, true)) { display.fillScreen(colourBG); return;}
    delay(60);
  }
}

void LCDSettings(){
  // int *colours[] = {&colourBG,&colourText,&colour1,&colour2,&colour3,&colour4,&colour5,&colour6};
  int sel = 0, prevSel = -1;
  while(true){
    if (sel != prevSel) {
      display.fillScreen(colourBG);
      display.fillCircle(120,120,120,colourBG);
      display.setTextSize(2);
      display.setTextColor(colourText);
      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds("LCD SETTINGS", 0,0, &x1, &y1, &w, &h);
      display.setCursor(120-w/2, 24);
      display.print("LCD SETTINGS");

      display.setTextSize(sel == 0 ? 2 : 1);
      display.setTextColor(sel == 0 ? colour6 : colour1);
      display.setCursor(40,90);
      display.print("Light/Dark");
      display.setTextSize(sel == 1 ? 2 : 1);
      display.setTextColor(sel == 1 ? colour6 : colour1);
      display.setCursor(40,140);
      display.print("Change Colours");

      prevSel = sel;
    }
    if (button_is_pressed(btn4)) { sel = (sel + 1) % 2;}
    else if (button_is_pressed(btn1)) { sel = (sel == 0 ? 1 : 0);}
    else if (button_is_pressed(btn3, true)) {
      if (sel == 0) {
        display.invertDisplay(inverted); 
        inverted = !inverted;
      }
      else break;
    }
    else if (button_is_pressed(btn6, true)) return;
    delay(60);
  }
}