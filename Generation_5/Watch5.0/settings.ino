// Includes: Settings, Prefs, Debug

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const int btn1, btn2, btn3, btn4, btn5, btn6;
extern byte Func1, Func2, Func3;

void settings() {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Preferences");
    display.println("2. Debug");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) prefs();
    else if (button_is_pressed(btn2)) debug();
    else if (button_is_pressed(btn6)) return;
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
        display.print(buttonOffset);
        if(button_is_pressed(btn2)) {
          buttonOffset++;
        }
        else if(button_is_pressed(btn1)) {
          buttonOffset--;
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
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Debug Info");
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("ADC: ");
    display.print(analogRead(buttonPin));
    display.setCursor(0, 30);
    display.print("Func1: ");
    display.print(digitalRead(Func1));
    display.setCursor(0, 40);
    display.print("Func2: ");
    display.print(digitalRead(Func2));
    display.setCursor(0, 50);
    display.print("Func3: ");
    display.print(digitalRead(Func3));
    display.display();
    
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(100);
  }
}
