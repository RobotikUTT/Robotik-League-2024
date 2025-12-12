#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#define MOT_L_1_PIN D0
#define MOT_L_2_PIN D1
#define MOT_R_1_PIN D3
#define MOT_R_2_PIN D4

#define ENABLE_X_PIN D2
#define ENABLE_Y_PIN D5

#define R_PIN D9
#define G_PIN D8
#define B_PIN D7

#define PUISSANCE_MIN 127

typedef struct {
  int8_t X, Y;
} potValues;

typedef struct {
  int gauche;
  int droite;
} moteur;

volatile potValues joystick = { 0, 0 };  // volatile -> used in ISR/callback
volatile moteur puissance = { 0, 0 };
volatile bool flag_com = false;

void writeSpeed(int8_t X, int8_t Y);

// Callback when data is received
void OnDataRecv(const esp_now_recv_info* mac, const uint8_t* incomingData, int len) {
  if (len == sizeof(joystick)) {
    memcpy((void*)&joystick, incomingData, sizeof(joystick));
    writeSpeed(joystick.X, joystick.Y);
    Serial.println("marche normalement askip");
    flag_com = true;
    /*analogWrite(R_PIN, 255);
    analogWrite(G_PIN, 200); // ça roule
    analogWrite(B_PIN, 255);*/
  } else {
    Serial.println("marche presque");
  }
}

void setup() {
  Serial.begin(115200);

  // Minimal Wi-Fi init for ESP-NOW only
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect(true);   // saves power by avoiding AP scanning

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur initialisation ESP-NOW");
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv);

  pinMode(MOT_L_1_PIN, OUTPUT);
  pinMode(MOT_R_1_PIN, OUTPUT);
  pinMode(MOT_L_2_PIN, OUTPUT);
  pinMode(MOT_R_2_PIN, OUTPUT);

  /*pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);*/

  // Motor driver enable pins
  pinMode(ENABLE_X_PIN, OUTPUT);
  pinMode(ENABLE_Y_PIN, OUTPUT);
  digitalWrite(ENABLE_X_PIN, LOW);  // start disabled
  digitalWrite(ENABLE_Y_PIN, LOW);

  delay(2000);  //pour que ça s'affiche quand tu le branche l'IDE Arduino est trop lent
  Serial.print("mon adresse mac : ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  delay(300);
  if (!flag_com) {
    writeSpeed(0, 0);
    /*analogWrite(R_PIN, 200); //plus de com
    analogWrite(G_PIN, 255);
    analogWrite(B_PIN, 255);*/
  }
  flag_com = false;
  /*puissance.gauche = 250;
  puissance.droite = 250;
  writeSpeed();
  delay(1000);
  puissance.gauche = -250;
  puissance.droite = 250;
  writeSpeed();
  delay(1000);
  puissance.gauche = 0;
  puissance.droite = 0;
  writeSpeed();*/
}

// ----------- Motor Functions -----------

void writeSpeed(int8_t X, int8_t Y) {
  // If both motors are stopped, disable driver for power saving
  if (X == 0 && Y == 0) {
    digitalWrite(ENABLE_X_PIN, LOW);
    digitalWrite(ENABLE_Y_PIN, LOW);
    return;
  } else {
    digitalWrite(ENABLE_X_PIN, HIGH);
    digitalWrite(ENABLE_Y_PIN, HIGH);
  }
  puissance.gauche = Y + X; //simple mix 127+127 = 254
  puissance.droite = Y - X;

  // Right motor
  if (puissance.droite > 0) {
    analogWrite(MOT_R_1_PIN, puissance.droite);
    analogWrite(MOT_R_2_PIN, 0);
  } else {
    analogWrite(MOT_R_1_PIN, 0);
    analogWrite(MOT_R_2_PIN, -puissance.droite);
  }

  // Left motor
  if (puissance.gauche > 0) {
    analogWrite(MOT_R_1_PIN, puissance.gauche);
    analogWrite(MOT_R_2_PIN, 0);
  } else {
    analogWrite(MOT_R_1_PIN, 0);
    analogWrite(MOT_R_2_PIN, -puissance.gauche);
  }
}
