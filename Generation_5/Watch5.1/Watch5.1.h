#ifndef WATCH5_1_H
#define WATCH5_1_H

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <esp_wifi.h>
#include <ctype.h>
#include <math.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SCREEN_CENTER_X (SCREEN_WIDTH/2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT/2)
#define SCREEN_RADIUS 120

#define CHAR_W(sz) (6*(sz))
#define CHAR_H(sz) (8*(sz))
#define STR_W(text, sz) (strlen(text)*CHAR_W(sz))

#define MAX_WIFI_NETWORKS 5
#define MAX_WIFI_SSID 32
#define MAX_WIFI_PASS 64

// Watch 5.1.ino
void drawMainUI(void);
void saveBtnVals(void);
void loadBtnVals(void);
void randomiseMac(void);
void timeSyncAndUI(void);
bool button_is_pressed(int btnVal, bool onlyOnce);
bool a_button_is_pressed(void);

// calendar.ino
void initializeCalendar(void);
void saveCalendarToNVS(void);
void loadCalendarFromNVS(void);
void calendar(void);
void createNewEvent(void);
void selectDateTime(struct CalendarEvent* event);
void incrementField(struct CalendarEvent* event, int field);
void decrementField(struct CalendarEvent* event, int field);
void viewEvents(void);
void deleteEvent(int idx);
void editEvent(int idx);
void serialAddEvent(void);
void serialViewEvents(void);
void serialDeleteEvent(void);
void serialEditEvent(void);
void serialCalendarMenu(void);
void checkCalendarAlarms(void);
void triggerAlarm(struct CalendarEvent* event);

// maths.ino
void maths(void);
void calculator(void);
void unitConverter(void);
void baseConverter(void);
void inputNum(char* buffer, int maxLen, int base);
void convertAndDisplay(const char* number, int sourceBase, int targetBase);
void graphPlotter(void);
bool plotGraph(char* equation, double xMin, double xMax, double yMin, double yMax);
double evaluateEquation(char* equation, double x);
void matrixCalculator(void);
void setIdentity(float m[3][3], int n);
void copyMatrix(float src[3][3], float dest[3][3], int rows, int cols);
void matrixAdd(float a[3][3], float b[3][3], float out[3][3], int rows, int cols);
void matrixSub(float a[3][3], float b[3][3], float out[3][3], int rows, int cols);
void matrixMul(float a[3][3], int aR, int aC, float b[3][3], int bR, int bC, float out[3][3], int& outR, int& outC);
void matrixScalar(float a[3][3], int rows, int cols, float factor, float out[3][3]);
void matrixTranspose(float in[3][3], int rows, int cols, float out[3][3]);
float matrixDet(float m[3][3], int n);
bool matrixInverse(float m[3][3], int n, float out[3][3]);
void drawMatrix(float m[3][3], int rows, int cols, int baseY);
void showResultMatrix(const char* op, float m[3][3], int rows, int cols, float scalar);
void editMatrix(float m[3][3], int& rows, int& cols, char name);
void showScalarMenu(float& scalar);
void showMatrixDet(float det);
void showMatrixError(const char* msg);
void primeFactorisation(void);

// games.ino
void games(void);
void arcadeGames(void);
void mathsGames(void);
void shooter(void);
void snake(void);
void flappyBird(void);
void geometryDash(void);
void flying3D(void);
void countdown(void);

// other.ino
void activateFunc(byte func, int blinkTime);
void watchFuncs(void);
void counter(void);
void randomNum(void);
void metronome(void);
void metronome_display_main(int bpm, int time_sig, int beat, int total_beats);
void metronome_pulse_beat(int beat, int total_beats);
void metronome_tap_tempo(void);
void metronome_time_signature_menu(void);
void metronome_subdivision_menu(void);
void saveNotesToNVS(void);
void loadNotesFromNVS(void);
void initializeNotesNVS(void);
void notesFunction(void);
void serialCreateEditNote(void);
void serialViewAllNotes(void);
void serialDeleteNote(void);
void serialClearAllNotes(void);
void serialNotesMenu(void);

// settings.ino
void settings(void);
void tuneButtonVals(void);
void prefs(void);
void debug(void);
void chipStats(void);
void runtimeStats(void);
void btnSettings(void);

// wifi.ino
void saveWiFiNetworksToNVS(void);
void loadWiFiNetworksFromNVS(void);
void timeSync(void);
void connectWiFi(void);
void addWiFiNetworkOnWatch(void);
bool inputStringOnWatch(const char* label, char* buffer, int maxLen);
void wifiNetworkMenu(void);
void deleteWiFiNetwork(int idx);
void scanWiFiNetworks(void);
void disconnectWiFi(void);
void wifiMenu(void);
void addWiFiNetworkSerial(void);
void listWiFiNetworksSerial(void);
void deleteWiFiNetworkSerial(void);
void connectWiFiSerial(void);
void serialDisconnectWiFi(void);
void serialShowWiFiStatus(void);
void serialWiFiMenu(void);
void wifiFuncs(void);
void getWeather(void);
void initializeTimeSettings(void);
int getTimeOffset(void);
void setTimeOffset(int offset);
bool get12HourMode(void);
void set12HourMode(bool mode);
int getDSTOffset(void);
void setDSTOffset(int dst);
void timeSettingsMenu(void);
void displayTime(void);
bool fetchWordDefinition(const char* word);
void dictCharacterInput(char* buffer, int maxLen);
void dictDisplayWord(int defIndex);
void dictDisplayResult(void);
void dictionary(void);

// Variables
const byte buttonPin = 2;
const int defBtn1 = 1433;  // 4.7K
const int defBtn2 = 812;   // 2.2K
const int defBtn3 = 202;   // 470
const int defBtn4 = 409;   // 1K
const int defBtn5 = 95;    // 220
const int defBtn6 = 2304;  // 10K
int btn1;
int btn2;
int btn3;
int btn4;
int btn5;
int btn6;

int buttonOffset = 0;
int buttonValRange = 30;

byte Func1 = 3;
byte Func2 = 0;
byte Func3 = 1;

int blinkTime1 = 500000;
int blinkTime2 = 1;
int blinkTime3 = 10000;

int selectedFunction = 1;
int lastSelectedFunction = -1;

bool wifiConnected = false;

unsigned long lastNavTime = 0;
const unsigned long NAV_DEBOUNCE = 120;

uint16_t COLOR_BG        = 0x1908;
uint16_t COLOR_ACCENT    = 0x053F;
uint16_t COLOR_FG        = 0xFFFF;
uint16_t COLOR_SELECTED  = 0xFDE0;
uint16_t COLOR_UNSELECTED= 0x4208;
uint16_t COLOR_ERROR     = 0xF800;

int *btnRefs[] = {&btn1, &btn2, &btn3, &btn4, &btn5, &btn6};
const int *defBtnRefs[] = {&defBtn1, &defBtn2, &defBtn3, &defBtn4, &defBtn5, &defBtn6};

const char *labels[] = {"Btn 1", "Btn 2", "Btn 3", "Btn 4", "Btn 5", "Btn 6"};

struct WiFiNetwork {
  char ssid[MAX_WIFI_SSID];
  char password[MAX_WIFI_PASS];
};

struct CalendarEvent {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  char message[MAX_EVENT_MESSAGE];
  bool used;
  bool alarmed;
};


struct DictResult {
  char word[MAX_WORD_LENGTH];
  char phonetic[MAX_WORD_LENGTH];
  char partOfSpeech[16];
  char definition[256];
  char example[256];
  int definitionCount;
  int currentDefinition;
};

struct Note {
  char text[MAX_NOTE_LENGTH];
  bool used;
};


#endif // WATCH5_1_H
