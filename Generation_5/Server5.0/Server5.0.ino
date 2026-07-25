#include <WiFi.h>
#include <ArduinoOTA.h>
#include <SD.h>

File file;

const char* ssid = "...";
const char* password = "...";

const int led = 8; 
unsigned long previousMillis = 0; 
const long interval = 100; 
int ledState = LOW; 

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  Serial.print("Starting!");
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.begin();

  if (!SD.begin()) {
    while (true) Serial.println("SD Card failed to load!");
  }
}

void loop() {
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !(ledState);
    digitalWrite(led, ledState);
  }
}