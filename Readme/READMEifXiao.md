DataSync-Watch is a fully working, power-conscious smartwatch base built on the tiny but powerful Seeed Studio Xiao ESP32-C3.
It shows time synced from NTP, displays it on an OLED screen, and turns off automatically to save battery — all with just a button press.

Whether you're just getting into embedded hardware or you're building your own smartwatch with step tracking, BLE notifications, or health monitoring — this is your dev-friendly starting point.

⚙️ Features
✅ Automatically syncs time from the internet via NTP

✅ Displays current time + full date on a 128x64 OLED

✅ Uses a single button (D3 / GPIO5) to trigger screen

✅ Auto-off screen after 10 seconds to save battery

✅ WiFi disconnects after time sync — fully offline-capable

✅ Periodic resync every 5 hours

🔩 Hardware Required
Seeed Studio XIAO ESP32-C3

0.96" SSD1306 OLED Display (I2C, 128x64)

Push Button (connected to D3 / GPIO 5)

Optional: Battery, enclosure, accelerometer (for future development)

🔌 Wiring
Component	Xiao Pin	Notes
OLED VCC	3.3V	Power the display
OLED GND	GND	Ground
OLED SDA	D4 (GPIO6)	I2C Data
OLED SCL	D5 (GPIO7)	I2C Clock
Button	D3 (GPIO5)	Pulled-up, active LOW
💻 Software Setup
Install Arduino IDE

Add Xiao ESP32-C3 Board

Go to File > Preferences

Add this URL to "Additional Board Manager URLs":
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Go to Tools > Board > Boards Manager, search esp32, install it.

Select Seeed XIAO ESP32-C3 from Boards list.

Install Required Libraries:

Adafruit GFX

Adafruit SSD1306

WiFi

time.h (built-in)

🚀 Uploading the Code
Clone this repository
git clone https://github.com/yourusername/DataSync-Watch.git

Open the .ino file inside Arduino IDE

Go to Tools > Board, select Seeed XIAO ESP32-C3

Choose the correct COM port

Upload the sketch — and you're good to go!

🔧 Customization Ideas
This is just the beginning. You can expand it with:

Step counting using accelerometer (like MPU6050 or ADXL345)

BLE phone notifications

Music control

Sleep or alarm functions

OTA update system

Custom fonts and graphics

Logging or syncing with a phone app

🌍 Offline Mode
Once the time is synced, WiFi is disconnected — the watch keeps running without needing internet again. Time updates every 5 hours (customizable), or you can manually trigger sync by rebooting.
