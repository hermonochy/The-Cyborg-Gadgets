// Includes: Settings, tuneButtonVals, Prefs, Debug

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const byte buttonPin;
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern byte Func1, Func2, Func3;

int *btnRefs[] = {&btn1, &btn2, &btn3, &btn4, &btn5, &btn6};
const char *labels[] = {"Btn 1", "Btn 2", "Btn 3", "Btn 4", "Btn 5", "Btn 6"};


void settings() {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Tune Btns");
    display.println("2. Preferences");
    display.println("3. Debug");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) tuneButtonVals();
    else if (button_is_pressed(btn2)) prefs();
    else if (button_is_pressed(btn3)) debug();
    else if (button_is_pressed(btn6)) return;
  }
}

// Similar in purpose to buttonOffset, but specific for every button
void tuneButtonVals() {
  // ensure no mis-measurements
  while (a_button_is_pressed()) {}
  const int sampleCount = 75;
  int samples[sampleCount];

  for (int i = 0; i < 6; i++) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 30);
    display.print("Push ");
    display.print(labels[i]);
    display.display();
    while (!a_button_is_pressed()) delay(50);

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

    display.clearDisplay();
    display.setCursor(0, 30);
    display.print(labels[i]);
    display.print(" Set");
    display.display();

    while(a_button_is_pressed()) delay(50);
  }
}

void prefs() {
  int settingIndex = 0;
  bool displayOff = false;
  while(!button_is_pressed(btn6, true)){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,5);
    display.print(settingFuncs[settingIndex]);

    display.setTextSize(2);
    display.setCursor(0,24);

    switch(settingIndex) {
      case 0:
        display.println(analogRead(buttonPin));
        display.println(buttonOffset);
        display.println(buttonValRange);
        if(button_is_pressed(btn2, false)) {
          buttonOffset++;
        }
        else if(button_is_pressed(btn1, false)) {
          buttonOffset--;
        }
        if(button_is_pressed(btn5, false)) {
          buttonValRange++;
        }
        else if(button_is_pressed(btn4, false)) {
          buttonValRange--;
        }
        else if(button_is_pressed(btn3)) buttonOffset = 0;
        break;
      case 1:
        display.print(Func1);
        if(button_is_pressed(btn1)) {
          Func1--;
          if(Func1 < 0) Func1 = 10;
        }
        if(button_is_pressed(btn2, true)) {
          Func1++;
          if(Func1 > 10) Func1 = 0;
        }
        break;
      case 2:
      display.print(Func2);
      if(button_is_pressed(btn1)) {
        Func2--;
        if(Func2 < 0) Func1 = 10;
      }
      if(button_is_pressed(btn2, true)) {
        Func2++;
        if(Func2 > 10) Func2 = 0;
      }
        break;
      case 3:
      display.print(Func3);
      if(button_is_pressed(btn1)) {
        Func3--;
        if(Func3 < 0) Func3 = 10;
      }
      if(button_is_pressed(btn2, true)) {
        Func3++;
        if(Func3 > 10) Func3 = 0;
      }
        break;
      case 4:
        display.print(rotation%4);
        if(button_is_pressed(btn1)) {
          rotation ++;
        }
        if(button_is_pressed(btn2, true)) {
          if(displayOff) {
            display.ssd1306_command(SSD1306_DISPLAYON);
            displayOff = false;
          }
          else {
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            displayOff = true;
          }
        }
        display.setRotation(rotation%4);
        delay(50);
        break;
    }
    display.display();
    
    if(button_is_pressed(btn3, true)) {
      settingIndex = (settingIndex + 1) % numSettings;
    }
  }
}

void debug() {
  int posY;
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("ADC: ");
    display.print(analogRead(buttonPin));
    display.setTextSize(1);
    for (int i = 0; i < 6;i++) {
      posY = 18 + i*8;
      display.setCursor(0, posY);
      display.print(labels[i]);
      display.print(": ");
      display.print(*btnRefs[i]);
    }
    display.display();
    // an alternative needs to be found here:
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(100);
  }
}
