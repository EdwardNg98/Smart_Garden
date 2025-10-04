#include <SoftwareSerial.h>

// Pin definitions
const int stepPin = 7; 
// const int dirPin  = 2; 
// const int enPin   = 8;

int limitSwitchPin_1 = 0; 
int limitSwitchPin_2 = 1;

int limitSwitchFeedbackPin_1 = 3;
int limitSwitchFeedbackPin_2 = 4;

void setup() {
  pinMode(stepPin, OUTPUT); 
  // pinMode(dirPin, OUTPUT);
  // pinMode(enPin, OUTPUT);

  pinMode(limitSwitchPin_1, INPUT_PULLUP); // Bật trở kéo lên
  pinMode(limitSwitchPin_2, INPUT_PULLUP); // Bật trở kéo lên

  pinMode(limitSwitchFeedbackPin_1, OUTPUT); 
  pinMode(limitSwitchFeedbackPin_2, OUTPUT);

  // digitalWrite(enPin, LOW); // Enable motor driver

  Serial.begin(9600);

  Serial.println("Hello from Arduino Nano!");
}

void loop() {
  // Check if ESP8266 replied
  Serial.println("Arduino nano is aliving !!!");

  digitalWrite(limitSwitchFeedbackPin_1, digitalRead(limitSwitchPin_1));
  digitalWrite(limitSwitchFeedbackPin_2, digitalRead(limitSwitchPin_2));

  controlMotor();
}

void controlMotor() {
  #define STEP 30

  for (int i = 0; i < STEP; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
}

// void rotateStepper(int direction, int steps) {
  // digitalWrite(dirPin, direction);

//   for (int i = 0; i < steps; i++) {
//     digitalWrite(stepPin, HIGH);
//     delayMicroseconds(500);
//     digitalWrite(stepPin, LOW);
//     delayMicroseconds(500);
//   }
// }