#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Addresses and colors can be set up later if needed
// uint8_t broadcastAddresses[6][6] = {...};
// uint8_t colors[6][3] = {...};

uint8_t my_car = 1;

uint8_t connectedAddress[6] = {0x64, 0xE8, 0x33, 0x89, 0xC2, 0xA8};

void getPotData();
void translateToWeelSpeed();
void setLedColor(uint8_t color[3]);
void configurePins();
void connectToCar(uint8_t* Address);

#define POT_X_PIN A0
#define POT_Y_PIN A1

#define BUTTON_PIN D10 // Button pin, replace if needed

#define R_PIN D9
#define G_PIN D8
#define B_PIN D7

typedef struct {
  int X, Y;
} potValues;

uint16_t pot_X, pot_Y;

potValues joystick;
esp_now_peer_info_t peerInfo;

unsigned long pre_time = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}
 
void setup() {
  Serial.begin(115200);

  configurePins();
  connectToCar(connectedAddress); // Connect to the car with the defined address
}
 

void loop() {
  getPotData();           // Get joystick data from potentiometers
  translateToWeelSpeed();  // Convert data to motor commands

  // Sending data over ESP-NOW
  esp_err_t result = esp_now_send(connectedAddress, (uint8_t *) &joystick, sizeof(joystick));

  delay(5); // Small delay to allow ESP-NOW background tasks to run
}

void getPotData() {
  pot_X = analogRead(POT_X_PIN); // Read X-axis potentiometer value
  pot_Y = analogRead(POT_Y_PIN); // Read Y-axis potentiometer value
}

#define medium 2220  // Define medium point for potentiometer input
#define max 4095     // Maximum input value
#define min 180      // Minimum input value

void translateToWeelSpeed() {
  // Convert potentiometer X to motor command
  if (pot_X > medium) {
    joystick.X = map(pot_X, medium, max, 0, 255);
  } else {
    joystick.X = map(pot_X, min, medium, -255, 0);
  }

  // Convert potentiometer Y to motor command
  if (pot_Y > medium) {
    joystick.Y = map(pot_Y, medium, max, 0, 255);
  } else {
    joystick.Y = map(pot_Y, min, medium, -255, 0);
  }

}

void setLedColor(uint8_t color[3]) {
  // Set the RGB LED color
  analogWrite(R_PIN, color[0]);
  analogWrite(G_PIN, color[1]);
  analogWrite(B_PIN, color[2]);
}

void connectToCar(uint8_t* Address) {
  WiFi.mode(WIFI_STA); // Set ESP32 to station mode

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent); // Register callback for sent data
  memcpy(peerInfo.peer_addr, Address, 6); // Set the peer address
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to Add Peer");
    return;
  }
}

void configurePins() {
  pinMode(POT_X_PIN, INPUT);  // Set potentiometer X pin as input
  pinMode(POT_Y_PIN, INPUT);  // Set potentiometer Y pin as input
  pinMode(R_PIN, OUTPUT);     // Set red LED pin as output
  pinMode(G_PIN, OUTPUT);     // Set green LED pin as output
  pinMode(B_PIN, OUTPUT);     // Set blue LED pin as output
  pinMode(BUTTON_PIN, INPUT); // Set button pin as input
}
