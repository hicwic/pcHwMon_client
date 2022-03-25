#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library

//----------for the TMP36 temp sensor---------
#define aref_voltage 3.3

//define of colors scheme
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#define TFT_W 320   //320
#define TFT_H 480   //480
#define TFT_DC 9
#define TFT_CS 10

#define FRAME_COLOR TFT_RED  //TFT_RED
#define FRAME_INNER_COLOR TFT_BLUE  //TFT_RED
#define FRAME_PADDING 10
#define FRAME_TITLE_H 25
#define FRAME_INFO_W 100
#define FRAME_LINE_SPACING 3

#define GRAPH_COLOR TFT_WHITE
//#define GRAPH_H 70


//define Keys (inherited from pcHwMon_server )
#define KEYCPUNAME 1
#define KEYCPUTEMP 2
#define KEYCPULOAD 3
#define KEYRAMLOAD 4

#define KEYGPUNAME 5
#define KEYGPUTEMP 6
#define KEYGPULOAD 7
#define KEYGPUFAN  8

#define KEYMBDTEMP 9
#define KEYDD1TEMP 10
#define KEYDD2TEMP 11

#define KEYFPS     12


const int RING_RADIUS = (TFT_W/2-FRAME_PADDING*2)/2;
const int RING_H = RING_RADIUS+FRAME_PADDING;

const int FRAME_BOTTOM_Y = FRAME_TITLE_H + RING_H*3.3 + FRAME_PADDING;

const int GRAPH_W = TFT_W - FRAME_INFO_W - FRAME_PADDING*2;
const int GRAPH_H = TFT_H - FRAME_BOTTOM_Y - 60 - FRAME_PADDING; //60 height of 3 lines (header, detailed FPS, current fps)

int graphBuffer[GRAPH_W];
int graphIndex = 0;


MCUFRIEND_kbv tft;



void setup() {
  Serial.begin(9600);
  //------------INIT FOR MCUFRIEND_KVB LIBRARY
  uint16_t ID = tft.readID();
  if (ID == 0xD3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  //Middle Vertical Line
  tft.drawRect(TFT_W/2-1, 1, 3, FRAME_BOTTOM_Y-1, FRAME_INNER_COLOR);
  tft. drawLine(TFT_W/2, 0, TFT_W/2, FRAME_BOTTOM_Y, FRAME_COLOR);

  //Bottom Horizontal Line  
  tft.drawRect(1, FRAME_BOTTOM_Y-1, TFT_W-2, 3, FRAME_INNER_COLOR);
  tft. drawLine(0, FRAME_BOTTOM_Y, TFT_W, FRAME_BOTTOM_Y, FRAME_COLOR);

  //Bottom Vertical Line
  tft.drawRect(FRAME_INFO_W-1, FRAME_BOTTOM_Y-1, 3, TFT_H-FRAME_BOTTOM_Y, FRAME_INNER_COLOR);
  tft. drawLine(FRAME_INFO_W, FRAME_BOTTOM_Y, FRAME_INFO_W, TFT_H, FRAME_COLOR);

  //Main Frame
  tft.drawRect(0, 0, TFT_W, TFT_H, FRAME_COLOR);
  tft.drawRect(1, 1, TFT_W-2, TFT_H-2, FRAME_INNER_COLOR);  

  //Print titles
  tft.setTextSize(2);
  tft.setTextColor(FRAME_INNER_COLOR, TFT_BLACK);
  drawHCenteredString("CPU", TFT_W/4+1, 3+1);
  drawHCenteredString("GPU", TFT_W/4*3+1, 3+1);
  tft.setTextColor(FRAME_COLOR);
  drawHCenteredString("CPU", TFT_W/4, 3);
  drawHCenteredString("GPU", TFT_W/4*3, 3);

  //Print subtitles
  tft.setTextSize(1);
  drawHCenteredString("Core I5-11400F", TFT_W/4, 20);
  drawHCenteredString("Geforce RTX 3060", TFT_W/4*3, 20);

  //Init FPS graph array to -1 (no value)
  for(int i=0; i<GRAPH_W; i++)
  {
    graphBuffer[i] = -1;
  }

}

void loop() {
  while (Serial.available() > 0)
  {

    String received = Serial.readStringUntil(':');
    //-------HANDSHAKE---------
    if (received == "*****") {
      Serial.println('R');
      break;
    }
    else {
      Serial.println("Instruction received : " + received);
      int key = atoi(received.c_str());
      int value = atoi(Serial.readStringUntil(';').c_str());

      Serial.println("Parsed to key : " + String(key) + ", value:" + String(value));
      updateMetric(key, value);
    }
  }
}

void updateMetric(int key, int value) {
  switch (key)
  {
  case KEYCPULOAD:
    ringMeter(value, 0, 100, FRAME_PADDING, FRAME_TITLE_H + RING_H + FRAME_PADDING, RING_RADIUS, "%", "usage", GREEN2RED); // Draw analogue meter
    break;
  case KEYCPUTEMP:
    ringMeter(value, 0, 100, FRAME_PADDING, FRAME_TITLE_H + FRAME_PADDING, RING_RADIUS, "C", "temp", BLUE2RED);
    break;
  case KEYRAMLOAD:
    //ram usage
    ringMeter(value, 0, 100, FRAME_PADDING, FRAME_TITLE_H + RING_H*2 + FRAME_PADDING, RING_RADIUS, "%", "ram", GREEN2RED); // Draw analogue meter
    break;
  case KEYGPUTEMP:
    //gpu temp
    ringMeter(value, 0, 100, TFT_W/2+FRAME_PADDING, FRAME_TITLE_H + FRAME_PADDING, RING_RADIUS, "C", "temp", BLUE2RED); // Draw analogue meter
    break;
  case KEYGPULOAD:
    //gpu usage
    ringMeter(value, 0, 100, TFT_W/2+FRAME_PADDING, FRAME_TITLE_H + RING_H + FRAME_PADDING, RING_RADIUS, "%", "usage", GREEN2RED); // Draw analogue meter
    break;
  case KEYGPUFAN:
    //gpu fan
    ringMeter(value, 0, 100, TFT_W/2+FRAME_PADDING, FRAME_TITLE_H + RING_H*2 + FRAME_PADDING, RING_RADIUS, "%", "fan", BLUE2BLUE);
    break;
  case KEYFPS:
    drawFPS(value, FRAME_INFO_W+1, FRAME_BOTTOM_Y);
    break;

  default:
    break;
  }



}


// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const String &units, const String &legend, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = 20;//r / 3;    // Width of outer ring is 1/4 of radius
  
  int angle = 110;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 6; // Draw segments every 5 degrees, increase to 10 for segmented ring

  tft.fillRect(x-r/2, y-r/2, r, r/2, TFT_BLACK);

  // Set the text colour to default
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawHCenteredString(String(value)+units, x, y-20);

  tft.setTextSize(1);
  drawHCenteredString(legend, x, y-5);

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = TFT_RED; break; // Fixed colour
      case 1: colour = TFT_GREEN; break; // Fixed colour
      case 2: colour = TFT_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = TFT_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_DARKGREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_DARKGREY);
    }
  }

  // Calculate and return right hand side x coordinate
  return x + r;
}


int barMeter(int value, int vmin, int vmax, int x, int y, int w, int h, const String &units, const String &legend, byte scheme)
{
  int rangedValue = map(value, vmin, vmax, 0, w-1);

  tft.fillRect(x, y, w, h, TFT_DARKGREY);

  for (int i=1; i<rangedValue; i++)
  {
    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = TFT_RED; break; // Fixed colour
      case 1: colour = TFT_GREEN; break; // Fixed colour
      case 2: colour = TFT_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, 1, w-1, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, 1, w-1, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, 1, w-1, 127, 63)); break; // Red to green (low battery etc)
      default: colour = TFT_BLUE; break; // Fixed colour
    }    

    tft.drawLine(x+i, y+1, x+i, y+h-2, colour);

  }

  tft.setTextColor(FRAME_COLOR);
  tft.setTextSize(1);
  drawVHCenteredString(String(value)+units+" "+legend, x+w/2, y+h/2);

}


// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}

void drawHCenteredString(const String &buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setCursor(x - w / 2, y);
  tft.print(buf);
}

void drawVHCenteredString(const String &buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setCursor(x - w / 2, y - h /2 );
  tft.print(buf);
}


void drawHRAlignedString(const String &buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setCursor(x - w , y);
  tft.print(buf);
}

void drawInfo(const String &buf, int line)
{
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(buf, 5, FRAME_BOTTOM_Y, &x1, &y1, &w, &h); //calc width of new string  
  tft.setCursor(5, FRAME_BOTTOM_Y+line*(h+FRAME_LINE_SPACING));
  tft.print(buf);
}

void drawFPS(int value, int x, int y)
{

  graphBuffer[graphIndex] = value;

  char curFPS[4] = {'-','-','-'};
  char avgFPS[4] = {'-','-','-'};
  char minFPS[4] = {'-','-','-'};
  char maxFPS[4] = {'-','-','-'};

  int graphW = TFT_W-3-x;
  int graphH = TFT_H-3-(y+50);

  int minValue = 999;
  int maxValue = 0;
  int avgValue = 0;
  int nbValue = 0;

  for(int i=0; i<GRAPH_W; i++)
  {
    if (graphBuffer[i] == -1) {
      continue;
    }
    if (graphBuffer[i] < minValue) {
      minValue = graphBuffer[i];
    }
    if (graphBuffer[i] > maxValue) {
      maxValue = graphBuffer[i];
    }

    avgValue += graphBuffer[i];
    nbValue++;
  }

  avgValue /= nbValue;

  if (value != -1) sprintf(curFPS, "%3d", value);
  if (avgValue != 0) sprintf(avgFPS, "%3d", avgValue);
  if (minValue != 999) sprintf(minFPS, "%3d", minValue);
  if (maxValue != 0) sprintf(maxFPS, "%3d", maxValue);

  minValue -= 10;
  maxValue += 10;
  int rangeValue = maxValue-minValue;

  // shift graph to the left pixel by pixel (maybe could do better)
  for(int i=1; i <= GRAPH_W; i++) 
  {
    int value = graphBuffer[(i+graphIndex)%GRAPH_W];
    int nxtValue = graphBuffer[(i+1+graphIndex)%GRAPH_W];

    //we clear the next horizontal line if nxtValue != -1
    if (nxtValue != -1) {
      tft.drawLine(FRAME_INFO_W + FRAME_PADDING + i + 1, TFT_H-3-GRAPH_H, FRAME_INFO_W + FRAME_PADDING + i + 1, TFT_H-3, TFT_BLACK);
    }

    if (value == -1) {
      continue;
    }

    if (nxtValue == -1) {
      tft.drawPixel(FRAME_INFO_W + FRAME_PADDING + i, TFT_H-3-GRAPH_H + (GRAPH_H-float(value-minValue)/rangeValue*GRAPH_H), GRAPH_COLOR);
    }
    else {
      tft.drawLine(FRAME_INFO_W + FRAME_PADDING + i, TFT_H-3-GRAPH_H + (GRAPH_H-float(value-minValue)/rangeValue*GRAPH_H), FRAME_INFO_W + FRAME_PADDING + i +1, TFT_H-3-GRAPH_H + (GRAPH_H-float(nxtValue-minValue)/rangeValue*GRAPH_H), GRAPH_COLOR);
    }

  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  
  // min | avg | max | cur 
  drawHRAlignedString("min | avg | max",TFT_W-3, FRAME_BOTTOM_Y + 3);
  drawHRAlignedString(String(minFPS) + " | " + String(avgFPS) + " | " + String(maxFPS),TFT_W-3, FRAME_BOTTOM_Y + 3 + 20);
  drawHRAlignedString(String(curFPS) + " fps",TFT_W-3, FRAME_BOTTOM_Y + 3 + 40);

  graphIndex = (graphIndex+1)%GRAPH_W;
}
