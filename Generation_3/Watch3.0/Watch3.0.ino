// Initial arduino pro mini watch. Uses blinks to communicate.

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define LONG_PRESS_MS 3000UL

const byte btn1 = 5;
const byte btn2 = 6;
const byte btn3 = 2;
const byte btn4 = 7;

const byte Func1 = 9;
const byte Func2 = 8;
const byte Func3 = 10;

volatile bool wakeup = false;

void setup() {
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  randomSeed(analogRead(1));
}
bool button_is_pressed(const byte btn, bool onlyOnce = true, bool longPress = false) {
  if (digitalRead(btn) == HIGH) return false;
  
  if (longPress) {
    unsigned long start = millis();
    while (digitalRead(btn) == LOW) {
      if (millis() - start >= LONG_PRESS_MS) {
        if (onlyOnce) {
          while (digitalRead(btn) == LOW) { delay(10); }
        }
        return true;
      }
      delay(10);
    }
    return false;
  } 
  else {
    if (onlyOnce) {
      while (digitalRead(btn) == LOW) { delay(10); }
      return true;
    } 
    else return true;
  }
}

void blinkNumber(int n, unsigned int onMs = 150, unsigned int offMs = 150) {
  if (n <= 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(onMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(offMs);
    return;
  }
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(onMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(offMs);
  }
  delay(offMs);
}

void wakeUp() {
  wakeup = true;
}

void goToSleep() {
  digitalWrite(LED_BUILTIN, LOW);

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
  
  while (button_is_pressed(btn3)){}
}

void activateFunc(const byte func, int blinkTime = 500) {
  bool blink = false;
  bool keepOn = false;
  while (true) {
    if (button_is_pressed(btn1, false) || keepOn) {
      digitalWrite(func, HIGH);
      delay(50);
    } else {
      digitalWrite(func, LOW);
    }

    if (blink) {
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    }
    digitalWrite(LED_BUILTIN, digitalRead(func));

    if (button_is_pressed(btn2))
      keepOn = !keepOn;

    else if (button_is_pressed(btn3))
      blink = !blink;

    else if (button_is_pressed(btn4))
      return;
  }
}

void watchFuncs(void) {
  digitalWrite(LED_BUILTIN, HIGH);
  while (true) {
    if (button_is_pressed(btn1)) {
      activateFunc(Func1);
      continue;
    }
    if (button_is_pressed(btn2)) {
      activateFunc(Func2);
      continue;
    }
    if (button_is_pressed(btn3)) {
      activateFunc(Func3);
      continue;
    }
    if (button_is_pressed(btn4)) {
      return;
    }
  }
}

void randomInt() {
  digitalWrite(LED_BUILTIN, LOW);
  int range = 1;
  while(true){
    if (button_is_pressed(btn1, false)) {
      range++;
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (button_is_pressed(btn2, false)) {
      range--;
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (button_is_pressed(btn3)){
      range+=10;
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (button_is_pressed(btn4)) {
      int randNumber = random(0, range);
      Serial.println(randNumber);
      blinkNumber(randNumber, 500, 500);
      return;
    }
  }
}

void counter() {
  int count = 0;

  delay(500);

  while (true) {

    if (button_is_pressed(btn1)) {
      count++;
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }

    else if (button_is_pressed(btn2, false)) {
      count += 10;
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }

    else if (button_is_pressed(btn3)) {
      count--;
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }

    else if (button_is_pressed(btn4)) {
      Serial.println(count);
      blinkNumber(count, 500, 500);
      return;
    }
  }

  delay(300);
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  delay(500);

}

void metronome(void){
  int bpm = 100;
  const int MIN_BPM = 10;
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

  digitalWrite(Func1, LOW);

  while (true){
    now = millis();
    interval = 60000UL / (unsigned long)max(1, bpm);
    
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
    if (button_is_pressed(btn3)) Func = Func2;
    if (button_is_pressed(btn4)){
      digitalWrite(Func, LOW);
      return;
    }  
    yield();
  }
}

void timer() {
  int minutes = 1;
  while (true) {
    blinkNumber(minutes, 100, 100);
    if (button_is_pressed(btn1, false, false)) minutes++;
    if (button_is_pressed(btn2, false, false)){minutes--; if (minutes < 0) minutes = 0;}
    if (button_is_pressed(btn4)) return;
    if (button_is_pressed(btn3)) break;
    delay(100);
  }

  unsigned long totalMs = (unsigned long)minutes * 60000UL;
  unsigned long start = millis();
  while (millis() - start < totalMs) {
    unsigned long remainingMs = totalMs - (millis() - start);
    unsigned long remainingSec = remainingMs / 1000;
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink >= 1000) {
      lastBlink = millis();
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1);
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (button_is_pressed(btn4)) return;
    delay(50);
  }
  Serial.println("Done!");
  while (true) {
    blinkNumber(1, 120, 120);
      if (button_is_pressed(btn4)) return;
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5);

  if (digitalRead(btn4) == LOW) {
    delay(50);
    if (digitalRead(btn4) == LOW) {
      unsigned long start = millis();
      bool longDetected = false;

      while (digitalRead(btn4) == LOW) {
        if (millis() - start >= LONG_PRESS_MS) {
          longDetected = true;
          break;
        }
        delay(10);
      }

      if (longDetected) {
        goToSleep();
        digitalWrite(LED_BUILTIN, LOW);
        delay(45);
        return;
      } else {
        timer();
        digitalWrite(LED_BUILTIN, LOW);
        delay(45);
        return;
      }
    }
  }

  if (digitalRead(btn1) == LOW) {
    delay(50);
    if (digitalRead(btn1) == LOW) {
      unsigned long start = millis();
      bool longDetected = false;

      while (digitalRead(btn1) == LOW) {
        if (millis() - start >= LONG_PRESS_MS) {
          longDetected = true;
          break;
        }
        delay(10);
      }

      if (longDetected) {
        metronome();
      } 
      else {
        watchFuncs();
      }
    }
  }

  if (button_is_pressed(btn2)) {
    randomInt();
  }
  else if (button_is_pressed(btn3)) {
    counter();
  }

  digitalWrite(LED_BUILTIN, LOW);
  delay(45);
}
