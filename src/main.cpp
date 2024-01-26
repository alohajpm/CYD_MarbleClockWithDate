/*
  CYD Marble Clock
  Inspired by Ivan Miranda's Marble Clock
*/

#include <Arduino.h>
#include <math.h>
#include "charbitmap.h"
#include "point2D.h"
#include "node.h"

#define TOUCH_CS // This sketch does not use touch, but this is defined to quiet the warning about not defining touch_cs.

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

/*-------- CYD (Cheap Yellow Display) ----------*/
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();              // Invoke custom library
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite class

uint16_t marbleDarkColor = TFT_BLACK;
uint16_t marbleLightColor = TFT_WHITE;
uint16_t noMarbleColor = TFT_LIGHTGREY;

uint16_t backgroundColor = TFT_CYAN;
int dotSize = 4;            // in pixels
int gridSize = dotSize + 1; // 1 for spacing between dots
int charWidthInPixels = charWidth * gridSize;
int charHeightInPixels = charHeight * gridSize;
int minR = 8;
int charSpacing = 2 * minR + 1;

//-- Paths --
const int curveMax = 33;
Point2D curveCoords[charHeight][curveMax];

const int pathMax = curveMax + 5 * charWidth + 5 * charWidth; // 5 chars on top bunk + longest curve + 5 chars on bottom bunk
Node paths[pathMax][charHeight];
int stopPoints[charHeight]; // Where marble should pile up while waiting for time to be released into bottom bunk
int pathLengths[charHeight]; // Although paths array is allocated for pathMax, most rows do not use the full capacity

int horizOffset = charHeightInPixels + minR; // Left offset of the straight channels

enum marbleCodes
{
  white = '*',
  black = '.',
  none = ' '
};

//-- Hidden Bunk --
const int columnMax = charWidth * 5;
char hiddenBunk[columnMax][charHeight]; // This is the buffer containing all the marbles to be animated into the top bunk
// Node topBunk[charHeight][pathMax];
// Node bottomBunk[charHeight][pathMax];
// int topBunkLength, bottomBunkLength; // How many of the pathMax is actual in use.

//-- Top Buckets --
int topBucketHeight = 10;
int blackBucketWidth = tft.width() * 3 / 4;
int whiteBucketWidth = tft.width() - blackBucketWidth;

//-- States --
enum States
{
  GetNewTime=0,
  IsFillingTopBunk=1,
  //IsAwaitingRelease=2,
  IsClearingBottomBunk=3,
  IsFillingBottomBunk=4
};
States state = GetNewTime;


//-- Animation Variables --
unsigned long timeToRelease;
int currentColumnIndex;
//char currentColumnContent[charHeight]; // data for current column being shifted into top bunk

//-- Clock --
String timeNowStr = "";
String upcomingTimeStr;

void SetupCYD()
{
  tft.init();
  tft.setRotation(1); // 1=Landscape USB on right. 2=Portrait USB on top
  tft.fillScreen(backgroundColor);
  tft.setTextColor(marbleLightColor, backgroundColor);
}

// void Compute1Curve(int row, int r)
// {
//   float spacing = dotSize + 2.5; // Make the spacing slightly bigger than the marble/dot size to ensure no overlaps
//   int n = PI * r / spacing;
//   int n2 = n*3/4;
//   Debug("Max # of marbles in the curve", n);
//   Point2D origin = {charHeightInPixels + minR - gridSize, tft.height() - 1 - charHeightInPixels - minR};
//   for (int i = 0; i <= n; i++)
//   {
//     float angle = PI / 2 + (PI * i / n);
//     Point2D p;
//     p.x = origin.x + r * cos(angle);
//     p.y = origin.y + r * sin(angle);
//     curveCoords[row][i] = p;




//     tft.fillRect(p.x, p.y, dotSize, dotSize, TFT_BLUE);
//     tft.drawPixel(p.x, p.y, TFT_BLUE);
//     tft.drawPixel(p.x + dotSize - 1, p.y, TFT_BLUE);
//     tft.drawPixel(p.x, p.y + dotSize - 1, TFT_BLUE);
//     tft.drawPixel(p.x + dotSize - 1, p.y + dotSize - 1, TFT_BLUE);
//   }
// }

void ComputePaths()
{
  Debug("Begin ComputePaths");
  Point2D origin = {charHeightInPixels + minR - gridSize, tft.height() - 1 - charHeightInPixels - minR};
  int r = minR;
  for (int row = 0; row < charHeight; row++)
  {
    int index = 0;
    // Start by filling the top straight path (right to left)
    for (int x = 5 * charWidth-1; x >= 0; x--)
    {
      int y = tft.height() - 1 - charHeightInPixels - charSpacing - row * gridSize;
      paths[index++][row] = {{horizOffset + x * gridSize, y},none}; // each element of paths is an x,y coordinate of that spot and what's at that spot.
    }

    // Followed by the curved coords
    float spacing = dotSize + 2.5; // Make the spacing slightly bigger than the marble/dot size to ensure no overlaps
    int curveMax = PI * r / spacing;
    int c2 = curveMax*1/4;
    stopPoints[row]=index; // Default stop point to left side of top bunk because the loop does not set it for row 0.

    for (int c = curveMax; c >= 0; c--)
    {
      float angle = PI / 2 + (PI * c / curveMax);
      Point2D p;
      p.x = origin.x + r * cos(angle);
      p.y = origin.y + r * sin(angle);
      if (c==c2) stopPoints[row]=index; // This is where top bunk marbles will stop awaiting release to bottom bunk
      paths[index++][row] = {{p.x, p.y},none};
    }

    // Finish by filling the bottom straight path (left to right)
    for (int x = 0; x < (5 * charWidth); x++)
    {
      int y = tft.height() - 1 - gridSize - (charHeight - 1 - row) * gridSize;
      paths[index++][row] = {{horizOffset + x * gridSize, y},none};
    }

    pathLengths[row] = index;
    r = r + gridSize; // Make the curve radius larger each time we loop to next row of the bunk

    Debug("Row" + String(row) + ": pathLength="+ pathLengths[row] + " StopPoint=" + stopPoints[row]);
  }

  Debug("Leaving ComputePaths\n");
}

// void TestPath()
// {
//   for (int i = 0; i < pathMax; i++)
//   {
//     for (int row = 0; row < charHeight; row++)
//     {
//       Point2D p = paths[row][i].coord;
//       tft.fillRect(p.x, p.y, dotSize, dotSize, TFT_YELLOW);
//       tft.drawPixel(p.x, p.y, TFT_BLUE);
//       tft.drawPixel(p.x + dotSize - 1, p.y, TFT_BLUE);
//       tft.drawPixel(p.x, p.y + dotSize - 1, TFT_BLUE);
//       tft.drawPixel(p.x + dotSize - 1, p.y + dotSize - 1, TFT_BLUE);
//     }

//     //delay(20);

//     for (int row = 0; row < charHeight; row++)
//     {
//       Point2D p = paths[row][i].coord;
//       tft.fillRect(p.x, p.y, dotSize, dotSize, TFT_BLUE);
//     }
//   }
// }

// void TestPattern()
// {
//   tft.fillRect(0, 0, tft.width() / 2, 1, TFT_YELLOW);
// }

// void DrawChar(int x, int y, char ch) // x,y is bottom left of where char should be drawn
// {
//   for (int col = 0; col < charWidth; col++)
//   {
//     int colX = x + (charWidth - col) * (dotSize + 1); // Columns are drawn right to left
//     for (int row = 0; row < charHeight; row++)
//     {
//       int rowY = y - row * gridSize;
//       int colBits = charbitmap[ch - '0'][col];
//       bool isOn = bitRead(colBits, row); // Least Significant bit is lowest vertically
//       tft.fillRect(colX, rowY, dotSize, dotSize, isOn ? TFT_YELLOW : TFT_BLUE);
//       tft.drawPixel(colX, rowY, TFT_BLUE);
//       tft.drawPixel(colX + dotSize - 1, rowY, TFT_BLUE);
//       tft.drawPixel(colX, rowY + dotSize - 1, TFT_BLUE);
//       tft.drawPixel(colX + dotSize - 1, rowY + dotSize - 1, TFT_BLUE);
//     }
//   }
// }

// void TestChars()
// {
//   for (int d = 0; d < 10; d++)
//   {
//     int y = d >= 5 ? tft.height() - 1 - gridSize : tft.height() - 1 - charHeightInPixels - charSpacing;
//     DrawChar(horizOffset + (d % 5) * charWidthInPixels, y, '0' + d);
//   }
// }

// void TestCurve(int row, int r)
// {
//   float spacing = dotSize + 2.5; // Make the spacing slightly bigger than the marble/dot size to ensure no overlaps
//   int n = PI * r / spacing;
//   Debug("Max # of marbles in the curve", n);
//   Point2D origin = {charHeightInPixels + minR - gridSize, tft.height() - 1 - charHeightInPixels - minR};
//   for (int i = 0; i <= n; i++)
//   {
//     float angle = PI / 2 + (PI * i / n);
//     Point2D p;
//     p.x = origin.x + r * cos(angle);
//     p.y = origin.y + r * sin(angle);
//     curveCoords[row][i] = p;

//     tft.fillRect(p.x, p.y, dotSize, dotSize, TFT_BLUE);
//     tft.drawPixel(p.x, p.y, TFT_BLUE);
//     tft.drawPixel(p.x + dotSize - 1, p.y, TFT_BLUE);
//     tft.drawPixel(p.x, p.y + dotSize - 1, TFT_BLUE);
//     tft.drawPixel(p.x + dotSize - 1, p.y + dotSize - 1, TFT_BLUE);
//   }
// }

void drawTopBuckets()
{
  // TODO
}

void GoIdleAnimations()
{
  // Move marbles from bottom bucket to top buckets
}

void GoGetNewTime()
{
  Debug("Begin GoGetNewTime");
  if (timeNowStr != "12:34")
  {
    timeNowStr = "12:34";
  
    // Get current time + 1 minute
    upcomingTimeStr = "12:35";
    timeToRelease = millis() + 10000; // This is when we will release top bunk to bottom bunk

    Debug("upcomingTimeStr",upcomingTimeStr);
    Debug("timeToRelease",String(timeToRelease));

    // Prepare HiddenBunk. These are marbles that will gradually fill top bunk.
    for (byte digitIndex = 0; digitIndex < 5; digitIndex++)
    {
        // "draw" digit into the hidden bunk (Left to right)
        int bitmapIndex = upcomingTimeStr[digitIndex]-'0'; // I can't believe that the ascii value of ':' is actually one larger than '9', so the charbitmap just works out!
        for (int c = 0; c < charWidth; c++)
        {
          int column = digitIndex*charWidth + (charWidth-1-c);
          for (int r = 0; r < charHeight; r++)
          {
            int invertR = charHeight-1-r;
            hiddenBunk[column][r] = bitRead(charbitmap[bitmapIndex][c], invertR)==1 ? white : black;
          }
        }
    }

    // Set counter variables
    currentColumnIndex = (5 * charWidth) - 1; // Rightmost will be put into top bunk first, so it will end up on rightmost of bottom bunk
    Debug("currentColumnIndex", currentColumnIndex);
    // for (byte row = 0; row < charHeight; row++)
    // {
    //   paths[0][row].content = hiddenBunk[currentColumnIndex][row]; // (array of '*','.', or ' ')
    //   Debug(String(paths[0][row].content));
    // }

    state = IsFillingTopBunk;
  }
  Debug("Leaving GoGetNewTime\n");
}

void ShiftMarblesInTopBunk()
{
  Debug("Begin ShiftMarblesInTopBunk");
  // Animate marbles en route to pile up awaiting for release into bottom bunk
   for (int row = 0; row < charHeight; row++)
  {
    int stopPoint = stopPoints[row];
    Debug("Row " + String(row) + ": StopPoint="+stopPoint);
    for (int i = stopPoint; i>0; i--)
    {
      if (paths[i][row].content == none) // If target is empty, move the marble to its right one spot left.
      {
        paths[i][row].content = paths[i-1][row].content; // move marble left.
        paths[i-1][row].content = none; // this marble has moved left.
      }
    }
  }
  Debug("Leaving ShiftMarblesInTopBunk\n");
}

void RefillTopBunkRightmostColumn()
{
  Debug("Begin RefillTopBunkRightmostColumn");
  if (currentColumnIndex >= 0) // If all columns from hidden bunk have been shifted into top bunk, then let ShiftMarblesInTopBunk() keep the marbles moving till they hit the stop points.
  {
    // Copy marbles from hidden bunk from right to left because that's how they will end up in the bottom bunk.
    Debug("currentColumnIndex", currentColumnIndex);
    for (byte row = 0; row < charHeight; row++)
    {
      paths[0][row].content = hiddenBunk[currentColumnIndex][row]; // (array of '*','.', or ' ')
      Debug(String(paths[0][row].content));
    }
    currentColumnIndex--;
  }
  else{
   for (byte row = 0; row < charHeight; row++)
      paths[0][row].content = {none};
  }
  Debug("Leaving RefillTopBunkRightmostColumn\n");
}

void GoAnimateMarbleCollection()
{
  Debug("Begin GoAnimateMarbleCollection");
  ShiftMarblesInTopBunk();

  RefillTopBunkRightmostColumn();

  // Even if some marbles are still rolling down in top bunk, release them all to bottom bunk because it's time!
  if (millis() >= timeToRelease)
    state = IsClearingBottomBunk;

  Debug("Leaving GoAnimateMarbleCollection\n");
}


void GoClearBottomBunk()
{
  // Drop previous time into bottom bucket
  //TODO: Animate, and keep track of actual marbles so we could raise them to upper buckets.
  for (int row = 0; row < charHeight; row++)
  {
    int stopPoint = stopPoints[row];
    int pathMax = pathLengths[row];
    for (int i = stopPoint+1; i <= pathMax; i++)
    {
       paths[i][row].content = none;
    }
  }

  state = IsFillingBottomBunk;
}

void GoFillBottomBunk()
{
  // Animate marbles being held on top bunk to flow into bottom bunk
  bool isStillRolling = false;
  for (int row = 0; row < charHeight; row++)
  {
    int pathMax = pathLengths[row];
    for (int i = pathMax-1-1; i > 0 ; i--)
    {
      char marbleTo = paths[i+1][row].content;
       if (marbleTo==none)
       {
        paths[i+1][row].content = paths[i][row].content; // Move marble forward (ignoring stop point)
        paths[i][row].content = none; // this marble has moved forward, make room for other marbles
        isStillRolling=true;
       }
    }
  }

  if (!isStillRolling) state = GetNewTime;
}

void DrawNode(Node node)
{
  Point2D p = node.coord;
  bool isWhiteMarble = node.content == white;
  bool isBlackMarble = node.content == black;
  bool isMarble =  isWhiteMarble || isBlackMarble;
  uint32_t nodeColor = !isMarble? TFT_BLUE : isWhiteMarble? marbleLightColor : marbleDarkColor;
  
  if (isMarble)
  {
    tft.fillRect(p.x, p.y, dotSize, dotSize, nodeColor);
    tft.drawPixel(p.x, p.y, noMarbleColor);
    tft.drawPixel(p.x + dotSize - 1, p.y, noMarbleColor);
    tft.drawPixel(p.x, p.y + dotSize - 1, noMarbleColor);
    tft.drawPixel(p.x + dotSize - 1, p.y + dotSize - 1, noMarbleColor);
  }
  else
    tft.fillRect(p.x, p.y, dotSize, dotSize, noMarbleColor);
}

void DrawPaths()
{
  Debug("Begin DrawPaths");
  for (int row = 0; row < charHeight; row++)
  {
    int pathMax = pathLengths[row];
    //if (row==3) Debug("pathMax",pathMax);
    for (int i = 0; i < pathMax; i++)
    {
      Node node = paths[i][row];
      // if (row==3)
      // {
      //   Debug("'" + String(node.content) +"'");
      // }
      
      DrawNode(node);
    }
  }
  Debug("Leaving DrawPaths\n");
}

void setupIO()
{
  pinMode(0, INPUT_PULLUP);
}

void setup()
{
  Serial.begin(115200);
  SetupCYD();
  ComputePaths();

if (false)
{

  for (size_t i = 0; i < 4; i++)
  {
    Debug("=== " + String(i) + " =========================");
    Debug("state",state);
    
      GoIdleAnimations();

    switch (state)
    {
      case GetNewTime: GoGetNewTime();break; // Get next minute
      case IsFillingTopBunk: GoAnimateMarbleCollection(); break; // Animation (fill top bunk)
      case IsClearingBottomBunk: GoClearBottomBunk(); break; // Before we can release new time into bottom bunk, we need to clear it first.
      case IsFillingBottomBunk: GoFillBottomBunk(); break; // When completed, set state back to GetNewTime
    }

    DrawPaths();
  }
}
}

  /*
  Things that needs to happen in main loop:
  - Animations:
    - Collecting new time marbles into top bunk
    - Releasing new time into lower bunk
    - Releasing prev time into lower bucket
    - Raising marbles to top buckets
    - Sorting marbles into black and white buckets
    - Flow of marbles from top buckets into the top bunk
  - States and flags:
    - new time ("09:11"), new time release (actual millis), IsNewTimeReady (for release into lower bunk)
    - States = GetNewTime, IsFillingTopBunk, IsAwaitingRelease, IsClearingBottomBunnk, IsFillingBottomBunk
    - During IsFillingTopBunk: currentDigitIndex, currentColumnIndex, currentColumnContent (array of '*','.', or ' ')
  - Preps:
    - Paths: Top Bunk (includes top curve), Bottom Bunk (includes bottom part of curve), white elevator, black elevator, white bucket to top bunk, black bucket to top bunk
  */

void loop()
{
  if (true)
  {
   Debug("============================");
    Debug("state",state);
    
      GoIdleAnimations();

    switch (state)
    {
      case GetNewTime: GoGetNewTime();break; // Get next minute
      case IsFillingTopBunk: GoAnimateMarbleCollection(); break; // Animation (fill top bunk)
      case IsClearingBottomBunk: GoClearBottomBunk(); break; // Before we can release new time into bottom bunk, we need to clear it first.
      case IsFillingBottomBunk: GoFillBottomBunk(); break; // When completed, set state back to GetNewTime
    }

    DrawPaths();
  }
}
