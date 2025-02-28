#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Dust sensor pins
#define PM_25_PIN 6

// MQ135 pins
#define MQ_PIN A2

// UV pins
#define UV_PIN A0

// DHT22 pins
#define DHTTYPE DHT22
#define DHTPIN 4

// LoRa Pins for Arduino Mega
#define SCK 52    // Clock (SPI)
#define MISO 50   // Master In Slave Out (SPI)
#define MOSI 51   // Master Out Slave In (SPI)
#define NSS 10    // Chip Select (NSS)
#define RST 9     // Reset pin
#define DIO0 2    // IRQ pin
#define FREQUENCY 433E6 // LoRa frequency

// DSM501A variables
unsigned long startTime = 0;
unsigned long lowPulseOccupancy = 0;
float dustConcentration = 0;
int AQI_PM25 = 0;

// Create DHT22 object
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  setupSensors();
  setupLoRa();
}

void loop() {
  readAndSendSensorData();
  delay(4000); // Send every 4 second
}

void setupSensors() {
  pinMode(PM_25_PIN, INPUT);
  dht.begin();
}

void setupLoRa() {
  LoRa.setPins(NSS, RST, DIO0);
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setTxPower(18);
  LoRa.setFrequency(433E6);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  
  Serial.println("LoRa Sender initialized");
}

void readAndSendSensorData() {
  float uvIntensity = readUVSensor();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float co2_ppm = readCO2Sensor();
  calculateDustConcentration();
  
  if (isnan(humidity)) humidity = 0.0;
  if (isnan(temperature)) temperature = 0.0;

  String data_packet = String(humidity) + " , " + String(temperature) + " , " + 
                        String(uvIntensity) + " , " + String(co2_ppm) + " , " + 
                        String(dustConcentration) + " , " + String(AQI_PM25);

  Serial.print("Sending: ");
  Serial.println(data_packet);
  sendLoRaPacket(data_packet);
}

float readUVSensor() {
  float uvAnalog = analogRead(UV_PIN);
  return uvAnalog * (3.3 / 1023.0);
}

float readCO2Sensor() {
  float co2 = analogRead(MQ_PIN);
  return (co2 - 100.0) * (2000.0 - 400.0) / (900.0 - 100.0) + 400.0;
}

void calculateDustConcentration() {
  unsigned long duration = pulseIn(PM_25_PIN, LOW);
  lowPulseOccupancy += duration;
  unsigned long elapsedTime = millis() - startTime;

  if (elapsedTime > 30000) { // Update every 30 seconds
    dustConcentration = (lowPulseOccupancy / (float)elapsedTime) * 100;
    dustConcentration = dustConcentration * 1.1; // Convert to PM2.5 µg/m³
    AQI_PM25 = calculateAQI(dustConcentration, 2.5);
    
    lowPulseOccupancy = 0;
    startTime = millis();
  }
}

int calculateAQI(float pm, float type) {
  int AQI_low[] = {0, 51, 101, 151, 201, 301, 401};
  int AQI_high[] = {50, 100, 150, 200, 300, 400, 500};
  float PM_low[] = {0.0, 12.1, 35.5, 55.5, 150.5, 250.5, 500.5};
  float PM_high[] = {12.0, 35.4, 55.4, 150.4, 250.4, 500.4, 999.9};

  for (int i = 0; i < 7; i++) {
    if (pm >= PM_low[i] && pm <= PM_high[i]) {
      return ((AQI_high[i] - AQI_low[i]) / (PM_high[i] - PM_low[i])) * (pm - PM_low[i]) + AQI_low[i];
    }
  }
  return 500;
}

void sendLoRaPacket(String data_packet) {
  LoRa.beginPacket();
  LoRa.print(data_packet);
  LoRa.endPacket();
  
  Serial.print("RSSI: ");
  Serial.print(LoRa.packetRssi());
  Serial.println(" dBm");

  Serial.print("SNR: ");
  Serial.print(LoRa.packetSnr());
  Serial.println(" dB");
}