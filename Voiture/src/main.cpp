#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define MOT_L_1_PIN D0
#define MOT_L_2_PIN D1
#define MOT_R_1_PIN D3
#define MOT_R_2_PIN D4

#define ENABLE_X_PIN  D2
#define ENABLE_Y_PIN  D5


typedef struct {
  int X, Y;
} potValues;

typedef struct {
  int gauche;
  int droite;
} moteur;

volatile potValues joystick = {0, 0};
volatile moteur puissance = {0, 0};

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

void writeSpeed();
void convertDataToCommand(int X, int Y);

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() == sizeof(potValues)) {
      memcpy((void*)&joystick, rxValue.data(), sizeof(potValues));
      convertDataToCommand(joystick.X, joystick.Y);
      writeSpeed();
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(MOT_L_1_PIN, OUTPUT);
  pinMode(MOT_L_2_PIN, OUTPUT);
  pinMode(MOT_R_1_PIN, OUTPUT);
  pinMode(MOT_R_2_PIN, OUTPUT);
  pinMode(ENABLE_X_PIN, OUTPUT);
  pinMode(ENABLE_Y_PIN, OUTPUT);

  // --- BLE init ---
  BLEDevice::init("RC_Car");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService("12345678-1234-1234-1234-1234567890ab");

  pCharacteristic = pService->createCharacteristic(
                      "abcdefab-1234-5678-1234-abcdefabcdef",
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("Car ready, waiting for BLE controller...");
}

void loop() {
  delay(200);
}

void writeSpeed() {
  if (abs(puissance.gauche) < 10 && abs(puissance.droite) < 10) {
    digitalWrite(ENABLE_X_PIN, LOW);
    digitalWrite(ENABLE_Y_PIN, LOW);
    return;
  } else {
    digitalWrite(ENABLE_X_PIN, HIGH);
    digitalWrite(ENABLE_Y_PIN, HIGH);
  }

  // Left motor
  if (puissance.gauche > 0) {
    analogWrite(MOT_L_1_PIN, puissance.gauche);
    analogWrite(MOT_L_2_PIN, 0);
  } else {
    analogWrite(MOT_L_1_PIN, 0);
    analogWrite(MOT_L_2_PIN, -puissance.gauche);
  }

  // Right motor
  if (puissance.droite > 0) {
    analogWrite(MOT_R_1_PIN, puissance.droite);
    analogWrite(MOT_R_2_PIN, 0);
  } else {
    analogWrite(MOT_R_1_PIN, 0);
    analogWrite(MOT_R_2_PIN, -puissance.droite);
  }
}

void convertDataToCommand(int X, int Y) {
  puissance.gauche = constrain(Y + X, -255, 255);
  puissance.droite = constrain(Y - X, -255, 255);
}
