#include <Arduino.h>
#include "ui.h"
#include "TFT_eSPI.h"
#include <string>
#include "home_assistant.h"

// Includes for WiFi and HTTPClient
#include <WiFi.h>

// Initialize the TFT_eSPI library
TFT_eSPI tft = TFT_eSPI();

//Constructor for UI class
UI::UI(){}

// Implementation of the UI::init() function
void UI::init()
{
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
}

void UI::drawMainScreen(String time, String date, String temp, bool lightState)
{
  // Set text color and size
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // Clear time area
  
  tft.fillRect(0, 0, tft.width(), 20, TFT_BLACK);
  // Draw the time
  tft.drawString(time, 0, 0);

  // Clear date area
  tft.fillRect(0, 20, tft.width(), 20, TFT_BLACK);
  // Draw the date
  tft.drawString(date, 0, 20);

  // Clear temperature area
  tft.fillRect(0, 40, tft.width(), 20, TFT_BLACK);
  tft.drawString(temp, 0, 40);

  // Draw the lightbulb in the middle
  // Placeholder code, replace with actual lightbulb drawing in the future
  tft.drawString(lightState ? "On" : "Off", tft.width() / 2 - 30, tft.height() / 2 - 10);

  // Draw settings in the bottom right
  tft.setTextSize(1);
  tft.drawString("Settings", tft.width() - 60, tft.height() - 20);
}

void UI::drawSettingsScreen() {
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 20); // Set cursor position for text
  tft.print("Settings Screen");
}

void UI::handleTouch() {
    int x, y;
    HomeAssistant homeAssistant;
    
  if(tft.getTouch(&x, &y)){
    
    // Check if the touch is within the lightbulb area
    if (x > tft.width() / 2 - 60 && x < tft.width() / 2 + 60 &&
        y > tft.height() / 2 - 20 && y < tft.height() / 2 + 20) {
          
      // Toggle the light
      std::vector<std::string> lights = homeAssistant.discoverLights();
      if (lights.size() > 0) {
        homeAssistant.toggleLight(lights[0]);
      }
    }
    // Check if the touch is within the settings area
    else if (x > tft.width() - 60 && x < tft.width() && 
        y > tft.height() - 20 && y < tft.height()) {
      drawSettingsScreen();
    }
  }
}
