/*
  Commander.h - Library for interfacing with ArbotiX Commander
  Copyright (c) 2009-2012 Michael E. Ferguson.  All right reserved.

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
*/

#ifndef Commander_h
#define Commander_h

/* bitmasks for buttons array */
#define BUT_R1      0x01
#define BUT_R2      0x02
#define BUT_R3      0x04
#define BUT_L4      0x08
#define BUT_L5      0x10
#define BUT_L6      0x20
#define BUT_RT      0x40
#define BUT_LT      0x80

/* the Commander will send out a frame at about 30hz, this class helps decipher the output. */
class Commander
{    
  public:
    Commander(); 
    void begin(unsigned long baud);
    void UseSouthPaw();     // enable southpaw configuration
    int ReadMsgs();         // must be called regularly to clean out Serial buffer

    // joystick values are -125 to 125
    signed char walkV;      // vertical stick movement = forward speed
    signed char walkH;      // horizontal stick movement = sideways or angular speed
    signed char lookV;      // vertical stick movement = tilt    
    signed char lookH;      // horizontal stick movement = pan (when we run out of pan, turn body?)
    // 0-1023, use in extended mode    
    int pan;
    int tilt;
    
    // buttons are 0 or 1 (PRESSED), and bitmapped
    unsigned char buttons;  // 
    unsigned char ext;      // Extended function set
        
    // Hooks are used as callbacks for button presses -- NOT IMPLEMENT YET
        
  private:
    // internal variables used for reading messages
    unsigned char vals[7];  // temporary values, moved after we confirm checksum
    int index;              // -1 = waiting for new packet
    int checksum;
    unsigned char status; 
};

#endif
