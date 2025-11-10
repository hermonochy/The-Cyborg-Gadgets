#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <ctype.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int selectedFunction = 1;
const int totalFunctions = 6;

const byte btn1 = 2;
const byte btn2 = 6;
const byte btn3 = 5;
const byte btn4 = 3; // needs to be an interrupt pin 

byte Func1 = 10;
byte Func2 = 12;
byte Func3 = 11;

volatile bool wakeup = false;

void setup() {
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(Func1, OUTPUT);
  pinMode(Func2, OUTPUT);
  pinMode(Func3, OUTPUT);
  pinMode(Func4, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
  while (true){
    digitalWrite(LED_BUILTIN, HIGH); delay(200);
    digitalWrite(LED_BUILTIN, LOW); delay(200);
    }
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(7, 0);
  display.print("Wecome to");
  display.setCursor(20, 20);
  display.print("Watch 2");
  display.setCursor(30, 50);
  display.print("Gen 3");
  display.setTextSize(1);
  display.setCursor(55, 40);
  display.print("of");
  display.display();

  delay(100);

  for (int i = 0; i < 200; i++) {
    delay(20);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    if (button_is_pressed(btn1)) {
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
    if (button_is_pressed(btn4)) {
      digitalWrite(Func1, HIGH);
    } 
    else if (button_is_pressed(btn2)) {
      digitalWrite(Func3, HIGH);
    }
    else if (button_is_pressed(btn3)) {
    digitalWrite(LED_BUILTIN, LOW);
    calculator();
    break;
    }
  }
}

bool button_is_pressed(const byte btn, bool onlyOnce = false) {
  unsigned long now = millis();

  if (now - lastActivityTime > inactivityPeriod){
    goToSleep();
    while (button_is_pressed(btnInterrupt1)) delay(100);
    while (button_is_pressed(btnInterrupt2)) delay(100);
    lastActivityTime = millis();
    return false;
  }
  if (digitalRead(btn) == LOW){
    if (onlyOnce){
      while (digitalRead(btn) == LOW){}
    }
    lastActivityTime = millis();
    return true;
  }
  return false;
}

void(* reset) (void) = 0;

void wakeUp() {
    wakeup = true;
}

void goToSleep() {

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

void activateFunc(const byte func, int blinkTime){
  bool blink = false;
  bool keepOn = false;

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
    display.setCursor(5, 0);
    display.print("State: ");
    display.print(digitalRead(func)? "On" : "Off");
    display.display();
    
    if (!blink) delay(50);
    if (keepOn){
      digitalWrite(func, HIGH);
    } 
    else if (blink){
      lastActivityTime = millis();
      digitalWrite(func, !digitalRead(func));
      delay(blinkTime);
    } 
    else{
      if (button_is_pressed(btn1)) digitalWrite(func, HIGH);
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
    else if (button_is_pressed(btn4)){
      return;
    delay(250);
    }
  }
}

void watchFuncs(void){
  delay(50);
  while (true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. White LED");
    display.setCursor(0, 30);
    display.println("2. Laser");
    display.setCursor(0, 40);
    display.println("3. UV LED");
    display.setCursor(0, 50);
    display.println("4. Return");
    display.display();

    delay(100);

    if (button_is_pressed(btn1, true)) activateFunc(Func1, blinkTime1);
    else if (button_is_pressed(btn2, true)) activateFunc(Func2, blinkTime2); 
    else if (button_is_pressed(btn3, true)) activateFunc(Func3, blinkTime3);
    else if (button_is_pressed(btn4, true)) return;
  }
}

void calculator(void){
    const char* options[] ={"1","2","3","4","5","6","7","8","9","0",".","+","-","*","/","^","r","%","(",")"};
    const int numOptions = sizeof(options)/sizeof(options[0]);
    int currentOption = 0;
    char expr[25] = "";
    int exprLen = 0;
    bool showResult = false;
    double result = 0;
    bool error = false;

    while (true){
        display.clearDisplay();

        display.setTextSize(2);
        display.setCursor(0, 0);
        if (showResult){
            if (error){
                display.print("Error!");
            } 
            else{
                display.print(result, 6);
            }
        }

        display.setTextSize(1);
        int startX = 0;
        int yOptions = 20;
        int spacing = 16;
        for (int i = 0; i < numOptions; i++){
            int x = startX + (i % 7) * spacing;
            int y = yOptions + (i / 7) * 10;
            if (i == currentOption){
                display.fillRect(x-1, y-1, 12, 10, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
                display.setCursor(x, y);
                display.print(options[i]);
                display.setTextColor(SSD1306_WHITE);
            } 
            else{
                display.setCursor(x, y);
                display.print(options[i]);
            }
        }

        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print(expr);

        display.display();

        if (button_is_pressed(btn1)){
            currentOption = (currentOption + 1) % numOptions;
            delay(150);
        }
        else if (button_is_pressed(btn2, true)){
            if (showResult){
                expr[0] = '\0';
                exprLen = 0;
                showResult = false;
            }
            if (exprLen < 24){
                strcat(expr, options[currentOption]);
                exprLen = strlen(expr);
            }
            delay(250);
        }
        else if (button_is_pressed(btn3)){
            expr[exprLen-1] = '\0';
            exprLen = strlen(expr);
        }
        else if (button_is_pressed(btn4, true)){
          if (!showResult){
            error = false;
            result = evaluateExpression(expr, error);
            showResult = true;
            delay(550);
          }
          else return;
        }
        delay(100);
    }
}

double parsePrimary(const char* &s, bool &error);

double parseNumber(const char* &s, bool &error){
    double value = 0.0;
    bool hasDecimal = false;
    double frac = 0.1;
    while (isdigit(*s) || *s == '.'){
        if (*s == '.'){
            if (hasDecimal){ error = true; return 0; }
            hasDecimal = true;
        } 
        else if (hasDecimal){
            value += (*s - '0') * frac;
            frac *= 0.1;
        } 
        else{
            value = value * 10 + (*s - '0');
        }
        s++;
    }
    return value;
}

double parseFactor(const char* &s, bool &error){
    while (*s == ' ') s++;
    if (*s == '+'){ s++; return parseFactor(s, error); }
    if (*s == '-'){ s++; return -parseFactor(s, error); }
    if (*s == 'r'){ s++; double val = parseFactor(s, error); return val<0?(error=true,0):sqrt(val); }
    if (*s == '('){
        s++;
        double val = parsePrimary(s, error);
        if (*s == ')') s++;
        else error = true;
        return val;
    }
    return parseNumber(s, error);
}

double parseExponent(const char* &s, bool &error){
    double left = parseFactor(s, error);
    while (*s == '^'){
        s++;
        double right = parseFactor(s, error);
        left = pow(left, right);
    }
    return left;
}

double parseTerm(const char* &s, bool &error){
    double left = parseExponent(s, error);
    while (*s == '*' || *s == '/' || *s == '%'){
        char op = *s++;
        double right = parseExponent(s, error);
        if (op == '*') left *= right;
        else if (op == '/'){ if (right == 0){ error = true; return 0; } left /= right; }
        else if (op == '%'){ if (right == 0){ error = true; return 0; } left = fmod(left, right); }
    }
    return left;
}

double parsePrimary(const char* &s, bool &error){
    double left = parseTerm(s, error);
    while (*s == '+' || *s == '-'){
        char op = *s++;
        double right = parseTerm(s, error);
        if (op == '+') left += right;
        else left -= right;
    }
    return left;
}

double evaluateExpression(const char* expr, bool &error){
    const char* s = expr;
    error = false;
    double result = parsePrimary(s, error);
    while (*s && !error){
        if (!isspace(*s)) error = true;
        s++;
    }
    return result;
}

void paddle() {
  int paddleY = 24, paddleH = 16, paddleW = 4;
  int ballX = 64, ballY = 32, ballDX = 2, ballDY = 1;
  int score = 0;
  bool gameOver = false;

  while (true) {
    if (button_is_pressed(btn4)) return;
    if (!gameOver && button_is_pressed(btn1) && paddleY > 0) paddleY -= 3;
    if (!gameOver && button_is_pressed(btn2) && paddleY < SCREEN_HEIGHT - paddleH) paddleY += 3;

    if (!gameOver) {
      ballX += ballDX;
      ballY += ballDY;
      if (ballY < 0 || ballY > SCREEN_HEIGHT - 3) ballDY = -ballDY;

      if (ballX < paddleW + 2 && ballY + 3 > paddleY && ballY < paddleY + paddleH) {
        ballDX = -ballDX;
        score++;
      }
      if (ballX > SCREEN_WIDTH - 3) ballDX = -ballDX;

      if (ballX < 0) gameOver = true;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Paddle");
    display.setCursor(80,0);
    display.print("Score:");
    display.print(score);
    
    display.fillRect(2, paddleY, paddleW, paddleH, SSD1306_WHITE);

    display.fillRect(ballX, ballY, 3, 3, SSD1306_WHITE);

    if (gameOver) {
      display.setCursor(30, 30);
      display.print("Game Over!");
      display.setCursor(5, 45);
      display.print("Btn3: Retry  Btn4: Quit");
      display.display();
      if (button_is_pressed(btn3)) {
        paddleY = 24; ballX = 64; ballY = 32; ballDX = 2; ballDY = 1; score = 0; gameOver = false;
        delay(300);
      }
      continue;
    }
    display.display();
    delay(16);
  }
}

void spaceInvaders() {
    const int playerW = 14, playerH = 4, playerY = 60;
    const int bulletW = 2, bulletH = 4, bulletSpeed = 3;
    const int alienW = 8, alienH = 6, alienCols = 6, alienRows = 2;
    const int alienGap = 4, alienStartY = 8, alienStartX = 10;
    int playerX = (SCREEN_WIDTH - playerW) / 2;

    struct Bullet { int x, y; bool active; } bullet = {0, 0, false};
    struct Alien { int x, y; bool alive; } aliens[alienCols * alienRows];

    int alienDir = 1, alienStep = 4, alienMoveDelay = 28, alienMoveCounter = 0;
    int alienCount = alienCols * alienRows;
    int score = 0;
    bool gameOver = false, win = false;

    for (int r = 0; r < alienRows; r++) {
        for (int c = 0; c < alienCols; c++) {
            aliens[r*alienCols+c].x = alienStartX + c * (alienW + alienGap);
            aliens[r*alienCols+c].y = alienStartY + r * (alienH + alienGap);
            aliens[r*alienCols+c].alive = true;
        }
    }

    while (true) {
        if (button_is_pressed(btn4)) return;

        if (!gameOver && button_is_pressed(btn1) && playerX > 0) {
            playerX -= 3;
        }
        if (!gameOver && button_is_pressed(btn3) && playerX < SCREEN_WIDTH - playerW) {
            playerX += 3;
        }

        if (!gameOver && button_is_pressed(btn2) && !bullet.active) {
            bullet.x = playerX + playerW/2 - bulletW/2;
            bullet.y = playerY - bulletH;
            bullet.active = true;
        }

        if (bullet.active) {
            bullet.y -= bulletSpeed;
            if (bullet.y < 0) bullet.active = false;
        }

        alienMoveCounter++;
        if (!gameOver && alienMoveCounter > alienMoveDelay) {
            int shift = alienStep * alienDir;
            bool changeDir = false;
            for (int i = 0; i < alienCols * alienRows; i++) {
                if (!aliens[i].alive) continue;
                aliens[i].x += shift;
                if (aliens[i].x < 0 || aliens[i].x + alienW > SCREEN_WIDTH) changeDir = true;
            }
            if (changeDir) {
                alienDir = -alienDir;
                for (int i = 0; i < alienCols * alienRows; i++)
                    if (aliens[i].alive) aliens[i].y += alienH + 1;
            }
            alienMoveCounter = 0;
        }

        for (int i = 0; i < alienCols * alienRows; i++) {
            if (!aliens[i].alive) continue;
            if (bullet.active &&
                bullet.x + bulletW > aliens[i].x &&
                bullet.x < aliens[i].x + alienW &&
                bullet.y < aliens[i].y + alienH &&
                bullet.y + bulletH > aliens[i].y)
            {
                aliens[i].alive = false;
                bullet.active = false;
                alienCount--;
                score++;
                break;
            }
        }

        for (int i = 0; i < alienCols * alienRows; i++) {
            if (!aliens[i].alive) continue;
            if (aliens[i].y + alienH >= playerY) {
                gameOver = true;
            }
        }

        if (alienCount == 0) {
            win = true;
            gameOver = true;
        }

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("Space Invaders");
        display.setCursor(80,0);
        display.print("Score:");
        display.print(score);

        for (int i = 0; i < alienCols * alienRows; i++) {
            if (aliens[i].alive)
                display.fillRect(aliens[i].x, aliens[i].y, alienW, alienH, SSD1306_WHITE);
        }

        display.fillRect(playerX, playerY, playerW, playerH, SSD1306_WHITE);

        if (bullet.active)
            display.fillRect(bullet.x, bullet.y, bulletW, bulletH, SSD1306_WHITE);

        if (gameOver) {
            display.setTextSize(1);
            display.setCursor(20, 27);
            if (win) display.print("YOU WIN!");
            else display.print("GAME OVER");
            display.display();
            if (button_is_pressed(btn3)) {
                playerX = (SCREEN_WIDTH - playerW) / 2;
                bullet.active = false;
                alienDir = 1;
                alienMoveCounter = 0;
                alienCount = alienCols * alienRows;
                score = 0;
                gameOver = false;
                win = false;
                for (int r = 0; r < alienRows; r++) {
                    for (int c = 0; c < alienCols; c++) {
                        aliens[r*alienCols+c].x = alienStartX + c * (alienW + alienGap);
                        aliens[r*alienCols+c].y = alienStartY + r * (alienH + alienGap);
                        aliens[r*alienCols+c].alive = true;
                    }
                }
                delay(300);
            }
            continue;
        }
        display.display();
        delay(16);
    }
}

void dinoRunner() {
  int dinoY = 30, velocity = 0, gravity = 2, jumpPower = -12;
  int cactusX = 128, cactusY = 40, cactusW = 6, cactusH = 16;
  bool jumping = false, gameOver = false;
  unsigned long lastMove = millis(), lastCactusMove = millis();
  int score = 0;

  while (true) {
    if (button_is_pressed(btn4)) return;

    if (!gameOver && button_is_pressed(btn3) && !jumping) {
      velocity = jumpPower;
      jumping = true;
    }

    if (!gameOver && millis() - lastMove > 40) {
      if (jumping) {
        dinoY += velocity;
        velocity += gravity;
        if (dinoY >= 40) {
          dinoY = 40;
          jumping = false;
        }
      }
      lastMove = millis();
    }

    if (!gameOver && millis() - lastCactusMove > 24) {
      cactusX -= 3;
      if (cactusX < -cactusW) {
        cactusX = 128;
        score++;
      }
      lastCactusMove = millis();
    }

    if (!gameOver && cactusX < 20 && cactusX + cactusW > 10 && dinoY + 16 > cactusY) {
      gameOver = true;
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(80,0);
    display.print(score);

    display.drawLine(0, 56, 128, 56, SSD1306_WHITE);

    display.fillRect(10, dinoY, 10, 16, SSD1306_WHITE);

    display.fillRect(cactusX, cactusY, cactusW, cactusH, SSD1306_WHITE);

    if (gameOver) {
      display.setTextSize(1);
      display.setCursor(30, 20);
      display.print("Game Over!");
      display.setCursor(5, 35);
      display.print("Btn3: Retry  Btn4: Quit");
      display.display();
      if (button_is_pressed(btn3)) {
        dinoY = 40; cactusX = 128; velocity = 0; jumping = false; score = 0; gameOver = false;
        delay(300);
      }
      continue;
    }
    display.display();
    delay(10);
  }
}

void loop() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Menu     ");
  display.print(selectedFunction);

  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("1. Output  2. Maths");
  display.setCursor(0, 40);
  display.print("3. Paddle 4. Invaders");
  display.setCursor(0, 55);
  display.print("5. Runner 6. Power");

  display.display();

  delay(100);

  if (button_is_pressed(btn2)) {
    selectedFunction++;
    if (selectedFunction > totalFunctions) selectedFunction = 1;
  } 
  else if (button_is_pressed(btn1)) {
    selectedFunction--;
    if (selectedFunction < 1) {
      selectedFunction = totalFunctions;
    }
  } 
  else if (button_is_pressed(btn4)) {
    switch (selectedFunction) {
      case 1:
        watchFuncs();
        break;
      case 2:
        calculator();
        break;
      case 3:
        paddle();
        break;
      case 4:
        spaceInvaders();
        break;
      case 5:
        dinoRunner();
        break;
      case 6:
        display.clearDisplay();
        display.display();
        while (button_is_pressed(btn4)) delay(10); // Ensure it does not immediately wake up again
        goToSleep();
        break;
    }
  }
  else if (button_is_pressed(btn3)){
    reset();
  }
}
