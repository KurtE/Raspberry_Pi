/*
  Commander.cpp - Library for interfacing with ArbotiX Commander
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

// Note: This one is hacked up to try to compile under linux...


#include "Commander.h"
#include "xsys.h"

/* Constructor */
Commander::Commander(){
    index = -1;
    status = 0;
}
void Commander::begin(unsigned long baud){
//    Serial.begin(baud);
}

/* SouthPaw Support */
void Commander::UseSouthPaw(){
    status |= 0x01;
}

/* process messages coming from Commander 
 *  format = 0xFF RIGHT_H RIGHT_V LEFT_H LEFT_V BUTTONS EXT CHECKSUM */
int Commander::ReadMsgs(){
#if 0
    while(/*Serial.available() >*/ 0){
        if(index == -1){         // looking for new packet
            if(Serial.read() == 0xff){
                index = 0;
                checksum = 0;
            }
        }else if(index == 0){
            vals[index] = (unsigned char) Serial.read();
            if(vals[index] != 0xff){            
                checksum += (int) vals[index];
                index++;
            }
        }else{
            vals[index] = (unsigned char) Serial.read();
            checksum += (int) vals[index];
            index++;
            if(index == 7){ // packet complete
                if(checksum%256 != 255){
                    // packet error!
                    index = -1;
                  return 0;
                }else{
                    if((status&0x01) > 0){     // SouthPaw
                        walkV = (signed char)( (int)vals[0]-128 );
                        walkH = (signed char)( (int)vals[1]-128 );
                        lookV = (signed char)( (int)vals[2]-128 );
                        lookH = (signed char)( (int)vals[3]-128 );
                    }else{
                        lookV = (signed char)( (int)vals[0]-128 );
                        lookH = (signed char)( (int)vals[1]-128 );
                        walkV = (signed char)( (int)vals[2]-128 );
                        walkH = (signed char)( (int)vals[3]-128 );
                    }
                    pan = (vals[0]<<8) + vals[1];
                    tilt = (vals[2]<<8) + vals[3];
                    buttons = vals[4];
                    ext = vals[5];
                }
                index = -1;
//				while (Serial.read() != -1)
					;
                //Serial.flush();  ' Not the same on Arduino 1.0+
                return 1;
            }
        }
    }
#endif	
    return 0;
}
