#include "WiFi.h"

#define R_PIN D9
#define G_PIN D8
#define B_PIN D7


void setup(){

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  Serial.begin(115200);

  WiFi.mode(WIFI_MODE_STA);

  Serial.println(WiFi.macAddress());


}

 

void loop(){
  digitalWrite(R_PIN, LOW); // leds activés à l'état bas
  digitalWrite(B_PIN, HIGH);
  delay(1000);
  digitalWrite(G_PIN, LOW);
  digitalWrite(R_PIN, HIGH);
  delay(1000);
  digitalWrite(B_PIN, LOW);
  digitalWrite(G_PIN, HIGH);
  delay(1000);

}