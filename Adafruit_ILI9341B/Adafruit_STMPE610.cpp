/*************************************************** 
  This is a library for the Adafruit STMPE610 Resistive
  touch screen controller breakout
  ----> http://www.adafruit.com/products/1571
 
  Check out the links above for our tutorials and wiring diagrams
  These breakouts use SPI or I2C to communicate

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_STMPE610.h"

#ifdef __ARDUINO_X86__
extern void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value);
extern uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
#endif


/**************************************************************************/
/*! 
    @brief  Instantiates a new STMPE610 class
*/
/**************************************************************************/
// hardware SPI
Adafruit_STMPE610::Adafruit_STMPE610(uint8_t cspin) {
  _cs = cspin;
  SPI = 0;
  _gpioCS = NULL;
}


/**************************************************************************/
/*! 
    @brief  Setups the HW
*/
/**************************************************************************/
boolean Adafruit_STMPE610::begin() {
    // hardware SPI
    _gpioCS = mraa_gpio_init(_cs);
    mraa_gpio_dir(_gpioCS, MRAA_GPIO_OUT);
    CSHigh();

    SPI = mraa_spi_init(1);   // which buss?   will experment here...
    mraa_spi_frequency(SPI, 1000000);
    mraa_spi_lsbmode(SPI, false);  
    mraa_spi_mode(SPI, MRAA_SPI_MODE0);

    m_spiMode = MRAA_SPI_MODE0;

    // try mode0
    if (getVersion() != 0x811) {
        //Serial.println("try MODE1");
        mraa_spi_frequency(SPI, 1000000);
        mraa_spi_lsbmode(SPI, false);  
        mraa_spi_mode(SPI, MRAA_SPI_MODE1);
        m_spiMode = MRAA_SPI_MODE1;

        if (getVersion() != 0x811) {
            return false;
        }
    }
    writeRegister8(STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
    delay(10);
  
    for (uint8_t i=0; i<65; i++) {
        readRegister8(i);
    }
  
    writeRegister8(STMPE_SYS_CTRL2, 0x0); // turn on clocks!
    writeRegister8(STMPE_TSC_CTRL, STMPE_TSC_CTRL_XYZ | STMPE_TSC_CTRL_EN); // XYZ and enable!
    //Serial.println(readRegister8(STMPE_TSC_CTRL), HEX);
    writeRegister8(STMPE_INT_EN, STMPE_INT_EN_TOUCHDET);
    writeRegister8(STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_10BIT | (0x6 << 4)); // 96 clocks per conversion
    writeRegister8(STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_6_5MHZ);
    writeRegister8(STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE | STMPE_TSC_CFG_DELAY_1MS | STMPE_TSC_CFG_SETTLE_5MS);
    writeRegister8(STMPE_TSC_FRACTION_Z, 0x6);
    writeRegister8(STMPE_FIFO_TH, 1);
    writeRegister8(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
    writeRegister8(STMPE_FIFO_STA, 0);    // unreset
    writeRegister8(STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
    writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
    writeRegister8(STMPE_INT_CTRL, STMPE_INT_CTRL_POL_HIGH | STMPE_INT_CTRL_ENABLE);

    return true;
}

void Adafruit_STMPE610::end(void) {
    // hardware SPI
    if (SPI)
        mraa_spi_stop(SPI);
    if (_gpioCS)
        mraa_gpio_close(_gpioCS);

}

boolean Adafruit_STMPE610::touched(void) {
  return (readRegister8(STMPE_TSC_CTRL) & 0x80);
}

boolean Adafruit_STMPE610::bufferEmpty(void) {
  return (readRegister8(STMPE_FIFO_STA) & STMPE_FIFO_STA_EMPTY);
}

uint8_t Adafruit_STMPE610::bufferSize(void) {
  return readRegister8(STMPE_FIFO_SIZE);
}

uint16_t Adafruit_STMPE610::getVersion() {
  uint16_t v;
  //Serial.print("get version");
  v = readRegister8(0);
  v <<= 8;
  v |= readRegister8(1);
  //Serial.print("Version: 0x"); Serial.println(v, HEX);
  return v;
}


/*****************************/

void Adafruit_STMPE610::readData(uint16_t *x, uint16_t *y, uint8_t *z) {
  uint8_t data[4];
  
  for (uint8_t i=0; i<4; i++) {
    data[i] = readRegister8(0xD7); //SPI.transfer(0x00); 
   // Serial.print("0x"); Serial.print(data[i], HEX); Serial.print(" / ");
  }
  *x = data[0];
  *x <<= 4;
  *x |= (data[1] >> 4);
  *y = data[1] & 0x0F; 
  *y <<= 8;
  *y |= data[2]; 
  *z = data[3];

  if (bufferEmpty())
    writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
}

TS_Point Adafruit_STMPE610::getPoint(void) {
  uint16_t x, y;
  uint8_t z;
  readData(&x, &y, &z);
  return TS_Point(x, y, z);
}

uint8_t Adafruit_STMPE610::spiIn() {
    mraa_spi_frequency(SPI, 1000000);
    mraa_spi_mode(SPI, m_spiMode);
    return mraa_spi_write(SPI, 0x00);
}

void Adafruit_STMPE610::spiOut(uint8_t x) {  
    mraa_spi_frequency(SPI, 1000000);
    mraa_spi_mode(SPI, m_spiMode);
    mraa_spi_write(SPI, x);
}

uint8_t Adafruit_STMPE610::readRegister8(uint8_t reg) {
  uint8_t x ;
    CSLow();
    spiOut(0x80 | reg); 
    spiOut(0x00);
    x = spiIn(); 
    CSHigh();

  return x;
}
uint16_t Adafruit_STMPE610::readRegister16(uint8_t reg) {
  uint16_t x;
    CSLow();
    spiOut(0x80 | reg); 
    spiOut(0x00);
    x = spiIn(); 
    x<<=8;
    x |= spiIn(); 
    CSHigh();

  //Serial.print("$"); Serial.print(reg, HEX); 
  //Serial.print(": 0x"); Serial.println(x, HEX);
  return x;
}

void Adafruit_STMPE610::writeRegister8(uint8_t reg, uint8_t val) {
    CSLow();
    spiOut(reg); 
    spiOut(val);
    CSHigh();
}

/****************/

TS_Point::TS_Point(void) {
  x = y = 0;
}

TS_Point::TS_Point(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}

bool TS_Point::operator==(TS_Point p1) {
  return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TS_Point::operator!=(TS_Point p1) {
  return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

