#include <Arduino.h>
#include "ui.h"
#include "TFT_eSPI.h"
#include <string>

// Initialize the TFT_eSPI library
TFT_eSPI tft = TFT_eSPI();

//Constructor for UI class
UI::UI(){}

// Implementation of the UI::init() function
void UI::init()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

void UI::drawMainScreen(String time, String date, String temp, bool lightState)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.drawString(time, 0, 0);

  tft.drawString(date, 0, 20);

    tft.drawString(temp, 0, 40);

  // Draw the lightbulb in the middle
  // Placeholder code, replace with actual lightbulb drawing
  tft.drawString(lightState ? "On" : "Off", tft.width() / 2 - 30, tft.height() / 2 - 10);

  // Draw settings in the bottom right
  tft.setTextSize(1);
  tft.drawString("Settings", tft.width() - 60, tft.height() - 20);
}