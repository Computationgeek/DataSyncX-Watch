DataSync-Watch is a personal project turned open-source platform — a smart, compact watch powered by the Xiao ESP32-C3 that syncs time using NTP, displays it on a crisp OLED screen, and handles everything with smart energy-saving logic.

It's designed as a developer-friendly starting point. Anyone can clone it, flash it, and immediately have a working watch that tells accurate time and is ready for further development — like step tracking, BLE features, notifications, or even smart home integration.

Key Features:

Automatically syncs time from NTP servers every 5 hours

Displays time and date on demand via a button

Automatically turns off display to save battery

Works completely offline after initial sync

WiFi disconnects automatically after sync for power efficiency

Hardware Used:

*Xiao ESP32-C3

*SSD1306 OLED Display (128x64, I2C)

*Push button connected to GPIO 5


How to Use:

--> Install the required libraries in Arduino IDE:
Adafruit GFX
Adafruit SSD1306
WiFi
time.h

--> Clone the repository to your computer.
--> Open the main .ino file in Arduino IDE.
--> Connect your ESP32-C3 board and upload the code.
--> Power it with a small Li-Po battery or USB — and you’re ready!

Who is this for?

This project is aimed at developers, tinkerers, and electronics hobbyists who want to explore embedded wearables or need a reliable timekeeping base to expand on.
It’s a fully functional, lightweight smartwatch framework that you can use to add:
*Step tracking with an accelerometer
*BLE notifications
*Alarms or reminders
*Custom apps (via buttons, gestures, etc.)
*Data logging or syncing to cloud

Want to Contribute?

Fork the repo, make something cool, and open a pull request!

You can also:

--> Open feature requests
--> Log bugs or issues
--> Discuss ideas in GitHub Discussions

Let’s grow this into a feature-rich, community-driven smartwatch dev base.

Built with ❤️ by Aamir Ayaaz
Passionate about consumer electronics, repairability, and developer-focused hardware. This is just the beginning!


