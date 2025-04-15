/*
  DataSyncX-Watch - Minimal Smartwatch Framework
  Developed by: Aamir Ayaaz
  Initial Upload Date: April 15, 2025
  GitHub: https://github.com/Computationgeek/DataSyncX-Watch
  License: MIT
  Feature: Added a Developer Menu For Developer to gain Information about System Critical Data
  Description: A low-power smartwatch base using Seeed Studio Xiao ESP32-C3 and SSD1306 OLED,
  featuring NTP time sync, auto screen-off, and extensibility for more features.
*/


#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid       = "";
const char* password   = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;  // India Time GMT+5:30 = 5.5*3600 = 19800
const int   daylightOffset_sec = 0;

const int buttonPin = 5; // D3 on Xiao ESP32-C3
bool screenOn = false;
unsigned long screenOnTime = 0;
unsigned long lastNTPSync = 0;
const unsigned long NTP_SYNC_INTERVAL = 5 * 60 * 60 * 1000; // 5 hours in milliseconds

// Watch modes
enum WatchMode {
  TIME_MODE,
  DEV_MENU_MODE
};
WatchMode currentMode = TIME_MODE;

// Button handling variables
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
const unsigned long BUTTON_DEBOUNCE_TIME = 300; // Debounce time in milliseconds
int buttonPressCount = 0;
const unsigned long BUTTON_PRESS_RESET_TIME = 2000; // Time to reset button press count

// Developer menu variables
bool longButtonPress = false;
unsigned long longPressStartTime = 0;
const unsigned long LONG_PRESS_DURATION = 10000; // 10 seconds for dev menu
const unsigned long EXIT_DEV_MENU_DURATION = 15000; // 15 seconds to exit dev menu
bool exitingDevMenu = false;
int devMenuSelection = 0;
const int DEV_MENU_ITEMS = 3; // Number of items in dev menu

// System start time for uptime calculation
unsigned long systemStartTime = 0;

void syncNTPTime() {
  // Connect to WiFi if not already connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    
    // Set a timeout for connection
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" CONNECTED");
    
    // Setup time sync
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to sync
    struct tm timeinfo;
    int retries = 0;
    while (!getLocalTime(&timeinfo) && retries < 5) {
      Serial.println("Waiting for NTP...");
      delay(1000);
      retries++;
    }
    
    lastNTPSync = millis();
    
    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    // Force the display to turn on and show time after sync
    screenOn = true;
    screenOnTime = millis();
    printLocalTimeOLED();
  }
}

void printLocalTimeOLED() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2); // Larger font for failure message
    display.setTextColor(SSD1306_WHITE);
    display.println("Failed to get time");
    display.display();
    return;
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Show time in large font
  display.setCursor(0, 0);
  display.setTextSize(2);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  display.println(timeStr);
  
  // Show date in small font
  display.setTextSize(1);
  char dateStr[30];
  strftime(dateStr, sizeof(dateStr), "%A, %d %b %Y", &timeinfo);
  display.println(dateStr);
  
  display.display();
}

void displayDevMenu() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Display Dev Menu title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("DEVELOPER MENU");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Display exit hint
  display.setCursor(0, 55);
  display.setTextSize(1);
  display.print("Hold 15s to exit");
  
  // Display menu options
  for (int i = 0; i < DEV_MENU_ITEMS; i++) {
    // Highlight selected item
    if (i == devMenuSelection) {
      display.fillRect(0, 15 + (i * 12), 128, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    
    display.setCursor(5, 17 + (i * 12));
    
    switch (i) {
      case 0:
        display.print("System Info");
        break;
      case 1:
        display.print("Memory Stats");
        break;
      case 2:
        display.print("Uptime");
        break;
    }
  }
  
  display.display();
}

void displaySystemInfo() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.println("SYSTEM INFO");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Show CPU frequency
  display.setCursor(0, 15);
  display.print("CPU: ");
  display.print(ESP.getCpuFreqMHz());
  display.println(" MHz");
  
  // Show SDK version
  display.setCursor(0, 25);
  display.print("FW: ");
  display.println(ESP.getSdkVersion());
  
  // Last NTP sync
  display.setCursor(0, 45);
  unsigned long syncMinutesAgo = (millis() - lastNTPSync) / 60000;
  display.print("Last Sync: ");
  if (lastNTPSync == 0) {
    display.println("Never");
  } else {
    display.print(syncMinutesAgo);
    display.println(" min ago");
  }
  
  // Exit hint
  display.setCursor(0, 55);
  display.print("Hold 15s to exit");
  
  display.display();
}

void displayMemoryStats() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.println("MEMORY STATS");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Free heap memory
  display.setCursor(0, 15);
  display.print("Free RAM: ");
  display.print(ESP.getFreeHeap() / 1024);
  display.println(" KB");
  
  // Total heap size
  display.setCursor(0, 25);
  display.print("Total RAM: ");
  display.print(ESP.getHeapSize() / 1024);
  display.println(" KB");
  
  // Flash size
  display.setCursor(0, 35);
  display.print("Flash: ");
  display.print(ESP.getFlashChipSize() / (1024 * 1024));
  display.println(" MB");
  
  // Sketch size
  display.setCursor(0, 45);
  display.print("Sketch: ");
  display.print(ESP.getSketchSize() / 1024);
  display.print("/");
  display.print(ESP.getFreeSketchSpace() / 1024);
  display.println(" KB");
  
  // Exit hint
  display.setCursor(0, 55);
  display.print("Hold 15s to exit");
  
  display.display();
}

void displayUptime() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.println("SYSTEM UPTIME");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Calculate uptime
  unsigned long currentTime = millis();
  unsigned long uptime = currentTime - systemStartTime;
  
  // Convert to days, hours, minutes, seconds
  unsigned long days = uptime / (24 * 60 * 60 * 1000);
  uptime %= (24 * 60 * 60 * 1000);
  unsigned long hours = uptime / (60 * 60 * 1000);
  uptime %= (60 * 60 * 1000);
  unsigned long minutes = uptime / (60 * 1000);
  uptime %= (60 * 1000);
  unsigned long seconds = uptime / 1000;
  
  // Display in a more readable format
  display.setCursor(0, 15);
  display.setTextSize(2);
  
  char uptimeStr[20];
  sprintf(uptimeStr, "%02ld:%02ld:%02ld", hours, minutes, seconds);
  display.println(uptimeStr);
  
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Days: ");
  display.println(days);
  
  // Show uptime in seconds
  display.setCursor(0, 45);
  display.print("Total seconds: ");
  display.println(uptime / 1000);
  
  // Exit hint
  display.setCursor(0, 55);
  display.print("Hold 15s to exit");
  
  display.display();
}

void executeDevMenuItem() {
  switch (devMenuSelection) {
    case 0:
      displaySystemInfo();
      break;
    case 1:
      displayMemoryStats();
      break;
    case 2:
      displayUptime();
      break;
  }
}

void showExitingMessage() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Exiting developer mode...");
  display.display();
  
  delay(1000); // Show exit message for 1 second
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Record system start time
  systemStartTime = millis();
  
  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  // Display boot screen with watch name
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Show watch name in large font at the top
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("DataSyncX");
  
  // Show "Booting..." below it
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Booting...");
  display.display();
  delay(1000);
  
  // Connect to WiFi
  display.setCursor(0, 30);
  display.println("Connecting to WiFi...");
  display.display();
  
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" CONNECTED");
    display.setCursor(0, 40);
    display.println("WiFi connected!");
    display.display();
    
    // Setup time sync
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to sync
    struct tm timeinfo;
    display.setCursor(0, 50);
    display.println("Syncing time...");
    display.display();
    
    int retries = 0;
    while (!getLocalTime(&timeinfo) && retries < 10) {
      Serial.println("Waiting for NTP...");
      delay(1000);
      retries++;
    }
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("DataSyncX");
    display.setTextSize(1);
    
    if (retries >= 10) {
      display.setCursor(0, 20);
      display.println("Time sync failed.");
      Serial.println("Time sync failed.");
    } else {
      display.setCursor(0, 20);
      display.println("Time synced!");
      Serial.println("Time synced!");
      lastNTPSync = millis();
    }
    
    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  } else {
    Serial.println("WiFi connection failed");
    display.setCursor(0, 40);
    display.println("WiFi connection failed");
  }
  
  display.display();
  delay(2000);
  
  // Start in time mode with screen on
  currentMode = TIME_MODE;
  screenOn = true;
  screenOnTime = millis();
  
  // Show time immediately after boot
  printLocalTimeOLED();
}

void handleButtonPress() {
  unsigned long currentTime = millis();
  
  // If the screen is off, turn it on and show time
  if (!screenOn) {
    screenOn = true;
    screenOnTime = currentTime;
    currentMode = TIME_MODE;
    buttonPressCount = 0;
    printLocalTimeOLED();
    return;
  }
  
  // Check if we're in dev menu mode
  if (currentMode == DEV_MENU_MODE) {
    // In dev menu, button press either navigates or selects
    buttonPressCount++;
    
    if (buttonPressCount == 1) {
      // Move selection down
      devMenuSelection = (devMenuSelection + 1) % DEV_MENU_ITEMS;
      displayDevMenu();
    } else if (buttonPressCount == 2) {
      // Execute selected menu item
      executeDevMenuItem();
      buttonPressCount = 0; // Reset for next press
    }
    
    screenOnTime = currentTime; // Reset timeout
    return;
  }
  
  // Increment button press count for normal operation
  buttonPressCount++;
  
  // Handle based on current mode and button press count
  if (currentMode == TIME_MODE) {
    // Just show time and reset timeout
    printLocalTimeOLED();
    screenOnTime = currentTime;
    buttonPressCount = 0;
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check if it's time to sync with NTP server (every 5 hours)
  if (currentTime - lastNTPSync >= NTP_SYNC_INTERVAL) {
    syncNTPTime();
  }
  
  // Handle button press with debounce
  if (digitalRead(buttonPin) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressTime = currentTime;
      
      // Start tracking for long press
      longPressStartTime = currentTime;
      longButtonPress = false;
      exitingDevMenu = false;
    } else {
      // Check for dev menu enter (10s press) or dev menu exit (15s press)
      unsigned long pressDuration = currentTime - longPressStartTime;
      
      // If in normal mode and pressed long enough, enter dev menu
      if (!longButtonPress && currentMode == TIME_MODE && pressDuration >= LONG_PRESS_DURATION) {
        longButtonPress = true;
        currentMode = DEV_MENU_MODE;
        devMenuSelection = 0;
        displayDevMenu();
        screenOn = true;
        screenOnTime = currentTime;
      }
      
      // If in dev menu and pressed for 15+ seconds, exit to time mode
      if (currentMode == DEV_MENU_MODE && pressDuration >= EXIT_DEV_MENU_DURATION && !exitingDevMenu) {
        exitingDevMenu = true;
        showExitingMessage();
        currentMode = TIME_MODE;
        longButtonPress = true; // Mark as handled
        buttonPressCount = 0;
        printLocalTimeOLED();
        screenOn = true;
        screenOnTime = currentTime;
      }
    }
  }
  
  if (buttonPressed && digitalRead(buttonPin) == HIGH) {
    // Button released
    if (currentTime - buttonPressTime >= BUTTON_DEBOUNCE_TIME && !longButtonPress) {
      handleButtonPress();
    }
    buttonPressed = false;
    longButtonPress = false;
    exitingDevMenu = false;
  }
  
  // Reset button press count if no press for a while
  if (buttonPressCount > 0 && currentTime - buttonPressTime > BUTTON_PRESS_RESET_TIME) {
    buttonPressCount = 0;
  }
  
  // Update display based on current mode
  if (screenOn) {
    if (currentMode == TIME_MODE) {
      // Update the time display every second
      static unsigned long lastTimeUpdate = 0;
      if (currentTime - lastTimeUpdate >= 1000) {
        lastTimeUpdate = currentTime;
        printLocalTimeOLED();
      }
    }
    
    // Turn off screen after 5 seconds of inactivity
    // But not in dev menu mode
    if (currentTime - screenOnTime >= 5000 && 
        currentMode != DEV_MENU_MODE) {
      display.clearDisplay();
      display.display();
      screenOn = false;
    }
  }
  
  delay(50); // Short delay to prevent high CPU usage
}
