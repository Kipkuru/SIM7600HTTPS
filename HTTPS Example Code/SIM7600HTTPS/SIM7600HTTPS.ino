#include <SIM7600HTTPS.h>  // Library for HTTP GET and POST with SIM7600 module

#define SerialAT Serial1  // Serial port for SIM7600 communication (Serial1 for Arduino Mega)
const char* apn = "safaricom";  // APN for GPRS connection (e.g., "safaricom" for Safaricom network)

const char* server = "https://voyager.mazimobility.com/inventory-management-service";  // Public HTTPS endpoint for testing
const char* resourceGet = "/api/v1/charging-station-mail/3/latest";  // GET endpoint to retrieve a sample post
const char* resourcePost = "/api/v1/charging-stations-status";   // POST endpoint to submit data
const char* postData = "{\"title\":\"Generic Test Post\",\"body\":\"This is a generic post for testing API endpoints.\",\"userId\":1}";  // Generic JSON data for POST
unsigned long previousMillis = 0;  // Tracks last request time
const long interval = 10000;  // Interval for HTTP requests (10 seconds)
SIM7600HTTPS modem;  // SIM7600HTTPS object for HTTP operations

void setup() {
  Serial.begin(115200);  // Initialize serial for debugging
  SerialAT.begin(115200);  // Initialize serial for SIM7600 module
  delay(1000);  // Brief delay for serial stabilization
  
  // Initialize modem and connect to GPRS
  if (modem.init()) {
    if (modem.gprsConnect(apn)) {
      Serial.println("SUCCESS");  // GPRS connection established
    } else {
      Serial.println("GPRS connection failed");  // GPRS connection error
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();  // Current time for timing logic

  // GET request every 2 seconds
  static unsigned long previousMillisGet = 0;
  const long getInterval = 2000;  // 2 seconds
  if (currentMillis - previousMillisGet >= getInterval) {
    previousMillisGet = currentMillis;  // Update last GET time

    if (modem.httpInit(server, resourceGet)) {  // Initialize HTTP session for GET
      String serverResponse;
      if (modem.httpGet(serverResponse)) {  // Execute GET request
        Serial.println("GET Response: " + serverResponse);  // Print server response
      }
    }
  }

  // POST request every 10 seconds
  static unsigned long previousMillisPost = 0;
  const long postInterval = 10000;  // 10 seconds
  if (currentMillis - previousMillisPost >= postInterval) {
    previousMillisPost = currentMillis;  // Update last POST time

    if (modem.httpInit(server, resourcePost, 1)) {  // Initialize HTTP session for POST
      String serverResponse;
      if (modem.httpPost(postData, serverResponse)) {  // Execute POST request with JSON data
        Serial.println("POST Response: " + serverResponse);  // Print server response
      }
    }
  }
}