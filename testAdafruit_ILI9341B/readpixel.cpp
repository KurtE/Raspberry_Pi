//=================================================================
// Test of the read a pixel and rectangle code of Adafruit_ILI9341
//=================================================================

#include "Adafruit_ILI9341.h"
#include "Adafruit_GFX.h"
#include <signal.h>
#include "WrapperSerial.h"


//#include <math.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <termios.h>
//#include <errno.h>
//#include <fcntl.h>
//#include <string.h>
//#include <ctype.h>
//#include <sys/stat.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <stdarg.h>
//#include <time.h>
//#include <inttypes.h>

//#include "ArduinoDefs.h"

// definition of some helper functions
extern void ScrollTextArea();

/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/



// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

WrapperSerial Serial;

#define READ_TOGGLE_PIN 2

#define YLINES_REPEAT 32
#define MAX_ROWS_PER_PASS 8

//--------------------------------------------------------------------------
// SignalHandler - Try to free up things like servos if we abort.
//--------------------------------------------------------------------------
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    tft.end();  // try to cleanup.
    
   exit(1); 

}

extern void setup(void);
extern void loop(void);

int main()
{
    // Install signal handler to allow us to do some cleanup...
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    Serial.begin();

    setup();

    for(;;)
    {
        loop();
    }
}


void setup() {
//  pinMode(READ_TOGGLE_PIN, OUTPUT);
  // Initialize ILI9341 Display
  tft.begin();
  tft.fillScreen(ILI9341_GREEN);
  tft.fillCircle(50,50,50,ILI9341_RED);
  tft.fillCircle(50,50,40,ILI9341_BLUE);
  tft.setCursor(20,20);
  tft.println("Test Test Test"); 

  // How about some text scrolling.
  tft.fillRect(0, 205, 110, 110, ILI9341_BLACK);

}

void loop() {
  uint16_t x;
  uint16_t y;
  uint16_t z;
  uint16_t col;
  uint16_t awColors[200*MAX_ROWS_PER_PASS];
  uint32_t ulStart, ulDelta;

#if 0
  // First lets try speed of read/write one pixel at a time
  Serial.print("Time for one pixel at a time: ");
  ulStart = millis();
  for(z=0; z<YLINES_REPEAT; z++){
    for(y=0; y<200; y++) {
//      digitalWrite(READ_TOGGLE_PIN, !digitalRead(READ_TOGGLE_PIN));
      for(x=0; x<120; x++) { 
        col = tft.readPixel(x,y);
        tft.drawPixel(x+120,y+z,col);
      }
    }
  }
  Serial.println(ulDelta = millis()-ulStart, DEC);
  ScrollTextArea();
  tft.print("S ");
  tft.print(ulDelta, DEC);
#endif
  // Now lets try reading and writing n whole rows at a time;
  for (int iRowsPerPass = 1; iRowsPerPass <= MAX_ROWS_PER_PASS; iRowsPerPass++) {
    // only for those who divide into 200
    int cLoops = 200 / iRowsPerPass;
    if ((cLoops * iRowsPerPass) != 200) continue;

    Serial.print("Read ");
    Serial.print(iRowsPerPass, DEC);
    Serial.print(" Rows per Pass writerect: ");
    ulStart = millis();

    for(z=0; z<200; z++){
      for(y=0; y<200; y+=iRowsPerPass) {
        tft.readRect(0, y, 120, iRowsPerPass, awColors);  // try to read N rows at a time...
        tft.writeRect(120, y+z, 120, iRowsPerPass, awColors);  // try to write N rows at a time...
      }
    }
    Serial.println(ulDelta = millis()-ulStart, DEC);
    ScrollTextArea();
    tft.print(iRowsPerPass, DEC);
    tft.print("W ");
    tft.print(ulDelta, DEC);
  }

}

void ScrollTextArea(){
  // Assume we are in the area (0, 205) - (110-314)
  uint16_t awColors[110];

  for (int y=205+16; y < 315; y++) { 
    tft.readRect(0, y, 110, 1, awColors);  // try to read one row at a time...
    tft.writeRect(0, y-16, 110, 1, awColors);  // try to write one row at a time...
  }
  tft.setTextSize(2);
  tft.fillRect(0, 316-16, 110, 16, ILI9341_BLACK);
  tft.setCursor(0,316-16);

}


