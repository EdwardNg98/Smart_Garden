#include <SoftwareSerial.h>

// Pin definitions
const int m_receiveRequestPin = 5; 
const int m_sendDoneFeedbackPin = 6; 

const int stepPin = 7; 
const int dirPin  = 2; 
const int enPin   = 8;

int limitSwitchPin_1 = 3; 
int limitSwitchPin_2 = 4;


enum {
  eMOVE_OPEN = HIGH,
  eMOVE_CLOSE = LOW,
};

void setup() {
  pinMode(m_sendDoneFeedbackPin, OUTPUT);
  // pinMode(reservedPin, OUTPUT);
  // digitalWrite(reservedPin, LOW);

  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);

  digitalWrite(m_sendDoneFeedbackPin, LOW);

  pinMode(m_receiveRequestPin, INPUT_PULLUP); // Bật trở kéo lên
  digitalWrite(enPin, LOW); // Enable motor driver

  pinMode(limitSwitchPin_1, INPUT_PULLUP); // Bật trở kéo lên
  pinMode(limitSwitchPin_2, INPUT_PULLUP); // Bật trở kéo lên


  Serial.begin(9600);

  Serial.println("Hello from Arduino Nano!");
}

bool checkTouchStatus()
{
  if (digitalRead(limitSwitchPin_1) == LOW)
  {
    Serial.println("Sensor 1 is touching !!!");
    // return true;
  }

  if(digitalRead(limitSwitchPin_2) == LOW)
  {
    Serial.println("Sensor 2 is touching !!!");
  }

  return false;
}

void loop() 
{
  // checkTouchStatus();
  if(digitalRead(m_receiveRequestPin) == HIGH)
  {
    if (digitalRead(limitSwitchPin_1) == LOW) // Roof is opened
    {

      while(digitalRead(limitSwitchPin_2) == HIGH)
      {
        runMotor(eMOVE_CLOSE);
      }
        
    } else // if (digitalRead(limitSwitchPin_2) == LOW) // Roof is closed
    {
      while(digitalRead(limitSwitchPin_1) == HIGH) {
        runMotor(eMOVE_OPEN);
      }
    }

    digitalWrite( m_sendDoneFeedbackPin, HIGH);

    while(digitalRead(m_receiveRequestPin) == HIGH)
    {
      // Waiting to request pin to LOW.
      digitalWrite( m_sendDoneFeedbackPin, LOW);
    }
  }
}

void runMotor(int direction) {
  digitalWrite(dirPin, direction);

  #define STEP 1

  for (int i = 0; i < STEP; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }

  // delay(10);
}