#ifndef UI_H
#define UI_H

#include <TFT_eSPI.h>

class UI {
public:
  UI();
  void drawMainScreen(String time, String date, String temp, bool lightState);
  void drawSettingsScreen();
  void handleTouch(int x, int y);
  void drawLightBulb(bool on);

private:
  TFT_eSPI tft;
};

#endif