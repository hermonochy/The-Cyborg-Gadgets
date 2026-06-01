extern Adafruit_GC9A01A display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern uint16_t colourBG, colourText, colour1, colour2, colour3, colour4, colour5, colour6;

void snake() {
  const int gridR = 5;  // radius in grid points
  const int gridCount = 2 * gridR + 1;
  const int cellR = 14; // cell "radius" in pixels (visual size)
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

  display.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, colourBG);
  // Draw all cells bg
  for(int i=0;i<numValid;++i) {
    int xp = SCREEN_WIDTH/2 + valid[i].x * 2 * cellR;
    int yp = SCREEN_HEIGHT/2 + valid[i].y * 2 * cellR;
    display.fillCircle(xp, yp, cellR - 1, colour3);
  }
  for(int i=0;i<snklen;++i) {
    int xp = SCREEN_WIDTH/2 + snake[i].x * 2 * cellR;
    int yp = SCREEN_HEIGHT/2 + snake[i].y * 2 * cellR;
    display.fillCircle(xp, yp, cellR - 2, colour2);
  }
  // Place food
  while (1) {
    food = valid[random(0, numValid)];
    bool bad=0;
    for(int i=0;i<snklen;++i) if(snake[i].x==food.x && snake[i].y==food.y) bad=1;
    if(!bad) break;
  }
  display.fillCircle(SCREEN_WIDTH/2 + food.x * 2*cellR, SCREEN_HEIGHT/2 + food.y * 2*cellR, cellR - 2, colour1);

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
        display.fillCircle(SCREEN_WIDTH/2 + food.x * 2*cellR, SCREEN_HEIGHT/2 + food.y * 2*cellR, cellR-2, colour1);
      } else {
        // erase tail
        int xp = SCREEN_WIDTH/2 + snake[snklen].x * 2 * cellR;
        int yp = SCREEN_HEIGHT/2 + snake[snklen].y * 2 * cellR;
        display.fillCircle(xp, yp, cellR-1, colour3);
      }
      // draw new head
      int xp = SCREEN_WIDTH/2 + snake[0].x * 2 * cellR;
      int yp = SCREEN_HEIGHT/2 + snake[0].y * 2 * cellR;
      display.fillCircle(xp, yp, cellR-2, colour2);
    }
    delay(20);
  }
  display.fillScreen(colourBG);
  display.setTextColor(colour2);
  display.setTextSize(3);
  display.setCursor(30, SCREEN_HEIGHT/2-25);
  display.print("GAME OVER");
  delay(1550);
}