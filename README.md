# RoboticArmCar
ESP8266-powered robotic arm car with a built-in web interface for servo and base movement control.
## Features
- Creates a Wi-Fi access point and hosts a control webpage
- Controls 4 servos with on-page sliders
- Controls base movement with an on-page joystick
- Exposes HTTP endpoints for movement and servo updates
## Main File
- `RoboticArmCar.ino` - complete firmware + embedded web UI
## Pin Mapping
- Servo pins: `D1, D2, D5, D6`
- Motor pins: `D3, D4, D7, D8`
## Setup
1. Open `RoboticArmCar.ino` in Arduino IDE.
2. Set custom AP credentials in `APSSID` and `APPSK`.
3. Select your ESP8266 board and upload.
4. Connect to the ESP AP and open the IP shown in serial monitor.
## Endpoints
- `/setServo0` to `/setServo3` with query `angle=<0-180>`
- `/forward`, `/backward`, `/left`, `/right`, `/stop`
