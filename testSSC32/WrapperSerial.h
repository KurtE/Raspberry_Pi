/*
  WrapperSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
*/

#ifndef WrapperSerial_h
#define WrapperSerial_h

#include <inttypes.h>

#include "Stream.h"
#include <termios.h>
#include <unistd.h>

struct ring_buffer;

class WrapperSerial : public Stream
{
  private:
    int  _fdOut;		// file descriptor
    int  _fdIn;		// file descript for input
    FILE *_pfileOut;		// Pointer to file
    FILE *_pfileIn;		// Make a seperate File pointer to handle stdin...
    int  _peek;		// last value we received from read/peek

  public:
    WrapperSerial();
    bool begin(char *pszDevice, speed_t baud, bool fRtsCts = false);
	bool begin();		// special version for stdin, stdout
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

#endif
