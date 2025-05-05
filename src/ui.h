#ifndef UI_H
#define UI_H

#include <TFT_eSPI.h>

class UI {
public:
  void init();
  void drawMainScreen(String time, String date, String temp, bool lightState);
  void drawSettingsScreen();
   UI();

  void handleTouch();
  void drawLightBulb(bool on);

private:
  TFT_eSPI tft;
};

#endif