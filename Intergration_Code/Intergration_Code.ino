#include <DHT.h>

// Chân kết nối DHT11
#define DHTPIN 2       // GPIO2 = D4 trên NodeMCU
#define DHTTYPE DHT11  // Loại cảm biến: DHT11 hoặc DHT22
DHT dht(DHTPIN, DHTTYPE);


float m_huminity = 0.0;
float m_temperature = 0.0;

// Pin definitions
const int motorRequestPin = 13; 
// const int arduinoFeedbackPin = 13; 

void setup() {
  // Pin definitions
  const int motorRequestPin = 13; 
  // const int arduinoFeedbackPin = 13; 

  // pinMode(rainSensorPin, INPUT_PULLUP); // Bật trở kéo lên

  // Initialize hardware Serial at 115200 baud (default for ESP8266)
  Serial.begin(115200);

  // Wait a little for Serial monitor to connect
  delay(1000);

  Serial.println("ESP8266 Serial Print Example");

  dht.begin();
}

float readHuminity()
{
  float value = analogRead(A0); // Huminity analog plug into A0
  float percent = map(value, 1023, 0, 20, 100);
  Serial.print("Huminity value: ");
  Serial.println(percent);
  return percent;
}

void loop() {
  readHuminity();
  float m_temperature = dht.readTemperature();   // đọc nhiệt độ (°C)
  float t = dht.readTemperature();

  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity from DHT11: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  // Print text
  // Serial.print("Millis since start: ");
  // Serial.println(millis());

  delay(1000); // wait 1 second
}
