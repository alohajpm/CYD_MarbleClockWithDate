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

void SetupCYD()
{
  tft.init();
  tft.setRotation(1); // 1=Landscape USB on right. 2=Portrait USB on top
  tft.fillScreen(clockBackgroundColor);
  tft.setTextColor(clockFontColor, clockBackgroundColor);
}

void TestPattern()
{
  tft.fillRect(0, 0, tft.width() / 2, 1, TFT_YELLOW);
}

void DrawChar(int x, int y, char ch) // x,y is bottom left of where char should be drawn
{
  for (int col=0; col<charWidth; col++)
  {
    int colX = x + (charWidth-1-col)*(dotSize+1); // Columns are drawn right to left
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
  int horizOffset = charHeightInPixels+minR+gridSize;
  for (byte d=0; d<10; d++)
  {
    int y = d>=5 ? tft.height()-1-gridSize : tft.height()-1-charHeightInPixels-charSpacing;
    DrawChar(horizOffset + (d %5)*charWidthInPixels,y,'0'+d);
  }
}

void TestCurve(int r)
{
  float spacing = dotSize+2.5; // Make the spacing slightly bigger than the marble/dot size to ensure no overlaps
  int n =PI*r/spacing;
  Point2D origin = {charHeightInPixels+minR,tft.height()-1-charHeightInPixels-minR};
  for (int i=0; i<=n; i++)
  {
    float angle = PI/2 + (PI*i/n);
    Point2D p;
    p.x = origin.x + r*cos(angle);
    p.y = origin.y + r*sin(angle);

    tft.fillRect(p.x, p.y, dotSize, dotSize, TFT_BLUE);
      tft.drawPixel(p.x, p.y,TFT_BLUE);
      tft.drawPixel(p.x+dotSize-1, p.y,TFT_BLUE);
      tft.drawPixel(p.x,p.y+dotSize-1,TFT_BLUE);
      tft.drawPixel(p.x+dotSize-1, p.y+dotSize-1,TFT_BLUE);
  }
}

void setup()
{
  Serial.begin(115200);
  SetupCYD();
  TestPattern();
  TestChars();

  int r = minR+1;
  for (int i=0; i<charHeight; i++)
  {
    TestCurve(r);
    r = r + gridSize;
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
