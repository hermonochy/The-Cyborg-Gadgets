// Includes: Shooter, Snake, Flappy Bird, Geometry Dash

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

void games() {
  int selected = 0;
  const char* gameList[] = {"Shooter", "Snake", "Flappy Bird", "Geometry Dash", "Pac-Man"};
  
  while (true) {
    display.clearDisplay();
    display.drawLine(0, 0, SCREEN_WIDTH, 0, SSD1306_WHITE);
    display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
    
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("Games");
    
    display.setTextSize(1);
    for (int i = 0; i < 5; i++) {
      display.setCursor(15, 18 + (i * 9));
      if (i == selected) {
        display.setTextColor(SSD1306_BLACK);
        display.fillRect(10, 17 + (i * 9), 108, 10, SSD1306_WHITE);
        display.print(gameList[i]);
        display.setTextColor(SSD1306_WHITE);
      } else {
        display.print(gameList[i]);
      }
    }
    
    display.display();
    delay(50);
    
    if (button_is_pressed(btn2)) {
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
          pacman();
          break;
      }
    }
    else if (button_is_pressed(btn6)) return;
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
  display.setCursor(10, 0);
  display.print("Game Over");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.print("Score: ");
  display.print(score);
  display.display();
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
    
    display.clearDisplay();
    display.drawLine(0, 0, SCREEN_WIDTH, 0, SSD1306_WHITE);
    display.drawLine(0, 9, SCREEN_WIDTH, 9, SSD1306_WHITE);
    display.drawLine(0, 63, SCREEN_WIDTH, 63, SSD1306_WHITE);
    
    display.fillRect(50, birdY, 4, 4, SSD1306_WHITE);
    display.drawPixel(50 + 3, birdY + 1, SSD1306_WHITE);
    
    for (int i = 0; i < 4; i++) {
      if (pipes[i].x > -5 && pipes[i].x < SCREEN_WIDTH) {
        display.fillRect(pipes[i].x, 10, 6, pipes[i].gap, SSD1306_WHITE);
        display.fillRect(pipes[i].x, pipes[i].gap + pipes[i].gapSize, 6, 53 - pipes[i].gap - pipes[i].gapSize, SSD1306_WHITE);
      }
    }
    
    display.setTextSize(1);
    display.setCursor(2, 1);
    display.print("Scr: ");
    display.print(score);
    display.setCursor(80, 1);
    display.print("Spd: ");
    display.print(pipeSpeed);
    
    display.display();
    delay(25);
  }
  
  display.clearDisplay();
  display.drawLine(0, 20, SCREEN_WIDTH, 20, SSD1306_WHITE);
  display.drawLine(0, 45, SCREEN_WIDTH, 45, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(8, 0);
  display.print("Crashed!");
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.print("Score: ");
  display.print(score);
  display.display();
  
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
    
    display.clearDisplay();
    
    display.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, SSD1306_WHITE);
    display.drawLine(0, SCREEN_HEIGHT - 9, SCREEN_WIDTH, SCREEN_HEIGHT - 9, SSD1306_WHITE);
    
    display.drawLine(0, 0, SCREEN_WIDTH, 0, SSD1306_WHITE);
    display.drawLine(0, 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
    
    display.fillRect(playerX, playerY, playerSize, playerSize, SSD1306_WHITE);
    display.drawPixel(playerX + 3, playerY - 1, SSD1306_WHITE);
    
    for (int i = 0; i < 8; i++) {
      if (obstacles[i].active && obstacles[i].x > -10 && obstacles[i].x < SCREEN_WIDTH) {
        int obsX = obstacles[i].x;
        int obsWidth = 5;
        int obsHeight = 0;
        int obsY = groundLevel;
        
        if (obstacles[i].type == 0) {
          obsHeight = 5;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, SSD1306_WHITE);
        } else if (obstacles[i].type == 1) {
          obsHeight = 8;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, SSD1306_WHITE);
          display.drawPixel(obsX + 2, obsY - 1, SSD1306_WHITE);
        } else if (obstacles[i].type == 2) {
          obsHeight = 11;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, SSD1306_WHITE);
          display.drawPixel(obsX + 2, obsY - 1, SSD1306_WHITE);
          display.drawPixel(obsX + 2, obsY - 2, SSD1306_WHITE);
        } else if (obstacles[i].type == 3) {
          obsHeight = 6;
          obsY = groundLevel - 8;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, SSD1306_WHITE);
          display.drawPixel(obsX + 1, obsY - 1, SSD1306_WHITE);
          display.drawPixel(obsX + 3, obsY - 1, SSD1306_WHITE);
        } else {
          obsHeight = 4;
          obsY = groundLevel - 12;
          display.fillRect(obsX, obsY, obsWidth, obsHeight, SSD1306_WHITE);
          display.drawPixel(obsX + 1, obsY - 1, SSD1306_WHITE);
          display.drawPixel(obsX + 3, obsY - 1, SSD1306_WHITE);
          display.drawPixel(obsX + 2, obsY - 2, SSD1306_WHITE);
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
    
    display.display();
    delay(20);
  }
  
  unsigned long finalTime = (millis() - gameStartTime) / 1000;
  
  display.clearDisplay();
  display.drawLine(0, 20, SCREEN_WIDTH, 20, SSD1306_WHITE);
  display.drawLine(0, 45, SCREEN_WIDTH, 45, SSD1306_WHITE);
  
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
  
  display.display();
  
  while (!a_button_is_pressed()) {
    delay(50);
  }
}

void generateMaze(int maze[8][16]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      maze[y][x] = 1;
    }
  }
  
  bool visited[8][16];
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      visited[y][x] = false;
    }
  }
  
  maze[1][1] = 0;
  visited[1][1] = true;
  
  int stackX[128], stackY[128];
  int stackPtr = 0;
  stackX[stackPtr] = 1;
  stackY[stackPtr] = 1;
  stackPtr++;
  
  while (stackPtr > 0) {
    int x = stackX[stackPtr - 1];
    int y = stackY[stackPtr - 1];
    
    int dx[] = {0, 0, -2, 2};
    int dy[] = {-2, 2, 0, 0};
    int directions[4] = {0, 1, 2, 3};
    
    for (int i = 0; i < 4; i++) {
      int r = random(0, 4);
      int temp = directions[i];
      directions[i] = directions[r];
      directions[r] = temp;
    }
    
    bool foundUnvisited = false;
    for (int i = 0; i < 4; i++) {
      int dir = directions[i];
      int nx = x + dx[dir];
      int ny = y + dy[dir];
      
      if (nx > 0 && nx < 15 && ny > 0 && ny < 7 && !visited[ny][nx]) {
        visited[ny][nx] = true;
        maze[ny][nx] = 0;
        
        maze[y + dy[dir] / 2][x + dx[dir] / 2] = 0;
        
        stackX[stackPtr] = nx;
        stackY[stackPtr] = ny;
        stackPtr++;
        foundUnvisited = true;
        break;
      }
    }
    
    if (!foundUnvisited) {
      stackPtr--;
    }
  }
  
  for (int y = 1; y < 7; y++) {
    for (int x = 1; x < 15; x++) {
      if (maze[y][x] == 0) {
        maze[y][x] = 2;
      }
    }
  }
  
  for (int y = 1; y < 3; y++) {
    for (int x = 1; x < 4; x++) {
      if (maze[y][x] == 2) {
        maze[y][x] = 3;
        break;
      }
    }
  }
  
  for (int y = 1; y < 3; y++) {
    for (int x = 13; x > 11; x--) {
      if (maze[y][x] == 2) {
        maze[y][x] = 3;
        break;
      }
    }
  }
  
  for (int y = 6; y > 4; y--) {
    for (int x = 1; x < 4; x++) {
      if (maze[y][x] == 2) {
        maze[y][x] = 3;
        break;
      }
    }
  }
  
  for (int y = 6; y > 4; y--) {
    for (int x = 13; x > 11; x--) {
      if (maze[y][x] == 2) {
        maze[y][x] = 3;
        break;
      }
    }
  }
}
void pacman(void) {
  int maze[8][16];
  generateMaze(maze);
  
  int pacmanX = 1, pacmanY = 1;
  int pacmanDir = 1;
  int score = 0;
  int lives = 3;
  bool gameOver = false;
  bool powerMode = false;
  unsigned long powerModeStart = 0;
  const int powerModeDuration = 8000;
  
  struct Ghost {
    int x;
    int y;
    int dirX;
    int dirY;
    int type;
  };
  
  Ghost ghosts[4];
  ghosts[0] = {13, 3, -1, 0, 0};
  ghosts[1] = {13, 4, -1, 0, 1};
  ghosts[2] = {13, 5, -1, 0, 2};
  ghosts[3] = {13, 6, -1, 0, 3};
  
  unsigned long lastGhostMove = 0;
  unsigned long lastPacMove = 0;
  int pelletsRemaining = 0;
  
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      if (maze[y][x] == 2 || maze[y][x] == 3) pelletsRemaining++;
    }
  }
  
  while (!gameOver && lives > 0 && pelletsRemaining > 0) {
    unsigned long now = millis();
    
    if (button_is_pressed(btn1, false)) pacmanDir = 0;
    if (button_is_pressed(btn3, false)) pacmanDir = 1;
    if (button_is_pressed(btn5, false)) pacmanDir = 2;
    if (button_is_pressed(btn2, false)) pacmanDir = 3;
    if (button_is_pressed(btn6)) return;
    
    if (now - lastPacMove > 150) {
      int nextX = pacmanX;
      int nextY = pacmanY;
      
      if (pacmanDir == 0) nextX--;
      if (pacmanDir == 1) nextX++;
      if (pacmanDir == 2) nextY++;
      if (pacmanDir == 3) nextY--;
      
      if (nextX >= 1 && nextX < 15 && nextY >= 1 && nextY < 7 && maze[nextY][nextX] != 1) {
        pacmanX = nextX;
        pacmanY = nextY;
      }
      
      if (maze[pacmanY][pacmanX] == 2) {
        maze[pacmanY][pacmanX] = 0;
        score += 10;
        pelletsRemaining--;
      } else if (maze[pacmanY][pacmanX] == 3) {
        maze[pacmanY][pacmanX] = 0;
        score += 50;
        powerMode = true;
        powerModeStart = now;
        pelletsRemaining--;
      }
      
      lastPacMove = now;
    }
    
    if (now - lastGhostMove > 300) {
      for (int g = 0; g < 4; g++) {
        int bestX = ghosts[g].x;
        int bestY = ghosts[g].y;
        int bestDist = 9999;
        
        int tryX[] = {ghosts[g].x - 1, ghosts[g].x + 1, ghosts[g].x, ghosts[g].x};
        int tryY[] = {ghosts[g].y, ghosts[g].y, ghosts[g].y - 1, ghosts[g].y + 1};
        
        for (int d = 0; d < 4; d++) {
          if (tryX[d] >= 1 && tryX[d] < 15 && tryY[d] >= 1 && tryY[d] < 7 && maze[tryY[d]][tryX[d]] != 1) {
            int dx = abs(tryX[d] - pacmanX);
            int dy = abs(tryY[d] - pacmanY);
            int dist = dx + dy;
            
            if (ghosts[g].type == 0 && !powerMode) {
              if (dist < bestDist) {
                bestDist = dist;
                bestX = tryX[d];
                bestY = tryY[d];
              }
            } else if (powerMode || ghosts[g].type == 3) {
              if (dist > bestDist) {
                bestDist = dist;
                bestX = tryX[d];
                bestY = tryY[d];
              }
            } else {
              if (random(0, 3) == 0 && dist < bestDist) {
                bestDist = dist;
                bestX = tryX[d];
                bestY = tryY[d];
              }
            }
          }
        }
        
        ghosts[g].x = bestX;
        ghosts[g].y = bestY;
      }
      lastGhostMove = now;
    }
    
    for (int g = 0; g < 4; g++) {
      if (ghosts[g].x == pacmanX && ghosts[g].y == pacmanY) {
        if (powerMode) {
          ghosts[g].x = 13;
          ghosts[g].y = 3 + g;
          score += 200;
        } else {
          lives--;
          pacmanX = 1;
          pacmanY = 1;
        }
      }
    }
    
    if (powerMode && now - powerModeStart > powerModeDuration) {
      powerMode = false;
    }
    
    display.clearDisplay();
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 16; x++) {
        int pixelX = x * 8;
        int pixelY = y * 8;
        
        if (maze[y][x] == 1) {
          display.fillRect(pixelX + 1, pixelY + 1, 6, 6, SSD1306_WHITE);
        } else if (maze[y][x] == 2) {
          display.drawPixel(pixelX + 3, pixelY + 3, SSD1306_WHITE);
          display.drawPixel(pixelX + 4, pixelY + 3, SSD1306_WHITE);
        } else if (maze[y][x] == 3) {
          display.fillRect(pixelX + 2, pixelY + 2, 4, 4, SSD1306_WHITE);
        }
      }
    }
    
    display.fillRect(pacmanX * 8 + 2, pacmanY * 8 + 2, 4, 4, SSD1306_WHITE);
    display.drawPixel(pacmanX * 8 + 6, pacmanY * 8 + 3, SSD1306_WHITE);
    
    for (int g = 0; g < 4; g++) {
      if (powerMode) {
        display.drawRect(ghosts[g].x * 8 + 2, ghosts[g].y * 8 + 2, 4, 4, SSD1306_WHITE);
      } else {
        display.fillRect(ghosts[g].x * 8 + 2, ghosts[g].y * 8 + 2, 4, 4, SSD1306_WHITE);
      }
    }
    
    display.setTextSize(1);
    display.setCursor(116, 0);
    display.print("S");
    display.setCursor(116, 10);
    display.print(score / 100);
    
    display.setCursor(116, 25);
    display.print("L");
    display.setCursor(116, 35);
    display.print(lives);
    
    display.setCursor(116, 50);
    display.print("P");
    display.setCursor(116, 58);
    display.print(pelletsRemaining / 10);
    
    display.display();
    delay(40);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 0);
  if (pelletsRemaining == 0) {
    display.print("YOU WIN!");
  } else {
    display.print("GAME OVER");
  }
  
  display.setTextSize(1);
  display.setCursor(20, 35);
  display.print("Score: ");
  display.print(score);
  display.display();
  
  while (!button_is_pressed(btn3)) {
    delay(50);
  }
}
