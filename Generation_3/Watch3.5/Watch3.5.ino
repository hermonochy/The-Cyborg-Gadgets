// Watch 3.5: Sensor watch

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

#define TEMP_PIN 10

#define totalFunctions 6
#define totalSensors 3

#define lightPin A2
#define lightInterrupt 2

#define LED A1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool button_is_pressed(const byte btn, bool onlyOnce = false);

OneWire oneWire(TEMP_PIN);

DallasTemperature tempSensor(&oneWire);

int selectedFunction = 1;
int selectedSensor = 1;

const byte btn1 = 2;
const byte btn2 = 5;
const byte btn3 = 4;
const byte btn4 = 5;

const char *settingFuncs[] = {"Display Settings", "LED Power"};

volatile bool wakeup = false;
int rotation = 0;

const char *Functions[] = {"Torch", "Sensors", "Random", "Counter", "Settings", "Sleep"};
const char *Sensors[] = {"Temp", "Light", "Sonar"};

void setup(){
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(lightPin, INPUT);
  pinMode(LED, OUTPUT);
  randomSeed(analogRead(7));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
  while (true){
    digitalWrite(LED_BUILTIN, HIGH); delay(50);
    digitalWrite(LED_BUILTIN, LOW); delay(200);
    }
  }

  tempSensor.begin();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 0);
  display.print("Watch 5");
  display.setCursor(10, 20);
  display.setTextSize(1);
  display.print("of Generation 3");
  display.display();

  delay(100);
  // shortcuts:
  for (int i = 0; i < 200; i++){
    delay(20);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    if (button_is_pressed(btn4)){
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
    if (button_is_pressed(btn1)){
      digitalWrite(LED, HIGH);
    } 
    else if (button_is_pressed(btn3)){
      continue;
    }
  }
}

bool button_is_pressed(const byte btn, bool onlyOnce = false) {
  if (digitalRead(btn) == LOW){
    if (onlyOnce){
      while (digitalRead(btn) == LOW){}
    }
    return true;
  }
  return false;
}

void(* reset) (void) = 0;

void wakeUp(){
    wakeup = true;
}

void goToSleep(){

    display.ssd1306_command(SSD1306_DISPLAYOFF);

    ADCSRA &= ~(1 << ADEN);
    sleep_bod_disable();

    wakeup = false;
    attachInterrupt(digitalPinToInterrupt(btn1), wakeUp, FALLING);
    attachInterrupt(digitalPinToInterrupt(lightInterrupt), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn1));
    detachInterrupt(digitalPinToInterrupt(lightInterrupt));

    if (digitalRead(lightInterrupt) == 0) {
      digitalWrite(LED, HIGH);
    }

    ADCSRA |= (1 << ADEN);
    display.ssd1306_command(SSD1306_DISPLAYON);
}

void torch(){
  bool blink = false;
  bool keepOn = false;
  int blinkTime = 500;

  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(10, 15);
    display.print("State: ");
    display.print(digitalRead(LED));
    display.display();
      
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(LED, HIGH);
    } 
    else if (blink){
      digitalWrite(LED, !digitalRead(LED));
      delay(blinkTime / 2);
    } 
    else{
      if (button_is_pressed(btn1)) digitalWrite(LED, HIGH);
      else digitalWrite(LED, 0);
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

void temp(){
  float tempValC;
  float tempValK;
  float tempValF;
  while (!button_is_pressed(btn4)){
    tempSensor.requestTemperatures();
    lightVal = analogRead(lightPin);

    tempValC = tempSensor.getTempCByIndex(0);
    tempValK = tempValC + 273.15;
    tempValF = tempSensor.getTempFByIndex(0);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 4);
    display.print(("%.2f°C", tempValC));
    display.setCursor(0, 14);
    display.print(("%.1f°K", tempValK));
    display.setCursor(0, 24);
    display.print(("%.1f°F", tempValF));
    display.display();
    }
}

void light(){
  int light;
  display.clearDisplay();
  while (!button_is_pressed(btn4)){
    light = analogRead(lightPin);
    display.clearDisplay();
    display.setCursor(10, 0);
    display.setTextSize(2);
    display.print("Light:");
    display.print(light);
  }
}

void distance(){
  bool distance;
  display.clearDisplay();
  while (!button_is_pressed(btn4)){
    distance = digitalRead(distancePin);
    display.clearDisplay();
    display.setCursor(10, 0);
    display.setTextSize(2);
    display.print("Dist:");
    display.print(distance ? "Close" : "Far");
  }
}

void sensors(void){
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 0);
    display.print("Sensors");
    display.setCursor(10, 8);
    display.print("1. Temperature");
    display.setCursor(10, 16);
    display.print("2. Light");
    display.setCursor(10, 24);
    display.print("3. Distance");
    display.display();

    delay(50);

    if (button_is_pressed(btn1)) temp();
    else if (button_is_pressed(btn2)) light();
    else if (button_is_pressed(btn3)) distance();
    else if (button_is_pressed(btn4, true)) return;
  }
}

void randomInt(){
  int range = 1;
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Random Int");
    display.setCursor(0, 10);
    display.print("Range:");
    display.setCursor(0, 24);
    display.print("0 - ");
    display.print(range);
    display.display();
    delay(100);
    if (button_is_pressed(btn1)){
      range += 10;
      delay(200);
    } 
    else if (button_is_pressed(btn2)){
      range--;
      delay(200);
    } 
    else if (button_is_pressed(btn3)){
      int randNumber = random(0, range + 1);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(10, 15);
      display.print(randNumber);
      display.display();
      delay(2000);
    } 
    else if (button_is_pressed(btn4)){
      return;
    }
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

    display.setCursor(15, 24);
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
    else if (button_is_pressed(btn4)){
      return;
    }
    delay(50);
  }
}


void settings() {
  const byte numSettings = 2;
  int settingIndex = 0;
  while(!button_is_pressed(btn4, true)){
    delay(50);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,5);
    display.print(settingFuncs[settingIndex]);
    display.setCursor(0,24);

    switch(settingIndex) {
      case 0:
        display.print(rotation%4);
        if(button_is_pressed(btn1)) {
          rotation++;
        }
        if(button_is_pressed(btn2, true)) {
          display.ssd1306_command(SSD1306_DISPLAYOFF);
        }
        display.setRotation(rotation%4);
        break;
      case 1:
        display.print(power);
        if(button_is_pressed(btn2)) {
          power+=5;
          if (power>1023) power = 0;
        }
        if(button_is_pressed(btn1)) {
          power-=5;
          if (power<0) power = 1023;
        }
        break;
    }
    display.display();
    
    if(button_is_pressed(btn3, true)) {
      settingIndex = (settingIndex + 1) % numSettings;
    }
  }
}

void loop(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 5);
  display.print(Functions[selectedFunction-1]);
  display.display();
  delay(200);

  if (button_is_pressed(btn2)){
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  } 
  else if (button_is_pressed(btn1)){
    selectedFunction--;
    if (selectedFunction < 1){
      selectedFunction = totalFunctions;
    }
  } 
  else if (button_is_pressed(btn4, true)){
    switch (selectedFunction){
      case 1:
        torch();
        break;
      case 2:
        sensors();
        break;
      case 3:
        randomInt();
        break;
      case 4:
        counter();
        break;
      case 5:
        settings();
        break;
      case 6:
        goToSleep();
        break;
    }
  }
  else if (button_is_pressed(btn3)){
    reset();
  }
}
