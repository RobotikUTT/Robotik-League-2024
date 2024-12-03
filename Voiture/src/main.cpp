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
  int X = 0, Y = 0;
} potValues;

typedef struct {
  int gauche = 0;
  int droite = 0;
} moteur;

potValues joystick;
moteur puissance;

void writeSpeed();
void convertDataToCommand(float X, float Y);

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  if (len == sizeof(joystick)) {  // Check data length to avoid errors
    memcpy(&joystick, incomingData, sizeof(joystick));
  } else {
    Serial.println("Taille inattendue des données reçues");
  }
  convertDataToCommand(joystick.X, joystick.Y);
  writeSpeed();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur initialisation ESP-NOW");
    return;
  }

  // Register the receive callback function
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(MOT_L_1_PIN, OUTPUT);
  pinMode(MOT_R_1_PIN, OUTPUT);
  pinMode(MOT_L_2_PIN, OUTPUT);
  pinMode(MOT_R_2_PIN, OUTPUT);

  // Enable motor drivers
  pinMode(ENABLE_X_PIN, OUTPUT);
  pinMode(ENABLE_Y_PIN, OUTPUT);
  digitalWrite(ENABLE_X_PIN, HIGH); // Enable motor X
  digitalWrite(ENABLE_Y_PIN, HIGH); // Enable motor Y
}

void loop() {
  Serial.print("X : ");
  Serial.print(joystick.X);
  Serial.print("\t");
  Serial.print("Y : ");
  Serial.print(joystick.Y);
  Serial.print("\t");
  Serial.print("Droite : ");
  Serial.print(puissance.droite);
  Serial.print("\t");
  Serial.print("Gauche : ");
  Serial.println(puissance.gauche);
}

void writeSpeed(){
  // Left motor control
  if(puissance.gauche > 0){
    analogWrite(MOT_L_1_PIN, puissance.gauche);  // Forward
    analogWrite(MOT_L_2_PIN, 0);
  } else {
    analogWrite(MOT_L_1_PIN, 0);
    analogWrite(MOT_L_2_PIN, abs(puissance.gauche));  // Reverse
  }

  // Right motor control
  if(puissance.droite > 0){
    analogWrite(MOT_R_1_PIN, puissance.droite);  // Forward
    analogWrite(MOT_R_2_PIN, 0);
  } else {
    analogWrite(MOT_R_1_PIN, 0);
    analogWrite(MOT_R_2_PIN, abs(puissance.droite));  // Reverse
  }
}


void convertDataToCommand(float X, float Y) {
  // Calculate total power (magnitude)
  float power = sqrtf(X * X + Y * Y);

  // Calculate angle in radians
  float angle = atan2f(Y, X);

  // Left and right power based on joystick angle
  puissance.gauche = (int)(power * sinf(angle + PI / 4));
  puissance.droite = (int)(power * sinf(angle - PI / 4));

  // Ensure power is within valid range for PWM (0 to 255)
  puissance.gauche = constrain(puissance.gauche, -255, 255);
  puissance.droite = constrain(puissance.droite, -255, 255);
}