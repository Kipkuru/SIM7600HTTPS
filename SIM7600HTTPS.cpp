#include "SIM7600HTTPS.h"

// Constructor
SIM7600HTTPS::SIM7600HTTPS() {
}

// Private: Generic AT command sender with flexible expected response
String SIM7600HTTPS::sendATCommand(const char* cmd, const char* expected, unsigned long timeout) {
  //clearSerialBuffer();  // Clear any residual data
  SerialAT.println(cmd);
  #ifdef DumpAtCommands
  SerialMon.print("Command: ");  
  SerialMon.println(cmd);       // Print command on timeout
  #else//print nothing
  #endif
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

      if (response.indexOf(expected) != -1 && 
          (response.indexOf("\n") > response.indexOf(expected) || response.endsWith(expected))) {
            #ifdef DumpAtCommands
            SerialMon.println("Response: ");  // Print on timeout           
            SerialMon.print(response);  // Dump raw response if defined
            #else//print nothing
           #endif
     SerialMon.println();
    return response;
      }
    }
    delay(10);  // Small delay to avoid tight loop
  }
  #ifdef DumpAtCommands
  SerialMon.println("Response: ");  // Print on timeout
  SerialMon.print(response);  // Dump raw response if defined
  #else//print nothing
 #endif
  return response;
}

// Private: Clear SerialAT buffer
void SIM7600HTTPS::clearSerialBuffer() {
  while (SerialAT.available()) {
    SerialAT.read();
  }
}

// Private: Send AT command
void SIM7600HTTPS::sendAT(bool& success) {
  String response = sendATCommand("AT", "OK", 5000);
  if (response.indexOf("OK") == -1) {
    SerialMon.println("Check GSM connection");  // Error if no OK
    success = false;
  }
}

// Private: Send AT+CPIN? command and check status
void SIM7600HTTPS::sendATCPIN(bool& success) {
    if (!success) return;  // Skip if previous step failed
    String response = sendATCommand("AT+CPIN?", "OK", 1000);  // Send AT+CPIN?, expect OK
    if (response.indexOf("+CPIN:") != -1 && response.indexOf("OK") != -1) {
      checkCPINStatus(response);  // Success: Check specific CPIN status
    } else {
      SerialMon.println("Error: SIM card response incomplete - Check SIM");
      success = false;  // Failure: Missing +CPIN: or OK
    }
  }

// Private: Check +CPIN: status message
void SIM7600HTTPS::checkCPINStatus(String response) {
  if (response.indexOf("+CPIN: READY") != -1) {
#ifndef DumpAtCommands
    SerialMon.println("SIM card ready");  // Success message
#else//print nothing
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

// Private: Send AT+CSQ command and check signal quality (Step 3)
void SIM7600HTTPS::sendATCSQ(bool& success) {
    if (!success) return;  // Skip if previous step failed
    String response = sendATCommand("AT+CSQ", "OK", 5000);
    if (response.indexOf("+CSQ:") == -1 || response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to get signal quality response");
      success = false;
    } else {
      int csqStart = response.indexOf("+CSQ:") + 6;
      int csqEnd = response.indexOf(",", csqStart);
      if (csqEnd == -1) csqEnd = response.indexOf("\r", csqStart);
      String rssiStr = response.substring(csqStart, csqEnd);
      int rssi = rssiStr.toInt();
  
      if (rssi < 10 || rssi == 99) {
        SerialMon.println("Error: Signal quality too weak (RSSI: " + String(rssi) + ")");
        success = false;
      } else {
  #ifndef DumpAtCommands
        SerialMon.println("Signal quality check passed (RSSI: " + String(rssi) + ")");
  #else
        SerialMon.println("Signal checked, RSSI: " + String(rssi));
  #endif
      }
    }
  }

// Public: Initialize modem (Step 1 and 2 - AT and CPIN checks)
void SIM7600HTTPS::init() {
    bool success = true;  // Start with success assumed
  sendAT(success);      // Step 1: Check basic communication
  sendATCPIN(success);  // Step 2: Check SIM status
  sendATCSQ(success);   // Step 3: Check signal quality
  
  #ifndef DumpAtCommands
  if (success) {
    SerialMon.println("GSM initialized successfully");  // Only if all steps pass
  }
  #else//print nothing
#endif
}