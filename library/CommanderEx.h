/*
  CommanderEx.h - Library for interfacing with ArbotiX Commander
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

// This version was modified from the original one by Kurt to run on Linux for Raspberry Pi.
// Also I renamed some members and the like to make it easier to understand...
// Also Added support to optionally use network sockets to 

#define CMDR_USE_XBEE
//#define CMDR_USE_SOCKET

#ifndef Commander_h
#define Commander_h
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
        bool begin(char *pszComm, speed_t baud);
        void end();                              // release any resources.
        void UseSouthPaw();                      // enable southpaw configuration
        int ReadMsgs();                          // must be called regularly to clean out Serial buffer

        // joystick values are -125 to 125
        signed char rightV;                      // vertical stick movement = forward speed
        signed char rightH;                      // horizontal stick movement = sideways or angular speed
        signed char leftV;                       // vertical stick movement = tilt
        signed char leftH;                       // horizontal stick movement = pan (when we run out of pan, turn body?)
        // 0-1023, use in extended mode
        int pan;
        int tilt;

        // buttons are 0 or 1 (PRESSED), and bitmapped
        unsigned char buttons;                    //
        unsigned char ext;                        // Extended function set

        // Feedback capabilities - Mainly for Sockets, but can also use with my debug stuff with XBees...
        void message(char *psz);                 //

    private:
        // internal variables used for reading messages
        unsigned char vals[6];                   // temporary values, moved after we confirm checksum
        int index;                               // -1 = waiting for new packet
        int checksum;
        unsigned char status;

        //Private stuff for Linux threading and file descriptor, and open file...
        pthread_mutex_t lock;                    // A lock to make sure we don't walk over ourself...
        bool fValidPacket;                       // Do we have a valid packet?
        unsigned char bInBuf[7];                 // Input buffer we use in the thread to process partial messages.
        char *_pszDevice;
        speed_t _baud;
        bool _fCancel;                           // Cancel any input threads.
#ifdef CMDR_USE_XBEE
        int fdXBee;                              // file descriptor
        pthread_t tidXBee;                       // Thread Id of our reader thread...
       	pthread_barrier_t _barrier;

        static void *XBeeThreadProc(void *);
#endif
#ifdef CMDR_USE_SOCKET
        pthread_t tidSocket;
        static void *SocketThreadProc(void *);
        static void SocketThreadCleanupProc(void *);
#endif        
};
#endif
