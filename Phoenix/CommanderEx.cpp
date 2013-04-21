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

#include "CommanderEx.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* Constructor */
Commander::Commander()
{
    index = -1;
    status = 0;
    fValidPacket = false;
}


void *Commander::XBeeThreadProc(void *pv)
{
    Commander *pcmdr = (Commander*)pv;
    fd_set readfs;                                // file descriptor set to wait on.
    timeval tv;                                   // how long to wait.

    //    printf("Thread start(%s)\n", pcmdr->_pszDevice);

    // Lets do our init of the xbee here.
    // We will do all of the stuff to intialize the serial port plus we will spawn off our thread.
    struct termios tc;

    if ((pcmdr->fd = open(pcmdr->_pszDevice, O_RDWR | O_NOCTTY | O_SYNC /* |  O_NONBLOCK */)) == -1) 
    {
        printf("Open Failed\n");
        return false;
    }


    if ((pcmdr->pfile = fdopen(pcmdr->fd, "r+")) == NULL)
    {
        return false;
    }


    setvbuf(pcmdr->pfile, NULL, _IONBF, BUFSIZ);
    fflush(pcmdr->pfile);

    if (tcgetattr(pcmdr->fd, &tc))
    {
        perror("tcgetattr()");
        return false;
    }


    /* input flags */
    tc.c_iflag &= ~ IGNBRK;                           /* enable ignoring break */
    tc.c_iflag &= ~(IGNPAR | PARMRK);                 /* disable parity checks */
    tc.c_iflag &= ~ INPCK;                            /* disable parity checking */
    tc.c_iflag &= ~ ISTRIP;                           /* disable stripping 8th bit */
    tc.c_iflag &= ~(INLCR | ICRNL);                   /* disable translating NL <-> CR */
    tc.c_iflag &= ~ IGNCR;                            /* disable ignoring CR */
    tc.c_iflag &= ~(IXON | IXOFF);                    /* disable XON/XOFF flow control */
    /* output flags */
    tc.c_oflag &= ~ OPOST;                            /* disable output processing */
    tc.c_oflag &= ~(ONLCR | OCRNL);                   /* disable translating NL <-> CR */
    /* not for FreeBSD */
    tc.c_oflag &= ~ OFILL;                            /* disable fill characters */
    /* control flags */
    tc.c_cflag |=   CLOCAL;                           /* prevent changing ownership */
    tc.c_cflag |=   CREAD;                            /* enable reciever */
    tc.c_cflag &= ~ PARENB;                           /* disable parity */
    tc.c_cflag &= ~ CSTOPB;                           /* disable 2 stop bits */
    tc.c_cflag &= ~ CSIZE;                            /* remove size flag... */
    tc.c_cflag |=   CS8;                              /* ...enable 8 bit characters */
    tc.c_cflag |=   HUPCL;                            /* enable lower control lines on close - hang up */
    #ifdef XBEE_NO_RTSCTS
    tc.c_cflag &= ~ CRTSCTS;                          /* disable hardware CTS/RTS flow control */
    #else
    tc.c_cflag |=   CRTSCTS;                          /* enable hardware CTS/RTS flow control */
    #endif
    /* local flags */
    tc.c_lflag &= ~ ISIG;                             /* disable generating signals */
    tc.c_lflag &= ~ ICANON;                           /* disable canonical mode - line by line */
    tc.c_lflag &= ~ ECHO;                             /* disable echoing characters */
    tc.c_lflag &= ~ ECHONL;                           /* ??? */
    tc.c_lflag &= ~ NOFLSH;                           /* disable flushing on SIGINT */
    tc.c_lflag &= ~ IEXTEN;                           /* disable input processing */

    /* control characters */
    memset(tc.c_cc,0,sizeof(tc.c_cc));

    /* set i/o baud rate */
    if (cfsetspeed(&tc, pcmdr->_baud))
    {
        perror("cfsetspeed()");
        return false;
    }


    if (tcsetattr(pcmdr->fd, TCSAFLUSH, &tc))
    {
        perror("tcsetattr()");
        return false;
    }


    /* enable input & output transmission */
    if (tcflow(pcmdr->fd, TCOON | TCION))
    {
        perror("tcflow()");
        return false;
    }


    fflush(pcmdr->pfile);                             // again discard anything we have not read...

    //    printf("Thread Init\n");

    // May want to add end code... But for now don't have any defined...
    int ch;
    for(;;)
    {
        // Lets try using select to block our thread until we have some input available...
        FD_ZERO(&readfs);
        FD_SET(pcmdr->fd, &readfs);                   // Make sure we are set to wait for our descriptor
        tv.tv_sec = 0;
        tv.tv_usec = 250000;                          // 1/4 of a second...
                                                      // wait until some input is available...
        select(pcmdr->fd + 1, &readfs, NULL, NULL, &tv);

        while((ch = getc(pcmdr->pfile)) != EOF)
        {
            if(pcmdr->index == -1)                    // looking for new packet
            {
                if(ch == 0xff)
                {
                    pcmdr->index = 0;
                    pcmdr->checksum = 0;
                }
            }
            else if(pcmdr->index == 0)
            {
                pcmdr->bInBuf[pcmdr->index] = (unsigned char) ch;
                if(pcmdr->bInBuf[pcmdr->index] != 0xff)
                {
                    pcmdr->checksum += ch;
                    pcmdr->index++;
                }
            }
            else
            {
                pcmdr->bInBuf[pcmdr->index] = (unsigned char) ch;
                pcmdr->checksum += ch;
                pcmdr->index++;
                if(pcmdr->index == 7)                 // packet complete
                {
                    if(pcmdr->checksum%256 == 255)
                    {
                        // Lets grab our mutex to keep things consistent
                        pthread_mutex_lock(&pcmdr->lock);
                        for (int i=0; i < 6; i++)
                            pcmdr->vals[i] = pcmdr->bInBuf[i];
                        pcmdr->fValidPacket = true;
                        pthread_mutex_unlock(&pcmdr->lock);
                    }
                    pcmdr->index = -1;                // Say we are ready to start looking for start of next message...
                }
            }
        }
        // If we get to here try sleeping for a little time
        usleep(1000);                                 // Note: we could maybe simply block the thread until input available!
    }


    return 0;
}


bool Commander::begin(char *pszDevice,  speed_t baud)
{
    // Create our lock to make sure we can do stuff safely
    if (pthread_mutex_init(&lock, NULL) != 0)
        return false;

    _pszDevice = pszDevice;
    _baud = baud;
    // Now we need to create our thread for doing the reading from the Xbee
    int err = pthread_create(&tid, NULL, &XBeeThreadProc, this);
    if (err != 0)
        return false;

    return true;

    //    Serial.begin(baud);
}


/* SouthPaw Support */
void Commander::UseSouthPaw()
{
    status |= 0x01;
}


/* process messages coming from Commander
 *  format = 0xFF RIGHT_H RIGHT_V LEFT_H LEFT_V BUTTONS EXT CHECKSUM */
int Commander::ReadMsgs()
{
    if (fValidPacket)
    {
        pthread_mutex_lock(&lock);
        // We have a valid packet so lets use it.  But lets do that using our mutex to keep things consistent...
        if((status&0x01) > 0)                     // SouthPaw
        {
            rightV = (signed char)( (int)vals[0]-128 );
            rightH = (signed char)( (int)vals[1]-128 );
            leftV = (signed char)( (int)vals[2]-128 );
            leftH = (signed char)( (int)vals[3]-128 );
        }
        else
        {
            leftV = (signed char)( (int)vals[0]-128 );
            leftH = (signed char)( (int)vals[1]-128 );
            rightV = (signed char)( (int)vals[2]-128 );
            rightH = (signed char)( (int)vals[3]-128 );
        }
        pan = (vals[0]<<8) + vals[1];
        tilt = (vals[2]<<8) + vals[3];
        buttons = vals[4];
        ext = vals[5];
        fValidPacket = false;                     // clear out so we know if something new comes in
        pthread_mutex_unlock(&lock);
        return 1;
    }
    return 0;
}
