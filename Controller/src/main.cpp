#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Car MAC address
uint8_t connectedAddress[6] = {0x64, 0xE8, 0x33, 0x89, 0xC2, 0xA8};

#define POT_X_PIN A0
#define POT_Y_PIN A1
#define BUTTON_PIN D10

#define R_PIN D9
#define G_PIN D8
#define B_PIN D7

// Midpoint and bounds for ADC calibration
#define ADC_CENTER 2220  
#define ADC_MAX    4095  
#define ADC_MIN     180  

typedef struct {
  int X, Y;
} potValues;

potValues joystick;
esp_now_peer_info_t peerInfo;

unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 20; // ms (50 Hz update rate)

bool connected = false;

// ---------------- ESP-NOW callbacks ----------------

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    if (!connected) {
      connected = true;
      Serial.println("Car connected ✅");
      analogWrite(G_PIN, 255); // Green LED = connected
    }
  } else {
    if (connected) {
      connected = false;
      Serial.println("Car disconnected ❌");
      analogWrite(R_PIN, 255); // Red LED = disconnected
    }
  }
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

  analogWrite(R_PIN, 128); // Red = searching
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);

  // Init WiFi only for ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true); // prevent scanning → save power

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
}

// ---------------- Main loop ----------------

void loop() {
  unsigned long now = millis();

  // Send joystick data at fixed rate
  if (now - lastSend >= SEND_INTERVAL) {
    lastSend = now;

    // Read joystick
    int pot_X = analogRead(POT_X_PIN);
    int pot_Y = analogRead(POT_Y_PIN);

    // Map to -255..255
    joystick.X = (pot_X > ADC_CENTER)
                   ? map(pot_X, ADC_CENTER, ADC_MAX, 0, 255)
                   : map(pot_X, ADC_MIN, ADC_CENTER, -255, 0);

    joystick.Y = (pot_Y > ADC_CENTER)
                   ? map(pot_Y, ADC_CENTER, ADC_MAX, 0, 255)
                   : map(pot_Y, ADC_MIN, ADC_CENTER, -255, 0);

    // Send via ESP-NOW
    esp_err_t result = esp_now_send(connectedAddress, (uint8_t*)&joystick, sizeof(joystick));

    if (result != ESP_OK) {
      Serial.println("Send error ❌");
      analogWrite(R_PIN, 255);
    }
  }

  // Short idle sleep to reduce CPU load
  delay(1);
}
