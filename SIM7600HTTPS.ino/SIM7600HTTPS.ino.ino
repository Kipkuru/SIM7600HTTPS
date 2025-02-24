#include <SIM7600HTTPS.h>  //This only supports GET and POST

//#define DumpAtCommands
#define SerialMon Serial // defines the Arduino serial to use for debug
#define SerialAT Serial1 //this defines the serial port connected to SIM7600; 1 for this case

#define DumpAtCommands  // Uncomment for AT command dump, comment for simple text

const char* server = "api.360.mazimobility.com";
const char* resourceGet = "/iot/api-key/station/mail/read?station_id=23";
const char* resourcePut1 = "/iot/api-key/station/update/door?station_id=23";
const char* resourcePut2 = "/iot/api-key/station/update/other?station_id=23";

const char* apn = "safaricom";

const char* postData1 = "{\"d\":[{\"b\":1,\"st\":1},{\"b\":2,\"st\":0}]}";
const char* postData2 = "{\"status\":\"update\",\"value\":42}";

SIM7600HTTPS modem;

void setup() {
  SerialMon.begin(115200); //initialize serial for debug
  SerialAT.begin(115200); //initialize gsm serial
  modem.init();
  // modem.gprsConnect(apn);
}

void loop() {
  // modem.httpInit(server);
  // modem.httpGetResource(resourceGet);
  // modem.httpPostData(resourcePut1, postData1);
  // modem.httpPostData(resourcePut2, postData2);
  // modem.httpTerm();
  // delay(20000);
}