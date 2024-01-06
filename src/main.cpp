/*
  CYD Marble Clock
  Inspired by Ivan Miranda's Marble Clock
*/

#include <Arduino.h>
#include <math.h>
#include "charbitmap.h"
#include "point2D.h"

#define TOUCH_CS // This sketch does not use touch, but this is defined to quiet the warning about not defining touch_cs.

/*-------- DEBUGGING ----------*/
void Debug(String label, int val)
{
  Serial.print(label);
  Serial.print("=");
  Serial.println(val);
}

/*-------- CYD (Cheap Yellow Display) ----------*/
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();              // Invoke custom library
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite class

uint16_t clockBackgroundColor = TFT_BLACK;
uint16_t clockFontColor = TFT_YELLOW;
int dotSize = 4; // in pixels
int gridSize = dotSize+1; // 1 for spacing between dots
int charWidthInPixels = charWidth*gridSize;
int charHeightInPixels = charHeight*gridSize;
int minR = 8;
int charSpacing = 2*minR + 1;

//-- Paths --
const int curveMax = 33;
const int pathMax = curveMax + 5*charWidth + 5*charWidth; // longest curve + 5 chars on the straight channels
Point2D pathCoords[charHeight][pathMax];
Point2D curveCoords[charHeight][pathMax];
int horizOffset = charHeightInPixels+minR; // Left offset of the straight channels

//-- Top Buckets --
int topBucketHeight = 10;
int blackBucketWidth = tft.width() *3/4;
int whiteBucketWidth = tft.width() - blackBucketWidth;

void SetupCYD()
{
  tft.init();
  tft.setRotation(1); // 1=Landscape USB on right. 2=Portrait USB on top
  tft.fillScreen(clockBackgroundColor);
  tft.setTextColor(clockFontColor, clockBackgroundColor);
}

void ComputePaths()
{
  for (int row=0; row<charHeight; row++) 
  {
    int index=0;
    // Start by filling the top straight path (right to left)
    for (int x=5*charWidth; x>=0; x--)
    {
      int y = tft.height()-1-charHeightInPixels-charSpacing-row*gridSize;
      pathCoords[row][index++] = {horizOffset + x*gridSize, y};
    }

    //Followed by the curved coords
    for (int c=curveMax-1; c>=0; c--)
    {
      Point2D p = curveCoords[row][c];
      if (p.x!=0 || p.y!=0)
        pathCoords[row][index++] = {p.x,p.y};
    }

    // Finish by filling the bottom straight path (left to right)
    for (int x=0; x<(5*charWidth); x++)
    {
      int y = tft.height()-1-gridSize-(charHeight-1-row)*gridSize;
      pathCoords[row][index++] = {horizOffset + x*gridSize, y};
    }
  }
}

void TestPath()
{
  for (int i=0; i<pathMax; i++)
  {
    for (int row=0; row<charHeight; row++)
    {
      Point2D p = pathCoords[row][i];
      tft.fillRect(p.x, p.y, dotSize, dotSize,TFT_YELLOW);
      tft.drawPixel(p.x, p.y,TFT_BLUE);
      tft.drawPixel(p.x+dotSize-1, p.y,TFT_BLUE);
      tft.drawPixel(p.x,p.y+dotSize-1,TFT_BLUE);
      tft.drawPixel(p.x+dotSize-1, p.y+dotSize-1,TFT_BLUE);
    }

    delay(20);

    for (int row=0; row<charHeight; row++)
    {
      Point2D p = pathCoords[row][i];
      tft.fillRect(p.x, p.y, dotSize, dotSize,TFT_BLUE );
    }
  }
}

void TestPattern()
{
  tft.fillRect(0, 0, tft.width() / 2, 1, TFT_YELLOW);
}

void DrawChar(int x, int y, char ch) // x,y is bottom left of where char should be drawn
{
  for (int col=0; col<charWidth; col++)
  {
    int colX = x + (charWidth-col)*(dotSize+1); // Columns are drawn right to left
    for (int row=0; row<charHeight; row++) 
    {
      int rowY = y - row*gridSize;
      int colBits = charbitmap[ch-'0'][col];
      bool isOn = bitRead(colBits, row); // Least Significant bit is lowest vertically
      tft.fillRect(colX, rowY, dotSize, dotSize, isOn ? TFT_YELLOW : TFT_BLUE);
      tft.drawPixel(colX,rowY,TFT_BLUE);
      tft.drawPixel(colX+dotSize-1,rowY,TFT_BLUE);
      tft.drawPixel(colX,rowY+dotSize-1,TFT_BLUE);
      tft.drawPixel(colX+dotSize-1,rowY+dotSize-1,TFT_BLUE);
    }
  }
}

void TestChars()
{
  for (int d=0; d<10; d++)
  {
    int y = d>=5 ? tft.height()-1-gridSize : tft.height()-1-charHeightInPixels-charSpacing;
    DrawChar(horizOffset + (d %5)*charWidthInPixels,y,'0'+d);
  }
}

void TestCurve(int row, int r)
{
  float spacing = dotSize+2.5; // Make the spacing slightly bigger than the marble/dot size to ensure no overlaps
  int n =PI*r/spacing;
  Debug("Max # of marbles in the curve",n);
  Point2D origin = {charHeightInPixels+minR-gridSize,tft.height()-1-charHeightInPixels-minR};
  for (int i=0; i<=n; i++)
  {
    float angle = PI/2 + (PI*i/n);
    Point2D p;
    p.x = origin.x + r*cos(angle);
    p.y = origin.y + r*sin(angle);
    curveCoords[row][i]=p;

    tft.fillRect(p.x, p.y, dotSize, dotSize, TFT_BLUE);
    tft.drawPixel(p.x, p.y,TFT_BLUE);
    tft.drawPixel(p.x+dotSize-1, p.y,TFT_BLUE);
    tft.drawPixel(p.x,p.y+dotSize-1,TFT_BLUE);
    tft.drawPixel(p.x+dotSize-1, p.y+dotSize-1,TFT_BLUE);
  }
}

void drawTopBuckets()
{
  //TODO 
}


void setupIO()
{
  pinMode(0, INPUT_PULLUP);
}

void setup()
{
  Serial.begin(115200);
  SetupCYD();
  //TestPattern();
  TestChars();

  int r = minR+1;
  for (int i=0; i<charHeight; i++)
  {
    TestCurve(i, r);
    r = r + gridSize;
  }

  ComputePaths();
  TestPath();
}

void loop()
{
  while (digitalRead(0)==HIGH)
    delay(100);
  
  TestChars();
  delay(1000);
  TestPath();
}
