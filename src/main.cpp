/*
  CYD Marble Clock - with a reserved top region for the date

  Key idea:
    - We define dateBarHeight to reserve a region at the top of the screen.
    - All marble paths are computed and drawn below that region.
*/

#include <Arduino.h>

#include "home_assistant.h"
#include <WiFi.h>
#include "ui.h"
UI ui;


#define TOUCH_CS // Defined to quiet warnings about touch

/*-------- DEBUGGING ----------*/
void Debug(String label, int val)
{
  Serial.print(label);
  Serial.print("=");
  Serial.println(val);
}
void Debug(String label, String str)
{
  Serial.print(label);
  Serial.print("=");
  Serial.println(str);
}
void Debug(String msg)
{
  Serial.println(msg);
}
/*-------- NTP ----------*/
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
// For Pacific Standard Time (UTC-8) use an offset of -28800 seconds.
NTPClient timeClient(ntpUDP, "pool.ntp.org", -28800, 60000);

/*-------- CYD (Cheap Yellow Display) ----------*/
#include "TFT_eSPI.h" // Hardware-specific library

//-- Clock --



/*-----------------------------------------------
   Setup and Loop
 -----------------------------------------------*/

void setupIO()
{
  pinMode(0, INPUT_PULLUP);
  
}

void setup()
{
  Serial.begin(115200);

  // --- Setup and WiFi ---

  
  HomeAssistant ha("http://192.168.1.10:8123/api/","homeassistant",std::string("password"));

  ui.init();
  
  // --- Setup and Home Assistant ---

  // --- Setup and UI ---

}



void loop()
{
  
  // --- Main Loop ---
  ui.drawMainScreen("12:00", "Jan 1, 2024", "70F", true);
  ui.handleTouch();
}
