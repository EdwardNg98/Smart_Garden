#include <SoftwareSerial.h>

// Pin definitions
const int stepPin = 5; 
const int dirPin  = 2; 
const int enPin   = 8;

int limitSwitchPin_1 = 3;    // Công tắc nối vào chân D2
int limitSwitchPin_2 = 4;    // Công tắc nối vào chân D2

enum{
  ROOF_RUNNING,
  ROOF_IDLE
} ;

int m_roof_status = ROOF_IDLE;

// Define pins for communication
SoftwareSerial espSerial(2, 3); // RX, TX  (D2 = RX, D3 = TX)

void setup() {
  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);

  digitalWrite(enPin, LOW); // Enable motor driver

  Serial.begin(9600);
  Serial.println("Stepper Motor Ready. Enter command like: CW 1800 or CCW 900");

  espSerial.println("Hello ESP8266 from Arduino Nano!");
  Serial.println("Message sent to ESP8266");

  pinMode(limitSwitchPin_1, INPUT_PULLUP); // Bật trở kéo lên
  pinMode(limitSwitchPin_2, INPUT_PULLUP); // Bật trở kéo lên
}

void loop() {
  // if (Serial.available()) {
  //   String input = Serial.readStringUntil('\n'); // Read line
  //   input.trim();  // Remove spaces/newlines

  // Check if ESP8266 replied
  if (espSerial.available()) {
    String response = espSerial.readStringUntil('\n'); // read response from ESP8266
    
    Serial.print("From ESP8266: ");
    Serial.println(response);

    response.trim();  // Remove spaces/newlines

    if (response.length() > 0) {
      processCommand(response);
    }

  }

  // Send Roof status : RUNNING/IDLE to ESP8266
  espSerial.print("Roof_status");
  espSerial.println(m_roof_status);

  delay(200);
}

void processCommand(String command) {
  command.toUpperCase(); // Normalize to uppercase
  int spaceIndex = command.indexOf(' ');

  if (spaceIndex == -1) {
    Serial.println("Invalid command. Use: CW <steps> or CCW <steps>");
    return;
  }

  String dir = command.substring(0, spaceIndex);
  int steps = command.substring(spaceIndex + 1).toInt();

  if (steps <= 0) {
    Serial.println("Invalid step count.");
    return;
  }

  if (dir == "CW") {
    while(digitalRead(limitSwitchPin_1) == HIGH)  // Công tắc hành trình thả ra.
    {
      rotateStepper(HIGH, steps);
      Serial.print("Rotated CW ");
      Serial.print(steps);
      Serial.println(" steps.");
      
      m_roof_status = ROOF_RUNNING;
    }

    
  } 
  else if (dir == "CCW") {
    while(digitalRead(limitSwitchPin_2) == HIGH)  // Công tắc hành trình thả ra.
    {
      rotateStepper(LOW, steps);
      Serial.print("Rotated CCW ");
      Serial.print(steps);
      Serial.println(" steps.");

      m_roof_status = ROOF_RUNNING;
    }
  } 
  else {
    Serial.println("Invalid direction. Use CW or CCW.");
  }

  m_roof_status = ROOF_IDLE;
}

void rotateStepper(int direction, int steps) {
  digitalWrite(dirPin, direction);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
}