#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define SS 5      // LoRa module NSS/CS pin
#define RST 14    // LoRa module RST pin
#define DIO0 2    // LoRa module DIO0 pin

// WiFi credentials
#define WIFI_SSID "Thirsday"
#define WIFI_PASSWORD "abcdefgh"

// Firebase credentials
#define FIREBASE_HOST "https://systeme-strasse-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_API_KEY "AIzaSyCe-yzbhTygglWS-TeLGHBEpdsszigClpI"

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("Starting LoRa Receiver...");

    LoRa.setPins(SS, RST, DIO0); // Set LoRa module pins

    if (!LoRa.begin(433E6)) { // Change to 433E6 or 868E6 based on your module
        Serial.println("LoRa initialization failed!");
        while (1);
    }

    Serial.println("LoRa Receiver Ready!");

    // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");

  // Configure Firebase
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;
  auth.user.email = "offiziell.thiranjaya@gmail.com";
  auth.user.password = "787898";
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Connected to Firebase!");
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.println("Received packet: ");

        String receivedData = "";
        
        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }

        Serial.println("Data: " + receivedData);

        // Parse sensor data
    int comma1 = receivedData.indexOf(',');
    int comma2 = receivedData.indexOf(',', comma1 + 1);
    int comma3 = receivedData.indexOf(',', comma2 + 1);
    int comma4 = receivedData.indexOf(',', comma3 + 1);
    int comma5 = receivedData.indexOf(',', comma4 + 1);

    if (comma1 == -1 || comma2 == -1 || comma3 == -1 || comma4 == -1 || comma5 == -1)
    {
      Serial.println("Invalid data format, skipping...");
      return;
    }

    float humidity = receivedData.substring(0, comma1).toFloat();
    float temperature = receivedData.substring(comma1 + 1, comma2).toFloat();
    float uv = receivedData.substring(comma2 + 1, comma3).toFloat();
    float co2 = receivedData.substring(comma3 + 1, comma4).toFloat();
    float dust = receivedData.substring(comma4 + 1).toFloat();
    int aqi = receivedData.substring(comma5 + 1).toInt();

    // Log sensor data
    Serial.printf("Humidity: %.2f %%\n", humidity);
    Serial.printf("Temperature: %.2f C\n", temperature);
    Serial.printf("UV: %.2f µW/cm²\n", uv);
    Serial.printf("CO2: %.2f ppm\n", co2);
    Serial.printf("Dust: %.2f µg/m³\n", dust);
    Serial.printf("AQI: %d\n", aqi);

    // Send data to Firebase
    FirebaseJson json;
    json.set("humidity", humidity);
    json.set("temperature", temperature);
    json.set("uv", uv);
    json.set("co2", co2);
    json.set("dust", dust);
    json.set("aqi", aqi);

    if (Firebase.RTDB.pushJSON(&firebaseData, "/sensor_data", &json))
    {
      Serial.println("Data sent to Firebase!");
    }
    else
    {
      Serial.println("Firebase error: " + firebaseData.errorReason());
    }

        Serial.print(" | RSSI: ");
        Serial.println(LoRa.packetRssi());
        Serial.println("=======");
    }
}
