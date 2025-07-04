#include "SIM7600HTTPS.h"

// Constructor
SIM7600HTTPS::SIM7600HTTPS() {
}

// Private: Generic AT command sender with flexible expected response
String SIM7600HTTPS::sendATCommand(const char* cmd, const char* expected, unsigned long timeout) {
  clearSerialBuffer();  // Clear any residual data
  SerialAT.println(cmd);
  DEBUG_PRINT("Command: ");  
  DEBUG_PRINTLN(cmd);       // Print command on timeout
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
            DEBUG_PRINT("Response: ");  // Print on timeout           
            DEBUG_PRINTLN(response);  // Dump raw response if defined
        return response;
      }
    }
    delay(10);  // Small delay to avoid tight loop
  }
  DEBUG_PRINT("Response: ");  // Print on timeout
  DEBUG_PRINTLN(response);  // Dump raw response if defined
  return response;
}

// Private: Clear SerialAT buffer
void SIM7600HTTPS::clearSerialBuffer() {
  while (SerialAT.available()) {
    SerialAT.read();
  }
}

//Private: Send AT+CRESET
void SIM7600HTTPS::sendATCRESET(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CFUN=1,1", " PB DONE", 60000);  // Reset and wait for RDY
    if (response.indexOf(" PB DONE") == -1) {
      SerialMon.println("Error: Failed to reset GSM module");
      success = false;
    }
    else {
    DEBUG_PRINTLN("GSM module reset successfully");
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
   DEBUG_PRINTLN("SIM card ready");  // Success message
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
        DEBUG_PRINTLN("Signal quality check passed (RSSI: " + String(rssi) + ")");
        DEBUG_PRINTLN("Signal checked, RSSI: " + String(rssi));
      }
    }
}
//Private: Send AT+CGREG? (Step 4 - Network Registration Check)
void SIM7600HTTPS::sendATCGREG(bool& success) {
  if (!success) return;
  String response = sendATCommand("AT+CGREG?", "OK", 5000);
  if (response.indexOf("OK") == -1 || 
  (response.indexOf("+CGREG: 0,1") == -1 && response.indexOf("+CGREG: 0,5") == -1)) {
    SerialMon.println("Error: SIM Not registered on network");
    success = false;
  }
  else {
    DEBUG_PRINTLN("Network registration confirmed");
  }
}
//Private: Send AT+CNMP=38 (Step 5 - Set Preferred Mode to LTE)
void SIM7600HTTPS::sendATCNMP(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CNMP=39", "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to set preferred mode to LTE");
      success = false;
    }
    else {
      DEBUG_PRINTLN("Preferred mode set to LTE");
    }
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
    DEBUG_PRINTLN("Operator selection set to automatic");
  }
#endif
}

//Private: Send AT+CGATT=1 (Step 7 - Attach to GPRS)
void SIM7600HTTPS::sendATCGATT(bool& success) {
    if (!success) return;
    String response = sendATCommand("AT+CGATT=1", "OK", 5000);
    if (response.indexOf("OK") != -1) {
            DEBUG_PRINTLN("PDP context activated");
    }else{
            SerialMon.println("Error: Failed to activate PDP context, refresh GSM");
            success = false;
          }
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
      DEBUG_PRINTLN("APN set to " + String(apn));
    }
  #endif
  }
//Private: Send AT+CGACT=1,1 (Step 9 - Activate PDP Context)
void SIM7600HTTPS::sendATCGACT(bool& success) {
    if (!success) return;

    // Step 1: Check current PDP context state with AT+CGACT?
    SerialAT.println("AT+CGACT?");
      DEBUG_PRINT("Command: ");
      DEBUG_PRINTLN("AT+CGACT?");

      String response = "";
      unsigned long startTime = millis();
      while (millis() - startTime < 1000) {  // 1-second timeout for CGACT?
        while (SerialAT.available()) {
          char c = SerialAT.read();
          response += c;
          if (response.indexOf("OK") != -1) {  // Wait for complete response ending with OK
            DEBUG_PRINTLN("Response: ");
            DEBUG_PRINTLN(response);
            if (response.indexOf("+CGACT: 1,1") != -1) {
              // PDP context 1 is already active - exit with success
              DEBUG_PRINTLN("PDP context 1 already active - skipping activation");
              return;  // success remains true
            }
            break;  // Proceed to activation if not active
          }
        }
        delay(10);
      }
// Step 2: If not active, send AT+CGACT=1,1
    response = sendATCommand("AT+CGACT=1,1", "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to activate PDP context");
      success = false;
    }
    else {
      DEBUG_PRINTLN("PDP context activated");
    }
}

//Private: Send AT+CGPADDR (Step 10 - Obtain IP Address)
void SIM7600HTTPS::sendATCGPADDR(bool& success) {
    if (!success) return;
    
    // Send command silently
    SerialAT.println("AT+CGPADDR=1");
    DEBUG_PRINT("Command: ");  
    DEBUG_PRINTLN("AT+CGPADDR=1");
  
    // Wait for complete response (+CGPADDR: 1,<ip>)
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {  // 5-second timeout
      while (SerialAT.available()) {
        char c = SerialAT.read();
        response += c;
        // Check for full IP address line (ends with newline after IP)
        if (response.indexOf("+CGPADDR: 1,") != -1 && response.indexOf("\r\n", response.indexOf("+CGPADDR: 1,")) != -1) {
          int ipStart = response.indexOf("1,") + 2;
          int ipEnd = response.indexOf("\r\n", ipStart);
          String ipAddress = response.substring(ipStart, ipEnd);
          ipAddress.trim();
  
          if (ipAddress == "0.0.0.0") {
            SerialMon.println("Error: No valid IP address assigned (0.0.0.0)");
            success = false;
          } else {
            DEBUG_PRINTLN("Assigned IP address: " + ipAddress);
          }
          return;  // Exit once full IP is received
        }
      }
      delay(10);
    }
  
    // Timeout case
    DEBUG_PRINTLN("Error: Failed to obtain IP address response");
    success = false;
}

//Private: Send AT+HTTPTERM
void SIM7600HTTPS::sendATHTTPTERM(bool& success) {
    if (!success) return;
  
    // Send command silently
    SerialAT.println("AT+HTTPTERM");
  #ifdef DumpAtCommands
    SerialMon.print("Command: ");
    SerialMon.println("AT+HTTPTERM");
  #endif
  
    // Wait for response
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {  // 1-second timeout
      while (SerialAT.available()) {
        char c = SerialAT.read();
        response += c;
        if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1) {
  #ifdef DumpAtCommands
          SerialMon.println("Response: ");
          SerialMon.print(response);
          SerialMon.println();
  #endif
    if(response.indexOf("OK")){
        SerialMon.println("Existing HTTP session terminated");
        return;
    }else if(response.indexOf("ERROR")){
        SerialMon.println("No existing HTTP session");
        return;
    }
    //return;  // Success - OK or ERROR means termination complete
    }
      }
      delay(10);
 }
  
    // Timeout or unexpected response
    SerialMon.println("Error: Failed to terminate HTTP session - No valid response");
    success = false;
}
//Private: Send AT+HTTPINIT
void SIM7600HTTPS::sendATHTTPINIT(bool& success) {
    if (!success) return;
  
    // Send AT+HTTPINIT silently
    SerialAT.println("AT+HTTPINIT");
    DEBUG_PRINT("Command: ");
    DEBUG_PRINTLN("AT+HTTPINIT");
  
    // Wait for response
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {  // 1-second timeout
      while (SerialAT.available()) {
        char c = SerialAT.read();
        response += c;
        if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1) {
          DEBUG_PRINT("Response: ");
          DEBUG_PRINTLN(response);
          if (response.indexOf("OK") != -1) {
            DEBUG_PRINTLN("HTTP session success");
            // Success - proceed
            return;
          } else if (response.indexOf("ERROR") != -1) {
            DEBUG_PRINTLN("Error: Active HTTP session running");
            return;
          }
        }
      }
      delay(10);
    }
  
    // Timeout or unexpected response
    SerialMon.println("Error: Failed to initialize HTTP session - No valid response");
    success = false;
}

//Private: Send AT+HTTPPARA
void SIM7600HTTPS::sendATHTTPPARA(bool& success, const char* param, const char* value) {
    if (!success) return;
      String cmd;
    // Check if param is "CONNECTTO" to send value without quotes
    if (strcmp(param, "CONNECTTO") == 0) {
      cmd = "AT+HTTPPARA=\"" + String(param) + "\"," + String(value);  // No quotes around value
    } else {
      cmd = "AT+HTTPPARA=\"" + String(param) + "\",\"" + String(value) + "\"";  // Quotes around value
    }
    String response = sendATCommand(cmd.c_str(), "OK", 5000);
    if (response.indexOf("OK") == -1) {
      SerialMon.println("Error: Failed to set HTTP parameter ");
      success = false;
    }
}

//Private: Send AT+HTTPDATA
void SIM7600HTTPS::sendATHTTPDATA(bool& success, const char* data) {
    if (!success) return;
    String cmd = "AT+HTTPDATA=" + String(strlen(data)) + ",10000";
    String response = sendATCommand(cmd.c_str(), "DOWNLOAD", 5000);
    if (response.indexOf("DOWNLOAD") != -1) {
      SerialAT.println(data);
      response = waitForResponse("OK", 3000);  // Wait for OK after data
      if (response.indexOf("OK") == -1) {
        SerialMon.println("Error: Failed to send HTTP data");
        success = false;
      }
    } else {
      SerialMon.println("Error: Failed to initiate HTTP data");
      success = false;
    }
  }

void SIM7600HTTPS::sendATHTTPACTION(bool& success, int method, int& responseLength) {
  if (!success) return;

  // Send command silently
  String cmd = "AT+HTTPACTION=" + String(method);
  SerialAT.println(cmd);
  DEBUG_PRINT("Command: ");
  DEBUG_PRINTLN(cmd);

  // Wait for complete response (+HTTPACTION: <method>,<status>,<length>)
  String response = "";
  String expectedStart = "+HTTPACTION: " + String(method) + ",";
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {  // 60-second timeout
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
      // Check for full response (ends with newline after length)
      if (response.indexOf(expectedStart) != -1 && response.indexOf("\r\n", response.indexOf(expectedStart)) != -1) {
        DEBUG_PRINT("Response: ");
        DEBUG_PRINTLN(response);
        int statusStart = response.indexOf(",", response.indexOf(expectedStart)) + 1;
        int statusEnd = response.indexOf(",", statusStart);
        String statusStr = response.substring(statusStart, statusEnd);
        int status = statusStr.toInt();

        int lengthStart = statusEnd + 1;
        int lengthEnd = response.indexOf("\r\n", lengthStart);
        String lengthStr = response.substring(lengthStart, lengthEnd);
        responseLength = lengthStr.toInt();

        // Log status and length
        if(method == 0) {  // GET method
          SerialMon.println("GET code: " + String(status) + ",Payload Length: " + String(responseLength));
        } else if (method == 1) {  // POST method
          SerialMon.println("POST code: " + String(status));
        }
        if (responseLength < 0) {
            SerialMon.println("Error: Invalid HTTP action response length");
            success = false;
          }
        return responseLength;  // Success - full response received
      }
    }
    delay(10);
  }

  // Timeout or incomplete response
  SerialMon.println("Error: HTTP action failed");
  success = false;
  responseLength = 0;
}

// Unchanged: Read HTTP Response
String SIM7600HTTPS::readHTTPResponse(int responseLength, int timeout) {
  if (responseLength <= 0) 
    return "";
  String fullResponse = "";
  int bytesRead = 0;
  int chunkSize = 128;  // Adjustable chunk size
  unsigned long startTime = millis();
  while (bytesRead < responseLength) {
    int remainingBytes = responseLength - bytesRead;
    int readSize = (remainingBytes < chunkSize) ? remainingBytes : chunkSize;
    String readCmd = "AT+HTTPREAD=" + String(readSize);
    String chunk = sendATCommandSilent(readCmd);
    
    int dataStart = chunk.indexOf("+HTTPREAD: DATA,");
    if (dataStart != -1) {
      int lengthStart = dataStart + 16;
      int lengthEnd = chunk.indexOf("\r\n", lengthStart);
      String lengthStr = chunk.substring(lengthStart, lengthEnd);
      int actualBytes = lengthStr.toInt();
      
      int dataBegin = lengthEnd + 2;
      int dataEnd = dataBegin + actualBytes;
      String dataChunk = chunk.substring(dataBegin, dataEnd);
      
      fullResponse += dataChunk;
      bytesRead += actualBytes;
    } else if (chunk.indexOf("ERROR") != -1 || chunk.indexOf("+HTTPREAD: 0") != -1) {
      break;
    }
    delay(10);
  }
  DEBUG_PRINTLN("Server Payload: " + fullResponse);  // Print full response always
  DEBUG_PRINTLN("Total Bytes Read: " + String(bytesRead));
  return fullResponse;
}

// Unchanged: for reading server payload
String SIM7600HTTPS::sendATCommandSilent(String cmd) {
  SerialAT.println(cmd);
  String response = "";
  unsigned long startTime = millis();
  while (millis() - startTime < 50) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
    }
    delay(10);
  }
  return response;
}

// Public: Initialize modem (Step 1 and 2 - AT and CPIN checks)
bool SIM7600HTTPS::init() {
    bool success = true;  // Start with success assumed
  //sendATCRESET(success);  // Reset module first
  //delay(2000);
  sendAT(success);      // Step 1: Check basic communication
  sendATCPIN(success);  // Step 2: Check SIM status
  sendATCSQ(success);   // Step 3: Check signal quality
return success;
}

// Public: Connect to GPRS (Steps 4-10)
bool SIM7600HTTPS::gprsConnect(const char* apn) {
    bool success = true;
    
    sendATCNMP(success);    // Step 4: Set LTE mode first
   //sendATCOPS(success);    // Step 5: Operator selection- keeps destabilizing the GSM board
    sendATCGREG(success);   // Step 6: Confirm registration
    sendATCGATT(success);   // Step 7: Attach GPRS
    sendATCGDCONT(success, apn);  // Step 8 with variable APN: Define PDP context
    //delay(1000); for testing purposes
    sendATCGACT(success);   // Step 9: Activate PDP context
   // delay(1000);for testing purposes
    sendATCGPADDR(success); // Step 10: Get IP
    sendATHTTPINIT(success);  // Start new HTTP session
  return success;
}
// Public: Initialize HTTP
bool SIM7600HTTPS::httpInit(const char* server, const char* resource) {
    bool success = true;

    sendATHTTPPARA(success, "URL", (String(server) + String(resource)).c_str());  // Set URL
    sendATHTTPPARA(success, "UA", "SIM7600");  // Set User-Agent
    sendATHTTPPARA(success, "CONTENT", "application/json");  // Set Content-Type
    return success;
}
//Public: Perform HTTP GET
bool SIM7600HTTPS::httpGet(String& response) {
    bool success = true;
    int responseLength = 0;
    sendATHTTPACTION(success, 0, responseLength);  // 0 = GET
    if (success) {
      response = readHTTPResponse(responseLength, 5000);
      SerialMon.flush();  // Ensure immediate print
    } else {
      response = "";
    }
    return success;
}
//Public: Perform HTTP POST
bool SIM7600HTTPS::httpPost(const char* data, String& response) {
    bool success = true;
    int responseLength = 0;
    sendATHTTPDATA(success, data);
    sendATHTTPACTION(success, 1, responseLength);  // 1 = POST
    if (success) {
      response = readHTTPResponse(responseLength, 5000);
    } else {
      response = "";
    }
    return success;
  }
 //Public: Terminate HTTP Session
bool SIM7600HTTPS::httpTerm() {
    bool success = true;
    sendATHTTPTERM(success);  // Terminate HTTP session
  #ifndef DumpAtCommands
    if (success) {
      SerialMon.println("HTTP session terminated");
    } else {
      SerialMon.println("Error: Failed to terminate HTTP session");
    }
  #endif
    return success;
  }