#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

#include <DHT.h>

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
const int rainSensorPin = 13; // D7 - GPIO13
const int relayForPumpPin = 16;

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


// Chân kết nối DHT11
#define DHTPIN 2       // GPIO2 = D4 trên NodeMCU
#define DHTTYPE DHT11  // Loại cảm biến: DHT11 hoặc DHT22
DHT dht(DHTPIN, DHTTYPE);

int udpPort = 1000;
float m_huminity = 0.0;
float m_temperature = 0.0;

enum {
  eMOVE_OPEN = HIGH,
  eMOVE_CLOSE = LOW,
};

const int motorRequestPin = 12; // D6
const int arduinoFeedbackDonePin = 14; // D5

const int dirRequestPin = 15; // D8

enum {
  eMANUAL_MODE,
  eAUTO_MODE,
};

String m_userMode = "Manual";

enum {
  eIDLE_COMMAND,
  eSTARTPUMP_COMMAND,
  eSTOPPUMP_COMMAND,
  eOPENROOF_COMMAND,
  eCLOSEROOF_COMMAND,
};

int m_userCommand = eIDLE_COMMAND;

enum {
  eRAINING,
  eNO_RAINING
};

void setup()
{
  // Initialize pump
  pinMode(relayForPumpPin, OUTPUT);
  digitalWrite(relayForPumpPin, LOW);

  // Motor setup
  pinMode(motorRequestPin, OUTPUT);
  digitalWrite(motorRequestPin, LOW);

  pinMode(dirRequestPin, OUTPUT);

  pinMode(arduinoFeedbackDonePin, INPUT_PULLUP);
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
  // DHT init
  dht.begin();

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
  float t = dht.readTemperature();

  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
 // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return m_temperature;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));

  return t;
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

  // 1 : means no rain, 0 : raining
  Serial.print("Gia tri cam bien mua: ");
  Serial.println(digitalRead(rainSensorPin));
  return digitalRead(rainSensorPin);
}


// Update the action
void openRoof()
{
  {
    m_roofMotorStatus = eROOF_OPENING;

    Serial.println("Opening the Roof !!!!");

    digitalWrite(motorRequestPin, HIGH);
    digitalWrite(dirRequestPin, eMOVE_OPEN);

    while(digitalRead(arduinoFeedbackDonePin) == LOW)
    {
      digitalWrite(motorRequestPin, LOW);
    }
  }

  m_roofStatus = eROOF_OPENED;
  Serial.println("Completed opening the roof !!!!");
}

void closeRoof()
{
  {
    m_roofMotorStatus = eROOF_CLOSING;

    Serial.println("Closing the Roof !!!!");

    digitalWrite(motorRequestPin, HIGH);
    digitalWrite(dirRequestPin, eMOVE_CLOSE);

    while(digitalRead(arduinoFeedbackDonePin) == LOW)
    {
      digitalWrite(motorRequestPin, LOW);
    }
  }

  m_roofStatus = eROOF_CLOSED;
  Serial.println("Completed closing the roof !!!!");
}

void pushDataToFirebase()
{
  unsigned long epochTime;
  String hoursStr, minuteStr, secondStr;
  if (WiFi.status() != WL_CONNECTED) {
    // delay(500);
    Serial.println("Wifi disconnected");
  } else {
    // if (timeClient.update()) // Fetch current time
    // {
    //   epochTime = timeClient.getEpochTime(); // Unix timestamp
    //   Serial.print("Epoch Time: ");
    //   Serial.println(epochTime);

    //   Serial.print("Formatted Time: ");
    //   Serial.println(timeClient.getFormattedTime()); // e.g., "14:35:10"
    //   unsigned long hours = (epochTime % 86400L) / 3600;
    //   hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

    //   unsigned long minutes = (epochTime % 3600) / 60;
    //   minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    //   unsigned long seconds = epochTime % 60;
    //   secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

    //   Serial.printf("%s:%s:%s\n", hoursStr, minuteStr, secondStr);
      
    // } else 
    // {
    //   Serial.println("NTP update time failed");
    //   // timeClient.begin(udpPort++);
    //   // Serial.print("Change UDP to new port : "); Serial.println(udpPort);
    // }
  }

  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  // if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    // sendDataPrevMillis = millis();
    // delay(5000);
    
    String prefix;
    // String prefix = "/GardenSensor/" + hoursStr + minuteStr + secondStr;
    // String huminityPathToFireBase = prefix + "/Huminity";
    // String timePathToFireBase = prefix + "/time";

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

void getDataFromFirebase()
{
  String prefix;
  prefix = "/GardenControl";
  String userModePathToFireBase = prefix + "/UserMode";

  if (Firebase.getString(fbdo, userModePathToFireBase.c_str())) {  // successful
    m_userMode = fbdo.stringData();   // get value as String
    Serial.println("User Mode from Firebase: " + m_userMode);
  } else {
    Serial.println("Error: " + fbdo.errorReason());
  }

  String userCommandPathToFireBase = prefix + "/UserCommand";

  if (Firebase.getInt(fbdo, userCommandPathToFireBase.c_str())) {  // successful
    m_userCommand = fbdo.intData();   // get value as String
    Serial.println("User Command from Firebase: " + m_userCommand);
  } else {
    Serial.println("Error: " + fbdo.errorReason());
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

  if ((m_userMode == "Auto" && m_huminity < HUM_PUMP_LIMIT) || 
        (m_userMode == "Manual" && m_userCommand == eSTARTPUMP_COMMAND)
    )
  {
    startPump();
  }
  else if ((m_userMode == "Auto" && m_huminity >= HUM_PUMP_LIMIT) || 
        (m_userMode == "Manual" && m_userCommand == eSTOPPUMP_COMMAND)
      )
  {
    stopPump();
  }

  // Serial.print("User Mode : ");
  // Serial.print(m_userMode);
  // Serial.print("User Command : ");
  // Serial.println(m_userCommand);

  // if (detectRain() == eRAINING)
  // {
  //   closeRoof();
  // } else {
  //   if (m_roofStatus == eROOF_CLOSED)
  //     openRoof();
  // }

  updateOnLcd();

  getDataFromFirebase();
  // pushDataToFirebase();
  
  delay(50);
}
