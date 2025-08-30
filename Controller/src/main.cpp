#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define POT_X_PIN A0
#define POT_Y_PIN A1
#define BUTTON_PIN D10

#define JOY_CENTER 2220
#define JOY_MAX    4095
#define JOY_MIN    180

typedef struct {
  int X, Y;
} potValues;

potValues joystick;
BLEAdvertisedDevice* myDevice;
BLERemoteCharacteristic* pRemoteCharacteristic;
bool connected = false;

static BLEUUID serviceUUID("12345678-1234-1234-1234-1234567890ab");
static BLEUUID charUUID("abcdefab-1234-5678-1234-abcdefabcdef");

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      Serial.println("Found Car!");
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      advertisedDevice.getScan()->stop();
    }
  }
};

void connectToCar() {
  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(myDevice);
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  connected = true;
  Serial.println("Connected to Car ✅");
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("RC_Controller");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  pinMode(POT_X_PIN, INPUT);
  pinMode(POT_Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if (!connected && myDevice) {
    connectToCar();
  }

  if (connected) {
    int pot_X = analogRead(POT_X_PIN);
    int pot_Y = analogRead(POT_Y_PIN);

    joystick.X = (pot_X > JOY_CENTER)
                   ? map(pot_X, JOY_CENTER, JOY_MAX, 0, 255)
                   : map(pot_X, JOY_MIN, JOY_CENTER, -255, 0);
    joystick.Y = (pot_Y > JOY_CENTER)
                   ? map(pot_Y, JOY_CENTER, JOY_MAX, 0, 255)
                   : map(pot_Y, JOY_MIN, JOY_CENTER, -255, 0);

    pRemoteCharacteristic->writeValue((uint8_t*)&joystick, sizeof(joystick));
  }

  delay(20); // 50 Hz update
}
