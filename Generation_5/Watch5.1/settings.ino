// Includes: Settings, tuneButtonVals, Prefs, Debug, save btn vals, chip stats

#include <esp_system.h>
#include <esp_cpu.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_spi_flash.h>
#include <rom/rtc.h>

extern Adafruit_GC9A01A display;
extern void loadBtnVals();
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern const byte buttonPin;
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern const int defBtn1, defBtn2,defBtn3,defBtn4,defBtn5,defBtn6;
extern byte Func1, Func2, Func3;

int *btnRefs[] = {&btn1, &btn2, &btn3, &btn4, &btn5, &btn6};
const int *defBtnRefs[] = {&defBtn1, &defBtn2, &defBtn3, &defBtn4, &defBtn5, &defBtn6};

const char *labels[] = {"Btn 1", "Btn 2", "Btn 3", "Btn 4", "Btn 5", "Btn 6"};

void settings() {
  while (true) {
    display.fillScreen(GC9A01A_BLACK);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Tune Btns");
    display.println("2. Preferences");
    display.println("3. Debug");
    display.println("4. System Info");
    display.println("5. Btn Settings");
    delay(50);
    
    if (button_is_pressed(btn1)) tuneButtonVals();
    else if (button_is_pressed(btn2)) prefs();
    else if (button_is_pressed(btn3)) debug();
    else if (button_is_pressed(btn4)) chipStats();
    else if (button_is_pressed(btn5)) btnSettings();
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
    display.fillScreen(GC9A01A_BLACK);
    display.setTextSize(2);
    display.setCursor(0, 30);
    display.print("Push ");
    display.print(labels[i]);
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

    display.fillScreen(GC9A01A_BLACK);
    display.setCursor(0, 30);
    display.print(labels[i]);
    display.print(" Set");

    while(a_button_is_pressed()) delay(50);
  }
}

int rotation = 0;
void prefs() {
  int settingIndex = 0;
  bool displayOff = false;
  while(!button_is_pressed(btn6, true)){
    display.fillScreen(GC9A01A_BLACK);
    display.setTextSize(1);
    display.setCursor(0,5);
    display.print(settingFuncs[settingIndex]);

    display.setTextSize(1);
    display.setCursor(0,24);

    switch(settingIndex) {
      case 0:
        display.print(analogRead(buttonPin));
        display.setCursor(0,36);
        display.print(buttonOffset);
        display.setCursor(0,48);
        display.print(buttonValRange);
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
        display.print("Blk 1: ");
        display.print(blinkTime1);
        display.print("us");
        display.setCursor(0,40);
        display.print("Func1: ");
        display.print(Func1);
        if(button_is_pressed(btn1)) {
          blinkTime1 *= 2;
          if(blinkTime1 > 5000000) blinkTime1 = 5000000;
        }
        if(button_is_pressed(btn2)) {
          blinkTime1 /= 2;
          if(blinkTime1 < 1) blinkTime1 = 1;
        }
        if(button_is_pressed(btn3)) {
          Func1--;
          if(Func1 < 0) Func1 = 10;
        }
        if(button_is_pressed(btn4, true)) {
          Func1++;
          if(Func1 > 10) Func1 = 0;
        }
        break;
      case 2:
        display.print("Blk 2: ");
        display.print(blinkTime2);
        display.print("us");
        display.setCursor(0,40);
        display.print("Func2: ");
        display.print(Func2);
        if(button_is_pressed(btn1)) {
          blinkTime2 *= 2;
          if(blinkTime2 > 5000000) blinkTime2 = 5000000;
        }
        if(button_is_pressed(btn2)) {
          blinkTime2 /= 2;
          if(blinkTime2 < 1) blinkTime2 = 1;
        }
        if(button_is_pressed(btn3)) {
        Func2--;
        if(Func2 < 0) Func1 = 10;
      }
      if(button_is_pressed(btn4, true)) {
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
            //display.GC9A01A_command(GC9A01A_DISPLAYON);
            displayOff = false;
          }
          else {
            //display.GC9A01A_command(GC9A01A_DISPLAYOFF);
            displayOff = true;
          }
        }
        display.setRotation(rotation%4);
        delay(50);
        break;
    }
    delay(100);
    if(button_is_pressed(btn3, true)) {
      settingIndex = (settingIndex + 1) % numSettings;
    }
  }
}

void debug() {
  int posY;
  while (true) {
    display.fillScreen(GC9A01A_BLACK);
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
      display.setCursor(90, posY);
      display.print("(");
      if (*defBtnRefs[i]-*btnRefs[i] > 0) display.print("+"); 
      display.print(*defBtnRefs[i]-*btnRefs[i]);
      display.print(")");
    }
    // TODO: an alternative needs to be found here:
    if (button_is_pressed(btn6)) {
      return;
    }
    delay(100);
  }
}

void chipStats() {
  char lines[12][36];
  int n = 0;
  snprintf(lines[n++], 36, "ESP32C3 Chip Info:");
  snprintf(lines[n++], 36, "Board: %s", ARDUINO_BOARD);
  snprintf(lines[n++], 36, "CPU MHz: %d", getCpuFrequencyMhz());
  snprintf(lines[n++], 36, "Flash: %luKB", ESP.getFlashChipSize() / 1024);
  snprintf(lines[n++], 36, "Sketch: %luKB", ESP.getSketchSize() / 1024);
  snprintf(lines[n++], 36, "Free Sketch: %luKB", ESP.getFreeSketchSpace() / 1024);

  uint64_t mac64 = ESP.getEfuseMac();
  snprintf(lines[n++], 36,
    "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
    (uint8_t)(mac64>>40), (uint8_t)(mac64>>32), (uint8_t)(mac64>>24),
    (uint8_t)(mac64>>16), (uint8_t)(mac64>>8), (uint8_t)mac64);

  snprintf(lines[n++], 36, "Heap: %lu", ESP.getFreeHeap());
  snprintf(lines[n++], 36, "Min Heap: %lu", ESP.getMinFreeHeap());
  snprintf(lines[n++], 36, "Chip Rev: %d", ESP.getChipRevision());
  snprintf(lines[n++], 36, "SDK: %s", ESP.getSdkVersion());

  int scroll = 0;
  while (true) {
    display.fillScreen(GC9A01A_BLACK);
    display.setTextSize(1);
    for (int i = 0; i < 5 && i + scroll < n; i++) {
      int y = 8 + i * 12;
      if (y >= 10 && y < 18) y = 20; // skip gap
      display.setCursor(0, y);
      display.print(lines[i + scroll]);
    }
    display.setCursor(0, 0); display.print("System Info");
    if (button_is_pressed(btn1)) { if (scroll > 0) scroll--; delay(120); }
    else if (button_is_pressed(btn2)) { if (scroll < n - 5) scroll++; delay(120); }
    else if (button_is_pressed(btn3)) runtimeStats();
    else if (button_is_pressed(btn6, true)) break;
    delay(40);
  }
}

void runtimeStats() {
  while (true) {
    unsigned long seconds = millis() / 1000;
    unsigned long days = seconds / 86400;
    unsigned long hrs = (seconds % 86400) / 3600;
    unsigned long mins = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;

    display.fillScreen(GC9A01A_BLACK);
    display.setTextSize(1);
    int y=8;
    display.setCursor(0,0); display.print("ESP32-C3 Runtime");
    display.setCursor(0,y); display.printf("Uptime: %lud %luh%lum%lus", days, hrs, mins, secs); y+=12;
    if (y >= 10 && y < 18) y = 20;
    display.setCursor(0,y); display.printf("Heap: %lu", ESP.getFreeHeap()); y+=12;
    if (y >= 10 && y < 18) y = 20;
    display.setCursor(0,y); display.printf("MinHeap: %lu", ESP.getMinFreeHeap()); y+=12;
    if (y >= 10 && y < 18) y = 20;
    display.setCursor(0,y); display.printf("CPU MHz: %d", getCpuFrequencyMhz()); y+=12;
    if (y >= 10 && y < 18) y = 20;
    display.setCursor(0,y); display.print("Sketch: ");
    display.print(ESP.getSketchSize()/1024); display.print("KB");
    y+=12; if (y >= 10 && y < 18) y = 20;
    display.setCursor(0,y); display.print("SketchFree: ");
    display.print(ESP.getFreeSketchSpace()/1024); display.print("KB");

    unsigned long ref = millis();
    while (millis() - ref < 950) {
      if (button_is_pressed(btn6, true)) return;
      delay(20);
    }
  }
}

void btnSettings(){
  display.fillScreen(GC9A01A_BLACK);
  display.setCursor(5, 20);
  display.println("1. Save Btn Vals");
  display.println("2. Revert Btn Vals");
  
  while(true){
    if (button_is_pressed(btn1)) saveBtnVals();
    else if (button_is_pressed(btn2)) {
      for (int i = 0; i < 6; i++){
        *btnRefs[i] = *defBtnRefs[i];
      }
    }
    else if (button_is_pressed(btn6)) return;
  }
}