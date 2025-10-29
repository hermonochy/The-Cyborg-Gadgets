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

#define totalFunctions 4
#define totalSensors 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

OneWire oneWire(TEMP_PIN);

DallasTemperature tempSensor(&oneWire);

unsigned long lastActivityTime = 0;
const unsigned long inactivityPeriod = 60000;

int selectedFunction = 1;
int selectedSensor = 1;

const byte btn1 = 3;
const byte btn2 = 4;
const byte btn3 = 5;
const byte btn4 = 2;

const byte TRIG_PIN = 9;
const byte ECHO_PIN = 8;

const byte lightPin = A0;

const byte LED = 7;

volatile bool wakeup = false;

const char *Functions[] = {"Torch", "Sensors", "Random", "Sleep"};
const char *Sensors[] = {"Temp", "Light", "Sonar"};

void setup(){
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(LED, OUTPUT);
  randomSeed(analogRead(1));

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
    if (button_is_pressed(btn1)){
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
    if (button_is_pressed(btn4)){
      digitalWrite(LED, HIGH);
    } 
    else if (button_is_pressed(btn3)){
      continue;
    }
  }
}

bool button_is_pressed(const byte btn){
    unsigned long now = millis();

    if (now - lastActivityTime > inactivityPeriod) {
        display.clearDisplay();
        display.display();
        goToSleep();
        lastActivityTime = millis();
        // Ensure interrupt does not trigger immediately
        while (button_is_pressed(btn3));
        return false;
    }

    if (digitalRead(btn) == LOW) {
        delay(50);
        lastActivityTime = millis();
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
    attachInterrupt(digitalPinToInterrupt(btn4), wakeUp, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();

    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(btn4));
 
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
    
    display.setTextSize(3);
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
      else digitalWrite(LED, LOW);
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

    tempValC = tempSensor.getTempCByIndex(0);
    tempValK = tempSensor.getTempCByIndex(0) - 273.15;
    tempValF = tempSensor.getTempCByIndex(0) * 9 / 5 + 32;

    display.setTextSize(2);
    display.setCursor(0, 5);
    display.print(("%.2f°C", tempValC));
    display.setCursor(64, 5);
    display.print(("%.1f°K", tempValK));
    display.setCursor(64, 20);
    display.print(("%.1f°F", tempValF));
    display.display();
    }
}

void light(){
  int lightVal;
  display.clearDisplay();
  while (!button_is_pressed(btn4)){

    lightVal = analogRead(lightPin);;

    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print(("Light: %d", lightVal));
    display.display();
    }
}

void ultraSound(){
  long duration;
  float distance;
  display.clearDisplay();
  while (!button_is_pressed(btn4)){

    digitalWrite(TRIG_PIN, HIGH);
    delay(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH);
    distance = 0.017 * duration;

    display.setTextSize(1);
    display.setCursor(0,15);
    display.print(("Distance: %.1fcm", distance));

    int barWidth = 128;
    float maxDistance = 500.0;
    int filled = barWidth - min(barWidth, int((distance / maxDistance) * barWidth));
    display.fillRect(0, 30, filled, 10, WHITE);

    display.display();
  }
}


void sensors(void){
  delay(50);
  display.clearDisplay();
  while (true){
    display.setTextSize(2);
    display.setCursor(10, 5);
    display.print(Sensors[selectedSensor-1]);
    display.display();

    delay(50);

    if (button_is_pressed(btn2)){
      selectedSensor++;
      if (selectedSensor > totalSensors) selectedSensor = 1;
    } 
    else if (button_is_pressed(btn1)){
      selectedSensor--;
      if (selectedSensor < 1) selectedSensor = totalSensors;
    } 
    else if (button_is_pressed(btn4)) return;
    else if (button_is_pressed(btn3)){
      switch (selectedSensor){
        case 1:
          temp();
          continue;
        case 2:
          light();
          continue;
        case 3:
          ultraSound();
          continue;
      }
    }
  }
}

void randomInt(){
  int range = 1;
  while (true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Random Int");
    display.setCursor(30, 25);
    display.print("Range:");
    display.setCursor(30, 45);
    display.print(("0 - "));
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
      display.setTextSize(4);
      display.setCursor(10, 25);
      display.print(randNumber);
      display.display();
      delay(2000);
    } 
    else if (button_is_pressed(btn4)){
      return;
    }
  }
}


void loop(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 5);

  display.print(Functions[selectedFunction-1]);

  display.display();

  delay(1000);

  //if (button_is_pressed(btn2)){
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  //} 
  else if (button_is_pressed(btn1)){
    selectedFunction--;
    if (selectedFunction < 1){
      selectedFunction = totalFunctions;
    }
  } 
  else if (button_is_pressed(btn4)){
    switch (selectedFunction){
      case 1:
        torch();
        break;
      case 2:
        sensors();
        break;
      case 3:
        randomInt();
      case 4:
        display.clearDisplay();
        display.display();
        // Ensure interrupt does not trigger immediately
        while (button_is_pressed(btn4));
        goToSleep();
        break;
    }
  }
  else if (button_is_pressed(btn3)){
    reset();
  }
}
