/*
  DataSyncX-Watch - Minimal Smartwatch Framework
  Developed by: Aamir Ayaaz
  Initial Upload Date: April 15, 2025
  GitHub: https://github.com/Computationgeek/DataSyncX-Watch
  License: MIT

  Description: A low-power smartwatch base using Seeed Studio Xiao ESP32-C3 and SSD1306 OLED,
  featuring NTP time sync, auto screen-off, and extensibility for more features.
*/

#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid       = "YOUR_SSID";
const char* password   = "YOUR_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;  // India Time GMT+5:30 = 5.5*3600 = 19800
const int   daylightOffset_sec = 0;

const int buttonPin = 5; // D3 on Xiao ESP32-C3
bool screenOn = false;
unsigned long screenOnTime = 0;
unsigned long lastNTPSync = 0;
const unsigned long NTP_SYNC_INTERVAL = 5 * 60 * 60 * 1000; // 5 hours in milliseconds or Syncs with NTP setver to fetch time every 5 hours 

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

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  
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
  display.clearDisplay();
  display.display();
}

void loop() {
  // Check if it's time to sync with NTP server (every 5 hours)
  if (millis() - lastNTPSync >= NTP_SYNC_INTERVAL) {
    syncNTPTime();
  }
  
  // Button press check
  if (digitalRead(buttonPin) == LOW && !screenOn) {
    screenOn = true;
    screenOnTime = millis();
    printLocalTimeOLED();
  }
  
  // Update the time display every second while screen is on
  static unsigned long lastTimeUpdate = 0;
  if (screenOn && millis() - lastTimeUpdate >= 1000) {
    lastTimeUpdate = millis();
    printLocalTimeOLED();
  }
  
  // Turn off screen after 10 seconds
  if (screenOn && millis() - screenOnTime >= 10000) {
    display.clearDisplay();
    display.display();
    screenOn = false;
  }
  
  delay(100); // Short delay to prevent button bounce and reduce CPU usage
}
