#ifndef SIM7600HTTPS_H  // Prevent multiple inclusions
#define SIM7600HTTPS_H

#include <Arduino.h>  // Include Arduino core for Serial, String, etc.

// Notes:
// - Requires SerialMon and SerialAT to be defined in the .ino (e.g., #define SerialMon Serial, #define SerialAT Serial1)

// Define serial ports if not already defined in .ino
#ifndef SerialMon
  #define SerialMon Serial  // Default debug serial
#endif
#ifndef SerialAT
  #define SerialAT Serial1  // Default SIM7600 serial
#endif
// Define debug macro if not already defined in .ino
#ifndef DumpAtCommands
  #define DumpAtCommands  // Default to enabled; .ino can override by defining it first
#endif

class SIM7600HTTPS {
public:
  // Constructor
  SIM7600HTTPS();

  // Initialization and GPRS connection
  bool init();                  // Initialize modem (AT, SIM, signal, etc.)
  bool gprsConnect(const char* apn);  // Connect to GPRS with APN

  // HTTP operations
  bool httpInit(const char* server, const char* resource);      // Initialize HTTP with server URL
  bool httpGet(String& response);// Perform GET request on a resource
  bool httpPost(const char* data, String& response);  // Perform POST request with data
  bool httpTerm();                 // Terminate HTTP session

private:
  // Private helper methods (implementation in .cpp)
  String sendATCommand(const char* cmd, const char* expected, unsigned long timeout);
  String waitForResponse(const char* expected, unsigned long timeout);
  void clearSerialBuffer();
  //init AT commands
  void sendATCRESET(bool& success);  // New reset function
  void sendAT(bool& success);
  void sendATCPIN(bool& success);
  void checkCPINStatus(String response);
  void sendATCSQ(bool& success);
  //gprsconnect AT commands
  void sendATCGREG(bool& success);
  void sendATCNMP(bool& success);
  void sendATCOPS(bool& success);
  void sendATCGATT(bool& success);
  void sendATCGDCONT(bool& success, const char* apn);
  void sendATCGACT(bool& success);
  void sendATCGPADDR(bool& success);
  //https AT commands
  void sendATHTTPTERM(bool& success);
  void sendATHTTPINIT(bool& success);
  void sendATHTTPPARA(bool& success, const char* param, const char* value);
  void sendATHTTPDATA(bool& success, const char* data);
  void sendATHTTPACTION(bool& success, int method, int& responseLength);
  String readHTTPResponse(int responseLength, int timeout);
};

#endif  // End of include guard