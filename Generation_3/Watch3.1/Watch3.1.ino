// Watch 3.1: communicates via neopixel

#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

int Wheel(byte WheelPos);

#define NEOPIXEL_PIN 10
#define NUM_NEOPIXEL 1

Adafruit_NeoPixel pixel(NUM_NEOPIXEL, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

int brightness = 255;

const byte btn1 = 3;
const byte btn2 = 11;
const byte btn3 = A0;
const byte btn4 = 6;

const byte Func1 = 8;
const byte Func2 = 9;
const byte Func3 = 11;
const byte Func4 = 7;

int selectedFunction = 1;
const int totalFunctions = 5;

int selectedPart = 1;
const int totalParts = 4;

const byte resetPin = 12;

volatile bool wakeup = false;

void setup(){
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  pinMode(Func4, OUTPUT);
  pixel.begin();
  pixel.clear();
  for (int i = 0; i <= 255; i++) {
    pixel.setPixelColor(0, Wheel(i));
    pixel.show();
    if (button_is_pressed(btn1)) brightness = 1;
    else if (button_is_pressed(btn2)) brightness = 4;
    else if (button_is_pressed(btn3)) brightness = 16;
    else if (button_is_pressed(btn4)) brightness = 64;
    delay(5);
  }
  
  pixel.clear();
  pixel.show();
  delay(200);
  showFunction(selectedFunction);
}

bool button_is_pressed(int btn){
  if (digitalRead(btn) == LOW){
    delay(100);
    return true;
  }
  return false;
}

void reset(){
  for (int pin=2; pin<=13; pin++){
    digitalWrite(pin, LOW);
  }
  digitalWrite(resetPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(resetPin, LOW);
}

void wakeUp(){
    wakeup = true;
}

void goToSleep(){
    pixel.clear();
    pixel.show();
    digitalWrite(LED_BUILTIN, LOW);
    
    sleep_bod_disable();
    wakeup = false;
    attachInterrupt(digitalPinToInterrupt(btn1), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn1));
}

void blinkColor(uint32_t color, int times, int on_ms = 150, int off_ms = 180){
  for (int i = 0; i < times; i++){
    pixel.setPixelColor(0, color);
    pixel.show();
    delay(on_ms);
    pixel.clear();
    pixel.show();
    delay(off_ms);
  }
}

int Wheel(int WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void showFunction(int funcNum){
  uint32_t colors[] = {
    pixel.Color(brightness, brightness, brightness),   // 1: Output Control
    pixel.Color(0, brightness, 0),                    // 2: Color Picker
    pixel.Color(brightness, 0, 0),                   // 3: Bomb/Detonate
    pixel.Color(brightness, brightness, 0),         // 4: Random Int
    pixel.Color(0, 0, brightness)                  // 5: Sleep Mode
  };
  pixel.clear();
  pixel.setPixelColor(0, colors[funcNum - 1]);
  pixel.show();
}

void activateOutput(const byte func, int blinkTime = 500){
  bool blink = false;
  bool keepOn = false;

  while (true){
    
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime / 2);
    } 
    else {
      if (button_is_pressed(btn1)) digitalWrite(func, HIGH);
      else digitalWrite(func, LOW);
    }

    if (button_is_pressed(btn2)){
      keepOn = !keepOn;
      if (keepOn) blink = false;
    } 
    else if (button_is_pressed(btn3)){
      blink = !blink;
      if (blink) keepOn = false;
    }
    else if (button_is_pressed(btn4)){
      return;
    delay(200);
    }
  }
}

void outputs(void){
  delay(50);
  int outputColours[] = {
    pixel.Color(brightness, brightness, brightness),    // 1: White LED
    pixel.Color(brightness, 0, 0),                     // 2: Laser
    pixel.Color(brightness, brightness, 0),           // 3: Built In LED
    pixel.Color(0, brightness, 0)                    // 4: 
  };

  while (true){

    pixel.clear();
    pixel.setPixelColor(0, outputColours[selectedPart - 1]);
    pixel.show();
    delay(50);

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
          activateOutput(Func1);
          continue;
        case 2:
          activateOutput(Func2, 1000);
          continue;
        case 3:
          activateOutput(LED_BUILTIN, 100);
          continue;
        case 4:
          continue;
      }
    }
  }
}

void colourPicker(){
  int r = 0, g = 0, b = 0;
  int sel = 0;
  while (true){
    pixel.setPixelColor(0, pixel.Color(r, g, b));
    pixel.show();
    delay(30);

    if (button_is_pressed(btn3)){sel = (sel + 1) % 3; delay(150);}
    else if (button_is_pressed(btn1)){
      if (sel == 0) r = (r - 16) % 256;
      else if (sel == 1) g = (g - 16) % 256;
      else b = (b - 16) % 256;
      delay(130);
    }
    else if (button_is_pressed(btn2)){
      if (sel == 0) r = (r + 16) % 256;
      else if (sel == 1) g = (g + 16) % 256;
      else b = (b + 16) % 256;
      delay(130);
    }
    else if (button_is_pressed(btn4)){
      pixel.clear(); 
      pixel.show();
      return;
    }
  }
}

void randomInt() {
  int maxVal = 9999;
  while (true) {
    blinkColor(pixel.Color(brightness,brightness,brightness), 2, 80, 80);

    bool changed = false;
    unsigned long t0 = millis();
    while (millis() - t0 < 1800) {
      if (button_is_pressed(btn1)) { maxVal = (maxVal < 9999) ? maxVal + 1 : 1; changed = true; t0 = millis(); }
      else if (button_is_pressed(btn2)) { maxVal = (maxVal > 1) ? maxVal - 1 : 9999; changed = true; t0 = millis(); }
      else if (button_is_pressed(btn4)) return; 
      else if (button_is_pressed(btn3)) break;
      delay(10);
    }
    if (changed) {
      int d[4];
      int tmp = maxVal;
      d[0] = tmp/1000; tmp %= 1000;
      d[1] = tmp/100;  tmp %= 100;
      d[2] = tmp/10;   tmp %= 10;
      d[3] = tmp;
      uint32_t colors[4] = { pixel.Color(brightness,0,0), pixel.Color(brightness,brightness,0), pixel.Color(0,brightness,0), pixel.Color(0,0,brightness) };
      for (int i=0; i<4; ++i) {
        delay(150);
        blinkColor(colors[i], d[i], 60, 60);
      }
    }

    if (button_is_pressed(btn3)) {
      randomSeed(analogRead(1));
      int val = random(0, maxVal+1);
      int d[4];
      d[0] = val/1000; val %= 1000;
      d[1] = val/100;  val %= 100;
      d[2] = val/10;   val %= 10;
      d[3] = val;
      uint32_t colors[4] = {pixel.Color(brightness,0,0), pixel.Color(0,brightness,0), pixel.Color(0,0,brightness), pixel.Color(brightness,brightness,0) };
      for (int i=0; i<4; ++i) {
        if (i>0) delay(200);
        blinkColor(colors[i], d[i], 100, 80);
      }
      delay(700);
    }
    if (button_is_pressed(btn4)) return;
  }
}

void bomb(){
    int interval = 1000;
    int minInterval = 75;
    int steps = 30;

    for (int i = 0; i < steps; i++){
        digitalWrite(LED_BUILTIN, HIGH);
        pixel.clear();
        pixel.setPixelColor(0, (255, 0, 0));
        pixel.show();
        delay(60);
        pixel.clear();
        pixel.show();
        digitalWrite(LED_BUILTIN, LOW);

        delay(interval - 60);

        interval = interval - (interval - minInterval) / 6;
        if (interval < minInterval) interval = minInterval;
    }
    digitalWrite(Func1, HIGH);
    delay(20);
    digitalWrite(Func1, LOW);
}

void loop(){
  
  showFunction(selectedFunction);
  
  if (button_is_pressed(btn2)){
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
    delay(150);
  } 
  else if (button_is_pressed(btn1)){
      selectedFunction--;
    if (selectedFunction < 1) selectedFunction = totalFunctions;
    delay(150);
  } else if (button_is_pressed(btn3)){
    switch (selectedFunction){
      case 1: outputs(); break;
      case 2: colourPicker(); break;
      case 3: bomb(); break;
      case 4: randomInt(); break;
      case 5: goToSleep(); break;
    }
  }
  delay(60);
}
