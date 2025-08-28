# SIM7600E-H HTTPS Client

This project provides an Arduino sketch for performing **HTTP GET and POST requests** using a SIM7600 cellular module. It connects to a public HTTPS endpoint to send and receive JSON data, making it suitable for IoT applications such as remote monitoring or data logging.  

The code is designed for **simplicity and reliability**, with clear documentation for easy integration.


## Features
- Perform HTTP GET and POST requests using the SIM7600 module.
- Connect to a public HTTPS endpoint (default: [jsonplaceholder.typicode.com](https://jsonplaceholder.typicode.com)).
- Send generic JSON data (e.g. `{"title":"Test","body":"...","userId":1}`).
- Configurable APN for cellular network connectivity.
- Periodic requests every 10 seconds (configurable).

## Requirements

### Hardware
- Arduino Mega (or compatible board with multiple hardware serial ports).
- SIM7600 module (e.g., SIM7600E-H) with a valid SIM card.
- Power supply for SIM7600 (5V, **2A recommended**).
- Antenna for stable signal.

### Software
- Arduino IDE (2.0 or later recommended).
- **SIM7600HTTPS** library (via Library Manager or GitHub).

### Network
- Active SIM card with a data plan.
- APN settings for your provider (e.g., `safaricom`).

## Installation

### 1. Hardware Setup
Connect the SIM7600 module to Arduino Mega:

- SIM7600 **TX** → Arduino Mega **RX1** (Pin 19)  
- SIM7600 **RX** → Arduino Mega **TX1** (Pin 18)  
- SIM7600 **VCC** → 5V (ensure sufficient current)  
- SIM7600 **GND** → Arduino GND  
- Insert SIM card and attach antenna  

### 2. Software Setup
1. [Download Arduino IDE](https://www.arduino.cc/en/software)  
2. Install the **SIM7600HTTPS** library:  
   - In Arduino IDE: `Sketch > Include Library > Manage Libraries`  
   - Search for `SIM7600HTTPS` and install it.  
3. Open the provided sketch: `charging_station_api.ino`

### 3. Configure APN
In the sketch, update the APN:

```
const char* apn = "your_apn_here";  // e.g., "safaricom"
```
### 4. Upload Sketch
- Select **Arduino Mega or Mega 2560** in `Tools > Board`.  
- Select the correct **Port** in `Tools > Port`.  
- Upload the sketch.  

## Usage

### Power On
- Power the Arduino + SIM7600 module.  
- Ensure the SIM7600 has a strong signal (check LED indicator).  

### Monitor Output
- Open Serial Monitor at **115200 baud**.  
- Look for `SUCCESS` to confirm GPRS connection.  

### Automatic Requests
The sketch performs:  
- **GET** → `https://jsonplaceholder.typicode.com/posts/1` every 10s.  
- **POST** → `https://jsonplaceholder.typicode.com/posts` with JSON payload every 10s.  

Responses are printed to Serial Monitor.

### Customize
- Modify JSON payload:  
```cpp
const char* postData = "{\"title\":\"Your Title\",\"body\":\"Your Content\",\"userId\":1}";
```
- Adjust request interval:
```
const long interval = 10000;  // 10 seconds
```
-Change endpoint to your own HTTPS server.
## Troubleshooting

### GPRS Connection Failed
- Check SIM card & APN settings.  
- Ensure antenna is connected.  
- Confirm power supply provides at least **2A**.  

### No Response from Server
- Verify SIM7600 module supports HTTPS (SSL/TLS may be required).  
- Test the endpoint with `curl` or Postman.  

### Serial Output Issues
- Ensure Serial Monitor baud is set to **115200**.  
- Double-check wiring (TX/RX pins).  

## Contributing
Contributions are welcome!  
- Open an issue for bugs/feature requests.  
- Submit a pull request with improvements.  

## License
This project is licensed under the **MIT License**.
