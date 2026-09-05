#define totalGames 3

const char *Games[] = {"Space Invaders", "Snake", "Geometry Dash"};

void games() {
  selectedFunction = drawMenu(Games, totalGames);
  switch (selectedFunction) {
    case 0:  spaceInvaders();  break; 
    case 1:  snake();          break;
    case 2:  geometryDash();   break;
  }
  delay(150);
}

void spaceInvaders() {
  while (a_button_is_pressed()) delay(10);

  const int SCREEN_SIZE = 240;
  const int CENTER = 120;
  const int PLAY_R = 100;
  const int SAFE_R = 28;

  struct Player {
    float angle;
    int shots[8];
    int shotAngles[8];
    int shotCount;
  } player;
  player.angle = 0;
  player.shotCount = 0;
  for (int i = 0; i < 8; i++) player.shots[i] = -1;

  struct Enemy {
    float angle;
    int radius;
    bool active;
  };
  Enemy enemies[12];
  int enemyCount = 0;
  unsigned long lastEnemySpawn = 0;
  unsigned long SPAWN_INTERVAL = 800;

  int score = 0;
  int health = 3;
  unsigned long gameTime = 0;
  unsigned long lastUpdate = millis();
  bool gameOver = false;
  unsigned long gameOverTime = 0;

  for (int i = 0; i < 12; i++) {
    enemies[i].active = false;
  }

  auto spawnEnemy = [&]() {
    if (enemyCount < 12) {
      enemies[enemyCount].angle = random(0, 360) * M_PI / 180.0;
      enemies[enemyCount].radius = PLAY_R - 2;
      enemies[enemyCount].active = true;
      enemyCount++;
    }
  };

  auto playerToCart = [&](float &x, float &y) {
    x = CENTER + cos(player.angle - M_PI/2) * SAFE_R;
    y = CENTER + sin(player.angle - M_PI/2) * SAFE_R;
  };

  auto enemyToCart = [&](const Enemy &e, float &x, float &y) {
    x = CENTER + cos(e.angle) * e.radius;
    y = CENTER + sin(e.angle) * e.radius;
  };

  auto drawPlayer = [&](float px, float py) {
    canvas.fillCircle(px, py, 6, display.color565(0, 200, 255));
    float tipX = px + cos(player.angle - M_PI/2) * 10;
    float tipY = py + sin(player.angle - M_PI/2) * 10;
    canvas.drawLine(px, py, tipX, tipY, display.color565(100, 240, 255));
  };

  auto drawEnemy = [&](float ex, float ey) {
    canvas.fillCircle(ex, ey, 5, display.color565(255, 100, 80));
    canvas.drawCircle(ex, ey, 5, display.color565(255, 150, 120));
  };

  auto drawShot = [&](float sx, float sy) {
    canvas.fillCircle(sx, sy, 2, display.color565(100, 255, 200));
  };

  while (!gameOver) {
    unsigned long now = millis();
    unsigned long dt = now - lastUpdate;
    lastUpdate = now;
    gameTime += dt;

    if (button_is_pressed(buttons[5])) {
      player.angle += 0.15;
      delay(60);
    }
    if (button_is_pressed(buttons[3])) {
      player.angle -= 0.15;
      delay(60);
    }
    if (button_is_pressed(buttons[4], true)) {
      if (player.shotCount < 8) {
        player.shots[player.shotCount] = PLAY_R - 10;
        player.shotAngles[player.shotCount] = player.angle;
        player.shotCount++;
      }
    }
    if (button_is_pressed(buttons[2], true)) {
      return;
    }

    if (now - lastEnemySpawn > SPAWN_INTERVAL) {
      spawnEnemy();
      lastEnemySpawn = now;
    }

    for (int i = 0; i < player.shotCount; i++) {
      if (player.shots[i] >= 0) {
        player.shots[i] -= 4;
      }
    }

    for (int i = 0; i < enemyCount; i++) {
      if (enemies[i].active) {
        enemies[i].radius -= 0.8;
        if (enemies[i].radius < SAFE_R + 8) {
          enemies[i].active = false;
          health--;
          if (health <= 0) {
            gameOver = true;
            gameOverTime = now;
          }
        }

        for (int j = 0; j < player.shotCount; j++) {
          if (player.shots[j] >= 0) {
            float shotX = CENTER + cos(player.shotAngles[j]) * player.shots[j];
            float shotY = CENTER + sin(player.shotAngles[j]) * player.shots[j];
            float enemyX, enemyY;
            enemyToCart(enemies[i], enemyX, enemyY);

            float dx = shotX - enemyX;
            float dy = shotY - enemyY;
            float dist = sqrt(dx*dx + dy*dy);

            if (dist < 9) {
              enemies[i].active = false;
              player.shots[j] = -1;
              score += 10;
            }
          }
        }
      }
    }

    int activeCount = 0;
    for (int i = 0; i < enemyCount; i++) {
      if (enemies[i].active) activeCount++;
    }
    if (activeCount == 0 && gameTime > 2000) {
      SPAWN_INTERVAL = max(400UL, SPAWN_INTERVAL - 100);
      lastEnemySpawn = now;
    }

    if (now % 60 < 30) {
      canvas.fillSprite(display.color565(8, 8, 10));
    } else {
      canvas.fillSprite(display.color565(10, 10, 12));
    }

    canvas.fillCircle(CENTER, CENTER, PLAY_R + 2, display.color565(14, 14, 16));
    canvas.fillCircle(CENTER, CENTER, PLAY_R, display.color565(12, 12, 14));

    canvas.drawCircle(CENTER, CENTER, SAFE_R, display.color565(50, 100, 150));
    canvas.drawCircle(CENTER, CENTER, SAFE_R - 2, display.color565(40, 80, 120));

    float px, py;
    playerToCart(px, py);
    drawPlayer(px, py);

    for (int i = 0; i < player.shotCount; i++) {
      if (player.shots[i] >= SAFE_R + 2) {
        float sx = CENTER + cos(player.shotAngles[i]) * player.shots[i];
        float sy = CENTER + sin(player.shotAngles[i]) * player.shots[i];
        drawShot(sx, sy);
      }
    }

    for (int i = 0; i < enemyCount; i++) {
      if (enemies[i].active) {
        float ex, ey;
        enemyToCart(enemies[i], ex, ey);
        drawEnemy(ex, ey);
      }
    }

    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200, 200, 200), display.color565(8, 8, 10));
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", score);
    canvas.drawString(scoreBuf, 12, 12);

    canvas.setTextDatum(TR_DATUM);
    char healthBuf[16];
    snprintf(healthBuf, sizeof(healthBuf), "HP: %d", health);
    canvas.drawString(healthBuf, SCREEN_SIZE - 12, 12);

    canvas.pushSprite(0, 0);
    delay(16);
  }

  unsigned long endTime = millis();
  while (millis() - endTime < 2500) {
    canvas.fillSprite(display.color565(10, 10, 12));
    canvas.fillCircle(CENTER, CENTER, PLAY_R + 2, display.color565(14, 14, 16));
    canvas.fillCircle(CENTER, CENTER, PLAY_R, display.color565(12, 12, 14));

    canvas.setTextDatum(MC_DATUM);
    canvas.setTextSize(2);
    canvas.setTextColor(display.color565(255, 100, 100), display.color565(12, 12, 14));
    canvas.drawString("GAME OVER", CENTER, CENTER - 20);

    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200, 200, 200), display.color565(12, 12, 14));
    char finalBuf[32];
    snprintf(finalBuf, sizeof(finalBuf), "Final Score: %d", score);
    canvas.drawString(finalBuf, CENTER, CENTER + 20);

    canvas.pushSprite(0, 0);
    delay(50);
  }
}

void snake() {
  while (a_button_is_pressed()) delay(10);

  const int CENTER = 120;
  const int PLAY_R = 100;
  const int GRID_SIZE = 8;
  const int CELL_SIZE = PLAY_R * 2 / GRID_SIZE;

  struct Segment {
    int x, y;
  };
  Segment snake[64];
  int snakeLen = 3;
  snake[0] = {GRID_SIZE/2, GRID_SIZE/2};
  snake[1] = {GRID_SIZE/2 - 1, GRID_SIZE/2};
  snake[2] = {GRID_SIZE/2 - 2, GRID_SIZE/2};

  int dirX = 1, dirY = 0;
  int nextDirX = 1, nextDirY = 0;
  int foodX = random(0, GRID_SIZE);
  int foodY = random(0, GRID_SIZE);
  
  int score = 0;
  unsigned long lastMove = millis();
  const unsigned long MOVE_INTERVAL = 120;
  bool gameOver = false;
  unsigned long gameOverTime = 0;

  auto gridToScreen = [&](int gx, int gy, float &sx, float &sy) {
    float centerGridX = GRID_SIZE / 2.0 - 0.5;
    float centerGridY = GRID_SIZE / 2.0 - 0.5;
    float localX = gx - centerGridX;
    float localY = gy - centerGridY;
    sx = CENTER + localX * CELL_SIZE;
    sy = CENTER + localY * CELL_SIZE;
  };

  auto isInBounds = [&](float sx, float sy) {
    float dx = sx - CENTER;
    float dy = sy - CENTER;
    return sqrt(dx*dx + dy*dy) <= PLAY_R;
  };

  while (!gameOver) {
    unsigned long now = millis();

    if (button_is_pressed(buttons[5])) {
      nextDirX = 1; nextDirY = 0;
      delay(80);
    }
    if (button_is_pressed(buttons[3])) {
      nextDirX = -1; nextDirY = 0;
      delay(80);
    }
    if (button_is_pressed(buttons[0])) {
      nextDirX = 0; nextDirY = -1;
      delay(80);
    }
    if (button_is_pressed(buttons[5])) {
      nextDirX = 0; nextDirY = 1;
      delay(80);
    }
    if (button_is_pressed(buttons[2], true)) {
      return;
    }

    if (now - lastMove >= MOVE_INTERVAL) {
      lastMove = now;

      if ((nextDirX != -dirX || nextDirY != -dirY)) {
        dirX = nextDirX;
        dirY = nextDirY;
      }

      int newX = snake[0].x + dirX;
      int newY = snake[0].y + dirY;

      for (int i = 0; i < snakeLen; i++) {
        if (snake[i].x == newX && snake[i].y == newY) {
          gameOver = true;
          gameOverTime = now;
          break;
        }
      }

      if (!gameOver) {
        float screenX, screenY;
        gridToScreen(newX, newY, screenX, screenY);
        if (!isInBounds(screenX, screenY)) {
          gameOver = true;
          gameOverTime = now;
        }
      }

      if (!gameOver) {
        for (int i = snakeLen; i > 0; i--) {
          snake[i] = snake[i-1];
        }
        snake[0] = {newX, newY};

        if (snake[0].x == foodX && snake[0].y == foodY) {
          if (snakeLen < 63) {
            snake[snakeLen] = snake[snakeLen-1];
            snakeLen++;
          }
          score += 10;
          foodX = random(0, GRID_SIZE);
          foodY = random(0, GRID_SIZE);
        }
      }
    }

    canvas.fillSprite(display.color565(8, 8, 10));
    canvas.fillCircle(CENTER, CENTER, PLAY_R + 2, display.color565(14, 14, 16));
    canvas.fillCircle(CENTER, CENTER, PLAY_R, display.color565(12, 12, 14));

    for (int i = 1; i < snakeLen; i++) {
      float x1, y1, x2, y2;
      gridToScreen(snake[i-1].x, snake[i-1].y, x1, y1);
      gridToScreen(snake[i].x, snake[i].y, x2, y2);
      canvas.drawLine(x1, y1, x2, y2, display.color565(80, 220, 140));
    }

    for (int i = 0; i < snakeLen; i++) {
      float sx, sy;
      gridToScreen(snake[i].x, snake[i].y, sx, sy);
      if (i == 0) {
        canvas.fillCircle(sx, sy, 5, display.color565(100, 240, 160));
        canvas.drawCircle(sx, sy, 5, display.color565(150, 255, 200));
      } else {
        canvas.fillCircle(sx, sy, 4, display.color565(60, 200, 120));
      }
    }

    float foodSx, foodSy;
    gridToScreen(foodX, foodY, foodSx, foodSy);
    canvas.fillCircle(foodSx, foodSy, 3, display.color565(255, 150, 100));
    canvas.drawCircle(foodSx, foodSy, 3, display.color565(255, 180, 140));

    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200, 200, 200), display.color565(8, 8, 10));
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", score);
    canvas.drawString(scoreBuf, 12, 12);

    canvas.setTextDatum(TR_DATUM);
    char lenBuf[16];
    snprintf(lenBuf, sizeof(lenBuf), "Len: %d", snakeLen);
    canvas.drawString(lenBuf, 228, 12);

    canvas.pushSprite(0, 0);
    delay(16);
  }

  unsigned long endTime = millis();
  while (millis() - endTime < 2500) {
    canvas.fillSprite(display.color565(10, 10, 12));
    canvas.fillCircle(CENTER, CENTER, PLAY_R + 2, display.color565(14, 14, 16));
    canvas.fillCircle(CENTER, CENTER, PLAY_R, display.color565(12, 12, 14));

    canvas.setTextDatum(MC_DATUM);
    canvas.setTextSize(2);
    canvas.setTextColor(display.color565(255, 100, 100), display.color565(12, 12, 14));
    canvas.drawString("GAME OVER", CENTER, CENTER - 20);

    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200, 200, 200), display.color565(12, 12, 14));
    char finalBuf[32];
    snprintf(finalBuf, sizeof(finalBuf), "Final Score: %d", score);
    canvas.drawString(finalBuf, CENTER, CENTER + 20);

    canvas.pushSprite(0, 0);
    delay(50);
  }
}

void geometryDash() {
  while (a_button_is_pressed()) delay(10);

  const int CENTER = 120;
  const int PLAY_R = 100;
  const int LANE_H = 30;
  const int LANES = 3;

  int playerLane = 1;
  float playerY = CENTER + (LANES/2 - playerLane) * LANE_H;
  bool jumping = false;
  float jumpVel = 0;
  const float GRAVITY = 0.5;
  const float JUMP_FORCE = -12;

  struct Obstacle {
    float x;
    int lane;
    bool active;
  };
  Obstacle obstacles[16];
  int obstacleCount = 0;
  for (int i = 0; i < 16; i++) obstacles[i].active = false;

  float scrollX = 0;
  int score = 0;
  unsigned long lastSpawn = millis();
  const unsigned long SPAWN_INTERVAL = 900;
  float speed = 5;
  const float MAX_SPEED = 9;
  bool gameOver = false;
  unsigned long gameOverTime = 0;

  auto isInPlayArea = [&](float x, float y) {
    float dx = x - CENTER;
    float dy = y - CENTER;
    return sqrt(dx*dx + dy*dy) <= PLAY_R;
  };

  auto drawLanes = [&]() {
    for (int l = 0; l < LANES; l++) {
      float laneY = CENTER + (1.5 - l) * LANE_H;
      
      float x1 = CENTER - PLAY_R;
      float x2 = CENTER + PLAY_R;
      float y1 = laneY;
      float y2 = laneY;
      
      if (isInPlayArea(x1, y1) && isInPlayArea(x2, y2)) {
        canvas.drawLine(x1, y1, x2, y2, display.color565(40, 44, 48));
      }
    }
  };

  while (!gameOver) {
    unsigned long now = millis();

    if (button_is_pressed(buttons[3]) || button_is_pressed(buttons[0])) {
      if (playerLane < LANES - 1) {
        playerLane++;
        playerY = CENTER + (1.5 - playerLane) * LANE_H;
      }
      delay(100);
    }
    if (button_is_pressed(buttons[5])) {
      if (playerLane > 0) {
        playerLane--;
        playerY = CENTER + (1.5 - playerLane) * LANE_H;
      }
      delay(100);
    }
    if (button_is_pressed(buttons[4], true)) {
      if (!jumping) {
        jumping = true;
        jumpVel = JUMP_FORCE;
      }
    }
    if (button_is_pressed(buttons[2], true)) {
      return;
    }

    if (jumping) {
      playerY += jumpVel;
      jumpVel += GRAVITY;
      if (playerY >= CENTER + (1.5 - playerLane) * LANE_H) {
        playerY = CENTER + (1.5 - playerLane) * LANE_H;
        jumping = false;
        jumpVel = 0;
      }
    }

    scrollX += speed;
    if (now - lastSpawn > SPAWN_INTERVAL) {
      for (int i = 0; i < 16; i++) {
        if (!obstacles[i].active) {
          obstacles[i].x = CENTER + PLAY_R + 20;
          obstacles[i].lane = random(0, LANES);
          obstacles[i].active = true;
          break;
        }
      }
      lastSpawn = now;
      if (SPAWN_INTERVAL > 500) SPAWN_INTERVAL -= 30;
      if (speed < MAX_SPEED) speed += 0.15;
    }

    for (int i = 0; i < 16; i++) {
      if (obstacles[i].active) {
        obstacles[i].x -= speed;
        if (obstacles[i].x < CENTER - PLAY_R - 20) {
          obstacles[i].active = false;
          score += 10;
        }

        float obsY = CENTER + (1.5 - obstacles[i].lane) * LANE_H;
        float dx = obstacles[i].x - CENTER;
        float dy = obsY - playerY;

        if (fabs(dx) < 15 && fabs(dy) < 20 && obstacles[i].lane == playerLane) {
          gameOver = true;
          gameOverTime = now;
        }
      }
    }

    canvas.fillSprite(display.color565(8, 8, 10));
    canvas.fillCircle(CENTER, CENTER, PLAY_R + 2, display.color565(14, 14, 16));
    canvas.fillCircle(CENTER, CENTER, PLAY_R, display.color565(12, 12, 14));

    drawLanes();

    for (int i = 0; i < 16; i++) {
      if (obstacles[i].active) {
        float obsY = CENTER + (1.5 - obstacles[i].lane) * LANE_H;
        if (isInPlayArea(obstacles[i].x, obsY)) {
          canvas.fillRect(obstacles[i].x - 8, obsY - 8, 16, 16, display.color565(255, 100, 100));
          canvas.drawRect(obstacles[i].x - 8, obsY - 8, 16, 16, display.color565(255, 150, 120));
        }
      }
    }

    if (isInPlayArea(CENTER - 30, playerY)) {
      canvas.fillCircle(CENTER - 30, playerY, 6, display.color565(100, 200, 255));
      canvas.drawCircle(CENTER - 30, playerY, 6, display.color565(150, 230, 255));
    }

    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200, 200, 200), display.color565(8, 8, 10));
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", score);
    canvas.drawString(scoreBuf, 12, 12);

    canvas.setTextDatum(TR_DATUM);
    char speedBuf[16];
    snprintf(speedBuf, sizeof(speedBuf), "Spd: %.1f", speed);
    canvas.drawString(speedBuf, 228, 12);

    canvas.pushSprite(0, 0);
    delay(16);
  }

  unsigned long endTime = millis();
  while (millis() - endTime < 2500) {
    canvas.fillSprite(display.color565(10, 10, 12));
    canvas.fillCircle(CENTER, CENTER, PLAY_R + 2, display.color565(14, 14, 16));
    canvas.fillCircle(CENTER, CENTER, PLAY_R, display.color565(12, 12, 14));

    canvas.setTextDatum(MC_DATUM);
    canvas.setTextSize(2);
    canvas.setTextColor(display.color565(255, 100, 100), display.color565(12, 12, 14));
    canvas.drawString("GAME OVER", CENTER, CENTER - 20);

    canvas.setTextSize(1);
    canvas.setTextColor(display.color565(200, 200, 200), display.color565(12, 12, 14));
    char finalBuf[32];
    snprintf(finalBuf, sizeof(finalBuf), "Final Score: %d", score);
    canvas.drawString(finalBuf, CENTER, CENTER + 20);

    canvas.pushSprite(0, 0);
    delay(50);
  }
}