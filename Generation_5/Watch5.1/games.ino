// Includes: Shooter, Snake, Flappy Bird, Geometry Dash

extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MAX_COUNT_NUMS 6
#define SOLUTION_DISPLAY_STEPS 4

struct CountdownStep {
    int a, b;
    char op; // '+','-','*','/','^','r'
    int res;
    CountdownStep(): a(0), b(0), op(0), res(0) {}
    CountdownStep(int A, int B, char O, int R): a(A), b(B), op(O), res(R) {}
};

struct CountdownSolution {
    int result;
    int nsteps;
    CountdownStep steps[64];
};


void games() {
  int selected = 0;
  const char* gameList[] = {"Arcade Games", "Maths Games"};
  
  while (true) {
    display.fillScreen(GC9A01A_BLACK);
    display.drawLine(0, 0, SCREEN_WIDTH, 0, GC9A01A_WHITE);
    display.drawLine(0, 10, SCREEN_WIDTH, 10, GC9A01A_WHITE);
    
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("Games");
    
    display.setTextSize(1);
    for (int i = 0; i < 2; i++) {
      display.setCursor(15, 18 + (i * 9));
      if (i == selected) {
        display.setTextColor(GC9A01A_BLACK);
        display.fillRect(10, 17 + (i * 9), 108, 10, GC9A01A_WHITE);
        display.print(gameList[i]);
        display.setTextColor(GC9A01A_WHITE);
      } else {
        display.print(gameList[i]);
      }
    }
    
    delay(50);
    
    if (button_is_pressed(btn4)) {
      selected = (selected + 1) % 2;
      delay(100);
    }
    else if (button_is_pressed(btn1)) {
      selected = (selected - 1 + 2) % 2;
      delay(100);
    }
    else if (button_is_pressed(btn3)) {
      switch (selected) {
        case 0:
          arcadeGames();
          break;
        case 1:
          mathsGames();
          break;
      }
    }
    else if (button_is_pressed(btn6, true)) return;
  }
}

void arcadeGames() {
  int selected = 0;
  const char* gameList[] = {"Shooter", "Snake", "Flappy Bird", "Geometry Dash", "3D Flight"};
  
  while (true) {
    display.fillScreen(GC9A01A_BLACK);
    display.drawLine(0, 0, SCREEN_WIDTH, 0, GC9A01A_WHITE);
    display.drawLine(0, 10, SCREEN_WIDTH, 10, GC9A01A_WHITE);
    
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("Arcade Games");
    
    display.setTextSize(1);
    for (int i = 0; i < 5; i++) {
      display.setCursor(15, 18 + (i * 9));
      if (i == selected) {
        display.setTextColor(GC9A01A_BLACK);
        display.fillRect(10, 17 + (i * 9), 108, 10, GC9A01A_WHITE);
        display.print(gameList[i]);
        display.setTextColor(GC9A01A_WHITE);
      } else {
        display.print(gameList[i]);
      }
    }
    
    delay(50);
    
    if (button_is_pressed(btn4)) {
      selected = (selected + 1) % 5;
      delay(100);
    }
    else if (button_is_pressed(btn1)) {
      selected = (selected - 1 + 5) % 5;
      delay(100);
    }
    else if (button_is_pressed(btn3)) {
      switch (selected) {
        case 0:
          shooter();
          break;
        case 1:
          snake();
          break;
        case 2:
          flappyBird();
          break;
        case 3:
          geometryDash();
          break;
        case 4:
          flying3D();
          break;
      }
    }
    else if (button_is_pressed(btn6, true)) return;
  }
}

void mathsGames() {
  int selected = 0;
  const char* gameList[] = {"Countdown"};
  
  while (true) {
    display.fillScreen(GC9A01A_BLACK);
    display.drawLine(0, 0, SCREEN_WIDTH, 0, GC9A01A_WHITE);
    display.drawLine(0, 10, SCREEN_WIDTH, 10, GC9A01A_WHITE);
    
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("Maths Games");
    
    display.setTextSize(1);
    for (int i = 0; i < 1; i++) {
      display.setCursor(15, 18 + (i * 9));
      if (i == selected) {
        display.setTextColor(GC9A01A_BLACK);
        display.fillRect(10, 17 + (i * 9), 108, 10, GC9A01A_WHITE);
        display.print(gameList[i]);
        display.setTextColor(GC9A01A_WHITE);
      } else {
        display.print(gameList[i]);
      }
    }
    
    delay(50);
    
    if (button_is_pressed(btn4)) {
      selected = (selected + 1) % 1;
      delay(100);
    }
    else if (button_is_pressed(btn1)) {
      selected = (selected - 1 + 1) % 1;
      delay(100);
    }
    else if (button_is_pressed(btn3)) {
      switch (selected) {
        case 0:
          countdown();
          break;
      }
    }
    else if (button_is_pressed(btn6, true)) return;
  }
}


void shooter() {
  int playerX = 60;
  int playerY = 55;
  int score = 0;
  int health = 3;
  
  struct Bullet { int x; int y; bool active; };
  Bullet bullets[8];
  for (int i = 0; i < 8; i++) bullets[i].active = false;
  
  struct Enemy { int x; int y; bool active; };
  Enemy enemies[3];
  enemies[1].x = 60;
  enemies[1].y = 5;
  enemies[1].active = true;

  unsigned long lastShot = 0;
  unsigned long lastSpawn = 0;

  while (health > 0) {
    unsigned long now = millis();

    if (button_is_pressed(btn1, false) && playerX > 0) {
      playerX -= 3;
    }
    if (button_is_pressed(btn2, false) && playerX < 120) {
      playerX += 3;
    }
    if (button_is_pressed(btn3, false) && now - lastShot > 200) {
      for (int i = 0; i < 8; i++) {
        if (!bullets[i].active) {
          bullets[i].x = playerX + 2;
          bullets[i].y = playerY - 2;
          bullets[i].active = true;
          lastShot = now;
          break;
        }
      }
    }
    if (button_is_pressed(btn6)) return;

    for (int i = 0; i < 8; i++) {
      if (bullets[i].active) {
        bullets[i].y -= 4;
        if (bullets[i].y < 0) bullets[i].active = false;
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        enemies[i].y += 1;
        if (enemies[i].y > 64) {
          enemies[i].active = false;
          health--;
        }
      }
    }

    if (now - lastSpawn > 2500) {
      for (int i = 0; i < 3; i++) {
        if (!enemies[i].active) {
          enemies[i].x = random(10, 118);
          enemies[i].y = 5;
          enemies[i].active = true;
          lastSpawn = now;
          break;
        }
      }
    }

    for (int b = 0; b < 8; b++) {
      if (bullets[b].active) {
        for (int e = 0; e < 3; e++) {
          if (enemies[e].active) {
            if (bullets[b].x >= enemies[e].x - 2 && bullets[b].x <= enemies[e].x + 6 &&
                bullets[b].y >= enemies[e].y - 2 && bullets[b].y <= enemies[e].y + 6) {
              bullets[b].active = false;
              enemies[e].active = false;
              score += 10;
            }
          }
        }
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        if (playerX >= enemies[i].x - 4 && playerX <= enemies[i].x + 6 &&
            playerY >= enemies[i].y - 4 && playerY <= enemies[i].y + 8) {
          enemies[i].active = false;
          health--;
        }
      }
    }

    display.fillScreen(GC9A01A_BLACK);
    
    for (int dx = 0; dx < 4; dx++) {
      for (int dy = 0; dy < 6; dy++) {
        display.drawPixel(playerX + dx, playerY + dy, 1);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (bullets[i].active) {
        display.drawPixel(bullets[i].x, bullets[i].y, 1);
        display.drawPixel(bullets[i].x + 1, bullets[i].y, 1);
      }
    }

    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        for (int dx = 0; dx < 4; dx++) {
          for (int dy = 0; dy < 4; dy++) {
            display.drawPixel(enemies[i].x + dx, enemies[i].y + dy, 1);
          }
        }
      }
    }

    display.setTextSize(1);
    display.setTextColor(GC9A01A_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(score);
    display.setCursor(100, 0);
    display.print("H:");
    display.print(health);
    
    delay(30);
  }

  display.fillScreen(GC9A01A_BLACK);
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.print("Score: ");
  display.print(score);
  
  while (!a_button_is_pressed()) {
    delay(50);
  }
}

void snake(void) {
  struct SnakeSegment { int x; int y; };
  SnakeSegment snake[50];
  int snakeLength = 3;
  
  snake[0].x = 60; snake[0].y = 30;
  snake[1].x = 56; snake[1].y = 30;
  snake[2].x = 52; snake[2].y = 30;
  
  int dirX = 4, dirY = 0;
  int nextDirX = 4, nextDirY = 0;
  
  int foodX = random(10, 120);
  int foodY = random(10, 55);
  
  int score = 0;
  bool gameOver = false;
  unsigned long lastMove = 0;
  
  while (!gameOver) {
    unsigned long now = millis();
    
    if (button_is_pressed(btn1, false) && dirX == 0) { nextDirX = -4; nextDirY = 0; }
    if (button_is_pressed(btn3, false) && dirX == 0) { nextDirX = 4; nextDirY = 0; }
    if (button_is_pressed(btn2, false) && dirY == 0) { nextDirX = 0; nextDirY = -4; }
    if (button_is_pressed(btn5, false) && dirY == 0) { nextDirX = 0; nextDirY = 4; }
    if (button_is_pressed(btn6)) return;
    
    if (now - lastMove > 150) {
      dirX = nextDirX;
      dirY = nextDirY;
      
      for (int i = snakeLength - 1; i > 0; i--) {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
      }
      
      snake[0].x += dirX;
      snake[0].y += dirY;
      
      lastMove = now;
      
      if (snake[0].x < 0) snake[0].x = 128;
      if (snake[0].x > 128) snake[0].x = 0;
      if (snake[0].y < 0) snake[0].y = 64;
      if (snake[0].y > 64) snake[0].y = 0;
      
      for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
          gameOver = true;
        }
      }
      
      if (snake[0].x >= foodX - 2 && snake[0].x <= foodX + 2 &&
          snake[0].y >= foodY - 2 && snake[0].y <= foodY + 2) {
        snakeLength++;
        score += 10;
        foodX = random(10, 120);
        foodY = random(10, 55);
      }
    }
    
    display.fillScreen(GC9A01A_BLACK);
    
    for (int i = 0; i < snakeLength; i++) {
      display.fillRect(snake[i].x, snake[i].y, 3, 3, GC9A01A_WHITE);
    }
    
    display.fillRect(foodX, foodY, 3, 3, GC9A01A_WHITE);
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Len:");
    display.print(snakeLength);
    display.setCursor(65, 0);
    display.print("Score:");
    display.print(score);
    
    delay(30);
  }
  
  display.fillScreen(GC9A01A_BLACK);
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("Score: ");
  display.print(score);
  
  while(!a_button_is_pressed()){}
}

void flappyBird(void) {
  int birdY = 30;
  int birdVelocity = 0;
  const int gravity = 1;
  const int flapPower = -3;
  int score = 0;
  bool gameOver = false;
  
  struct Pipe { int x; int gap; int gapSize; bool passed; };
  Pipe pipes[4];
  for (int i = 0; i < 4; i++) {
    pipes[i].x = SCREEN_WIDTH + i * 45;
    pipes[i].gapSize = 30;
    pipes[i].gap = random(25, 35);
    pipes[i].passed = false;
  }
  
  int pipeSpeed = 2;
  unsigned long lastFlap = 0;
  
  while (!gameOver) {
    if (button_is_pressed(btn3, false) || button_is_pressed(btn5, false) || button_is_pressed(btn1, false)) {
      birdVelocity = flapPower;
      lastFlap = millis();
    }
    if (button_is_pressed(btn6)) return;
    
    birdVelocity += gravity;
    birdY += birdVelocity;
    
    if (birdY < 0 || birdY > 62) gameOver = true;
    
    pipeSpeed = 2 + (score / 5);
    
    for (int i = 0; i < 4; i++) {
      pipes[i].x -= pipeSpeed;
      
      if (pipes[i].x < -10) {
        pipes[i].x = SCREEN_WIDTH + 30;
        pipes[i].gapSize = max(14, 20 - score / 10);
        pipes[i].gap = random(10, 40 - pipes[i].gapSize);
        pipes[i].passed = false;
      }
      
      if (pipes[i].x == 50 && !pipes[i].passed) {
        score++;
        pipes[i].passed = true;
      }
    }
    
    for (int i = 0; i < 4; i++) {
      if (pipes[i].x > -5 && pipes[i].x < SCREEN_WIDTH) {
        if (50 > pipes[i].x - 3 && 50 < pipes[i].x + 8) {
          if (birdY < pipes[i].gap || birdY + 4 > pipes[i].gap + pipes[i].gapSize) {
            gameOver = true;
          }
        }
      }
    }
    
    display.fillScreen(GC9A01A_BLACK);
    display.drawLine(0, 0, SCREEN_WIDTH, 0, GC9A01A_WHITE);
    display.drawLine(0, 9, SCREEN_WIDTH, 9, GC9A01A_WHITE);
    display.drawLine(0, 63, SCREEN_WIDTH, 63, GC9A01A_WHITE);
    
    display.fillRect(50, birdY, 4, 4, GC9A01A_WHITE);
    display.drawPixel(50 + 3, birdY + 1, GC9A01A_WHITE);
    
    for (int i = 0; i < 4; i++) {
      if (pipes[i].x > -5 && pipes[i].x < SCREEN_WIDTH) {
        display.fillRect(pipes[i].x, 10, 6, pipes[i].gap, GC9A01A_WHITE);
        display.fillRect(pipes[i].x, pipes[i].gap + pipes[i].gapSize, 6, 53 - pipes[i].gap - pipes[i].gapSize, GC9A01A_WHITE);
      }
    }
    
    display.setTextSize(1);
    display.setCursor(2, 1);
    display.print("Scr: ");
    display.print(score);
    display.setCursor(80, 1);
    display.print("Spd: ");
    display.print(pipeSpeed);
    
    delay(25);
  }
  
  display.fillScreen(GC9A01A_BLACK);
  display.drawLine(0, 20, SCREEN_WIDTH, 20, GC9A01A_WHITE);
  display.drawLine(0, 45, SCREEN_WIDTH, 45, GC9A01A_WHITE);
  display.setTextSize(2);
  display.setCursor(8, 0);
  display.print("Crashed!");
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.print("Score: ");
  display.print(score);
  
  
  while (!a_button_is_pressed) {
    delay(50);
  }
}

void geometryDash(void) {
  const int playerSize = 5;
  int playerY = SCREEN_HEIGHT - 10 - playerSize;
  int score = 0;
  bool gameOver = false;
  bool isJumping = false;
  int jumpVelocity = 0;
  const int gravity = 1;
  const int jumpPower = -7;
  const int groundLevel = SCREEN_HEIGHT - 10 - playerSize;
  
  struct Obstacle { int x; int type; bool active; };
  Obstacle obstacles[8];
  for (int i = 0; i < 8; i++) obstacles[i].active = false;
  
  unsigned long lastSpawn = 0;
  unsigned long gameStartTime = millis();
  int speed = 2;
  int baseSpeed = 2;
  bool spacePressed = false;
  
  while (!gameOver) {
    unsigned long now = millis();
    
    if (button_is_pressed(btn1, false) || button_is_pressed(btn4, false) || button_is_pressed(btn3, false)) {
      if (!spacePressed && !isJumping) {
        isJumping = true;
        jumpVelocity = jumpPower;
        spacePressed = true;
      }
    } else {
      spacePressed = false;
    }
    
    if (button_is_pressed(btn6)) return;
    
    if (isJumping) {
      jumpVelocity += gravity;
      playerY += jumpVelocity;
      
      if (playerY >= groundLevel) {
        playerY = groundLevel;
        isJumping = false;
        jumpVelocity = 0;
      }
    }
    
    baseSpeed = 3 + (score / 15);
    speed = baseSpeed + ((now - gameStartTime) / 10000);
    
    if (now - lastSpawn > max(600, 1000 - score * 8)) {
      for (int i = 0; i < 8; i++) {
        if (!obstacles[i].active) {
          obstacles[i].x = SCREEN_WIDTH;
          obstacles[i].type = random(0, 5);
          obstacles[i].active = true;
          lastSpawn = now;
          break;
        }
      }
    }
    
    for (int i = 0; i < 8; i++) {
      if (obstacles[i].active) {
        obstacles[i].x -= speed;
        
        if (obstacles[i].x < -10) {
          obstacles[i].active = false;
          score++;
        }
      }
    }
    
    int playerX = 8;
    int playerWidth = playerSize;
    
    for (int i = 0; i < 8; i++) {
      if (obstacles[i].active) {
        int obsX = obstacles[i].x;
        int obsWidth = 5;
        int obsHeight = 0;
        int obsY = groundLevel;
        
        if (obstacles[i].type == 0) {
          obsHeight = 5;
        } else if (obstacles[i].type == 1) {
          obsHeight = 8;
        } else if (obstacles[i].type == 2) {
          obsHeight = 11;
        } else if (obstacles[i].type == 3) {
          obsHeight = 6;
          obsY = groundLevel - 8;
        } else {
          obsHeight = 4;
          obsY = groundLevel - 12;
        }
        
        if (playerX < obsX + obsWidth &&
            playerX + playerWidth > obsX &&
            playerY < obsY + obsHeight &&
            playerY + playerSize > obsY) {
          gameOver = true;
        }
      }
    }
    
    display.fillScreen(GC9A01A_BLACK);
    
    display.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, GC9A01A_WHITE);
    display.drawLine(0, SCREEN_HEIGHT - 9, SCREEN_WIDTH, SCREEN_HEIGHT - 9, GC9A01A_WHITE);
    
    display.drawLine(0, 0, SCREEN_WIDTH, 0, GC9A01A_WHITE);
    display.drawLine(0, 8, SCREEN_WIDTH, 8, GC9A01A_WHITE);
    
    display.fillRect(playerX, playerY, playerSize, playerSize, GC9A01A_WHITE);
    display.drawPixel(playerX + 3, playerY - 1, GC9A01A_WHITE);
    
    for (int i = 0; i < 8; i++) {
      if (obstacles[i].active && obstacles[i].x > -10 && obstacles[i].x < SCREEN_WIDTH) {
        int obsX = obstacles[i].x;
        int obsWidth = 5;
        int obsHeight = 0;
        int obsY = groundLevel;
        
        if (obstacles[i].type == 0) {
          obsHeight = 5;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, GC9A01A_WHITE);
        } else if (obstacles[i].type == 1) {
          obsHeight = 8;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, GC9A01A_WHITE);
          display.drawPixel(obsX + 2, obsY - 1, GC9A01A_WHITE);
        } else if (obstacles[i].type == 2) {
          obsHeight = 11;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, GC9A01A_WHITE);
          display.drawPixel(obsX + 2, obsY - 1, GC9A01A_WHITE);
          display.drawPixel(obsX + 2, obsY - 2, GC9A01A_WHITE);
        } else if (obstacles[i].type == 3) {
          obsHeight = 6;
          obsY = groundLevel - 8;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, GC9A01A_WHITE);
          display.drawPixel(obsX + 1, obsY - 1, GC9A01A_WHITE);
          display.drawPixel(obsX + 3, obsY - 1, GC9A01A_WHITE);
        } else {
          obsHeight = 4;
          obsY = groundLevel - 12;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, GC9A01A_WHITE);
          display.drawPixel(obsX + 1, obsY - 1, GC9A01A_WHITE);
          display.drawPixel(obsX + 3, obsY - 1, GC9A01A_WHITE);
          display.drawPixel(obsX + 2, obsY - 2, GC9A01A_WHITE);
        }
      }
    }
    
    display.setTextSize(1);
    display.setCursor(2, 1);
    display.print("Scr: ");
    display.print(score);
    display.setCursor(80, 1);
    display.print("Spd:");
    display.print(speed);
    
    delay(20);
  }
  
  unsigned long finalTime = (millis() - gameStartTime) / 1000;
  
  display.fillScreen(GC9A01A_BLACK);
  display.drawLine(0, 20, SCREEN_WIDTH, 20, GC9A01A_WHITE);
  display.drawLine(0, 45, SCREEN_WIDTH, 45, GC9A01A_WHITE);
  
  display.setTextSize(2);
  display.setCursor(8, 0);
  display.print("Crashed!");
  
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("Score: ");
  display.print(score);
  
  display.setCursor(10, 35);
  display.print("Speed: ");
  display.print(speed);
  
  display.setCursor(10, 50);
  display.print("Time: ");
  display.print(finalTime);
  display.print("s");
  
  
  
  while (!a_button_is_pressed()) {
    delay(50);
  }
}

void flying3D() {
  const int horizonY = 14;
  const int horizonX = 64;
  const int shipSize = 5;
  const int minX = 8, maxX = 120, minY = 13, maxY = 60;
  const int num_coins = 4;
  float tunnel_length = 2.8f;

  const int coinMinX = 10, coinMaxX = 118;
  const int coinMinY = 14, coinMaxY = 60;

  float shipX = 64;
  float shipY = 50;

  float shipSpeed = 0.045f;
  int score = 0;

  struct Coin {
    float z;
    int x, y;
    bool active;
  };

  Coin coins[num_coins];
  for (int i = 0; i < num_coins; ++i) {
    coins[i].z = 1.1f + i * (tunnel_length / num_coins);
    coins[i].x = random(coinMinX, coinMaxX);
    coins[i].y = random(coinMinY, coinMaxY);
    coins[i].active = true;
  }

  while (true) {
    // Controls
    if (button_is_pressed(btn1, false) && shipX > minX)    shipX -= 2;
    if (button_is_pressed(btn3, false) && shipX < maxX)    shipX += 2;
    if (button_is_pressed(btn2, false) && shipY > minY)    shipY -= 2;
    if (button_is_pressed(btn5, false) && shipY < maxY)    shipY += 2;
    if (button_is_pressed(btn6)) return;

    for (int i = 0; i < num_coins; ++i) {
      if (!coins[i].active) continue;
      coins[i].z -= shipSpeed;
      if (coins[i].z < 0.13f) {
        coins[i].z = tunnel_length;
        coins[i].x = random(coinMinX, coinMaxX);
        coins[i].y = random(coinMinY, coinMaxY);
      }
    }

    for (int i = 0; i < num_coins; ++i) {
      if (!coins[i].active) continue;
      if (coins[i].z < 0.38f && coins[i].z > 0.13f) {
        float scale = 1.4f / coins[i].z;
        int coin_x = horizonX + int((coins[i].x - horizonX) * scale);
        int coin_y = horizonY + int((coins[i].y - horizonY) * scale);
        int coin_r = 3 + int(2.0f * scale);

        int dx = int(shipX) - coin_x;
        int dy = int(shipY) - coin_y;
        int dist2 = dx*dx + dy*dy;
        if (dist2 < (coin_r + 7)*(coin_r + 7)) {
          score++;
          coins[i].z = tunnel_length;
          coins[i].x = random(coinMinX, coinMaxX);
          coins[i].y = random(coinMinY, coinMaxY);
        }
      }
    }

    if (score && (score % 20 == 0) && shipSpeed < 0.13f)
      shipSpeed += 0.008f;

    display.fillScreen(GC9A01A_BLACK);
    for (int i = -2; i <= 2; ++i)
      display.drawLine(horizonX, horizonY, horizonX + i * 20, 63, GC9A01A_WHITE);

    for (int i = 0; i < num_coins; ++i) {
      if (!coins[i].active) continue;
      float scale = 1.4f / coins[i].z;
      int coin_x = horizonX + int((coins[i].x - horizonX) * scale);
      int coin_y = horizonY + int((coins[i].y - horizonY) * scale);
      int coin_r = 3 + int(2.0f * scale);
      display.drawCircle(coin_x, coin_y, coin_r, GC9A01A_WHITE);
      display.setCursor(coin_x - 2, coin_y - 3);
      display.print("C");
    }

    int x0 = int(shipX), y0 = int(shipY);
    display.fillTriangle(x0 - shipSize, y0 + shipSize + 2,
                        x0 + shipSize, y0 + shipSize + 2,
                        x0, y0 - shipSize + 1, GC9A01A_WHITE);
    display.drawPixel(x0, y0 - 2, GC9A01A_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);  display.print("3D COINS");
    display.setCursor(88, 2); display.print("Score:");
    display.print(score);

    delay(26);
  }
}

void countdown() {
    const int bigNums[4] = {25, 50, 75, 100};
    const int smallNums[10] = {1,2,3,4,5,6,7,8,9,10};
    int selectedCount = 6;
    int numbers[6];
    int target = 0;
    bool readyForNewRound = true;

    while (true) {
        while (readyForNewRound) {
            display.fillScreen(GC9A01A_BLACK);
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.print("COUNTDOWN MATHS");

            display.setCursor(0, 15);
            display.print("Select # of numbers:");
            display.setTextSize(2);
            display.setCursor(54, 38);
            display.print(selectedCount);

            display.setTextSize(1);
            display.setCursor(0, 55);
            display.print("1/3:< >  5:Start  6:Exit");
        
            if (button_is_pressed(btn1, false) && selectedCount > 2) {
                selectedCount--;
                delay(180);
            } else if (button_is_pressed(btn3, false) && selectedCount < 6) {
                selectedCount++;
                delay(180);
            } else if (button_is_pressed(btn5, true)) {
                readyForNewRound = false;
            } else if (button_is_pressed(btn6)) {
                return; 
            }
            delay(30);
        }

        int bigTaken[4] = {0,0,0,0}, smallTaken[10] = {0,0,0,0,0,0,0,0,0,0};
        int i = 0;
        while (i < selectedCount) {
            if (random(0, 2)) { 
                int idx = random(0, 10);
                if (!smallTaken[idx]) {
                    numbers[i++] = smallNums[idx];
                    smallTaken[idx] = 1;
                }
            } else {
                int idx = random(0, 4);
                if (!bigTaken[idx]) {
                    numbers[i++] = bigNums[idx];
                    bigTaken[idx] = 1;
                }
            }
        }
        target = random(100, 1000);

        while (true) {
            display.fillScreen(GC9A01A_BLACK);
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.print("TARGET:");
            display.setTextSize(2);
            display.setCursor(64, 0);
            display.print(target);

            display.setTextSize(1);
            display.setCursor(0, 22);
            display.print("Numbers: ");
            for (int j = 0; j < selectedCount; j++) {
                display.print(numbers[j]);
                if (j != selectedCount-1) display.print(", ");
            }

            display.setCursor(0, 52);
            display.print("1: Solve 5:New");
        
            if (button_is_pressed(btn5, true)) {
                readyForNewRound = true;
                break;
            }
            // else if (button_is_pressed(btn1)) countdownSolver();
            if (button_is_pressed(btn6)) return;

            delay(80);
        }
    }
}
/*
int intPow(int base, int exp) {
    int res = 1;
    for (int i = 0; i < exp; ++i) res *= base;
    return res;
}

// integerRoot returns the n-th root of value if exact, else -1
int integerRoot(int value, int rootN) {
    if (rootN <= 1 || value < 0) return -1;
    int guess = 1;
    while (true) {
        int powRes = intPow(guess, rootN);
        if (powRes == value) return guess;
        if (powRes > value) return -1;
        ++guess;
    }
    return -1;
}

bool solveCountdownRecursive(int numbers[], int numCount, int target, CountdownStep steps[], int nsteps, CountdownSolution &best, int &bestDiff) {
    // Check if any current number is exact target
    for (int i = 0; i < numCount; ++i) {
        if (numbers[i] == target) {
            if (abs(target - best.result) < bestDiff) {
                best.result = numbers[i];
                best.nsteps = nsteps;
                for (int s = 0; s < nsteps; ++s) best.steps[s] = steps[s];
                bestDiff = 0;
            }
            return true; // Found exact!
        }
    }
    bool foundExact = false;

    for (int i = 0; i < numCount; ++i) {
        for (int j = 0; j < numCount; ++j) {
            if (i == j) continue;
            int a = numbers[i], b = numbers[j];
            int remain[MAX_COUNT_NUMS];
            int pos = 0;
            for (int k = 0; k < numCount; ++k)
                if (k != i && k != j)
                    remain[pos++] = numbers[k];

            // --- Try all operators ---
            for (int opidx = 0; opidx < 6; ++opidx) {
                int res = 0;
                char op = 0;

                // 0: +   1: -   2: *   3:/   4: ^    5: r
                if (opidx == 0) { res = a + b; op = '+'; }
                if (opidx == 1) { res = a - b; op = '-'; }
                if (opidx == 2) { res = a * b; op = '*'; }
                if (opidx == 3) {
                    if (b == 0 || a % b != 0) continue;
                    res = a / b; op = '/';
                }
                if (opidx == 4) { // POWER: a^b, only b>=1, result <= 1000000 (avoid overflow)
                    if (b <= 1 || a==1 || a==0) continue; // Skip for degenerate powers
                    if (b > 8 || abs(a) > 200) continue; // Avoid huge numbers
                    res = intPow(a, b); op = '^';
                    if (res == a || res == b || abs(res) > 1000000 || res < 0) continue;
                }
                if (opidx == 5) { // ROOT: a r b (b-th root of a)
                    if (b <= 1 || a <= 0) continue;
                    int root = integerRoot(a, b);
                    if (root == -1 || root == a || root == b) continue;
                    res = root; op = 'r';
                }

                // Exclude duplicate/dangerous cases
                if ((op == '-' || op == '/' || op == '^' || op == 'r') && i < j) continue;

                remain[pos] = res;
                steps[nsteps] = CountdownStep(a, b, op, res);

                int diff = abs(res - target);
                if (diff < bestDiff) {
                    best.result = res;
                    best.nsteps = nsteps + 1;
                    for (int s = 0; s <= nsteps; ++s) best.steps[s] = steps[s];
                    bestDiff = diff;
                }
                if (diff == 0) {
                    return true;
                }

                if (pos + 1 > 1)
                  if (solveCountdownRecursive(remain, pos+1, target, steps, nsteps+1, best, bestDiff))
                      return true;
            }
        }
    }
    return foundExact;
}


// Utilities for data entry
int enterIntInput(const char *prompt, int initial, int minv, int maxv) {
    int val = initial;
    while (true) {
        display.fillScreen(GC9A01A_BLACK);
        display.setTextSize(1);
        display.setCursor(0, 8);
        display.print(prompt);
        display.setTextSize(2);
        display.setCursor(40, 32);
        display.print(val);
        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print("1/3: < >   5:OK   6:Exit");
    
        if (button_is_pressed(btn1, false) && val > minv) {
            val--;
            delay(120);
        } else if (button_is_pressed(btn3, false) && val < maxv) {
            val++;
            delay(120);
        } else if (button_is_pressed(btn5, true)) {
            return val;
        } else if (button_is_pressed(btn6)) {
            return -1;
        }
        delay(30);
    }
}

// ----------- Main countdown solver game -----------
void countdownSolver() {
    int target = 100;
    int nCount = 6;
    int numbers[MAX_COUNT_NUMS];

    // Step 1: Target entry
    int entry = enterIntInput("Target (100-999):", target, 100, 999);
    if (entry == -1) return;
    target = entry;

    // Step 2: # numbers to use
    entry = enterIntInput("# numbers (2-6):", nCount, 2, 6);
    if (entry == -1) return;
    nCount = entry;

    // Step 3: Enter numbers
    for (int i = 0; i < nCount; ++i) {
        char buf[22]; sprintf(buf, "Num %d (1-100):", i+1);
        entry = enterIntInput(buf, 10, 1, 100);
        if (entry == -1) return;
        numbers[i] = entry;
    }

    // Show entered data
    while (true) {
        display.fillScreen(GC9A01A_BLACK);
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("Target: "); display.print(target);
        display.setCursor(0, 12);
        display.print("Numbers: ");
        for (int i = 0; i < nCount; ++i) {
            display.print(numbers[i]);
            if (i != nCount-1) display.print(", ");
        }
        display.setCursor(0, 30);
        display.print("5:Solve   6:Exit");
    
        if (button_is_pressed(btn5, true)) break;
        if (button_is_pressed(btn6)) return;
        delay(60);
    }

    // Step 4: Solve!
    CountdownStep steps[64];
    CountdownSolution best;
    best.result = 0; best.nsteps = 0;
    int bestDiff = 9999999;
    display.fillScreen(GC9A01A_BLACK);
    display.setTextSize(2);
    display.setCursor(14, 24); display.print("SOLVING...");

    solveCountdownRecursive(numbers, nCount, target, steps, 0, best, bestDiff);

    // Step 5: Display solution
    int page = 0;
    int totalPages = (best.nsteps + SOLUTION_DISPLAY_STEPS - 1) / SOLUTION_DISPLAY_STEPS;
    if (best.nsteps == 0) totalPages = 1;

    while (true) {
        display.fillScreen(GC9A01A_BLACK);
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("Target: "); display.print(target);
        display.setCursor(83, 0);
        display.print("Best: "); display.print(best.result);

        if (best.nsteps == 0) {
            display.setCursor(0, 18);
            display.print("No solution found.");
        } else {
            display.setCursor(0, 12);
            for (int i = 0; i < SOLUTION_DISPLAY_STEPS; ++i) {
                int idx = page * SOLUTION_DISPLAY_STEPS + i;
                if (idx >= best.nsteps) break;
                CountdownStep &st = best.steps[idx];
                char line[28];
                if (st.op == '^')
                    sprintf(line, "%d ^ %d = %d", st.a, st.b, st.res);
                else if (st.op == 'r')
                    sprintf(line, "%d r %d = %d", st.a, st.b, st.res);
                else
                    sprintf(line, "%d %c %d = %d", st.a, st.op, st.b, st.res);
                display.setCursor(0, 19 + 11 * i);
                display.print(line);
            }
        }
        display.setCursor(0, 58);
        if (totalPages > 1)
            display.print("1/3:< > 5:More  6:Exit");
        else
            display.print("5:Again  6:Exit");
    
        if (best.nsteps > 0 && totalPages > 1) {
            if (button_is_pressed(btn1, false) && page > 0) {
                page--;
                delay(130);
            }
            if (button_is_pressed(btn3, false) && page < totalPages-1) {
                page++;
                delay(130);
            }
        }
        if (button_is_pressed(btn5, true)) {
            if (best.nsteps == 0 || totalPages == 1)
                return countdownSolver(); // Start again
            else if (page < totalPages-1) {
                page++;
            } else {
                page = 0;
            }
            delay(130);
        }
        if (button_is_pressed(btn6))
            return;
        delay(60);
    }
}
*/
