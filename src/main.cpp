#include <Arduino.h>

// Motor A
const int pwmA = 4;
const int Ain1 = 5;
const int Ain2 = 6;

// Motor B
const int pwmB = 7;
const int Bin1 = 15;
const int Bin2 = 16;

// PWM channels
const int channelA = 0;
const int channelB = 1;

void setup() {

  pinMode(Ain1, OUTPUT);
  pinMode(Ain2, OUTPUT);
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);

  // OLD ESP32 PWM setup (THIS IS THE CORRECT ONE FOR YOU)
  ledcSetup(channelA, 1000, 8);
  ledcSetup(channelB, 1000, 8);

  ledcAttachPin(pwmA, channelA);
  ledcAttachPin(pwmB, channelB);

  // Forward direction
  digitalWrite(Ain1, HIGH);
  digitalWrite(Ain2, LOW);

  digitalWrite(Bin1, HIGH);
  digitalWrite(Bin2, LOW);
}

void loop() {

  // START full speed
  ledcWrite(channelA, 255);
  ledcWrite(channelB, 255);

  delay(3000);

  // STOP
  ledcWrite(channelA, 0);
  ledcWrite(channelB, 0);

  delay(3000);
}