#include <Arduino.h>
#include "ui.h"
#include "TFT_eSPI.h"
#include <TimeLib.h>

// Initialize the TFT_eSPI library
TFT_eSPI tft = TFT_eSPI();

// Implementation of the UI::init() function
void UI::init()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

void UI::drawMainScreen()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // Draw the time in the top left
  time_t now = time(nullptr);
  char timeString[6];
  sprintf(timeString, "%02d:%02d", hour(now), minute(now));
  tft.drawString(timeString, 0, 0);

  // Draw the date below the time
  char dateString[11];
  sprintf(dateString, "%04d-%02d-%02d", year(now), month(now), day(now));
  tft.drawString(dateString, 0, 20);

  // Draw the lightbulb in the middle
  // Placeholder code, replace with actual lightbulb drawing
  tft.drawString("Lightbulb", tft.width() / 2 - 30, tft.height() / 2 - 10);

  // Draw settings in the bottom right
  tft.setTextSize(1);
  tft.drawString("Settings", tft.width() - 60, tft.height() - 20);
}