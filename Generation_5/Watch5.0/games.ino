// Includes: Shooter, Snake, Flappy Bird, Geo Dash

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

void games() {
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("1. Shooter");
    display.println("2. Snake");
    display.println("3. Flappy Bird");
    display.println("4. Geometry Dash");
    display.display();
    delay(50);
    
    if (button_is_pressed(btn1)) shooter();
    else if (button_is_pressed(btn2)) snake();
    else if (button_is_pressed(btn3)) flappyBird();
    else if (button_is_pressed(btn4)) geometryDash();
    else if (button_is_pressed(btn6)) return;
  }
}

void shooter() {
  int playerX = 120;
  int playerY = 55;
  int score = 0;
  int health = 3;
  
  struct Bullet { int x; int y; bool active; };
  Bullet bullets[8];
  for (int i = 0; i < 8; i++) bullets[i].active = false;
  
  struct Enemy { int x; int y; bool active; };
  Enemy enemies[3];
  for (int i = 0; i < 3; i++) {
    enemies[i].x = (i * 40) + 10;
    enemies[i].y = 5;
    enemies[i].active = true;
  }

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

    display.clearDisplay();
    
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
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(score);
    display.setCursor(100, 0);
    display.print("H:");
    display.print(health);
    
    display.display();
    delay(30);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(3000);
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
    
    if (button_is_pressed(btn2, false) && dirX == 0) { nextDirX = -4; nextDirY = 0; }
    if (button_is_pressed(btn3, false) && dirX == 0) { nextDirX = 4; nextDirY = 0; }
    if (button_is_pressed(btn1, false) && dirY == 0) { nextDirX = 0; nextDirY = -4; }
    if (button_is_pressed(btn4, false) && dirY == 0) { nextDirX = 0; nextDirY = 4; }
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
    
    display.clearDisplay();
    
    for (int i = 0; i < snakeLength; i++) {
      display.fillRect(snake[i].x, snake[i].y, 3, 3, SSD1306_WHITE);
    }
    
    display.fillRect(foodX, foodY, 3, 3, SSD1306_WHITE);
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Len:");
    display.print(snakeLength);
    display.setCursor(65, 0);
    display.print("Score:");
    display.print(score);
    
    display.display();
    delay(30);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(3000);
}

void flappyBird(void) {
  int birdY = 30;
  int birdVelocity = 0;
  const int gravity = 1;
  const int flapPower = -2;
  int score = 0;
  bool gameOver = false;
  
  struct Pipe { int x; int gap; bool passed; };
  Pipe pipes[3];
  for (int i = 0; i < 3; i++) {
    pipes[i].x = 128 + i * 50;
    pipes[i].gap = random(30, 100);
    pipes[i].passed = false;
  }
  
  unsigned long lastFlap = 0;
  
  while (!gameOver) {
    if (button_is_pressed(btn3, false) || button_is_pressed(btn5, false)) {
      birdVelocity = flapPower;
      lastFlap = millis();
    }
    if (button_is_pressed(btn6)) return;
    
    birdVelocity += gravity;
    birdY += birdVelocity;
    
    if (birdY < 0 || birdY > 64) gameOver = true;
    
    for (int i = 0; i < 3; i++) {
      pipes[i].x--;
      
      if (pipes[i].x < 0) {
        pipes[i].x = 128;
        pipes[i].gap = random(50, 100);
        pipes[i].passed = false;
      }
      
      if (pipes[i].x == 60 && !pipes[i].passed) {
        score++;
        pipes[i].passed = true;
      }
    }
    
    for (int i = 0; i < 3; i++) {
      if (pipes[i].x > 0 && pipes[i].x < 128) {
        if (pipes[i].x < 65 && pipes[i].x + 3 > 60) {
          if (birdY < pipes[i].gap) gameOver = true;
          if (birdY + 4 > pipes[i].gap + 20) gameOver = true;
        }
      }
    }
    
    display.clearDisplay();
    
    display.fillRect(60, birdY, 4, 4, SSD1306_WHITE);
    
    for (int i = 0; i < 3; i++) {
      if (pipes[i].x > 0 && pipes[i].x < 128) {
        display.fillRect(pipes[i].x, 0, 3, pipes[i].gap, SSD1306_WHITE);
        display.fillRect(pipes[i].x, pipes[i].gap + 20, 3, 64 - pipes[i].gap - 20, SSD1306_WHITE);
      }
    }
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Score:");
    display.print(score);
    
    display.display();
    delay(30);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 20);
  display.print("CRASHED!");
  display.setTextSize(1);
  display.setCursor(20, 45);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(3000);
}

void geometryDash(void) {
  const int playerSize = 6;
  int playerY = SCREEN_HEIGHT - 8 - playerSize;
  int score = 0;
  bool gameOver = false;
  bool isJumping = false;
  int jumpVelocity = 0;
  const int gravity = 1;
  const int jumpPower = -8;
  const int groundLevel = SCREEN_HEIGHT - playerSize;
  
  struct Obstacle { int x; int y; int type; bool active; };
  Obstacle obstacles[6];
  for (int i = 0; i < 6; i++) obstacles[i].active = false;
  
  unsigned long lastSpawn = 0;
  unsigned long gameStartTime = millis();
  int speed = 2;
  
  while (!gameOver) {
    unsigned long now = millis();
    
    if ((button_is_pressed(btn1, false) || button_is_pressed(btn4, false)) && !isJumping) {
      isJumping = true;
      jumpVelocity = jumpPower;
    }
    if (button_is_pressed(btn6)) return;
    
    if (isJumping) {
      jumpVelocity += gravity;
      playerY += jumpVelocity;
      
      if (playerY >= groundLevel) {
        playerY = groundLevel - playerSize;
        isJumping = false;
        jumpVelocity = 0;
      }
    }
    
    speed = 2 + (score / 10);
    
    if (now - lastSpawn > max(1000, 1500 - score * 15)) {
      for (int i = 0; i < 6; i++) {
        if (!obstacles[i].active) {
          obstacles[i].x = SCREEN_WIDTH;
          obstacles[i].type = random(0, 3);
          
          if (obstacles[i].type == 0) {
            obstacles[i].y = groundLevel - 4;
          } else if (obstacles[i].type == 1) {
            obstacles[i].y = groundLevel - 10;
          } else {
            obstacles[i].y = groundLevel - 16;
          }
          
          obstacles[i].active = true;
          lastSpawn = now;
          break;
        }
      }
    }
    
    for (int i = 0; i < 6; i++) {
      if (obstacles[i].active) {
        obstacles[i].x -= speed;
        
        if (obstacles[i].x < 0) {
          obstacles[i].active = false;
          score++;
        }
      }
    }
    
    int playerX = 8;
    int playerWidth = 6;
    
    for (int i = 0; i < 6; i++) {
      if (obstacles[i].active) {
        int obstacleWidth = 4;
        int obstacleHeight = 0;
        
        if (obstacles[i].type == 0) obstacleHeight = 4;
        else if (obstacles[i].type == 1) obstacleHeight = 10;
        else obstacleHeight = 6;
        
        if (playerX < obstacles[i].x + obstacleWidth &&
            playerX + playerWidth > obstacles[i].x &&
            playerY < obstacles[i].y + obstacleHeight &&
            playerY + playerSize > obstacles[i].y) {
          gameOver = true;
        }
      }
    }
    
    display.clearDisplay();
    
    display.drawLine(0, groundLevel, SCREEN_WIDTH, groundLevel, SSD1306_WHITE);
    
    display.fillRect(playerX, playerY, playerSize, playerSize, SSD1306_WHITE);
    
    for (int i = 0; i < 6; i++) {
      if (obstacles[i].active) {
        int obsWidth = 4;
        int obsHeight = 0;
        
        if (obstacles[i].type == 0) {
          obsHeight = 4;
        } else if (obstacles[i].type == 1) {
          obsHeight = 10;
        } else {
          obsHeight = 6;
        }
        
        display.fillRect(obstacles[i].x, obstacles[i].y, obsWidth, obsHeight, SSD1306_WHITE);
        
        if (obstacles[i].type == 1) {
          display.drawPixel(obstacles[i].x + 2, obstacles[i].y - 1, SSD1306_WHITE);
        }
      }
    }
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Score:");
    display.print(score);
    
    display.setCursor(SCREEN_WIDTH - 32, 0);
    display.print("Spd:");
    display.print(speed);
    
    display.display();
    delay(20);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 0);
  display.print("CRASHED!");
  display.setTextSize(1);
  display.setCursor(15, 25);
  display.print("Score: ");
  display.print(score);
  display.setCursor(15, 40);
  display.print("Speed: ");
  display.print(speed);
  display.display();
  delay(3000);
}
