#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

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

volatile potValues joystick = {0, 0};   // volatile -> used in ISR/callback
volatile moteur puissance = {0, 0};

void writeSpeed();
void convertDataToCommand(int X, int Y);

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  if (len == sizeof(joystick)) {
    memcpy((void*)&joystick, incomingData, sizeof(joystick));
    convertDataToCommand(joystick.X, joystick.Y);
    writeSpeed();
  }
}

void setup() {
  Serial.begin(115200);

  // Minimal Wi-Fi init for ESP-NOW only
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);   // saves power by avoiding AP scanning

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur initialisation ESP-NOW");
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv);

  pinMode(MOT_L_1_PIN, OUTPUT);
  pinMode(MOT_R_1_PIN, OUTPUT);
  pinMode(MOT_L_2_PIN, OUTPUT);
  pinMode(MOT_R_2_PIN, OUTPUT);

  // Motor driver enable pins
  pinMode(ENABLE_X_PIN, OUTPUT);
  pinMode(ENABLE_Y_PIN, OUTPUT);
  digitalWrite(ENABLE_X_PIN, LOW); // start disabled
  digitalWrite(ENABLE_Y_PIN, LOW);
}

void loop() {
  // Sleep to reduce CPU load when idle
  delay(200);
}

// ----------- Motor Functions -----------

void writeSpeed() {
  // If both motors are stopped, disable driver for power saving
  if (puissance.gauche == 0 && puissance.droite == 0) {
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
  // Approximation avoids sqrtf + atan2f (heavy floating point!)
  // Simple mix: forward/backward = Y, turn = X
  puissance.gauche = constrain(Y + X, -255, 255);
  puissance.droite = constrain(Y - X, -255, 255);
}
