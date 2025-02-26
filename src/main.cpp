/*
  CYD Marble Clock - with a reserved top region for the date

  Key idea:
    - We define dateBarHeight to reserve a region at the top of the screen.
    - All marble paths are computed and drawn below that region.
*/

#include <Arduino.h>
#include <math.h>
#include "charbitmap.h"
#include "point2D.h"
#include "node.h"

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

/*-------- WiFi and NTP ----------*/
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Update these with your WiFi network settings:
const char* ssid = "JN24";
const char* password = "D3skt0pK1ng";

WiFiUDP ntpUDP;
// For Pacific Standard Time (UTC-8) use an offset of -28800 seconds.
NTPClient timeClient(ntpUDP, "pool.ntp.org", -28800, 60000);

/*-------- CYD (Cheap Yellow Display) ----------*/
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();             // Invoke custom library
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite class

uint16_t marbleDarkColor = TFT_BLACK;
uint16_t marbleLightColor = TFT_WHITE;
uint16_t noMarbleColor = TFT_LIGHTGREY;
uint16_t backgroundColor = TFT_RED;   // We'll use red for the main background so you can see it clearly

// If your TFT_eSPI doesn't define TFT_DARKGREY, you can define or replace it:
#ifndef TFT_DARKGREY
  #define TFT_DARKGREY 0x7BEF
#endif

// ------------------------------
// Reserve some vertical space at the top for the date
int dateBarHeight = 40; // Adjust to suit your screen size
// ------------------------------

int dotSize = 4;            // in pixels
int gridSize = dotSize + 1; // 1 for spacing between dots
int charWidthInPixels = charWidth * gridSize;
int charHeightInPixels = charHeight * gridSize;
int minR = 8;
int charSpacing = 2 * minR + 1;

//-- Paths --
const int curveMax = 33;
Point2D curveCoords[charHeight][curveMax];

const int numDigits = 5;
const int numColumns = numDigits * charWidth;
const int pathMax = curveMax + numColumns + numColumns; 
Node paths[pathMax][charHeight];
int stopPoints[charHeight];
int pathLengths[charHeight];

// We'll shift all marble coordinates down by `dateBarHeight`.
int horizOffset = charHeightInPixels + minR; // Left offset of the straight channels

enum marbleCodes
{
  white = '*',
  black = '.',
  none = ' '
};

//-- Hidden Bunk --
const int columnMax = charWidth * 5;
char hiddenBunk[columnMax][charHeight];

//-- States --
enum States
{
  GetNewTime = 0,
  IsFillingTopBunk = 1,
  IsClearingBottomBunk = 3,
  IsFillingBottomBunk = 4
};
States state = GetNewTime;

//-- Animation Variables --
unsigned long timeToRelease;
int hiddenColumnIndex[charHeight];
int fallingOffBottomBunkColumn;

//-- Clock --
String timeNowStr = "";
String upcomingTimeStr = "";


/*-----------------------------------------------
   Setup and Path Computation
 -----------------------------------------------*/

void SetupCYD()
{
  tft.init();
  tft.setRotation(1); // 1=Landscape with USB on right
  tft.fillScreen(backgroundColor);  // Fill entire screen with 'backgroundColor'
  tft.setTextColor(TFT_WHITE, backgroundColor);
}

void ComputePaths()
{
  Debug("Begin ComputePaths");

  // We'll define an "availableHeight" for the marble clock portion
  // so that everything is drawn below the date bar.
  int availableHeight = tft.height() - dateBarHeight;

  // We'll shift the swirl origin down by dateBarHeight.
  Point2D origin = {
    charHeightInPixels + minR - gridSize,
    dateBarHeight + (availableHeight - 1) - charHeightInPixels - minR
  };

  int r = minR;
  for (int row = 0; row < charHeight; row++)
  {
    int index = 0;
    // Top straight path (right to left)
    for (int x = numColumns - 1; x >= 0; x--)
    {
      // Instead of tft.height() - 1, we do dateBarHeight + (availableHeight - 1).
      int y = dateBarHeight + (availableHeight - 1) - charHeightInPixels - charSpacing - row * gridSize;
      paths[index++][row] = {{horizOffset + x * gridSize, y}, none};
    }

    // Curved portion
    float spacing = dotSize + 2.5;
    int curveMaxCalc = PI * r / spacing;
    int c2 = curveMaxCalc / 4;
    stopPoints[row] = index;
    for (int c = curveMaxCalc; c >= 0; c--)
    {
      float angle = PI / 2 + (PI * c / curveMaxCalc);
      Point2D p;
      p.x = origin.x + r * cos(angle);
      p.y = origin.y + r * sin(angle);
      if (c == c2)
        stopPoints[row] = index;
      paths[index++][row] = {{p.x, p.y}, none};
    }

    // Bottom straight path (left to right)
    for (int x = 0; x < numColumns; x++)
    {
      // Instead of tft.height() - 1, we do dateBarHeight + (availableHeight - 1).
      int y = dateBarHeight + (availableHeight - 1) - gridSize - (charHeight - 1 - row) * gridSize;
      paths[index++][row] = {{horizOffset + x * gridSize, y}, none};
    }

    pathLengths[row] = index;
    r += gridSize;

    Debug("Row " + String(row) + ": pathLength=" + String(pathLengths[row]) +
          " StopPoint=" + String(stopPoints[row]));
  }

  Debug("Leaving ComputePaths");
}

/*-----------------------------------------------
   Drawing the date bar at the top
 -----------------------------------------------*/

void drawDateBar()
{
  // Fill the top region with dark grey
  tft.fillRect(0, 0, tft.width(), dateBarHeight, TFT_DARKGREY);

  // Grab the current date
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = localtime(&epochTime);
  char dateString[20];
  sprintf(dateString, "%04d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

  // Draw the date in white, centered horizontally, near the middle of dateBarHeight
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(2);

  int w = tft.textWidth(dateString);
  int h = tft.fontHeight();

  int posX = (tft.width() - w) / 2;
  int posY = (dateBarHeight - h) / 2;
  tft.setCursor(posX, posY);
  tft.print(dateString);
}

/*-----------------------------------------------
   State Machine and Marble Clock Logic
 -----------------------------------------------*/

void GoIdleAnimations()
{
  // (Optional) Place any idle animations here
  // For example, you might raise marbles from a bottom bucket to a top bucket, etc.
}

void GoGetNewTime()
{
  Debug("Begin GoGetNewTime");
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime(); // "HH:MM:SS"
  upcomingTimeStr = formattedTime.substring(0, 5);      // "HH:MM"
  timeNowStr = upcomingTimeStr;
  timeToRelease = millis() + 20000;
  Debug("upcomingTimeStr", upcomingTimeStr);

  // Prepare hidden bunk
  for (byte digitIndex = 0; digitIndex < 5; digitIndex++)
  {
    int bitmapIndex = upcomingTimeStr[digitIndex] - '0';
    if (upcomingTimeStr[digitIndex] == ':') {
      bitmapIndex = 10; // If colon is index 10 in charbitmap
    }
    for (int c = 0; c < charWidth; c++)
    {
      int column = digitIndex * charWidth + (charWidth - 1 - c);
      for (int r = 0; r < charHeight; r++)
      {
        int invertR = charHeight - 1 - r;
        hiddenBunk[column][r] = bitRead(charbitmap[bitmapIndex][c], invertR) == 1 ? white : black;
      }
    }
  }
  for (int row = 0; row < charHeight; row++)
    hiddenColumnIndex[row] = numColumns - 1;

  state = IsFillingTopBunk;
  Debug("Leaving GoGetNewTime");
}

void ShiftMarblesInTopBunk()
{
  Debug("Begin ShiftMarblesInTopBunk");
  for (int row = 0; row < charHeight; row++)
  {
    int stopPoint = stopPoints[row];
    for (int i = stopPoint; i > 0; i--)
    {
      if (paths[i][row].content == none)
      {
        paths[i][row].content = paths[i - 1][row].content;
        paths[i - 1][row].content = none;
      }
    }
  }
  Debug("Leaving ShiftMarblesInTopBunk");
}

void RefillTopBunkRightmostColumn(int row)
{
  if (hiddenColumnIndex[row] >= 0)
  {
    paths[0][row].content = hiddenBunk[hiddenColumnIndex[row]][row];
    hiddenColumnIndex[row]--;
  }
  else
  {
    paths[0][row].content = none;
  }
}

void RefillTopBunkRightmostColumn()
{
  for (int row = 0; row < charHeight; row++)
  {
    RefillTopBunkRightmostColumn(row);
  }
}

void GoAnimateMarbleCollection()
{
  ShiftMarblesInTopBunk();
  RefillTopBunkRightmostColumn();
  if (millis() >= timeToRelease)
  {
    state = IsClearingBottomBunk;
    fallingOffBottomBunkColumn = 0;
  }
}

void DropBottomBunkColumn(int colR)
{
  int rMax = charHeight - 1;
  for (int row = rMax; row > 0; row--)
  {
    int srcR = row - 1;
    char t = paths[pathLengths[row] - colR][row].content;
    paths[pathLengths[row] - colR][row].content = paths[pathLengths[srcR] - colR][srcR].content;
    paths[pathLengths[srcR] - colR][srcR].content = t;
  }
  paths[pathLengths[0] - colR][0].content = none;
}

void GoClearBottomBunk()
{
  for (int c = 0; c <= fallingOffBottomBunkColumn; c++)
  {
    DropBottomBunkColumn(c);
  }
  if (fallingOffBottomBunkColumn < numColumns)
  {
    fallingOffBottomBunkColumn++;
  }

  // Once the bottom bunk is empty, move on
  int rMax = charHeight - 1;
  if (paths[pathLengths[rMax] - columnMax][rMax].content == none)
  {
    state = IsFillingBottomBunk;
  }
}

void GoFillBottomBunk()
{
  bool isStillRolling = false;
  for (int row = 0; row < charHeight; row++)
  {
    int pathMaxVal = pathLengths[row];
    for (int i = pathMaxVal - 2; i > 0; i--)
    {
      char marbleTo = paths[i + 1][row].content;
      if (marbleTo == none)
      {
        char movingMarble = paths[i][row].content;
        paths[i + 1][row].content = movingMarble;
        paths[i][row].content = none;
        if (movingMarble != none) isStillRolling = true;
      }
    }
  }
  if (!isStillRolling)
    state = GetNewTime;
}

/*-----------------------------------------------
   Drawing the Marbles in the Paths
 -----------------------------------------------*/

void DrawNode(Node node)
{
  Point2D p = node.coord;
  bool isWhiteMarble = node.content == white;
  bool isBlackMarble = node.content == black;
  bool isMarble = isWhiteMarble || isBlackMarble;

  uint32_t nodeColor = noMarbleColor; // default
  if (isWhiteMarble) nodeColor = marbleLightColor;
  if (isBlackMarble) nodeColor = marbleDarkColor;

  // Draw a small square or rectangle
  tft.fillRect(p.x, p.y, dotSize, dotSize, nodeColor);

  // Optional highlight/border
  if (isMarble)
  {
    tft.drawPixel(p.x, p.y, noMarbleColor);
    tft.drawPixel(p.x + dotSize - 1, p.y, noMarbleColor);
    tft.drawPixel(p.x, p.y + dotSize - 1, noMarbleColor);
    tft.drawPixel(p.x + dotSize - 1, p.y + dotSize - 1, noMarbleColor);
  }
}

void DrawPaths()
{
  for (int row = 0; row < charHeight; row++)
  {
    int pathMaxVal = pathLengths[row];
    for (int i = 0; i < pathMaxVal; i++)
    {
      Node node = paths[i][row];
      DrawNode(node);
    }
  }
}

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

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected.");
  timeClient.begin();

  SetupCYD();
  ComputePaths();
}

void loop()
{
  // Draw the top bar for date
  drawDateBar();

  // Optional idle animations
  GoIdleAnimations();

  // Marble clock state machine
  switch (state)
  {
    case GetNewTime:
      GoGetNewTime();
      break;
    case IsFillingTopBunk:
      GoAnimateMarbleCollection();
      break;
    case IsClearingBottomBunk:
      GoClearBottomBunk();
      break;
    case IsFillingBottomBunk:
      GoFillBottomBunk();
      break;
  }

  // Draw the marbles in the paths (below the top date bar)
  DrawPaths();
}
