//=============================================================================
//Project Lynxmotion Phoenix
//Description: Phoenix software
//
//
// Arduino(ish) defines to get the Phoenix software to run on  RPI
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.

//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//=============================================================================
#ifndef _ARDUINO_DEFS_
#define _ARDUINO_DEFS_
#include <stdint.h>
#define PROGMEM
#define pgm_read_byte(x)        (*((char *)x))
//  #define pgm_read_word(x)        (*((short *)(x & 0xfffffffe)))
#define pgm_read_word(x)        ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
#define pgm_read_byte_near(x)   (*((char *)x))
#define pgm_read_byte_far(x)    (*((char *)x))
//  #define pgm_read_word_near(x)   (*((short *)(x & 0xfffffffe))
//  #define pgm_read_word_far(x)    (*((short *)(x & 0xfffffffe)))
#define pgm_read_word_near(x)   ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
#define pgm_read_word_far(x)    ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x))))
#define PSTR(x)  x
#define F(x) x
#define PGM_P   const char *

// Define some data types?
#define byte unsigned char
//#define uint8_t unsigned char
//#define uint16_t unsigned short
//#define uint32_t unsigned long
#define word unsigned short
#define boolean bool

#define delay(x)  usleep((x)*1000)
#define delayMicroseconds(us) usleep((us))
extern unsigned long millis(void);
extern unsigned long micros(void);
extern long min(long a, long b);
extern long max(long a, long b);
extern long map(long x, long in_min, long in_max, long out_min, long out_max);
#endif
