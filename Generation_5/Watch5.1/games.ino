extern TFT_eSPI display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG, colourText, colour1, colour2, colour3, colour4, colour5, colour6;

void snake() {
  const int gridR = 5;
  const int gridCount = 2 * gridR + 1;
  const int cellR = 14;
  struct cell { int x, y; } valid[gridCount * gridCount];
  int numValid = 0;
  for (int y = -gridR; y <= gridR; ++y)
    for (int x = -gridR; x <= gridR; ++x)
      if (x * x + y * y <= gridR * gridR) valid[numValid++] = { x, y };
  auto idx = [&](cell c) -> int {
    for(int i=0;i<numValid;++i)if(valid[i].x==c.x&&valid[i].y==c.y)return i;
    return -1;
  };

  cell snake[75], food;
  int snklen = 4, dx = 1, dy = 0;
  snake[0] = { 0, 0 };
  for(int i=1;i<snklen;++i) snake[i]={-i,0};

  display.fillCircle(120, 120, 120, colourBG);
  for(int i=0;i<numValid;++i) {
    int xp = 120 + valid[i].x * 2 * cellR;
    int yp = 120 + valid[i].y * 2 * cellR;
    display.fillCircle(xp, yp, cellR - 1, colour3);
  }
  for(int i=0;i<snklen;++i) {
    int xp = 120 + snake[i].x * 2 * cellR;
    int yp = 120 + snake[i].y * 2 * cellR;
    display.fillCircle(xp, yp, cellR - 2, colour2);
  }
  while (1) {
    food = valid[random(0, numValid)];
    bool bad=0;
    for(int i=0;i<snklen;++i) if(snake[i].x==food.x && snake[i].y==food.y) bad=1;
    if(!bad) break;
  }
  display.fillCircle(120 + food.x * 2*cellR, 120 + food.y * 2*cellR, cellR - 2, colour1);

  bool alive = true;
  unsigned long lastMove = millis();
  while (alive) {
    if (button_is_pressed(btn1, true) && dx!=1)  { dx=-1; dy= 0; }
    if (button_is_pressed(btn3, true) && dx!=-1){ dx= 1; dy= 0; }
    if (button_is_pressed(btn2, true) && dy!=1)  { dx= 0; dy=-1; }
    if (button_is_pressed(btn6, true) && dy!=-1) { dx= 0; dy= 1; }
    if (button_is_pressed(btn5, true)) break;

    if (millis() - lastMove > 275) {
      lastMove = millis();
      cell next = { snake[0].x+dx, snake[0].y+dy };
      if (idx(next)==-1) { alive=0; break; }
      for(int i=0;i<snklen;++i) if(snake[i].x==next.x && snake[i].y==next.y) alive=0;
      for(int i=snklen;i>0;--i) snake[i]=snake[i-1];
      snake[0]=next;
      if (snake[0].x==food.x && snake[0].y==food.y) {
        ++snklen;
        if(snklen>74)snklen=74;
        while (1) {
          food = valid[random(0, numValid)];
          bool bad=0;
          for(int i=0;i<snklen;++i) if(snake[i].x==food.x && snake[i].y==food.y) bad=1;
          if(!bad) break;
        }
        display.fillCircle(120 + food.x * 2*cellR, 120 + food.y * 2*cellR, cellR-2, colour1);
      } else {
        int xp = 120 + snake[snklen].x * 2 * cellR;
        int yp = 120 + snake[snklen].y * 2 * cellR;
        display.fillCircle(xp, yp, cellR-1, colour3);
      }
      int xp = 120 + snake[0].x * 2 * cellR;
      int yp = 120 + snake[0].y * 2 * cellR;
      display.fillCircle(xp, yp, cellR-2, colour2);
    }
    delay(20);
  }
  display.fillScreen(colourBG);
  display.setTextSize(3);
  display.setTextColor(colour2);
  display.drawString("GAME OVER", 120 - 54, 95, 2);
  display.setTextSize(1);
  display.setTextColor(colour4);
  display.drawString("Score: " + String(snklen - 4), 120 - 30, 135, 1);
  delay(1550);
}

void geometryDash() {
  int playerY = 100, playerVel = 0;
  const int playerX = 40;
  const int playerSize = 8;
  const int gravity = 1, jumpPower = -12;
  int obstacleX = 200, obstacleY = 108;
  const int obstacleW = 12, obstacleH = 16;
  int score = 0, gameSpeed = 4;
  bool alive = true;
  unsigned long lastFrame = millis();

  display.fillCircle(120, 120, 120, colourBG);

  while (alive) {
    unsigned long now = millis();
    if (now - lastFrame < 30) { delay(5); continue; }
    lastFrame = now;

    if (button_is_pressed(btn3, true) && playerY >= 100) {
      playerVel = jumpPower;
    }
    if (button_is_pressed(btn5, true)) break;

    playerVel += gravity;
    if (playerVel > 8) playerVel = 8;
    playerY += playerVel;
    if (playerY >= 100) { playerY = 100; playerVel = 0; }
    if (playerY < 10) playerY = 10;

    obstacleX -= gameSpeed;
    if (obstacleX < -20) {
      obstacleX = 220;
      score += 10;
      gameSpeed = min(8, 4 + score / 100);
    }

    if (playerX + playerSize > obstacleX && playerX < obstacleX + obstacleW &&
        playerY + playerSize > obstacleY && playerY < obstacleY + obstacleH) {
      alive = false;
    }

    display.fillRect(0, 0, 240, 240, colourBG);
    display.fillRect(playerX - playerSize, playerY - playerSize, playerSize * 2, playerSize * 2, colour2);
    display.fillRect(obstacleX, obstacleY, obstacleW, obstacleH, colour1);
    display.setTextSize(1);
    display.setTextColor(colour4);
    display.drawString("Score: " + String(score), 10, 10, 1);
    display.drawLine(0, 108, 240, 108, colour3);

    delay(30);
  }

  display.fillScreen(colourBG);
  display.setTextSize(3);
  display.setTextColor(colour2);
  display.drawString("GAME OVER", 120 - 54, 80, 2);
  display.setTextSize(1);
  display.setTextColor(colour4);
  display.drawString("Score: " + String(score), 120 - 30, 130, 1);
  delay(1800);
}

void flightSimulator() {
  float pitch = 0, roll = 0, yaw = 0;
  float altitude = 5000, speed = 150;
  float throttle = 0.5;
  int heading = 0;
  
  int oldPitch = -999, oldRoll = -999, oldYaw = -999;
  int oldAlt = -999, oldSpd = -999, oldThrottle = -999, oldHeading = -999;
  
  display.fillCircle(120, 120, 120, colourBG);
  
  auto drawHorizon = [&]() {
    int cx = 120, cy = 100;
    int horizonY = cy + (int)(pitch * 0.5);
    
    display.fillRect(30, 60, 180, 80, colourBG);
    
    uint16_t skyCol = display.color565(50, 100, 200);
    uint16_t groundCol = display.color565(100, 80, 50);
    
    for (int x = 30; x < 210; x++) {
      int y = horizonY + (x - 120) * tan(roll * 0.01745);
      for (int py = 60; py < 140; py++) {
        if (py < y) {
          if ((py - 60 + x) % 20 < 10) display.drawPixel(x, py, skyCol);
        } else {
          if ((py - 60 + x) % 20 < 10) display.drawPixel(x, py, groundCol);
        }
      }
    }
    
    display.drawLine(60, horizonY, 180, horizonY, colour2);
    display.drawCircle(cx, cy, 30, colour3);
  };
  
  auto drawAltitude = [&]() {
    display.fillRect(10, 70, 40, 60, colourBG);
    display.setTextSize(1);
    display.setTextColor(colourText);
    display.drawString("ALT", 12, 75, 1);
    display.drawString(String((int)altitude), 12, 90, 1);
    display.drawString("ft", 12, 102, 1);
  };
  
  auto drawSpeed = [&]() {
    display.fillRect(190, 70, 50, 60, colourBG);
    display.setTextSize(1);
    display.setTextColor(colourText);
    display.drawString("SPD", 192, 75, 1);
    display.drawString(String((int)speed), 192, 90, 1);
    display.drawString("kt", 192, 102, 1);
  };
  
  auto drawHeading = [&]() {
    display.fillRect(80, 20, 80, 24, colourBG);
    display.setTextSize(1);
    display.setTextColor(colour4);
    String hdg = "HDG:" + String(heading, 3);
    display.drawString(hdg, 100, 22, 1);
  };
  
  auto drawThrottle = [&]() {
    display.fillRect(10, 200, 220, 24, colourBG);
    display.setTextSize(1);
    display.setTextColor(colour2);
    display.drawString("THR", 12, 202, 1);
    int thrW = (int)(throttle * 60);
    display.fillRect(50, 204, thrW, 12, colour3);
    display.drawRect(50, 204, 60, 12, colour4);
    display.drawString(String((int)(throttle * 100)) + "%", 115, 202, 1);
  };
  
  auto drawAttitude = [&]() {
    display.fillRect(70, 155, 100, 30, colourBG);
    display.setTextSize(1);
    display.setTextColor(colour2);
    display.drawString("P:" + String(pitch, 1), 75, 157, 1);
    display.drawString("R:" + String(roll, 1), 75, 168, 1);
  };
  
  drawHorizon();
  drawAltitude();
  drawSpeed();
  drawHeading();
  drawThrottle();
  drawAttitude();
  
  while (true) {
    if (button_is_pressed(btn1, true)) {
      pitch = constrain(pitch + 5, -90, 90);
    } else if (button_is_pressed(btn2, true)) {
      pitch = constrain(pitch - 5, -90, 90);
    } else if (button_is_pressed(btn3, true)) {
      roll = constrain(roll + 5, -180, 180);
    } else if (button_is_pressed(btn4, true)) {
      roll = constrain(roll - 5, -180, 180);
    } else if (button_is_pressed(btn5, true)) {
      throttle = constrain(throttle + 0.1, 0, 1);
    } else if (button_is_pressed(btn6, true)) {
      break;
    }
    
    heading = (heading + (int)(yaw * 0.5)) % 360;
    if (heading < 0) heading += 360;
    
    altitude += (pitch * speed) * 0.01;
    altitude = constrain(altitude, 0, 35000);
    
    speed += (throttle - 0.3) * 0.5;
    speed = constrain(speed, 50, 350);
    
    yaw += roll * 0.01;
    pitch *= 0.95;
    roll *= 0.93;
    
    if (oldPitch != (int)pitch || oldRoll != (int)roll) {
      drawHorizon();
      oldPitch = (int)pitch;
      oldRoll = (int)roll;
    }
    
    if (oldAlt != (int)altitude) {
      drawAltitude();
      oldAlt = (int)altitude;
    }
    
    if (oldSpd != (int)speed) {
      drawSpeed();
      oldSpd = (int)speed;
    }
    
    if (oldHeading != heading) {
      drawHeading();
      oldHeading = heading;
    }
    
    if (oldThrottle != (int)(throttle * 100)) {
      drawThrottle();
      oldThrottle = (int)(throttle * 100);
    }
    
    if (oldPitch != (int)pitch || oldRoll != (int)roll) {
      drawAttitude();
    }
    
    delay(50);
  }
}

void games() {
  int sel = 0, oldSel = -1;
  const char* gameNames[] = {"Snake", "Geometry Dash", "Flight Sim"};
  const int gameCount = 3;

  while (true) {
    if (sel != oldSel) {
      display.fillCircle(120, 120, 120, colourBG);
      display.drawCircle(120, 120, 120, display.color565(40, 40, 40));
      
      display.setTextSize(2);
      display.setTextColor(colour6);
      display.drawString("GAMES", 120 - 30, 24, 2);

      int Y = 70, spacing = 35;
      for (int i = 0; i < gameCount; i++) {
        if (i == sel) {
          display.fillRoundRect(30, Y + i * spacing - 4, 180, 26, 6, colour6);
          display.setTextSize(2);
          display.setTextColor(colourBG);
        } else {
          display.setTextSize(1);
          display.setTextColor(colour1);
        }
        display.drawString(gameNames[i], 40, Y + i * spacing, 1);
      }
      
      display.setTextSize(1);
      display.setTextColor(colour4);
      display.drawString("3:Play  6:Back", 50, 200, 1);
      
      oldSel = sel;
    }

    if (button_is_pressed(btn2, true)) { sel = (sel + 1) % gameCount; }
    else if (button_is_pressed(btn1, true)) { sel = (sel + gameCount - 1) % gameCount; }
    else if (button_is_pressed(btn3, true)) {
      display.fillScreen(colourBG);
      if (sel == 0) snake();
      else if (sel == 1) geometryDash();
      else if (sel == 2) flightSimulator();
      oldSel = -1;
    }
    else if (button_is_pressed(btn6, true)) return;
    delay(40);
  }
}