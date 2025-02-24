**SIM7600HTTPS Library**
A lightweight Arduino library for the SIM7600 module to handle modem initialization, GPRS connection, and HTTP operations (GET/POST). Designed for reliable, step-by-step setup and periodic HTTP requests.

**Debugging**
Enable AT Commands: Define #define DumpAtCommands in .ino before #include <SIM7600HTTPS.h> to print commands/responses.
Default: Without DumpAtCommands, prints simplified status (e.g., "SIM card ready").

**Dependencies**
Arduino core (<Arduino.h>).
SIM7600 module connected to a serial port (e.g., Serial1) for Arduino Mega.


