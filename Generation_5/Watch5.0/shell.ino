// Interactive Shell for Watch 5.0

extern Adafruit_SSD1306 display;
extern bool button_is_pressed(int btnVal, bool onlyOnce);
extern int btn1, btn2, btn3, btn4, btn5, btn6;
extern byte Func1, Func2, Func3;
extern Preferences preferences;

#define MAX_COMMAND_LENGTH 512
#define MAX_COMMAND_HISTORY 20
#define MAX_VARIABLES 50
#define MAX_LOOPS 10

struct Variable {
  char name[32];
  int value;
  bool used;
};

struct LoopStack {
  int count;
  int remaining;
  int startLine;
};

Variable variables[MAX_VARIABLES];
char commandHistory[MAX_COMMAND_HISTORY][MAX_COMMAND_LENGTH];
int historyIndex = 0;
int historyCount = 0;
LoopStack loopStack[MAX_LOOPS];
int loopStackPtr = 0;


bool inputString(const char* label, char* buffer, int maxLen) {
  int cursorPos = 0;
  buffer[0] = '\0';
  
  char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_+-*/=0123456789 ";
  int charsetSize = strlen(charset);
  int charIndex = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(label);
    
    display.setCursor(0, 20);
    display.print("> ");
    display.print(buffer);
    if (cursorPos == strlen(buffer)) {
      display.print("_");
    }
    
    display.setCursor(0, 30);
    display.print("Char: ");
    display.setTextSize(2);
    display.setCursor(40, 28);
    display.print(charset[charIndex]);
    
    display.display();
    
    if (button_is_pressed(btn1)) {
      charIndex = (charIndex - 1 + charsetSize) % charsetSize;
      delay(100);
    }
    else if (button_is_pressed(btn2)) {
      charIndex = (charIndex + 1) % charsetSize;
      delay(100);
    }
    else if (button_is_pressed(btn3)) {
      if (strlen(buffer) < maxLen - 1) {
        buffer[strlen(buffer)] = charset[charIndex];
        buffer[strlen(buffer) + 1] = '\0';
      }
      delay(150);
    }
    else if (button_is_pressed(btn4)) {
      if (strlen(buffer) > 0) {
        buffer[strlen(buffer) - 1] = '\0';
      }
      delay(150);
    }
    else if (button_is_pressed(btn5)) {
      buffer[0] = '\0';
      delay(150);
    }
    else if (button_is_pressed(btn6)) {
      if (strlen(buffer) > 0) {
        return true;
      } else {
        return false;
      }
    }
    delay(30);
  }
}


void initializeShell() {
  for (int i = 0; i < MAX_VARIABLES; i++) {
    variables[i].used = false;
  }
  for (int i = 0; i < MAX_COMMAND_HISTORY; i++) {
    commandHistory[i][0] = '\0';
  }
  loopStackPtr = 0;
}

void shell(void) {
  initializeShell();
  char inputBuffer[MAX_COMMAND_LENGTH] = "";
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("SHELL 5.0");
    display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    display.print("Command:");
    display.setCursor(0, 35);
    
    int dispLen = strlen(inputBuffer);
    if (dispLen > 20) {
      display.print(&inputBuffer[dispLen - 20]);
    } else {
      display.print(inputBuffer);
    }
    
    display.setCursor(0, 55);
    display.setTextSize(1);
    display.print("2:Input 3:Run 6:Exit");
    
    display.display();
    
    if (button_is_pressed(btn2)) {
      if (inputString("Enter cmd:", inputBuffer, MAX_COMMAND_LENGTH)) {
        executeCommand(inputBuffer);
        addToHistory(inputBuffer);
        inputBuffer[0] = '\0';
      }
      delay(200);
    }
    else if (button_is_pressed(btn3)) {
      if (strlen(inputBuffer) > 0) {
        executeCommand(inputBuffer);
        addToHistory(inputBuffer);
        inputBuffer[0] = '\0';
      }
      delay(200);
    }
    else if (button_is_pressed(btn5)) {
      if (historyCount > 0) {
        historyIndex = (historyIndex - 1 + historyCount) % historyCount;
        strncpy(inputBuffer, commandHistory[historyIndex], MAX_COMMAND_LENGTH - 1);
      }
      delay(150);
    }
    else if (button_is_pressed(btn6)) {
      return;
    }
    
    delay(50);
  }
}

void addToHistory(const char* command) {
  strncpy(commandHistory[historyIndex], command, MAX_COMMAND_LENGTH - 1);
  historyIndex = (historyIndex + 1) % MAX_COMMAND_HISTORY;
  if (historyCount < MAX_COMMAND_HISTORY) {
    historyCount++;
  }
}

int getVariableValue(const char* name) {
  for (int i = 0; i < MAX_VARIABLES; i++) {
    if (variables[i].used && strcmp(variables[i].name, name) == 0) {
      return variables[i].value;
    }
  }
  return 0;
}

void setVariableValue(const char* name, int value) {
  bool found = false;
  for (int i = 0; i < MAX_VARIABLES; i++) {
    if (variables[i].used && strcmp(variables[i].name, name) == 0) {
      variables[i].value = value;
      found = true;
      break;
    }
  }
  
  if (!found) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
      if (!variables[i].used) {
        strncpy(variables[i].name, name, 31);
        variables[i].value = value;
        variables[i].used = true;
        break;
      }
    }
  }
}

int evaluateExpression(const char* expr) {
  int result = 0;
  int i = 0;
  
  char token[64] = "";
  int tokenPos = 0;
  
  while (expr[i] && expr[i] != '+' && expr[i] != '-' && expr[i] != '*' && expr[i] != '/') {
    if (expr[i] != ' ') {
      token[tokenPos++] = expr[i];
    }
    i++;
  }
  token[tokenPos] = '\0';
  
  if (isalpha(token[0])) {
    result = getVariableValue(token);
  } else {
    result = atoi(token);
  }
  
  while (expr[i]) {
    if (expr[i] == ' ') {
      i++;
      continue;
    }
    
    char op = expr[i];
    i++;
    
    tokenPos = 0;
    while (expr[i] && expr[i] != '+' && expr[i] != '-' && expr[i] != '*' && expr[i] != '/') {
      if (expr[i] != ' ') {
        token[tokenPos++] = expr[i];
      }
      i++;
    }
    token[tokenPos] = '\0';
    
    int val = 0;
    if (isalpha(token[0])) {
      val = getVariableValue(token);
    } else {
      val = atoi(token);
    }
    
    if (op == '+') result += val;
    else if (op == '-') result -= val;
    else if (op == '*') result *= val;
    else if (op == '/') result = (val != 0) ? result / val : 0;
  }
  
  return result;
}

void executeCommand(const char* command) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Executing...");
  display.display();
  delay(300);
  
  char cmd[64] = "";
  char args[MAX_COMMAND_LENGTH] = "";
  
  int spacePos = -1;
  for (int i = 0; i < strlen(command); i++) {
    if (command[i] == ' ') {
      spacePos = i;
      break;
    }
  }
  
  if (spacePos == -1) {
    strncpy(cmd, command, 63);
    cmd[63] = '\0';
  } else {
    strncpy(cmd, command, spacePos);
    cmd[spacePos] = '\0';
    strncpy(args, command + spacePos + 1, MAX_COMMAND_LENGTH - 1);
  }
  
  if (strcmp(cmd, "digitalWrite") == 0 || strcmp(cmd, "dw") == 0) {
    cmdDigitalWrite(args);
  }
  else if (strcmp(cmd, "digitalRead") == 0 || strcmp(cmd, "dr") == 0) {
    cmdDigitalRead(args);
  }
  else if (strcmp(cmd, "analogWrite") == 0 || strcmp(cmd, "aw") == 0) {
    cmdAnalogWrite(args);
  }
  else if (strcmp(cmd, "analogRead") == 0 || strcmp(cmd, "ar") == 0) {
    cmdAnalogRead(args);
  }
  else if (strcmp(cmd, "pinMode") == 0 || strcmp(cmd, "pm") == 0) {
    cmdPinMode(args);
  }
  else if (strcmp(cmd, "var") == 0) {
    cmdVariable(args);
  }
  else if (strcmp(cmd, "let") == 0) {
    cmdLet(args);
  }
  else if (strcmp(cmd, "inc") == 0) {
    cmdIncrement(args);
  }
  else if (strcmp(cmd, "dec") == 0) {
    cmdDecrement(args);
  }
  else if (strcmp(cmd, "vars") == 0) {
    showVariables();
  }
  else if (strcmp(cmd, "clear") == 0) {
    clearVariables();
  }
  else if (strcmp(cmd, "blink") == 0) {
    cmdBlink(args);
  }
  else if (strcmp(cmd, "ptn") == 0) {
    cmdPattern(args);
  }
  else if (strcmp(cmd, "pulse") == 0) {
    cmdPulse(args);
  }
  else if (strcmp(cmd, "delay") == 0) {
    cmdDelay(args);
  }
  else if (strcmp(cmd, "millis") == 0) {
    cmdMillis(args);
  }
  else if (strcmp(cmd, "calc") == 0) {
    cmdCalc(args);
  }
  else if (strcmp(cmd, "random") == 0) {
    cmdRandomNum(args);
  }
  else if (strcmp(cmd, "print") == 0) {
    cmdPrint(args);
  }
  else if (strcmp(cmd, "cls") == 0) {
    display.clearDisplay();
    display.display();
    delay(500);
  }
  else if (strcmp(cmd, "save") == 0) {
    cmdSave(args);
  }
  else if (strcmp(cmd, "load") == 0) {
    cmdLoad(args);
  }
  else if (strcmp(cmd, "for") == 0) {
    cmdFor(args);
  }
  else if (strcmp(cmd, "while") == 0) {
    cmdWhile(args);
  }
  else if (strcmp(cmd, "if") == 0) {
    cmdIf(args);
  }
  else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
    showHelp1();
  }
  else if (strcmp(cmd, "help2") == 0 || strcmp(cmd, "h2") == 0) {
    showHelp2();
  }
  else if (strcmp(cmd, "help3") == 0 || strcmp(cmd, "h3") == 0) {
    showHelp3();
  }
  else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Unknown: ");
    display.println(cmd);
    display.setCursor(0, 20);
    display.print("Type 'help' for");
    display.setCursor(0, 30);
    display.print("command list");
    display.display();
    delay(2000);
  }
}

void cmdDigitalWrite(const char* args) {
  int pin = 0, state = 0;
  if (sscanf(args, "%d %d", &pin, &state) == 2) {
    digitalWrite(pin, state ? HIGH : LOW);
    showSuccessStr("digitalWrite", state ? "HIGH" : "LOW");
  } else {
    showError("dw pin state");
  }
}

void cmdDigitalRead(const char* args) {
  int pin = 0;
  if (sscanf(args, "%d", &pin) == 1) {
    int result = digitalRead(pin);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("digitalRead(");
    display.print(pin);
    display.println(")");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(result);
    display.display();
    delay(2000);
  } else {
    showError("dr pin");
  }
}

void cmdAnalogWrite(const char* args) {
  int pin = 0, value = 0;
  if (sscanf(args, "%d %d", &pin, &value) == 2) {
    analogWrite(pin, constrain(value, 0, 255));
    showSuccess("analogWrite", value);
  } else {
    showError("aw pin value");
  }
}

void cmdAnalogRead(const char* args) {
  int pin = 0;
  if (sscanf(args, "%d", &pin) == 1) {
    int result = analogRead(pin);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("analogRead(");
    display.print(pin);
    display.println(")");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(result);
    display.display();
    delay(2000);
  } else {
    showError("ar pin");
  }
}

void cmdPinMode(const char* args) {
  int pin = 0;
  char mode[16] = "";
  if (sscanf(args, "%d %s", &pin, mode) == 2) {
    if (strcmp(mode, "INPUT") == 0) {
      pinMode(pin, INPUT);
    } else if (strcmp(mode, "OUTPUT") == 0) {
      pinMode(pin, OUTPUT);
    } else if (strcmp(mode, "INPUT_PULLUP") == 0) {
      pinMode(pin, INPUT_PULLUP);
    }
    showSuccessStr("pinMode", mode);
  } else {
    showError("pm pin INPUT/OUTPUT");
  }
}

void cmdVariable(const char* args) {
  char name[32] = "";
  int value = 0;
  if (sscanf(args, "%s %d", name, &value) == 2) {
    setVariableValue(name, value);
    showSuccess("var", value);
  } else {
    showError("var name value");
  }
}

void cmdLet(const char* args) {
  char name[32] = "";
  char expr[256] = "";
  
  int eqPos = -1;
  for (int i = 0; i < strlen(args); i++) {
    if (args[i] == '=') {
      eqPos = i;
      break;
    }
  }
  
  if (eqPos != -1) {
    int pos = 0;
    for (int i = 0; i < eqPos; i++) {
      if (args[i] != ' ') {
        name[pos++] = args[i];
      }
    }
    name[pos] = '\0';
    
    strncpy(expr, args + eqPos + 1, 255);
    int result = evaluateExpression(expr);
    setVariableValue(name, result);
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(name);
    display.print(" = ");
    display.println(result);
    display.display();
    delay(1500);
  } else {
    showError("let var = expr");
  }
}

void cmdIncrement(const char* args) {
  char name[32] = "";
  if (sscanf(args, "%s", name) == 1) {
    int val = getVariableValue(name);
    setVariableValue(name, val + 1);
    showSuccess("inc", val + 1);
  } else {
    showError("inc var");
  }
}

void cmdDecrement(const char* args) {
  char name[32] = "";
  if (sscanf(args, "%s", name) == 1) {
    int val = getVariableValue(name);
    setVariableValue(name, val - 1);
    showSuccess("dec", val - 1);
  } else {
    showError("dec var");
  }
}

void cmdBlink(const char* args) {
  int pin = 0, count = 0, delayMs = 500;
  if (sscanf(args, "%d %d %d", &pin, &count, &delayMs) >= 2) {
    for (int i = 0; i < count; i++) {
      digitalWrite(pin, HIGH);
      delay(delayMs / 2);
      digitalWrite(pin, LOW);
      delay(delayMs / 2);
      if (button_is_pressed(btn6)) break;
    }
    showSuccess("Blink", count);
  } else {
    showError("blink pin count [delay]");
  }
}

void cmdPattern(const char* args) {
  int pin = 0;
  char sequence[128] = "";
  int delayMs = 200;
  
  if (sscanf(args, "%d %s %d", &pin, sequence, &delayMs) >= 2) {
    for (int i = 0; i < strlen(sequence); i++) {
      digitalWrite(pin, (sequence[i] == '1') ? HIGH : LOW);
      delay(delayMs);
      if (button_is_pressed(btn6)) break;
    }
    digitalWrite(pin, LOW);
    showSuccess("Pattern", strlen(sequence));
  } else {
    showError("ptn pin seq [delay]");
  }
}

void cmdPulse(const char* args) {
  int pin = 0, duration = 1000;
  if (sscanf(args, "%d %d", &pin, &duration) >= 1) {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    showSuccess("Pulse", duration);
  } else {
    showError("pulse pin [duration]");
  }
}

void cmdDelay(const char* args) {
  int ms = 0;
  if (sscanf(args, "%d", &ms) == 1) {
    delay(ms);
    showSuccess("Delayed", ms);
  } else {
    showError("delay ms");
  }
}

void cmdMillis(const char* args) {
  unsigned long now = millis();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("millis()");
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(now);
  display.display();
  delay(2000);
}

void cmdCalc(const char* args) {
  int result = evaluateExpression(args);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Result:");
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(result);
  display.display();
  delay(2000);
}

void cmdRandomNum(const char* args) {
  int min = 0, max = 100;
  if (sscanf(args, "%d %d", &min, &max) >= 1) {
    int result = random(min, max + 1);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("random(");
    display.print(min);
    display.print(",");
    display.print(max);
    display.println(")");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(result);
    display.display();
    delay(2000);
  } else {
    showError("random [min max]");
  }
}

void cmdPrint(const char* args) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(">> ");
  display.println(args);
  display.display();
  
  while (true) {
    if (button_is_pressed(btn6)) break;
    delay(50);
  }
}

void cmdSave(const char* args) {
  char key[32] = "";
  if (sscanf(args, "%s", key) == 1) {
    preferences.begin("shell", false);
    for (int i = 0; i < MAX_VARIABLES; i++) {
      if (variables[i].used) {
        String varKey = String(key) + "_" + String(i) + "_n";
        String varVal = String(key) + "_" + String(i) + "_v";
        preferences.putString(varKey.c_str(), variables[i].name);
        preferences.putInt(varVal.c_str(), variables[i].value);
      }
    }
    preferences.end();
    showSuccessStr("Saved", key);
  } else {
    showError("save key");
  }
}

void cmdLoad(const char* args) {
  char key[32] = "";
  if (sscanf(args, "%s", key) == 1) {
    preferences.begin("shell", true);
    for (int i = 0; i < MAX_VARIABLES; i++) {
      String varKey = String(key) + "_" + String(i) + "_n";
      String varVal = String(key) + "_" + String(i) + "_v";
      String name = preferences.getString(varKey.c_str(), "");
      if (name.length() > 0) {
        strncpy(variables[i].name, name.c_str(), 31);
        variables[i].value = preferences.getInt(varVal.c_str(), 0);
        variables[i].used = true;
      }
    }
    preferences.end();
    showSuccessStr("Loaded", key);
  } else {
    showError("load key");
  }
}

bool evaluateCondition(const char* condition) {
  char left[64] = "";
  char right[64] = "";
  char op[4] = "";
  int opPos = -1;
  
  for (int i = 0; i < strlen(condition); i++) {
    if (condition[i] == '<' || condition[i] == '>' || condition[i] == '=' || condition[i] == '!') {
      opPos = i;
      break;
    }
  }
  
  if (opPos == -1) {
    showError("Invalid condition");
    return false;
  }
  
  int pos = 0;
  for (int i = 0; i < opPos; i++) {
    if (condition[i] != ' ') {
      left[pos++] = condition[i];
    }
  }
  left[pos] = '\0';
  
  pos = 0;
  int i = opPos;
  while (i < strlen(condition) && (condition[i] == '<' || condition[i] == '>' || condition[i] == '=' || condition[i] == '!')) {
    op[pos++] = condition[i];
    i++;
  }
  op[pos] = '\0';
  
  pos = 0;
  while (i < strlen(condition) && condition[i] == ' ') {
    i++;
  }
  while (i < strlen(condition)) {
    if (condition[i] != ' ') {
      right[pos++] = condition[i];
    }
    i++;
  }
  right[pos] = '\0';
  
  int leftVal = 0;
  if (isalpha(left[0])) {
    leftVal = getVariableValue(left);
  } else {
    leftVal = atoi(left);
  }
  
  int rightVal = 0;
  if (isalpha(right[0])) {
    rightVal = getVariableValue(right);
  } else {
    rightVal = atoi(right);
  }
  
  if (strcmp(op, "<") == 0) return leftVal < rightVal;
  else if (strcmp(op, ">") == 0) return leftVal > rightVal;
  else if (strcmp(op, "<=") == 0) return leftVal <= rightVal;
  else if (strcmp(op, ">=") == 0) return leftVal >= rightVal;
  else if (strcmp(op, "==") == 0) return leftVal == rightVal;
  else if (strcmp(op, "!=") == 0) return leftVal != rightVal;
  
  showError("Unknown operator");
  return false;
}

void cmdFor(const char* args) {
  char varName[32] = "";
  int start = 0, end = 0, step = 1;
  
  if (sscanf(args, "%s %d %d %d", varName, &start, &end, &step) >= 3) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Loop ");
    display.print(varName);
    display.print(": ");
    display.print(start);
    display.print("-");
    display.println(end);
    display.display();
    delay(1000);
    
    for (int i = start; i <= end; i += step) {
      setVariableValue(varName, i);
      if (button_is_pressed(btn6)) break;
    }
  } else {
    showError("for var start end");
  }
}

void cmdWhile(const char* args) {
  char condition[256] = "";
  char command[256] = "";
  
  int spaceCount = 0;
  int commandStart = -1;
  
  for (int i = 0; i < strlen(args); i++) {
    if (args[i] == ' ') {
      spaceCount++;
      if (spaceCount >= 4) {
        commandStart = i + 1;
        break;
      }
    }
  }
  
  if (commandStart == -1) {
    showError("while cond cmd");
    return;
  }
  
  int pos = 0;
  for (int i = 0; i < commandStart - 1; i++) {
    if (args[i] == ' ' && pos > 0 && condition[pos-1] != ' ') {
      condition[pos++] = ' ';
    } else if (args[i] != ' ') {
      condition[pos++] = args[i];
    }
  }
  condition[pos] = '\0';
  
  strncpy(command, args + commandStart, 255);
  command[255] = '\0';
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("While Loop:");
  display.setCursor(0, 15);
  display.println(condition);
  display.setCursor(0, 30);
  display.println("Running...");
  display.display();
  delay(1000);
  
  int iterations = 0;
  unsigned long startTime = millis();
  const unsigned long MAX_RUNTIME = 60000; 
  
  while (evaluateCondition(condition)) {
    executeCommand(command);
    iterations++;
    
    if (millis() - startTime > MAX_RUNTIME) {
      showError("Loop timeout!");
      return;
    }
    
    if (button_is_pressed(btn6)) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.println("Loop interrupted!");
      display.display();
      delay(1000);
      return;
    }
    
    delay(100);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Loop done!");
  display.setCursor(0, 20);
  display.print("Iterations: ");
  display.println(iterations);
  display.display();
  delay(1500);
}

void cmdIf(const char* args) {
  char condition[256] = "";
  char trueCommand[256] = "";
  char falseCommand[256] = "";
  
  int elsePos = -1;
  for (int i = 0; i < strlen(args) - 3; i++) {
    if (strncmp(&args[i], " else ", 6) == 0) {
      elsePos = i;
      break;
    }
  }
  
  int spaceCount = 0;
  int commandStart = -1;
  
  for (int i = 0; i < strlen(args); i++) {
    if (args[i] == ' ') {
      spaceCount++;
      if (spaceCount == 3) {
        commandStart = i + 1;
        break;
      }
    }
  }
  
  if (commandStart == -1) {
    showError("if cond cmd");
    return;
  }
  
  int pos = 0;
  for (int i = 0; i < commandStart - 1; i++) {
    if (args[i] == ' ' && pos > 0 && condition[pos-1] != ' ') {
      condition[pos++] = ' ';
    } else if (args[i] != ' ') {
      condition[pos++] = args[i];
    }
  }
  condition[pos] = '\0';
  
  int cmdEnd = (elsePos == -1) ? strlen(args) : elsePos;
  pos = 0;
  for (int i = commandStart; i < cmdEnd; i++) {
    if (args[i] != ' ' || (pos > 0 && trueCommand[pos-1] != ' ')) {
      trueCommand[pos++] = args[i];
    }
  }
  while (pos > 0 && trueCommand[pos-1] == ' ') {
    pos--;
  }
  trueCommand[pos] = '\0';
  
  if (elsePos != -1) {
    int falseStart = elsePos + 6;
    pos = 0;
    for (int i = falseStart; i < strlen(args); i++) {
      falseCommand[pos++] = args[i];
    }
    falseCommand[pos] = '\0';
  }
  
  bool conditionMet = evaluateCondition(condition);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("If condition:");
  display.println(condition);
  display.setCursor(0, 30);
  display.print("Result: ");
  display.println(conditionMet ? "TRUE" : "FALSE");
  display.display();
  delay(1000);
  
  if (conditionMet) {
    if (strlen(trueCommand) > 0) {
      executeCommand(trueCommand);
    }
  } else {
    if (strlen(falseCommand) > 0) {
      executeCommand(falseCommand);
    }
  }
}

void showConditionResult(const char* condition, bool result) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Condition:");
  display.println(condition);
  display.setCursor(0, 35);
  display.setTextSize(2);
  display.print(result ? "TRUE" : "FALSE");
  display.display();
  delay(1500);
}

void showSuccess(const char* cmd, int val) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(cmd);
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(val);
  display.display();
  delay(1000);
}

void showSuccessStr(const char* cmd, const char* val) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(cmd);
  display.setCursor(0, 20);
  display.print(val);
  display.display();
  delay(1000);
}

void showError(const char* usage) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Usage:");
  display.setCursor(0, 20);
  display.print(usage);
  display.display();
  delay(1500);
}

void showVariables() {
  int varCount = 0;
  for (int i = 0; i < MAX_VARIABLES; i++) {
    if (variables[i].used) varCount++;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Variables (");
  display.print(varCount);
  display.print("/");
  display.print(MAX_VARIABLES);
  display.println(")");
  
  int y = 15;
  for (int i = 0; i < MAX_VARIABLES && y < 60; i++) {
    if (variables[i].used) {
      display.setCursor(0, y);
      display.print(variables[i].name);
      display.print(": ");
      display.println(variables[i].value);
      y += 10;
    }
  }
  
  display.display();
  
  while (true) {
    if (button_is_pressed(btn6)) break;
    delay(50);
  }
}

void clearVariables() {
  for (int i = 0; i < MAX_VARIABLES; i++) {
    variables[i].used = false;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Variables cleared!");
  display.display();
  delay(1500);
}

void showHelp1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("dw/dr pin [val]");
  display.println("aw/ar pin val");
  display.println("pm pin INPUT/OUTPUT");
  display.println("delay <ms>");
  display.println("millis");
  display.println("blink <pin> <cnt> <dly>");
  display.println("ptn <pin> <seq> <dly>");
  display.println("pm <pin> <mode>");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn2)) {
      showHelp2();
      return;
    }
    else if (button_is_pressed(btn3)) {
      showHelp3();
      return;
    }
    else if (button_is_pressed(btn6)) break;
    delay(100);
  }
}

void showHelp2() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("var <name> <val>");
  display.println("let <var> = <expr>");
  display.println("calc <expr>");
  display.println("inc/dec <var>");
  display.println("vars (all vars)");
  display.println("clear (all vars)");
  display.println("save <key> (NVS)");
  display.println("load <key> (NVS)");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn1)) {
      showHelp1();
      return;
    }
    else if (button_is_pressed(btn3)) {
      showHelp3();
      return;
    }
    if (button_is_pressed(btn6)) break;
    delay(100);
  }
}

void showHelp3() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("random <min> <max>");
  display.println("for <v> <srt> <end> <stp>");
  display.println("while <expr> <cmd>");
  display.println("if <expr> <cmd>");
  display.println("print <txt>");
  display.println("cls (clear screen)");
  display.display();
  
  while (true) {
    if (button_is_pressed(btn2)) {
      showHelp2();
      return;
    }
    else if (button_is_pressed(btn1)) {
      showHelp1();
      return;
    }
    if (button_is_pressed(btn6)) break;
    delay(100);
  }
}
