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
    if (response.indexOf("+CSQ:") == -1 && response.indexOf("OK") == -1) {
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
//Private: Send AT+CGREG? (Step 4 - Network Registration Check)
void SIM7600HTTPS::sendATCGREG(bool& success) {
  if (!success) return;
  String response = sendATCommand("AT+CGREG?", "OK", 5000);
  if (response.indexOf("OK") == -1 || 
  (response.indexOf("+CGREG: 0,1") == -1 && response.indexOf("+CGREG: 0,5") == -1)) {
    SerialMon.println("Error: Not registered on network");
    success = false;
  }
#ifndef DumpAtCommands
  else {
    SerialMon.println("Network registration confirmed");
  }
#endif
}
//Private: Send AT+CNMP=38 (Step 5 - Set Preferred Mode to LTE)
void SIM7600HTTPS::sendATCNMP(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CNMP=38", "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to set preferred mode to LTE");
      success = false;
    }
  #ifndef DumpAtCommands
    else {
      SerialMon.println("Preferred mode set to LTE");
    }
  #endif
}
//Private: Send AT+COPS=0 (Step 6 - Set Operator Selection to Automatic)
void SIM7600HTTPS::sendATCOPS(bool& success) {
  if (!success) return;
  String response = sendATCommand("AT+COPS=0", "OK", 5000);
  if (response.indexOf("OK") == -1) {
    SerialMon.println("Error: Failed to set automatic operator selection");
    success = false;
  }
#ifndef DumpAtCommands
  else {
    SerialMon.println("Operator selection set to automatic");
  }
#endif
}

//Private: Send AT+CGATT=1 (Step 7 - Attach to GPRS)
void SIM7600HTTPS::sendATCGATT(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CGATT=1", "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to attach to GPRS");
      success = false;
    }
  #ifndef DumpAtCommands
    else {
      SerialMon.println("GPRS attached");
    }
  #endif
}

// Private: Send AT+CGDCONT with variable APN (Step 8)
void SIM7600HTTPS::sendATCGDCONT(bool& success, const char* apn) {
    if (!success) return;
    String cmd = "AT+CGDCONT=1,\"IP\",\"" + String(apn) + "\"";  // Construct command with apn
    String response = sendATCommand(cmd.c_str(), "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to set APN");
      success = false;
    }
  #ifndef DumpAtCommands
    else {
      SerialMon.println("APN set to " + String(apn));
    }
  #endif
  }
//Private: Send AT+CGACT=1,1 (Step 9 - Activate PDP Context)
void SIM7600HTTPS::sendATCGACT(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CGACT=1,1", "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to activate PDP context");
      success = false;
    }
  #ifndef DumpAtCommands
    else {
      SerialMon.println("PDP context activated");
    }
  #endif
}

//Private: Send AT+CGPADDR (Step 10 - Obtain IP Address)
void SIM7600HTTPS::sendATCGPADDR(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CGPADDR", "+CGPADDR: 1,", 5000);  // Expect +CGPADDR: 1,<number>
    if (response.indexOf("+CGPADDR: 1,") == -1) {
      SerialMon.println("Error: Failed to obtain IP address response");
      success = false;
      return;
    }
    int ipStart = response.indexOf("1,") + 2;  // Start after "1,"
    int ipEnd = response.indexOf("\r", ipStart);  // End before newline
    if (ipEnd == -1) ipEnd = response.length();  // Fallback if no newline
    String ipAddress = response.substring(ipStart, ipEnd);
    ipAddress.trim();
  
    if (ipAddress == "0.0.0.0") {
      SerialMon.println("Error: No valid IP address assigned (0.0.0.0)");
      success = false;
    }
  #ifndef DumpAtCommands
    else {
      SerialMon.println("Assigned IP address: " + ipAddress);
    }
#else //print nothing
#endif
}

// Public: Initialize modem (Step 1 and 2 - AT and CPIN checks)
bool SIM7600HTTPS::init() {
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
return success;
}

// Public: Connect to GPRS (Steps 4-10)
bool SIM7600HTTPS::gprsConnect(const char* apn) {
    bool success = true;
    sendATCGREG(success);   // Step 4
    sendATCNMP(success);    // Step 5
    sendATCOPS(success);    // Step 6
    sendATCGATT(success);   // Step 7
    sendATCGDCONT(success, apn);  // Step 8 with variable APN
    sendATCGACT(success);   // Step 9
    sendATCGPADDR(success); // Step 10
  #ifndef DumpAtCommands
    if (success) {
      SerialMon.println("GPRS connected successfully");
    }
  #else //print nothing
  #endif
  return success;
  }