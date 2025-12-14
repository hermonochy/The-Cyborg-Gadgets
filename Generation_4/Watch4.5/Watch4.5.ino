// Watch 4.5: Designed for expeditions. Includes sensors and GPS.

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define TEMP_PIN 7

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte WhiteLED = A1;
const byte Laser = A2;
const byte UVLED = A3;

const byte btn1 = 2;
const byte btn2 = 3;
const byte btn3 = 4;
const byte btn4 = 5;

const byte lightSensor = A0;

const byte GPS_RX = 11;
const byte GPS_TX = 12;
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;

OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

int selectedFunction = 1;
const int totalFunctions = 9;

int selectedPart = 1;
const int totalParts = 3;

volatile bool wakeup = false;

struct FuncState {
  bool blink = false;
  bool keepOn = false;
  int blinkTime = 500;
  unsigned long lastBlink = 0;
  bool outputState = false;
};
FuncState funcStates[totalParts+1];

bool button_is_pressed(int btn){
  if (digitalRead(btn) == LOW){
    delay(100);
    return true;
  }
  return false;
}

void(* reset) (void) = 0;

void setup() {
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(WhiteLED, OUTPUT);
  pinMode(Laser, OUTPUT);
  pinMode(UVLED, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  tempSensor.begin();
  gpsSerial.begin(9600);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(7, 0);
  display.print("Wecome to");
  display.setCursor(20, 20);
  display.print("Watch 5");
  display.setCursor(30, 50);
  display.print("Gen 4");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();
  delay(1200);
}

void activateFunc(const byte func, int idx, int blinkTime = 500){
  FuncState &state = funcStates[idx];
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Quick Flash");
    display.setCursor(0, 30);
    display.println("2. Always On");
    display.setCursor(0, 40);
    display.println("3. Blink");
    display.setCursor(0, 50);
    display.println("4. Return");
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.print("State: ");
    display.print(digitalRead(func));
    display.display();

    if (!state.blink) delay(50);
    if (state.keepOn){
      digitalWrite(func, HIGH);
    }
    else if (state.blink){
      digitalWrite(func, !digitalRead(func));
      delay(state.blinkTime / 2);
    }
    else{
      if (button_is_pressed(btn1)) digitalWrite(func, HIGH);
      else digitalWrite(func, LOW);
    }

    if (button_is_pressed(btn2)){
      state.keepOn = !state.keepOn;
      if (state.keepOn) state.blink = false;
    }
    else if (button_is_pressed(btn3)){
      state.blink = !state.blink;
      if (state.blink) state.keepOn = false;
    }
    else if (button_is_pressed(btn4)){
      return;
    }
    delay(120);
  }
}

void outputFuncs(void){
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. White LED");
    display.setCursor(0, 30);
    display.println("2. Laser");
    display.setCursor(0, 40);
    display.println("3. UV LED");
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.print("Sel: ");
    display.print(selectedPart);
    display.display();

    if (button_is_pressed(btn2)){
      selectedPart++;
      if (selectedPart > totalParts) selectedPart = 1;
    }
    else if (button_is_pressed(btn1)){
      selectedPart--;
      if (selectedPart < 1) selectedPart = totalParts;
    }
    else if (button_is_pressed(btn4)) return;
    else if (button_is_pressed(btn3)){
      switch (selectedPart){
        case 1:
          activateFunc(WhiteLED, 0);
          break;
        case 2:
          activateFunc(Laser, 1);
          break;
        case 3:
          activateFunc(UVLED, 2);
          break;
      }
    }
    delay(120);
  }
}

void clock(){
  while (!button_is_pressed(btn4)){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    if (gps.time.isValid() && gps.date.isValid()) {
      char buf[12];
      sprintf(buf, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
      display.print(buf);
      display.setTextSize(1);
      display.setCursor(0, 30);
      sprintf(buf, "%02d/%02d/%04d", gps.date.day(), gps.date.month(), gps.date.year());
      display.print(buf);
    } 
    else {
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Waiting for GPS ...");
    }
    display.display();
  }
}

void GPS(){
  while (!button_is_pressed(btn4)){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    if (gps.location.isValid()) {
      display.print("Lat: "); display.println(gps.location.lat(), 6);
      display.print("Lon: "); display.println(gps.location.lng(), 6);
      display.print("Alt: "); display.print(gps.altitude.meters(), 1); display.println("m");
      display.print("Sat: "); display.print(gps.satellites.value());
    } else {
      display.print("Waiting for GPS...");
    }
    display.display();
  }
}

void sensors(){
  static float tempVal = 0;
  while (!button_is_pressed(btn4)){
    tempSensor.requestTemperatures();
    tempVal = tempSensor.getTempCByIndex(0);
    int lightVal = analogRead(lightSensor);

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Sensor Values");
    display.setCursor(0, 25);
    display.print("Temp: ");
    display.print(tempVal, 1); 
    display.print("C");
    display.setCursor(0, 45);
    display.print("Light: ");
    display.print(lightVal, 1);
    display.display();
  }
}

void speed() {
  while (!button_is_pressed(btn4)){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    if (gps.speed.isValid()) {
      display.print("SPD ");
      display.setCursor(0, 25);
      display.print(gps.speed.kmph(), 1);
      display.print("km/h");
      display.setTextSize(1);
      display.setCursor(0, 40);
      display.print("m/s: ");
      display.print(gps.speed.mps(), 1);
      display.setCursor(0, 55);
      display.print("mph: ");
      display.print(gps.speed.mph(), 1);
    } 
    else {
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print("Waiting for GPS...");
    }
  }
}

void unitConverter(void){
    const char* types[] ={
        "cm->in", "in->cm",
        "C->F",   "F->C",
        "kg->lb", "lb->kg",
        "km->mi", "mi->km",
        "g->oz",  "oz->g",
        "L->gal", "gal->L"
    };
    enum{LEN=0, LEN2, TEMP, TEMP2, WT, WT2, KM_MI, MI_KM, G_OZ, OZ_G, L_GAL, GAL_L};
    const int numTypes = sizeof(types) / sizeof(types[0]);
    int selectedType = 0;
    float inputValue = 0;
    bool enteringValue = true;
    float result = 0;

    while (true){
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 25);
        display.print("Type: ");
        display.print(types[selectedType]);

        display.setCursor(0, 50);
        display.print("Input: ");
        display.setTextSize(2);
        display.setCursor(50, 45);
        display.print(inputValue, 2);

        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("= ");
        if (!enteringValue)
            display.print(result, 2);

        display.display();

        if (button_is_pressed(btn3)){
            selectedType = (selectedType + 1) % numTypes;
            enteringValue = true;
            delay(200);
        }
        else if (button_is_pressed(btn2)){
            if (enteringValue) inputValue--;
            delay(100);
        }
        else if (button_is_pressed(btn1)){
            if (enteringValue) inputValue++;
            delay(100);
        }
        else if (button_is_pressed(btn4)){
            if (enteringValue){
                switch (selectedType){
                    case LEN:   result = inputValue / 2.54; break;         // cm to in
                    case LEN2:  result = inputValue * 2.54; break;         // in to cm
                    case TEMP:  result = inputValue * 9.0 / 5.0 + 32.0; break; // C to F
                    case TEMP2: result = (inputValue - 32.0) * 5.0 / 9.0; break; // F to C
                    case WT:    result = inputValue * 2.20462; break;      // kg to lb
                    case WT2:   result = inputValue / 2.20462; break;      // lb to kg
                    case KM_MI: result = inputValue * 0.621371; break;     // km to mi
                    case MI_KM: result = inputValue / 0.621371; break;     // mi to km
                    case G_OZ:  result = inputValue * 0.035274; break;     // g to oz
                    case OZ_G:  result = inputValue / 0.035274; break;     // oz to g
                    case L_GAL: result = inputValue * 0.264172; break;     // L to gal (US)
                    case GAL_L: result = inputValue / 0.264172; break;     // gal (US) to L
                }
                enteringValue = false;
                delay(500);
            }
            else{
                return;
            }
        }
        delay(30);
    }
}

void ruler() {
  while (!button_is_pressed(btn4)){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Ruler");
    int barLen = 100;
    int startY = 30;
    display.drawLine(10, startY, 10+barLen, startY, SSD1306_WHITE);
    for (int i = 0; i <= 10; i++) {
      int x = 10 + i*10;
      display.drawLine(x, startY-5, x, startY+5, SSD1306_WHITE);
      char lbl[2]; sprintf(lbl, "%d", i);
      display.setCursor(x-2, startY+8); display.print(lbl);
    }
    display.setCursor(10, startY-14);
    display.print("0     1    2    3    4    5cm");
    display.display();
  }
}

void goToSleep(){

    display.ssd1306_command(SSD1306_DISPLAYOFF);

    ADCSRA &= ~(1 << ADEN);
    sleep_bod_disable();

    wakeup = false;
    attachInterrupt(digitalPinToInterrupt(btn3), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn3));
 
    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);

}

void loop() {
  if (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Home     ");
  display.print(selectedFunction);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("1. Clock    5. Speed");
  display.setCursor(0, 32);
  display.print("2. GPS      6. Sensors");
  display.setCursor(0, 44);
  display.print("3. Outputs  7. Ruler");
  display.setCursor(0, 56);
  display.print("4. Convert  8. Power");

  display.display();

  if (button_is_pressed(btn2)){
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  }
  else if (button_is_pressed(btn1)){
    selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
  }
  else if (button_is_pressed(btn3)){
    switch (selectedFunction){
      case 1:
        clock();
        break;
      case 2:
        GPS();
        break;
      case 3:
        outputFuncs();
        break;
      case 4:
        unitConverter();
        break;
      case 5:
        speed();
        break;
      case 6:
        sensors();
        break;
      case 7:
        ruler();
        break;
      case 8:
        goToSleep();
        break;

    }
  }
  else if (button_is_pressed(btn4)){
    reset();
  }
  delay(100);
}
