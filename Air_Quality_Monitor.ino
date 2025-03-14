#include "arduino_secrets.h"
#include "thingProperties.h"
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQ7.h>

// DHT sensor definitions
#define DHTpin 13
#define DHTTYPE DHT11
DHT dht(DHTpin, DHTTYPE);

// OLED display definitions
#define SCREEN_WIDTH 128  // OLED display width in pixels
#define SCREEN_HEIGHT 64  // OLED display height in pixels
#define OLED_RESET    -1  // Reset pin (or -1 if sharing Arduino reset)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define sensor pins for ESP32
int mq135SensorPin = 34;    // MQ135 sensor analog pin (general air quality)
int mq7SensorPin   = 35;    // MQ7 sensor analog pin (CO measurement)

// Instantiate MQ7 sensor object with analog pin and input voltage (5V)
MQ7 mq7(mq7SensorPin, 5.0);

// Calibration parameters for MQ135 (using linear interpolation)
const int MQ135_MIN_RAW = 150;   // Hypothetical raw value for 10 ppm (clean air)
const int MQ135_MAX_RAW = 3500;  // Hypothetical raw value for 1000 ppm

// Function to average multiple analog readings (for MQ135)
int readRawValue(int pin, int samples = 10, int delayMs = 10) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(delayMs);
  }
  return sum / samples;
}

// Convert MQ135 raw value to ppm (linear interpolation)
float convertMQ135ToPPM(int rawValue) {
  if (rawValue <= MQ135_MIN_RAW) {
    return 10.0;
  } else if (rawValue >= MQ135_MAX_RAW) {
    return 1000.0;
  } else {
    float ppm = 10.0 + (rawValue - MQ135_MIN_RAW) * (1000.0 - 10.0) / (MQ135_MAX_RAW - MQ135_MIN_RAW);
    return ppm;
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000); // Allow time for Serial monitor initialization
  
  // Initialize the DHT sensor
  dht.begin();

  // Initialize cloud properties (defined in thingProperties.h)
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is common I2C address for OLED
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Halt if OLED not detected
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Air Quality Monitor");
  display.display();
  delay(2000);

  // Initialize MQ7 sensor (the library's init function may handle heater timing if needed)
}

void loop() {
  ArduinoCloud.update();
  
  // Read temperature and humidity from DHT11
  float humidityValue = dht.readHumidity();
  float temperatureValue = dht.readTemperature();
  
  if (isnan(humidityValue) || isnan(temperatureValue)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("----------------");
    Serial.print("Humidity: ");
    Serial.println(humidityValue);
    Serial.print("Temperature: ");
    Serial.println(temperatureValue);
    
    humidity = humidityValue;
    temperature = temperatureValue;
  }
  
  // Read and average raw value from the MQ135 sensor and convert to ppm
  int rawAirQuality = readRawValue(mq135SensorPin);
  float airQualityPPM = convertMQ135ToPPM(rawAirQuality);
  Serial.print("Raw Air Quality (MQ135): ");
  Serial.println(rawAirQuality);
  Serial.print("Air Quality (ppm): ");
  Serial.println(airQualityPPM);
  airquality = airQualityPPM;
  
  // Read CO concentration using MQ7 library (this function returns ppm)
  float coPPM = mq7.getPPM();
  Serial.print("MQ7 CO (ppm): ");
  Serial.println(coPPM);
  carbonMonoxide = coPPM;
  
  // Update message string (cloud variable)
  message = "Temp=" + String(temperatureValue, 1) + "C  Hum=" + String(humidityValue, 0) +
            "%  AQ=" + String(airQualityPPM, 0) + "ppm  CO=" + String(coPPM, 0) + "ppm";
  
  // Display Temperature reading
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Temperature:");
  display.println("------------");
  display.setTextSize(2);
  display.println(String(temperatureValue, 1) + " C");
  display.display();
  delay(2000);
  
  // Display Humidity reading
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Humidity:");
  display.println("---------");
  display.setTextSize(2);
  display.println(String(humidityValue, 0) + "%");
  display.display();
  delay(2000);
  
  // Display Air Quality reading
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Air Quality:");
  display.println("------------");
  display.setTextSize(2);
  display.println(String(airQualityPPM, 0) + " ppm");
  display.display();
  delay(2000);
  
  // Display Carbon Monoxide reading
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("CO Concentration:");
  display.println("-----------------");
  display.setTextSize(2);
  display.println(String(coPPM, 0) + " ppm");
  display.display();
  delay(2000);
}

/*
  Callback functions triggered by changes from Arduino IoT Cloud.
*/

void onTemperatureChange() {
  // Code to execute when Temperature variable is updated from the cloud
}

void onHumidityChange() {
  // Code to execute when Humidity variable is updated from the cloud
}

void onAirqualityChange() {
  // Code to execute when Air Quality variable is updated from the cloud
}

void onCarbonMonoxideChange() {
  // Code to execute when Carbon Monoxide variable is updated from the cloud
}

void onMessageChange() {
  // Code to execute when Message variable is updated from the cloud
}
