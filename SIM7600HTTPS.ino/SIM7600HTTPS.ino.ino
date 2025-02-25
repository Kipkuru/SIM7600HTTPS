#include <SIM7600HTTPS.h>  //This only supports GET and POST

#define DumpAtCommands
#define SerialMon Serial  // defines the Arduino serial to use for debug
#define SerialAT Serial1  //this defines the serial port connected to SIM7600; 1 for this case

#define DumpAtCommands  // Uncomment for AT command dump, comment for simple text
const char* apn = "safaricomiot";

const char* server = "api.360.mazimobility.com";
const char* resourceGet = "/iot/api-key/station/mail/read?station_id=23";
const char* resourcePut = "/iot/api-key/station/update/door?station_id=23";

const char* postData = "{\"d\":[{\"b\":1,\"st\":1},{\"b\":2,\"st\":0}]}";
unsigned long previousMillis = 0;
const long interval = 10000;
SIM7600HTTPS modem;

void setup() {
  SerialMon.begin(115200);  //initialize serial for debug
  SerialAT.begin(115200);   //initialize gsm serial
  if (modem.init()) {
    if (modem.gprsConnect(apn)) {
      Serial.println("SUCCESS");
    }else{
      SerialMon.println("GPRS connection failed");
    }
  }
  delay(1000);
}

void loop() {
   unsigned long currentMillis = millis();
   if (currentMillis - previousMillis >= interval) {
     previousMillis = currentMillis;
     if (modem.httpInit(server, resourcePut)) {  // Start HTTP session at beginning of loop
//       String serverResponse;
//       if (modem.httpPostData(postData, serverResponse)) {  // Perform POST
       //  modem.httpTerm();  // Terminate session after successful POST
//       } else {
//         SerialMon.println("HTTP post failed");
//       }
//     } else {
//       SerialMon.println("HTTP initialization failed");
     }
   }
}