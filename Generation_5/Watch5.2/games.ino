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
  const unsigned long SPAWN_INTERVAL = 800;

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

