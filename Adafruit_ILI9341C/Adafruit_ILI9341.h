/***************************************************
  This is our library for the Adafruit  ILI9341 Breakout and Shield
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

#ifndef _ADAFRUIT_ILI9341H_
#define _ADAFRUIT_ILI9341H_

#include "ArduinoDefs.h"
#include "Print.h"
#define __ARDUINO_X86__
#include <mraa.h>
#include <Adafruit_GFX.h>


#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
/*
#define ILI9341_PWCTR6  0xFC

*/

// Color definitions
#define	ILI9341_BLACK   0x0000
#define	ILI9341_BLUE    0x001F
#define	ILI9341_RED     0xF800
#define	ILI9341_GREEN   0x07E0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW  0xFFE0  
#define ILI9341_WHITE   0xFFFF

//=================================================================
// Hardware SPI version
//=================================================================

class Adafruit_ILI9341 : public Adafruit_GFX {

 public:

  Adafruit_ILI9341(int8_t _CS, int8_t _DC, int8_t _RST = -1);

  void     begin(void),
           end(void),
           setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1),
           pushColor(uint16_t color),
           fillScreen(uint16_t color),
           drawPixel(int16_t x, int16_t y, uint16_t color),
           drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
           drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
           fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
             uint16_t color),
           setRotation(uint8_t r),
            drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
                uint16_t bg, uint8_t size),
         invertDisplay(boolean i);
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

   /* readdata is now used in readPixel/readRect, readcommand8 used in test programs */
  uint8_t  readdata(void),
           readcommand8(uint8_t reg, uint8_t index = 0);

	// Added functions to read pixel data...
  uint16_t readPixel(int16_t x, int16_t y);
  void     readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors);
  void     writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors);


  void     spiwrite(uint8_t) __attribute__((always_inline)),
           spiwrite16(uint16_t) __attribute__((always_inline)),
           spiwrite16X2(uint16_t, uint16_t) __attribute__((always_inline)),
           spiwriteN(uint32_t, uint16_t) __attribute__((always_inline)),
           writecommand(uint8_t c),
           writecommand_cont(uint8_t c),
           writedata(uint8_t d),
           writedata_cont(uint8_t d),
           writedata16(uint16_t color),
           writedata16_cont(uint16_t color),
           writedata16X2_cont(uint16_t w1, uint16_t w2),
           commandList(uint8_t *addr);
  uint8_t  spiread(void);

 private:
  uint8_t  tabcolor;
  void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) __attribute__((always_inline)) ;
  void spi_begin(void) __attribute__((always_inline));
  void spi_end(void) __attribute__((always_inline));

  uint8_t mySPCR;
  
  int8_t  _cs, _dc, _rst;
  
  // for MRAA on Intel Edison
  // Use C versions since c++ is simply wrapper anyway
  mraa_gpio_context _gpioDC;
  mraa_gpio_context _gpioCS;
  
  uint8_t _fDCHigh; // is the DC High
  uint8_t _fCSHigh; // is the CS high... 
  
  // try the C++ version
  mraa_spi_context SPI;
//#define MINIMIZE_CALLS  
#ifndef MINIMIZE_CALLS
  void DCHigh()  __attribute__((always_inline)) {
            mraa_gpio_write(_gpioDC, 1);
    }
  
  void DCLow()  __attribute__((always_inline)) {
            mraa_gpio_write(_gpioDC, 0);
	}

  void CSHigh()  __attribute__((always_inline)) {
            mraa_gpio_write(_gpioCS, 1);
	}
  void CSLow()  __attribute__((always_inline)) {
            mraa_gpio_write(_gpioCS, 0);
    }
#else
  void DCHigh()  __attribute__((always_inline)) {
        if (!_fDCHigh) {
            mraa_gpio_write(_gpioDC, 1);
            _fDCHigh = 1;
        }
    }
  
  void DCLow()  __attribute__((always_inline)) {
        if (_fDCHigh) {
            mraa_gpio_write(_gpioDC, 0);
            _fDCHigh = 0;
        }
	}

  void CSHigh()  __attribute__((always_inline)) {
        if (!_fCSHigh) {
            mraa_gpio_write(_gpioCS, 1);
            _fCSHigh = 1;
        }
	}
  void CSLow()  __attribute__((always_inline)) {
        if (_fCSHigh) {
            mraa_gpio_write(_gpioCS, 0);
            _fCSHigh = 0;
        }
    }
#endif    
};


#endif
