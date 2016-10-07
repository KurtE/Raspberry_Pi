/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
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

#include "Adafruit_ILI9341.h"
#include <limits.h>

// Test to see if not needing malloc/free will speed up SPI transfers
#define MRAA_SPI_TRANSFER_BUF
#ifdef MRAA_SPI_TRANSFER_BUF
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#endif

//Hardware SPI version. 
#define X86_BUFFSIZE 128
#define SPI_FREQ 8000000


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  SPI = NULL;
  _gpioCS = NULL;
  _gpioDC = NULL;
  _gpioRST = NULL;

}

void inline Adafruit_ILI9341::spiwrite(uint8_t c) {

  //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

  // transaction sets mode
  mraa_spi_write(SPI, c);
}

void inline Adafruit_ILI9341::spiwrite16(uint16_t c) {
  uint8_t txData[2];
  txData[0] = (c>>8) & 0xff;
  txData[1] = c & 0xff; 
#ifdef MRAA_SPI_TRANSFER_BUF
//  uint8_t rxData[2];
  mraa_spi_transfer_buf(SPI, txData, NULL/*rxData*/, 2);
#else
  uint8_t *prxData = mraa_spi_write_buf(SPI, txData, 2);
  if (prxData)
    free(prxData);
#endif    
}

void inline Adafruit_ILI9341::spiwrite16X2(uint16_t w1, uint16_t w2) {
  uint8_t txData[4];
  txData[0] = (w1>>8) & 0xff;
  txData[1] = w1 & 0xff; 
  txData[2] = (w2>>8) & 0xff;
  txData[3] = w2 & 0xff; 
#ifdef MRAA_SPI_TRANSFER_BUF
  //uint8_t rxData[4];
  mraa_spi_transfer_buf(SPI, txData, NULL/*rxData*/, 4);
#else
  uint8_t *prxData = mraa_spi_write_buf(SPI, txData, 4);
  if (prxData)
    free(prxData);
#endif    
}

void inline Adafruit_ILI9341::spiwriteN(uint32_t count, uint16_t c) {
    uint8_t txData[2*X86_BUFFSIZE];
    uint16_t cbWrite = min(count, X86_BUFFSIZE) * 2;
    count *=2 ; // shift it up one bit so don't have to multiply each time...
    for (uint16_t i = 0; i < cbWrite; i+=2) {
      txData[i] = (c>>8) & 0xff;
      txData[i+1] = c & 0xff;
    }   
    while (count) {
      mraa_spi_transfer_buf(SPI, txData, NULL, cbWrite);
	  count -= cbWrite;
      if (count < cbWrite)
        cbWrite = count;
    }
}

void Adafruit_ILI9341::writecommand(uint8_t c) {
  DCLow();
  CSLow();
  spiwrite(c);

  CSHigh();
}

// Like above, but does not raise CS at end
void Adafruit_ILI9341::writecommand_cont(uint8_t c) {
  DCLow();
  CSLow();

  spiwrite(c);
}


void Adafruit_ILI9341::writedata(uint8_t c) {
  DCHigh();
  CSLow();
  
  spiwrite(c);

  CSHigh();
} 

void Adafruit_ILI9341::writedata_cont(uint8_t c) {
  DCHigh();
  CSLow();
  
  spiwrite(c);
} 

void Adafruit_ILI9341::writedata16(uint16_t color) {
  DCHigh();
  CSLow();

  spiwrite16(color);

  CSHigh();
}

void Adafruit_ILI9341::writedata16_cont(uint16_t color) {
  DCHigh();
  CSLow();
  spiwrite16(color);
}

void Adafruit_ILI9341::writedata16X2_cont(uint16_t w1, uint16_t w2) {
  DCHigh();
  CSLow();
  spiwrite16X2(w1, w2);
}

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
inline void Adafruit_ILI9341::spi_begin(void) {
  mraa_spi_frequency(SPI, SPI_FREQ);
  mraa_spi_lsbmode(SPI, false);  
  mraa_spi_mode(SPI, MRAA_SPI_MODE0);
}

inline void Adafruit_ILI9341::spi_end(void) {
}


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ILI9341::commandList(uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


void Adafruit_ILI9341::begin(uint8_t spi_buss) {
  if (_rst > 0) 
    _gpioRST = mraa_gpio_init(_rst);

  if (_gpioRST) {
    mraa_gpio_dir(_gpioRST, MRAA_GPIO_OUT_LOW);
    //mraa_gpio_write(_gpioRST, 0);
  }


  SPI = mraa_spi_init(spi_buss);   // which buss?   will experment here...
  mraa_spi_frequency(SPI, SPI_FREQ);
  mraa_spi_lsbmode(SPI, false);  
  mraa_spi_mode(SPI, MRAA_SPI_MODE0);

  _gpioDC = mraa_gpio_init(_dc);
  mraa_gpio_dir(_gpioDC, MRAA_GPIO_OUT);
  mraa_gpio_use_mmaped(_gpioDC, 1);

  mraa_gpio_write(_gpioDC, 1);  
  _fDCHigh = 1; // init to high
  
  // If -1 passed in use hardware CS...
  if (_cs >= 0) {
    _gpioCS = mraa_gpio_init(_cs);
    mraa_gpio_dir(_gpioCS, MRAA_GPIO_OUT);
    mraa_gpio_use_mmaped(_gpioCS, 1);
    mraa_gpio_write(_gpioCS, 1);  
    _fCSHigh = 1; // init to high
  }

  // toggle RST low to reset
  if (_gpioRST) {
    mraa_gpio_write(_gpioRST, 1);
    delay(5);
    mraa_gpio_write(_gpioRST, 0);
    delay(20);
    mraa_gpio_write(_gpioRST, 1);
  }

  
  spi_begin();
  writecommand(0xEF);
  writedata(0x03);
  writedata(0x80);
  writedata(0x02);

  writecommand(0xCF);  
  writedata(0x00); 
  writedata(0XC1); 
  writedata(0X30); 

  writecommand(0xED);  
  writedata(0x64); 
  writedata(0x03); 
  writedata(0X12); 
  writedata(0X81); 
 
  writecommand(0xE8);  
  writedata(0x85); 
  writedata(0x00); 
  writedata(0x78); 

  writecommand(0xCB);  
  writedata(0x39); 
  writedata(0x2C); 
  writedata(0x00); 
  writedata(0x34); 
  writedata(0x02); 
 
  writecommand(0xF7);  
  writedata(0x20); 

  writecommand(0xEA);  
  writedata(0x00); 
  writedata(0x00); 
 
  writecommand(ILI9341_PWCTR1);    //Power control 
  writedata(0x23);   //VRH[5:0] 
 
  writecommand(ILI9341_PWCTR2);    //Power control 
  writedata(0x10);   //SAP[2:0];BT[3:0] 
 
  writecommand(ILI9341_VMCTR1);    //VCM control 
  writedata(0x3e); //对比度调节
  writedata(0x28); 
  
  writecommand(ILI9341_VMCTR2);    //VCM control2 
  writedata(0x86);  //--
 
  writecommand(ILI9341_MADCTL);    // Memory Access Control 
  writedata(0x48);

  writecommand(ILI9341_PIXFMT);    
  writedata(0x55); 
  
  writecommand(ILI9341_FRMCTR1);    
  writedata(0x00);  
  writedata(0x18); 
 
  writecommand(ILI9341_DFUNCTR);    // Display Function Control 
  writedata(0x08); 
  writedata(0x82);
  writedata(0x27);  
 
  writecommand(0xF2);    // 3Gamma Function Disable 
  writedata(0x00); 
 
  writecommand(ILI9341_GAMMASET);    //Gamma curve selected 
  writedata(0x01); 
 
  writecommand(ILI9341_GMCTRP1);    //Set Gamma 
  writedata(0x0F); 
  writedata(0x31); 
  writedata(0x2B); 
  writedata(0x0C); 
  writedata(0x0E); 
  writedata(0x08); 
  writedata(0x4E); 
  writedata(0xF1); 
  writedata(0x37); 
  writedata(0x07); 
  writedata(0x10); 
  writedata(0x03); 
  writedata(0x0E); 
  writedata(0x09); 
  writedata(0x00); 
  
  writecommand(ILI9341_GMCTRN1);    //Set Gamma 
  writedata(0x00); 
  writedata(0x0E); 
  writedata(0x14); 
  writedata(0x03); 
  writedata(0x11); 
  writedata(0x07); 
  writedata(0x31); 
  writedata(0xC1); 
  writedata(0x48); 
  writedata(0x08); 
  writedata(0x0F); 
  writedata(0x0C); 
  writedata(0x31); 
  writedata(0x36); 
  writedata(0x0F); 

  writecommand(ILI9341_SLPOUT);    //Exit Sleep 
  spi_end();
  delay(120); 		
  spi_begin();
  writecommand(ILI9341_DISPON);    //Display on 
  spi_end();

}

void Adafruit_ILI9341::end(void) {
    // hardware SPI
    if (SPI) {
        mraa_spi_stop(SPI);
        SPI = NULL;
    }
    if (_gpioCS) {
        mraa_gpio_close(_gpioCS);
        _gpioCS = NULL;
    }
    if (_gpioDC) {
        mraa_gpio_close(_gpioDC);
        _gpioDC = NULL;
    }
    if (_gpioRST) {
        mraa_gpio_dir(_gpioRST, MRAA_GPIO_IN);
        mraa_gpio_close(_gpioRST);
        _gpioRST = NULL;
    }
  }


inline void Adafruit_ILI9341::setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  writecommand_cont(ILI9341_CASET); // Column addr set
  writedata16X2_cont(x0, x1);     // XSTA

  writecommand_cont(ILI9341_PASET); // Row addr set
  writedata16X2_cont(y0, y1);     // XSTA
}


void Adafruit_ILI9341::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
 uint16_t y1) {

  setAddr(x0, y0, x1, y1);
  writecommand(ILI9341_RAMWR); // write to RAM
}


void Adafruit_ILI9341::pushColor(uint16_t color) {
  spi_begin();
  writedata16(color);
  spi_end();
}

void Adafruit_ILI9341::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  spi_begin();
  setAddrWindow(x,y,x+1,y+1);

  DCHigh();
  CSLow();

  spiwrite16(color);

  CSHigh();
  spi_end();
}


void Adafruit_ILI9341::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;

  if((y+h-1) >= _height) 
    h = _height-y;

  spi_begin();
  setAddrWindow(x, y, x, y+h-1);

  DCHigh();
  CSLow();

  spiwriteN(h, color);
  CSHigh();
  //CSHigh();
  spi_end();
}


void Adafruit_ILI9341::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  spi_begin();
  setAddrWindow(x, y, x+w-1, y);

  DCHigh();
  CSLow();
  //DCHigh();
  //CSLow();
  spiwriteN(w, color);
  CSHigh();
  //CSHigh();
  spi_end();
}

void Adafruit_ILI9341::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;
  //printf("Fill Rect: %d %d %d %d\n\r", x, y, w, h);
  spi_begin();
  setAddrWindow(x, y, x+w-1, y+h-1);

  DCHigh();
  //DCHigh();
  CSLow();
  //CSLow();

  spiwriteN((uint32_t)h*w, color);
  //CSHigh();
  CSHigh();
  spi_end();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ILI9341::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ILI9341::setRotation(uint8_t m) {

  spi_begin();
  writecommand(ILI9341_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(MADCTL_MX | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
     break;
   case 1:
     writedata(MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  case 2:
    writedata(MADCTL_MY | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
    break;
   case 3:
     writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  }
  spi_end();
}


void Adafruit_ILI9341::invertDisplay(boolean i) {
  spi_begin();
  writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
  spi_end();
}


//---------------------------------------------------
// Some limited reading is in place.

uint8_t inline Adafruit_ILI9341::spiread(void) {
  
  return mraa_spi_write(SPI, 0x00);
}

uint8_t Adafruit_ILI9341::readdata(void) {
   DCHigh();
   CSLow();
   uint8_t r = spiread();
   CSHigh();
   
   return r;
}
 
uint8_t Adafruit_ILI9341::readcommand8(uint8_t c, uint8_t index) {
   spi_begin();
   DCLow(); // command
   CSLow();
   spiwrite(0xD9);  // woo sekret command?
   DCHigh();
   spiwrite(0x10 + index);
   CSHigh();

   DCLow();
   CSLow();
   spiwrite(c);
 
   DCHigh();
   uint8_t r = spiread();
   CSHigh();
   spi_end();
   return r;
}

// Read Pixel at x,y and get back 16-bit packed color
uint16_t Adafruit_ILI9341::readPixel(int16_t x, int16_t y)
{
    uint16_t wColor = 0;

    spi_begin();

    setAddr(x, y, x, y);
    writecommand_cont(ILI9341_RAMRD); // read from RAM
    DCHigh();  // make sure we are in data mode

	// Read Pixel Data
    uint8_t txData[4] = {0,0,0,0};
    uint8_t *prxData = mraa_spi_write_buf(SPI, txData, 4);
    
    if (prxData) {
        wColor = color565(prxData[1], prxData[2], prxData[3]);
   //     printf("%x %x %x %x\n", prxData[0], prxData[1], prxData[2], prxData[3]);
        free(prxData);
    }    
    CSHigh();
    spi_end();
    return wColor;
}

// Now lets see if we can read in multiple pixels
void Adafruit_ILI9341::readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors) 
{
    uint16_t c = w * h;
    spi_begin();

	setAddr(x, y, x+w-1, y+h-1);
    writecommand_cont(ILI9341_RAMRD); // read from RAM
    DCHigh();  // make sure we are in data mode

    uint8_t txdata[X86_BUFFSIZE * 3];   // how many to read at a time
    memset(txdata, 0, X86_BUFFSIZE * 3);

#ifdef MRAA_SPI_TRANSFER_BUF
    uint8_t rxdata[X86_BUFFSIZE * 3];   // how many to read at a time
#else
    uint8_t *prxData;
#endif    
   	rxdata [0]= spiread();	        // Read a DUMMY byte of GRAM


    while (c){
        uint16_t cRead = min (c, X86_BUFFSIZE);
#ifdef MRAA_SPI_TRANSFER_BUF
        mraa_spi_transfer_buf(SPI, txdata, rxdata, cRead * 3);
        for (uint16_t i=0; i < cRead*3; i+=3) {
            *pcolors++ = color565(rxdata[i], rxdata[i+1], rxdata[i+2]);
        }
#else
        prxData = mraa_spi_write_buf(SPI, txdata, cRead * 3);
        if (prxData) {
            for (uint16_t i=0; i < cRead*3; i+=3) {
                *pcolors++ = color565(prxData[i], prxData[i+1], prxData[i+2]);
            }
            free (prxData);
        }
#endif
        c -= cRead;
    }
   CSHigh();
   spi_end();
}

// Now lets see if we can writemultiple pixels
void Adafruit_ILI9341::writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors) 
{
    uint8_t txdata[X86_BUFFSIZE * 2];   // how many to write at a time.
    uint8_t *prxData;
    uint16_t iOut = 0;
    spi_begin();
	setAddr(x, y, x+w-1, y+h-1);
	writecommand_cont(ILI9341_RAMWR);
    DCHigh();  // make sure we are in data mode
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
            txdata[iOut++] = *pcolors >> 8;
            txdata[iOut++] = *pcolors++ & 0xff;
            if (iOut == (X86_BUFFSIZE * 2)) {
                prxData = mraa_spi_write_buf(SPI, txdata, X86_BUFFSIZE * 2);
                if (prxData)
                    free (prxData);
                iOut = 0;    
            }
		}
	}
    
    if (iOut) {
        prxData = mraa_spi_write_buf(SPI, txdata, iOut);
        if (prxData)
            free (prxData);
    }
    
    CSHigh();
   spi_end();
}

// Lets try doing our own drawchar...
// Borrow inspiration from ili0341_t
#define MFONT_SIZE  6 // how big a character can we handle here...
#define DCA_SIZE 300    // how big of a buffer to allocate   

void Adafruit_ILI9341::drawChar(int16_t x, int16_t y, unsigned char c,
			    uint16_t fgcolor, uint16_t bgcolor, uint8_t size)
{
    uint16_t acolors[DCA_SIZE]; // do one logical scan line of the char per write

    if((x >= _width)            || // Clip right
       (y >= _height)           || // Clip bottom
       ((x + 6 * size - 1) < 0) || // Clip left  TODO: is this correct?
       ((y + 8 * size - 1) < 0))   // Clip top   TODO: is this correct?
        return;

    if (fgcolor == bgcolor) {
        //printf("draw Char f=b : %d %d %c %u %u %u\n\r", x, y, c, fgcolor, bgcolor, size);
        uint8_t line;
        for (int8_t i=0; i<5; i++ ) {
            line = pgm_read_byte(font+(c*5)+i);
            int8_t j=0;
                
            while (line) {
                // will try to output multiple pixels at a time 
                if (line == 0x7f) {
                    fillRect(x+(i*size), y+(j*size), size, 7*size, fgcolor);
                    j+=7;
                    line >>= 7;
                } else if ((line & 0x3f) == 0x3f) {
                    fillRect(x+(i*size), y+(j*size), size, 6*size, fgcolor);
                    j+=6;
                    line >>= 6;
                } else if ((line & 0x1f) == 0x1f) {
                    fillRect(x+(i*size), y+(j*size), size, 5*size, fgcolor);
                    j+=5;
                    line >>= 5;
                } else if ((line & 0xf) == 0xf) {
                    fillRect(x+(i*size), y+(j*size), size, 4*size, fgcolor);
                    j+=4;
                    line >>= 4;
                } else if ((line & 0x7) == 0x7) {
                    fillRect(x+(i*size), y+(j*size), size, 3*size, fgcolor);
                    j+=3;
                    line >>= 3;
                } else if ((line & 0x3) == 0x3) {
                    fillRect(x+(i*size), y+(j*size), size, 2*size, fgcolor);
                    j+=2;
                    line >>= 2;
                } else if (line & 0x1) {
                    fillRect(x+(i*size), y+(j*size), size, size, fgcolor);
                    j++;
                    line >>= 1;
                } else {
                    line >>=1;
                    j++;
                }
            }
        }
    } else {
        if (size > MFONT_SIZE)
            return; // too big to fit in our array
        uint8_t xr, yr;
        uint8_t mask = 0x01;
        uint16_t color;
        int16_t yOut = y;
        int16_t xOut = x;
        uint8_t cRowsPerWrite = DCA_SIZE / (size*size*8);
        uint8_t iRowPerWrite = 0;
        uint16_t *pcolors = acolors;
        //printf("draw Char f!=b : %d %d %c %u %u %u\n\r", x, y, c, fgcolor, bgcolor, size);
        for (y=0; y < 8; y++) {
            for (yr=0; yr < size; yr++) {
                for (x=0; x < 5; x++) {
                    if (font[c * 5 + x] & mask) {
                        color = fgcolor;
                    } else {
                        color = bgcolor;
                    }
                    for (xr=0; xr < size; xr++) {
                        *pcolors++ = color;
                    }
                }
                for (xr=0; xr < size; xr++) {
                    *pcolors++ = bgcolor;
                }
            }
            iRowPerWrite++;
            if (iRowPerWrite == cRowsPerWrite) {
                writeRect(xOut, yOut, 6*size, size*iRowPerWrite, acolors);
                yOut += size*iRowPerWrite;
                iRowPerWrite = 0;
                pcolors = acolors;
            }
            mask = mask << 1;
        }
        if (iRowPerWrite)   // any other rows to still output
            writeRect(xOut, yOut, 6*size, size*iRowPerWrite, acolors);
    }
}

