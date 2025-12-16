#include "WiFi.h"

#define R_PIN D9
#define G_PIN D8
#define B_PIN D7

#define POT_X_PIN A0
#define POT_Y_PIN A1
#define BUTTON_PIN D5


void setup() {

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(POT_X_PIN, INPUT);
  pinMode(POT_Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  Serial.begin(115200);

  WiFi.mode(WIFI_MODE_STA);

  Serial.println(WiFi.macAddress());
  analogReadResolution(9);
}


void loop() {
  /*digitalWrite(R_PIN, LOW); // leds activés à l'état bas
  digitalWrite(B_PIN, HIGH);
  delay(100);
  digitalWrite(G_PIN, LOW);
  digitalWrite(R_PIN, HIGH);
  delay(100);
  digitalWrite(B_PIN, LOW);
  digitalWrite(G_PIN, HIGH);
  delay(100);*/
  int pot_X = analogRead(POT_X_PIN);
  int pot_Y = analogRead(POT_Y_PIN);
  Serial.print("valeur X : ");
  Serial.println(pot_X);
  Serial.print("valeur Y : ");
  Serial.println(pot_Y);
  Serial.print("pot : ");
  Serial.println(digitalRead(BUTTON_PIN));

  delay(200);
  analogWrite(G_PIN, 255 - pot_X/4 );  // éteint la led au prochain passage
  analogWrite(R_PIN, 255 - pot_Y/4);
  analogWrite(B_PIN, 255);
}