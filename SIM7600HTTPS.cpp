#include "SIM7600HTTPS.h"

// Constructor
SIM7600HTTPS::SIM7600HTTPS() {
}

// Private: Generic AT command sender with flexible expected response
String SIM7600HTTPS::sendATCommand(const char* cmd, const char* expected, unsigned long timeout) {
  clearSerialBuffer();  // Clear any residual data
  SerialAT.println(cmd);
  return waitForResponse(expected, timeout);  // Wait for the specified response
}

// Private: Wait for specific response with debug handling
String SIM7600HTTPS::waitForResponse(const char* expected, unsigned long timeout) {
  String response = "";
  unsigned long startTime = millis();
  while (millis() - startTime < timeout) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
#ifdef DumpAtCommands
      SerialMon.print("Command: ");  
      SerialMon.println(expected); // Dump the expected command
      SerialMon.print("Response: ");  
      SerialMon.println(response);  // Dump raw response if defined
#endif
      if (response.indexOf(expected) != -1 && 
          (response.indexOf("\n") > response.indexOf(expected) || response.endsWith(expected))) {
        return response;
      }
    }
    delay(10);  // Small delay to avoid tight loop
  }
  return response;
}

// Private: Clear SerialAT buffer
void SIM7600HTTPS::clearSerialBuffer() {
  while (SerialAT.available()) {
    SerialAT.read();
  }
}

// Private: Send AT command
void SIM7600HTTPS::sendAT() {
  String response = sendATCommand("AT", "OK", 1000);
  if (response.indexOf("OK") == -1) {
    SerialMon.println("Check GSM connection");  // Error if no OK
  }
}

// Private: Send AT+CPIN? command and check status
void SIM7600HTTPS::sendATCPIN() {
    String response = sendATCommand("AT+CPIN?", "OK", 1000);  // Expect OK as final response
    if (response.indexOf("+CPIN: READY") != -1 && response.indexOf("OK") != -1) {
      checkCPINStatus(response);  // Check +CPIN: status only if both are present
    } else {
      SerialMon.println("Check GSM connection");  // Error if either +CPIN: READY or OK is missing
    }
  }

// Private: Check +CPIN: status message
void SIM7600HTTPS::checkCPINStatus(String response) {
  if (response.indexOf("+CPIN: READY") != -1) {
#ifndef DumpAtCommands
    SerialMon.println("SIM card ready");  // Success message
#endif
  } else if (response.indexOf("+CPIN: SIM PIN") != -1) {
    SerialMon.println("SIM card locked - Remove SIM PIN");  // Prompt user action
  } else if (response.indexOf("+CPIN: SIM PUK") != -1) {
    SerialMon.println("SIM locked (PUK required) - Contact provider for PUK code");
  } else if (response.indexOf("+CPIN: NOT READY") != -1) {
    SerialMon.println("SIM not ready - Check hardware or reinsert SIM");
  } else if (response.indexOf("+CPIN: PH-SIM PIN") != -1) {
    SerialMon.println("Phone locked to SIM - Use correct SIM or unlock device");
  } else if (response.indexOf("+CPIN: ERROR") != -1) {
    SerialMon.println("No SIM detected - Insert SIM card");
  } else {
    SerialMon.println("Unknown SIM status - Check SIM card");
  }
}

// Public: Initialize modem (Step 1 and 2 - AT and CPIN checks)
void SIM7600HTTPS::init() {
  sendAT();      // Step 1: Check basic communication
  sendATCPIN();  // Step 2: Check SIM status
  // Success message only if both pass (added later as steps complete)
}