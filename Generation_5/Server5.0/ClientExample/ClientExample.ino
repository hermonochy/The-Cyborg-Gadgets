#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <time.h>
#include <sys/time.h>

#define SERVICE_UUID       "19348c39-0336-474b-ad5a-6a805b0e2486"
#define CHARACTERISTIC_UUID "b83bd7fa-a073-428c-bb58-1eb565ceec46"
#define MIN_VALID_EPOCH    1700000000UL   // Nov 2023; rejects garbage/unsynced values

static BLEAdvertisedDevice* scanResult = nullptr;

class ScanCb : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      scanResult = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

void initBLE() {
  BLEDevice::init("CG");
  Serial.println("Client initialized");
}

uint32_t bleTimeSync(uint32_t timeoutMs) {
  const uint32_t t0 = millis();
  scanResult = nullptr;

  Serial.println("Scanning...");
  BLEScan* pBLEScan = BLEDevice::getScan();
  ScanCb scanCb;
  pBLEScan->setAdvertisedDeviceCallbacks(&scanCb, false);
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  if (scanResult == nullptr) {
    Serial.println("Server not found.");
    pBLEScan->clearResults();
    return 0;
  }

  BLEClient* pClient = BLEDevice::createClient();
  if (!pClient->connect(scanResult)) {
    Serial.println("Connection failed.");
    delete scanResult;
    scanResult = nullptr;
    pBLEScan->clearResults();
    return 0;
  }

  BLERemoteService* svc = pClient->getService(SERVICE_UUID);
  if (!svc) {
    Serial.println("Time Sync Service not found.");
    pClient->disconnect();
    delete scanResult;
    scanResult = nullptr;
    pBLEScan->clearResults();
    return 0;
  }

  BLERemoteCharacteristic* ch = svc->getCharacteristic(CHARACTERISTIC_UUID);
  if (!ch || !ch->canRead()) {
    Serial.println("Characteristic not found/readable.");
    pClient->disconnect();
    delete scanResult;
    scanResult = nullptr;
    pBLEScan->clearResults();
    return 0;
  }

  uint32_t epoch;
  while (millis() - t0 < timeoutMs) {
    String v = ch->readValue();
    uint32_t val = strtoul(v.c_str(), NULL, 10);
    if (val >= MIN_VALID_EPOCH) {
      epoch = val;
      break;
    }
    delay(500);   // retry — server clock may not be ready yet
  }

  pClient->disconnect();
  delete scanResult;
  scanResult = nullptr;
  pBLEScan->clearResults();

  if (epoch == 0) {
    Serial.println("Timeout");
    return 0;
  }

  struct timeval tv;
  tv.tv_sec = (time_t)epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  struct tm timeinfo;
  getLocalTime(&timeinfo, 1000);
  char buf[80];
  strftime(buf, sizeof(buf), "%A, %Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.printf("[BLE] Time synced! %s\n", buf);

  return epoch;
}

void setup() {
  Serial.begin(115200);

  setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
  tzset();

  initBLE();
  uint32_t t = bleTimeSync(15000);
  if (t == 0) {
    Serial.println("Sync failed — running without valid time.");
  }
  // from here on: time(), localtime(), getLocalTime(), tm_wday all work
}

void loop() {
  // your application code — time() is now valid
}