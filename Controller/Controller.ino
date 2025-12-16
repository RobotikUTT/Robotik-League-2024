#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_sleep.h>

// Car MAC address
uint8_t connectedAddress[6] = {0x64, 0xE8, 0x33, 0x89, 0xC2, 0xA8 };

#define POT_X_PIN A0
#define POT_Y_PIN A1
#define BUTTON_PIN D10

#define R_PIN D9
#define G_PIN D8
#define B_PIN D7

// Midpoint and bounds for ADC calibration
#define ADC_CENTER 256
#define ADC_MAX 512   //si sur 9 bits
#define SEND_MAX 127  // si int8_t -127/127

#define ZONE_MORTE 25

typedef struct {
  int8_t X, Y;
} potValues;

potValues joystick;
esp_now_peer_info_t peerInfo;

volatile bool sent = true;
volatile bool connected = false;

// ---------------- ESP-NOW callbacks ----------------

void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    if (!connected) {
      connected = true;
      Serial.println("Car connected ✅");
      analogWrite(G_PIN, 200);  // Green LED = connected
      analogWrite(R_PIN, 255);
      analogWrite(B_PIN, 255);
    } else {
      delay(200);               // attention à ce delay la il peut être chiant
      analogWrite(G_PIN, 255);  // éteint la led au prochain passage
      analogWrite(R_PIN, 255);
      analogWrite(B_PIN, 255);
    }
  } else {
    if (connected) {
      connected = false;
      Serial.println("Car disconnected ❌");
      analogWrite(R_PIN, 200);  // Red LED = disconnected
      analogWrite(G_PIN, 255);
      analogWrite(B_PIN, 255);
    }
  }
  sent = true;
}

// ---------------- Setup ----------------

void setup() {
  Serial.begin(115200);
  

  // Configure pins
  pinMode(POT_X_PIN, INPUT);
  pinMode(POT_Y_PIN, INPUT);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  analogWrite(R_PIN, 255);
  analogWrite(G_PIN, 255);
  analogWrite(B_PIN, 200);  // blue: searching

  // Init WiFi only for ESP-NOW
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect(true); // prevent scanning → save power

  

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    ESP.restart();
  }

  // Register peer
  memcpy(peerInfo.peer_addr, connectedAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();
  }

  esp_now_register_send_cb(OnDataSent);

  Serial.println("Controller ready 🎮");
  delay(1000);  //pour que ça s'affiche quand tu le branche, l'IDE Arduino est trop lent
  Serial.print("send data to : ");
  for (int i = 0; i < 6; i++) {
    Serial.print(", 0x");
    Serial.print(connectedAddress[i], HEX);
  }
  Serial.println();
  analogReadResolution(9);
  
  if(Serial.isConnected()){
    analogWrite(R_PIN, 200);
    analogWrite(G_PIN, 255);
    analogWrite(B_PIN, 200);
  }
  
  setCpuFrequencyMhz(80);
}

// ---------------- Main loop ----------------

void loop() {

  while (!sent)
    ;

  if(Serial.isConnected()){
   delay(100);
   Serial.println("connected");
   
  }else{
     // Configuration du timer de réveil
  esp_sleep_enable_timer_wakeup(100 * 1000);  // 500 ms en microsecondes

  // Passe en mode light sleep
  esp_light_sleep_start();
  }
  
  // Read joystick

  int pot_X = analogRead(POT_X_PIN) - ADC_CENTER;
  int pot_Y = analogRead(POT_Y_PIN) - ADC_CENTER;
  if (pot_X > -ZONE_MORTE && pot_X < ZONE_MORTE && pot_Y > -ZONE_MORTE && pot_Y < ZONE_MORTE) {
    sent = true;
    /*analogWrite(R_PIN, 250);
    analogWrite(G_PIN, 250);
    analogWrite(B_PIN, 250);*/
  } else {
    if (pot_X < -ZONE_MORTE) {
      pot_X += ZONE_MORTE;
    } else {
      if (pot_X > ZONE_MORTE) {
        pot_X -= ZONE_MORTE;
      } else {
        pot_X = 0;
      }
    }
    if (pot_Y < -ZONE_MORTE) {
      pot_Y += ZONE_MORTE;
    } else {
      if (pot_Y > ZONE_MORTE) {
        pot_Y -= ZONE_MORTE;
      } else {
        pot_Y = 0;
      }
    }
    if (pot_X > SEND_MAX) pot_X = SEND_MAX;
    if (pot_X < -SEND_MAX) pot_X = -SEND_MAX;
    if (pot_Y > SEND_MAX) pot_Y = SEND_MAX;
    if (pot_Y < -SEND_MAX) pot_Y = -SEND_MAX;

    joystick.X = pot_X;
    joystick.Y = pot_Y;

    if(Serial.isConnected()){
      Serial.printf("joystick X : %d, joystick Y : %d\n", joystick.X, joystick.Y);
    }
    sent = false;
    // Send via ESP-NOW
    esp_err_t result = esp_now_send(connectedAddress, (uint8_t *)&joystick, sizeof(joystick));

    if (result != ESP_OK) {
      Serial.println("Send error ❌");
      analogWrite(R_PIN, 200);
      analogWrite(G_PIN, 255);
      analogWrite(B_PIN, 255);
    }
    /*analogWrite(R_PIN, 255-abs(joystick.X));
    analogWrite(G_PIN, 255-abs(joystick.Y));
    analogWrite(B_PIN, abs((joystick.X+joystick.Y)/2));*/
  }
}
