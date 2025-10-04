#include <SoftwareSerial.h>

// Pin definitions
const int m_receiveRequestPin = 5; 
const int m_sendDoneFeedbackPin = 6; 
const int m_dirReceivePin = 9; 

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
  pinMode(m_dirReceivePin, INPUT);
  pinMode(m_receiveRequestPin, INPUT_PULLUP); // Bật trở kéo lên

  // digitalWrite(reservedPin, LOW);

  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);

  digitalWrite(m_sendDoneFeedbackPin, LOW);

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

bool checkRequestStatus()
{
  if (digitalRead(m_receiveRequestPin) == HIGH)
  {
    Serial.println("Receive request touching !!!");
    // return true;
  }

  if(digitalRead(m_dirReceivePin) == LOW)
  {
    Serial.println("Dir request is LOW !!!");
  }

  if(digitalRead(m_dirReceivePin) == eMOVE_OPEN)
  {
    Serial.println("Dir request is eMOVE_OPEN !!!");
  } else {
    Serial.println("Dir request is eMOVE_CLOSED !!!");
  }

  return false;
}

void loop() 
{
  // checkTouchStatus();
  // checkRequestStatus();

  if(digitalRead(m_receiveRequestPin) == HIGH)
  {
    Serial.println("Receive the request from ESP !!!");
    if (digitalRead(m_dirReceivePin) == eMOVE_OPEN)
    {
      Serial.println ("Opening the roof !!!!");
    } else if (digitalRead(m_dirReceivePin) == eMOVE_CLOSE)
      Serial.println ("Closing the roof !!!!");

    if (digitalRead(m_dirReceivePin) == eMOVE_CLOSE) // Roof is opened
    {
goback_close:
      while(digitalRead(limitSwitchPin_2) == HIGH)
      {
        runMotor(eMOVE_CLOSE);
      }
      delay(100);
      // This goto to avoid missing the sensor read
      if (digitalRead(limitSwitchPin_2) == HIGH) {
        goto goback_close;
      }
    } 
    else if (digitalRead(m_dirReceivePin) == eMOVE_OPEN) // Roof is closed
    {
goback_open:
      while(digitalRead(limitSwitchPin_1) == HIGH) {
        runMotor(eMOVE_OPEN);
      }
      delay(100);
      // This goto to avoid missing the sensor read
      if (digitalRead(limitSwitchPin_1) == HIGH) {
        goto goback_open;
      }
        
    }
    Serial.println("Finish the movement and send done feedback");
    digitalWrite( m_sendDoneFeedbackPin, HIGH);

    while(digitalRead(m_receiveRequestPin) == HIGH);

    // Waiting to request pin to LOW.
    Serial.println("Pin donefeedback to LOW");
    digitalWrite( m_sendDoneFeedbackPin, LOW);
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