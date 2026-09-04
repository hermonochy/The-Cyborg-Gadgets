#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SD.h>
#include <time.h>

File file;

const char* ssid = "...";
const char* password = "...";

#define SERVICE_UUID "19348c39-0336-474b-ad5a-6a805b0e2486"
#define CHARACTERISTIC_UUID "b83bd7fa-a073-428c-bb58-1eb565ceec46"

BLECharacteristic *pCharacteristic;

bool deviceConnected = false;
unsigned long previousTime = 0;
unsigned long interval = 1000;

class serverCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Client Connected");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Client Disconnected");
    pServer->getAdvertising()->start();
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Welcome to Server 5.0!");

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  delay(10000);

  Serial.println("Syncing time...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10000)) {
    Serial.print("Time: ");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  } else {
    Serial.println("NTP sync FAILED - clock still at epoch!");
  }

  Serial.println("Initialising SD...");
  if (!SD.begin()) {
    Serial.println("SD Card failed to load!");
  }

  Serial.println(F("Initialising BLE..."));
  BLEDevice::init("Server5.0");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new serverCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("Server 5.0 Ready!");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  WiFi.disconnect();

  Serial.println("Waiting for client connection...");
}

void loop() {
  if (deviceConnected) {
    if (millis() - previousTime >= interval) {
      previousTime = millis();

      time_t now = time(nullptr);

      String data = String((uint32_t)now);
      pCharacteristic->setValue((uint8_t*)data.c_str(), data.length());
      pCharacteristic->notify();

      Serial.println("Sent Data: " + data);
    }
  }
}