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

//Hardware SPI version. 

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  SPI = NULL;
  _gpioCS = NULL;
  _gpioDC = NULL;

}

void inline Adafruit_ILI9341::spiwrite(uint8_t c) {

  //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

  // transaction sets mode
  SPI->write((char)c);
}

void inline Adafruit_ILI9341::spiwrite16(uint16_t c) {
  uint8_t txData[2];
  txData[0] = (c>>8) & 0xff;
  txData[1] = c & 0xff; 
  char *prxData = SPI->write((char*)txData, 2);
  if (prxData)
    free(prxData);
}

#define X86_BUFFSIZE 32
void inline Adafruit_ILI9341::spiwriteN(uint32_t count, uint16_t c) {
  if (count < X86_BUFFSIZE)
	while(count--) spiwrite16(c);
  else {	
    uint8_t txData[2*X86_BUFFSIZE];
    char *prxData;
    for (uint8_t i = 0; i < X86_BUFFSIZE*2; i+=2) {
      txData[i] = (c>>8) & 0xff;
      txData[i+1] = c & 0xff;
    }   
    while (count >= X86_BUFFSIZE) {
	  prxData = SPI->write((char*)txData, 2*X86_BUFFSIZE);
      if (prxData)
        free(prxData);
	  count -= X86_BUFFSIZE;
    }
    if (count)
	  prxData = SPI->write((char*)txData, 2*count);
      if (prxData)
        free(prxData);
  }
}

void Adafruit_ILI9341::writecommand(uint8_t c) {
  DCLow();
  //DCLow();

  CSLow();
  //CSLow();

  spiwrite(c);

  CSHigh();
  //CSHigh();
}

// Like above, but does not raise CS at end
void Adafruit_ILI9341::writecommand_cont(uint8_t c) {
  DCLow();
  //DCLow();
  CSLow();
  //CSLow();

  spiwrite(c);
}


void Adafruit_ILI9341::writedata(uint8_t c) {
  DCHigh();
  //DCHigh();
  CSLow();
  //CSLow();
  
  spiwrite(c);

  //CSHigh();
  CSHigh();
} 

void Adafruit_ILI9341::writedata_cont(uint8_t c) {
  DCHigh();
  //DCHigh();
  CSLow();
  //CSLow();
  
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


// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
inline void Adafruit_ILI9341::spi_begin(void) {
  SPI->frequency(8000000);
  SPI->lsbmode(false);  
  SPI->mode(MRAA_SPI_MODE0);
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


void Adafruit_ILI9341::begin(void) {
  mraa::Gpio* gpioRST = NULL;
  if (_rst > 0) 
    gpioRST = new mraa::Gpio(_rst);

  if (gpioRST) {
    gpioRST->dir(mraa::DIR_OUT);
    gpioRST->write(0);
  }

  _gpioDC = new mraa::Gpio(_dc);
  _gpioDC->dir(mraa::DIR_OUT);
  
  _gpioCS = new mraa::Gpio(_cs);
  _gpioCS->dir(mraa::DIR_OUT);

  SPI = new mraa::Spi(1);   // which buss?   will experment here...
  SPI->frequency(8000000);
  SPI->lsbmode(false);  
  SPI->mode(MRAA_SPI_MODE0);

  // toggle RST low to reset
  if (gpioRST) {
    gpioRST->write(1);
    delay(5);
    gpioRST->write(0);
    delay(20);
    gpioRST->write(1);
    delay(150);
    delete gpioRST;
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
        delete SPI;
        SPI = NULL;
    }
    if (_gpioCS) {
        delete _gpioCS;
        _gpioCS = NULL;
    }
    if (_gpioDC) {
        delete _gpioDC;
        _gpioDC = NULL;
    }
}


inline void Adafruit_ILI9341::setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  writecommand_cont(ILI9341_CASET); // Column addr set
  writedata16_cont(x0);     // XSTART
  writedata16_cont(x1);     // XEND

  writecommand_cont(ILI9341_PASET); // Row addr set
  writedata16_cont(y0);     // YSTART
  writedata16_cont(y1);     // YEND

  writecommand(ILI9341_RAMWR); // write to RAM
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

  //DCHigh();
  DCHigh();
  //CSLow();
  CSLow();

  spiwrite16(color);

  CSHigh();
  //CSHigh();
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
  //DCHigh();
  CSLow();
  //CSLow();

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
  uint8_t r = 0;

#if defined (__AVR__)
  uint8_t backupSPCR = SPCR;
  SPCR = mySPCR;
  SPDR = 0x00;
  while(!(SPSR & _BV(SPIF)));
  r = SPDR;
  SPCR = backupSPCR;
#elif defined(TEENSYDUINO)
  r = SPI.transfer(0x00);
#elif defined (__arm__)
  SPI.setClockDivider(11); // 8-ish MHz (full! speed!)
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  r = SPI.transfer(0x00);
#endif
  //Serial.print("read: 0x"); Serial.print(r, HEX);
  
  return r;
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
//   digitalWrite(_sclk, LOW);
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
	uint8_t r,g,b;

   spi_begin();

    setAddr(x, y, x, y);
    writecommand_cont(ILI9341_RAMRD); // read from RAM
//	waitTransmitComplete();
    DCHigh();  // make sure we are in data mode

	// Read Pixel Data
	r = spiread();	    // Read a DUMMY byte of GRAM
	r = spiread();		// Read a RED byte of GRAM
	g = spiread();		// Read a GREEN byte of GRAM
	b = spiread();		// Read a BLUE byte of GRAM
   CSHigh();
 //  CSHigh();
   spi_end();
	return color565(r,g,b);
}

// Now lets see if we can read in multiple pixels
void Adafruit_ILI9341::readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors) 
{
	uint8_t r,g,b;
    uint16_t c = w * h;
    spi_begin();

	setAddr(x, y, x+w-1, y+h-1);
    writecommand_cont(ILI9341_RAMRD); // read from RAM
    DCHigh();  // make sure we are in data mode

//    spiwrite(c);
   	r = spiread();	        // Read a DUMMY byte of GRAM

    while (c--) {
        r = spiread();		// Read a RED byte of GRAM
        g = spiread();		// Read a GREEN byte of GRAM
        b = spiread();		// Read a BLUE byte of GRAM
        *pcolors++ = color565(r,g,b);
    }
   CSHigh();
   spi_end();
}

// Now lets see if we can writemultiple pixels
void Adafruit_ILI9341::writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors) 
{
    spi_begin();
	setAddr(x, y, x+w-1, y+h-1);
	writecommand_cont(ILI9341_RAMWR);
    DCHigh();  // make sure we are in data mode
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
            spiwrite(*pcolors >> 8);
            spiwrite(*pcolors++ & 0xff);
		}
	}
    CSHigh();
   spi_end();
}
