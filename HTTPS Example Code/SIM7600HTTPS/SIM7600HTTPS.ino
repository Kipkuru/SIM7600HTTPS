#include <SIM7600HTTPS.h>  // Library for HTTP GET and POST with SIM7600 module

#define SerialAT Serial1  // Serial port for SIM7600 communication (Serial1 for Arduino Mega)
const char* apn = "safaricom";  // APN for GPRS connection (e.g., "safaricom" for Safaricom network)

const char* server = "https://jsonplaceholder.typicode.com";  // Public HTTPS endpoint for testing
const char* resourceGet = "/posts/1";  // GET endpoint to retrieve a sample post
const char* resourcePost = "/posts";   // POST endpoint to submit data
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

  // Perform HTTP requests every 10 seconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update last request time

    // Perform HTTP GET request
    if (modem.httpInit(server, resourceGet)) {  // Initialize HTTP session for GET
      String serverResponse;
      if (modem.httpGet(serverResponse)) {  // Execute GET request
        Serial.println("GET Response: " + serverResponse);  // Print server response
        modem.httpTerm();  // Terminate HTTP session
      }
    }

    delay(3000);  // Delay between GET and POST requests

    // Perform HTTP POST request
    if (modem.httpInit(server, resourcePost)) {  // Initialize HTTP session for POST
      String serverResponse;
      if (modem.httpPost(postData, serverResponse)) {  // Execute POST request with JSON data
        Serial.println("POST Response: " + serverResponse);  // Print server response
        modem.httpTerm();  // Terminate HTTP session
      }
    }
  }
}