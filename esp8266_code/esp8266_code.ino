#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "The Wow Coffee & Tea"
#define WIFI_PASSWORD "thewowxinchao"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDa9u4v4WPvSiM3iHENRMM6rq6ypjmlEz4"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://gardencontrol-us-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "thanhhiepvt412zz@gmail.com"
#define USER_PASSWORD "123456"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

enum{
  eROOF_RUNNING,
  eROOF_OPENING,
  eROOF_CLOSING,
  eROOF_IDLE
} ;

int m_roofMotorStatus = eROOF_IDLE;

enum {
  eROOF_CLOSED,
  eROOF_OPENED,
};

enum {
  ePUMP_STARTED,
  ePUMP_STOPED,
};

bool m_pumpStatus = ePUMP_STOPED;
bool m_roofStatus = eROOF_CLOSED;


// Pin definitions for motor
// const int stepPin = 7; 
const int dirPin  = 5; 
const int enPin   = 6;
const int rainSensorPin = 7;
const int relayForPumpPin = 0;

String serverList[3] = {
  "pool.ntp.org",
  "eu.pool.ntp.org",
  "asia.pool.ntp.org",
};

int serverGMTList[3] = {
  7,
  7,
  7
};


// NTP client setup: server, timezone offset in seconds (e.g., GMT+7 => 7*3600)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.asia.pool.ntp.org", 3600, 0);


int udpPort = 1000;
float m_huminity = 0.0;
float m_temperature = 0.0;

int limitSwitchPin_1 = 3; 
int limitSwitchPin_2 = 4;

void setup()
{
  // Motor setup
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);

  digitalWrite(enPin, HIGH); // Disble motor driver

  // Cam bien hanh trinh setup
  pinMode(limitSwitchPin_1, INPUT_PULLUP); // Bật trở kéo lên
  pinMode(limitSwitchPin_2, INPUT_PULLUP); // Bật trở kéo lên

  pinMode(rainSensorPin, INPUT_PULLUP); // Bật trở kéo lên

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize NTP
  timeClient.begin(udpPort);

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP8266#about-firebasedata-object
  // fbdo.keepAlive(5, 5, 1);

  /** Timeout options.

  //Network reconnect timeout (interval) in ms (10 sec - 5 min) when network or WiFi disconnected.
  config.timeout.networkReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */

  // Initialize the screen
  lcd.init();
  lcd.backlight();

  lcd.print("Welcome to the Smart Garden !!!");
  delay(100);
  lcd.clear();
}


float readHuminity()
{
  float value = analogRead(A0); // Huminity analog plug into A0
  float percent = map(value, 0, 1023, 0, 100);
  Serial.println("Huminity value: ");
  Serial.println(percent);
  return percent;
}

float readTemperature()
{
  // float value = analogRead(A0); // Huminity analog plug into A0
  // float percent = map(value, 0, 1023, 0, 100);
  // Serial.println("Temperature value: ");
  // Serial.println(percent);
  float value = 35.6;
  return value;
}

void startPump()
{
  Serial.println(" Start Pump !!!");
  m_pumpStatus = ePUMP_STARTED;

  digitalWrite(relayForPumpPin,HIGH);
}

void stopPump()
{
  Serial.println(" Stop Pump !!!");
  m_pumpStatus = ePUMP_STOPED;

  digitalWrite(relayForPumpPin,LOW);
}

bool detectRain()
{
  Serial.print("Gia tri cam bien mua: ");
  Serial.println(digitalRead(rainSensorPin));
  return digitalRead(rainSensorPin);
}


// Update the action
void openRoof()
{
  while (digitalRead(limitSwitchPin_1) == HIGH)
  {
    m_roofMotorStatus = eROOF_OPENING;

    Serial.println("Opening the Roof !!!!");

    digitalWrite(enPin, LOW); // Disble motor driver
    digitalWrite(dirPin, LOW); // Set dir
  }

  m_roofStatus = eROOF_OPENED;
  Serial.println("Completed opening the roof !!!!");
}

void closeRoof()
{
  while (digitalRead(limitSwitchPin_2) == HIGH)
  {
    m_roofMotorStatus = eROOF_CLOSING;

    Serial.println("Closing the Roof !!!!");

    digitalWrite(enPin, LOW); // Disble motor driver
    digitalWrite(dirPin, HIGH); // Set dir
  }

  m_roofStatus = eROOF_CLOSED;
  Serial.println("Completed closing the roof !!!!");
}

void handleFirebase()
{
  unsigned long epochTime;
  String hoursStr, minuteStr, secondStr;
  if (WiFi.status() != WL_CONNECTED) {
    // delay(500);
    Serial.println("Wifi disconnected");
  } else {
    if (timeClient.update()) // Fetch current time
    {
      epochTime = timeClient.getEpochTime(); // Unix timestamp
      Serial.print("Epoch Time: ");
      Serial.println(epochTime);

      Serial.print("Formatted Time: ");
      Serial.println(timeClient.getFormattedTime()); // e.g., "14:35:10"
      unsigned long hours = (epochTime % 86400L) / 3600;
      hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

      unsigned long minutes = (epochTime % 3600) / 60;
      minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

      unsigned long seconds = epochTime % 60;
      secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

      Serial.printf("%s:%s:%s\n", hoursStr, minuteStr, secondStr);
      
    } else 
    {
      Serial.println("NTP update time failed");
      // timeClient.begin(udpPort++);
      // Serial.print("Change UDP to new port : "); Serial.println(udpPort);
    }
  }

  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  // if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    delay(5000);
    
    String prefix;
    // String prefix = "/GardenSensor/" + hoursStr + minuteStr + secondStr;
    // String huminityPathToFireBase = prefix + "/Huminity";
    // String timePathToFireBase = prefix + "/time";

    // Should be remove in reality
    static int r1 = 0;
    r1 = r1+ 3; if (r1 > 40) r1=0;

    // String firebaseString = "13-09-2025;" + timeClient.getFormattedTime() + ";Huminity;" + String(huminity + r1);
    // Serial.printf("Push huminity to Firebase... %s\n", Firebase.setString(fbdo, huminityPathToFireBase.c_str(), firebaseString) ? "ok" : fbdo.errorReason().c_str());
    // Serial.printf("Push time to Firebase... %s\n", Firebase.setString(fbdo, timePathToFireBase.c_str(), timeClient.getFormattedTime()) ? "ok" : fbdo.errorReason().c_str());
  
    prefix = "/GardenControl";
    String huminityPathToFireBase = prefix + "/Huminity";

    Serial.printf("Push huminity to Firebase... %s\n", Firebase.setFloat(fbdo, huminityPathToFireBase.c_str(), m_huminity) ? "ok" : fbdo.errorReason().c_str());
  
    String temperaturePathToFireBase = prefix + "/Temperature";
    Serial.printf("Push temperature to Firebase... %s\n", Firebase.setFloat(fbdo, temperaturePathToFireBase.c_str(), m_temperature) ? "ok" : fbdo.errorReason().c_str());

    String pumpStatusPathToFireBase = prefix + "/Pump_Status";
    Serial.printf("Push pumpStatus to Firebase... %s\n", Firebase.setString(fbdo, pumpStatusPathToFireBase.c_str(), m_pumpStatus) ? "ok" : fbdo.errorReason().c_str());

    String roofMotorStatusPathToFireBase = prefix + "/RoofMotor_Status";
    Serial.printf("Push roofMotorStatus to Firebase... %s\n", Firebase.setString(fbdo, roofMotorStatusPathToFireBase.c_str(), m_roofMotorStatus) ? "ok" : fbdo.errorReason().c_str());
  }
}

void updateOnLcd()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("%Huminity : ");
  lcd.print(m_huminity);

  // Enter
  lcd.setCursor(2,1);

  lcd.print(m_temperature);
  lcd.print((char)223); // ký hiệu độ °
  lcd.print("C :");

  lcd.print(100);
  lcd.print("%");
}

void loop()
{
  m_huminity = readHuminity();
  m_temperature = readTemperature();

  #define HUM_PUMP_LIMIT    (40) // Percent

  if (m_huminity < HUM_PUMP_LIMIT)
    startPump();
  else {
    if (m_pumpStatus == ePUMP_STARTED)
      stopPump();
  }

  if (detectRain())
  {
    closeRoof();
  } else {
    if (m_roofStatus == eROOF_CLOSED)
      openRoof();
  }

  updateOnLcd();
  delay(500);
}
