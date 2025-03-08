#include "arduino_secrets.h"
#include "thingProperties.h"
#include "DHT.h"

#define DHTpin 13 // enter your pin
#define DHTTYPE DHT11
DHT dht(DHTpin, DHTTYPE);

int mqSensorPin = 34;  // MQ135 sensor analog pin (enter your pin)

// Calibration parameters (adjust these values based on your calibration data)
const int MQ135_MIN_RAW = 150;
const int MQ135_MAX_RAW = 3500;

// Function to average multiple analog readings for the MQ135 sensor
int readRawAirQuality() {
  const int samples = 10;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(mqSensorPin);
    delay(10);  // Short delay between readings
  }
  return sum / samples;
}

// Function to convert raw analog value to ppm based on a linear approximation.
// The result is constrained between 10 ppm and 1000 ppm.
float convertToPPM(int rawValue) {
  if (rawValue <= MQ135_MIN_RAW) {
    return 10.0;
  } else if (rawValue >= MQ135_MAX_RAW) {
    return 1000.0;
  } else {
    // Linear interpolation between the min and max raw values.
    float ppm = 10.0 + (rawValue - MQ135_MIN_RAW) * (1000.0 - 10.0) / (MQ135_MAX_RAW - MQ135_MIN_RAW);
    return ppm;
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000); // Wait for the serial monitor to initialize
  
  // Initialize the DHT sensor
  dht.begin();

  // Initialize cloud properties (defined in thingProperties.h)
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  
  // Read temperature and humidity from DHT11
  float humidityValue = dht.readHumidity();
  float temperatureValue = dht.readTemperature();
  
  // Validate sensor readings before updating the cloud
  if (isnan(humidityValue) || isnan(temperatureValue)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.println(humidityValue);
    Serial.print("Temperature: ");
    Serial.println(temperatureValue);
    
    humidity = humidityValue;
    temperature = temperatureValue;
  }
  
  // Read and average raw value from the MQ135 sensor
  int rawAirQuality = readRawAirQuality();
  // Convert the raw value to gas concentration (ppm)
  float airQualityPPM = convertToPPM(rawAirQuality);
  
  Serial.print("Raw Air Quality: ");
  Serial.println(rawAirQuality);
  Serial.print("Air Quality (ppm): ");
  Serial.println(airQualityPPM);
  
  // Update the cloud variable with the computed ppm value
  airquality = airQualityPPM;
  
  // Create a combined message string
  message = "Temperature = " + String(temperatureValue) +
            "  Humidity = " + String(humidityValue) +
            "  Air Quality = " + String(airQualityPPM) + " ppm";
            
  // Optional delay to manage sensor read intervals and cloud update frequency
  delay(2000);
}

void onTemperatureChange() {
  // Code to execute when the Temperature variable is updated from the cloud
}

void onHumidityChange() {
  // Code to execute when the Humidity variable is updated from the cloud
}

void onAirqualityChange() {
  // Code to execute when the Air Quality variable is updated from the cloud
}

void onMessageChange() {
  // Code to execute when the Message variable is updated from the cloud
}
