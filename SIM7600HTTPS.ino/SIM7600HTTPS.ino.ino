 #include <SIM7600HTTPS.h>  //This only supports GET and POST

//#define DumpAtCommands
#define SerialMon Serial  // defines the Arduino serial to use for debug
//#define SerialAT Serial1  //this defines the serial port connected to SIM7600; 1 for this case
HardwareSerial SerialAT(2);  // Use UART2 on ESP32 (TX = 17, RX = 16)

#define DumpAtCommands  // Uncomment for AT command dump, comment for simple text
const char* apn = "safaricomiot";

const char* server = "http://voyager.mazimobility.com:5500";  //note that it is a must to include https before the url, else error
const char* resourceGet = "/api/v1/charging-stations-status/0";
const char* resourcePost = "/api/v1/charging-stations-status";

const char* postData = "{\"station_id\":1,\"temperature\":[0],\"smoke\":[0],\"door_status\":[0],\"state_of_charge\":[100],\"battery_voltages\":[0],\"station_humidity\":[100],\"charger_status\":[\"idle\"],\"power_consumption_reading\":1,\"is_running_on_backup\":true}";
unsigned long previousMillis = 0;
const long interval = 10000;

SIM7600HTTPS modem;

void setup() {
  SerialMon.begin(115200);  //initialize serial for debug
  //SerialAT.begin(115200);   //initialize gsm serial
    SerialAT.begin(115200, SERIAL_8N1, 16, 17);  // TX = 17, RX = 16
  if (modem.init()) {
    if (modem.gprsConnect(apn)) {
      Serial.println("SUCCESS");
    } else {
      SerialMon.println("GPRS connection failed");
    }
  }
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();

  // HTTP POST every 10 seconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (modem.httpInit(server, resourceGet)) {  // Start HTTP session at beginning of loop
      String serverResponse;
      if (modem.httpGet(serverResponse)) {  //Perform HTTP GET
        SerialMon.println("Server Response: " + serverResponse);
        modem.httpTerm();
      }
    }
    delay(3000);
    if (modem.httpInit(server, resourcePost)) {  // Start HTTP session at beginning of loop
      String serverResponse;
      if (modem.httpPost(postData, serverResponse)) {  //Perform HTTP POST
        SerialMon.println("Server Response: " + serverResponse);
        modem.httpTerm();
      }
    }
  }
}