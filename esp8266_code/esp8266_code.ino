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

#include <SoftwareSerial.h>
// Khai báo các chân RX và TX cho espsoftwareserial
// https://www.electronicwings.com/nodemcu/nodemcu-uart-with-esplorer-ide
// esp8266 co 17/16 va 1/3 (TX/RX)
#define RX_PIN 16
#define TX_PIN 17

// Tạo một đối tượng espsoftwareserial với tốc độ baud là 9600
SoftwareSerial arduinoSerial(RX_PIN, TX_PIN);

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

enum{
  ROOF_RUNNING,
  ROOF_IDLE
} ;
int m_roofStatus = ROOF_IDLE;

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

void setup()
{

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
}


int readHuminity()
{
  int value = analogRead(A0); // Huminity analog plug into A0
  int percent = map(value, 0, 1023, 0, 100);
  Serial.println("Huminity value: ");
  Serial.println(percent);
  return percent;
}

void startPump()
{
  Serial.println(" Start Pump !!!");
  m_pumpStatus = ePUMP_STARTED;
}

void stopPump()
{
  Serial.println(" Stop Pump !!!");
  m_pumpStatus = ePUMP_STOPED;
}

bool detectRain()
{
  return false;
}

void openRoof()
{
  m_roofStatus = eROOF_OPENED;
}

void closeRoof()
{
  m_roofStatus = eROOF_CLOSED;
}

void handleUART_fromArduino()
{
  // Check if Nano sends something
  if (arduinoSerial.available()) {
    String msg = arduinoSerial.readStringUntil('\n');
    arduinoSerial.print("Got from Nano: ");
    arduinoSerial.println(msg);

    msg.toUpperCase(); // Normalize to uppercase
    int spaceIndex = msg.indexOf(' ');

    if (spaceIndex == -1) {
      arduinoSerial.println("Invalid command. Use: CW <steps> or CCW <steps>");
      return;
    }

    String dir = msg.substring(0, spaceIndex);
    int steps = msg.substring(spaceIndex + 1).toInt();

    if (steps <= 0) {
      arduinoSerial.println("Invalid step count.");
      return;
    }
  }
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
  
    String prefix = "/GardenControl";
    String huminityPathToFireBase = prefix + "/Huminity";

    Serial.printf("Push huminity to Firebase... %s\n", Firebase.setFloat(fbdo, huminityPathToFireBase.c_str(), m_huminity) ? "ok" : fbdo.errorReason().c_str());
  
    String temperaturePathToFireBase = prefix + "/Temperature";
    Serial.printf("Push temperature to Firebase... %s\n", Firebase.setFloat(fbdo, temperaturePathToFireBase.c_str(), m_temperature) ? "ok" : fbdo.errorReason().c_str());

    String pumpstatusPathToFireBase = prefix + "/Pump_Status";
    Serial.printf("Push temperature to Firebase... %s\n", Firebase.setString(fbdo, pumpstatusPathToFireBase.c_str(), m_pumpstatus) ? "ok" : fbdo.errorReason().c_str());
  }
}

void loop()
{
    m_huminity = readHuminity();
    #define HUM_PUMP_LIMIT    (40) // Percent

    if (huminity > HUM_PUMP_LIMIT)
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

    // Serial.printf("Get float... %s\n", Firebase.getFloat(fbdo, F("/test/float")) ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());

    // Serial.printf("Set double... %s\n", Firebase.setDouble(fbdo, F("/test/double"), count + 35.517549723765) ? "ok" : fbdo.errorReason().c_str());

    // Serial.printf("Get double... %s\n", Firebase.getDouble(fbdo, F("/test/double")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());

    // Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/test/string"), "Hello World!") ? "ok" : fbdo.errorReason().c_str());

    // Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

    // // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Parse_Edit.ino
    // FirebaseJson json;

    // if (count == 0)
    // {
    //   json.set("value/round/" + String(count), F("cool!"));
    //   json.set(F("vaue/ts/.sv"), F("timestamp"));
    //   Serial.printf("Set json... %s\n", Firebase.set(fbdo, F("/test/json"), json) ? "ok" : fbdo.errorReason().c_str());
    // }
    // else
    // {
    //   json.add(String(count), "smart!");
    //   Serial.printf("Update node... %s\n", Firebase.updateNode(fbdo, F("/test/json/value/round"), json) ? "ok" : fbdo.errorReason().c_str());
    // }

    Serial.println();

    // For generic set/get functions.

    // For generic set, use Firebase.set(fbdo, <path>, <any variable or value>)

    // For generic get, use Firebase.get(fbdo, <path>).
    // And check its type with fbdo.dataType() or fbdo.dataTypeEnum() and
    // cast the value from it e.g. fbdo.to<int>(), fbdo.to<std::string>().

    // The function, fbdo.dataType() returns types String e.g. string, boolean,
    // int, float, double, json, array, blob, file and null.

    // The function, fbdo.dataTypeEnum() returns type enum (number) e.g. firebase_rtdb_data_type_null (1),
    // firebase_rtdb_data_type_integer, firebase_rtdb_data_type_float, firebase_rtdb_data_type_double,
    // firebase_rtdb_data_type_boolean, firebase_rtdb_data_type_string, firebase_rtdb_data_type_json,
    // firebase_rtdb_data_type_array, firebase_rtdb_data_type_blob, and firebase_rtdb_data_type_file (10)

    // count++;
  // }
}
