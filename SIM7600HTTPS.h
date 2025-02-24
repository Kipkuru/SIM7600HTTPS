#ifndef SIM7600HTTPS_H  // Prevent multiple inclusions
#define SIM7600HTTPS_H

#include <Arduino.h>  // Include Arduino core for Serial, String, etc.

// Define debug macro if not already defined in .ino
#ifndef DumpAtCommands
  #define DumpAtCommands  // Default to enabled; .ino can override by defining it first
#endif

class SIM7600HTTPS {
public:
  // Constructor
  SIM7600HTTPS();

  // Initialization and GPRS connection
  void init();                  // Initialize modem (AT, SIM, signal, etc.)
  void gprsConnect(const char* apn);  // Connect to GPRS with APN

  // HTTP operations
  void httpInit(const char* server);       // Initialize HTTP with server URL
  void httpGetResource(const char* resource);  // Perform GET request on a resource
  void httpPostData(const char* resource, const char* data);  // Perform POST request with data
  void httpTerm();                 // Terminate HTTP session

private:
  // Private helper methods (implementation in .cpp)
  void sendATCommand(const char* cmd, unsigned long timeout = 1000);  // Send AT command and wait for response
  String waitForResponse(const char* expected, unsigned long timeout);  // Wait for specific response
  void clearSerialBuffer();       // Clear SerialAT buffer
  
};

#endif  // End of include guard