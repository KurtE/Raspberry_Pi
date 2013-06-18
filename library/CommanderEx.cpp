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

#ifdef CMDR_USE_SOCKET
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define DEBUG_SOCKET

/* Constructor */
Commander::Commander()
{
    index = -1;
    status = 0;
    fValidPacket = false;
}


#ifdef CMDR_USE_XBEE
void *Commander::XBeeThreadProc(void *pv)
{
    Commander *pcmdr = (Commander*)pv;
    fd_set readfs;                                // file descriptor set to wait on.
    timeval tv;                                   // how long to wait.

    //    printf("Thread start(%s)\n", pcmdr->_pszDevice);

    // Lets do our init of the xbee here.
    // We will do all of the stuff to intialize the serial port plus we will spawn off our thread.
    struct termios tc;

    if ((pcmdr->fdXBee = open(pcmdr->_pszDevice, O_RDWR | O_NOCTTY | O_SYNC /* |  O_NONBLOCK */)) == -1) 
    {
        printf("Open Failed\n");
        return 0;
    }


    if ((pcmdr->pfileXBee = fdopen(pcmdr->fdXBee, "r+")) == NULL)
    {
        return 0;
    }


    setvbuf(pcmdr->pfileXBee, NULL, _IONBF, BUFSIZ);
    fflush(pcmdr->pfileXBee);

    if (tcgetattr(pcmdr->fdXBee, &tc))
    {
        perror("tcgetattr()");
        return 0;
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
        return 0;
    }


    if (tcsetattr(pcmdr->fdXBee, TCSAFLUSH, &tc))
    {
        perror("tcsetattr()");
        return 0;
    }


    /* enable input & output transmission */
    if (tcflow(pcmdr->fdXBee, TCOON | TCION))
    {
        perror("tcflow()");
        return 0;
    }


    fflush(pcmdr->pfileXBee);                             // again discard anything we have not read...

    //    printf("Thread Init\n");

    // May want to add end code... But for now don't have any defined...
    int ch;
    while(!pcmdr->_fCancel)
    {
        // Lets try using select to block our thread until we have some input available...
        FD_ZERO(&readfs);
        FD_SET(pcmdr->fdXBee, &readfs);                   // Make sure we are set to wait for our descriptor
        tv.tv_sec = 0;
        tv.tv_usec = 250000;                          // 1/4 of a second...
                                                      // wait until some input is available...
        select(pcmdr->fdXBee + 1, &readfs, NULL, NULL, &tv);

        while((!pcmdr->_fCancel) && (ch = getc(pcmdr->pfileXBee)) != EOF)
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

    printf("Commander - XBee thread exit\n");
    return 0;
}
#endif

#ifdef CMDR_USE_SOCKET

void Commander::SocketThreadCleanupProc(void *pv)
{
    printf("Commander - Socket thread exit\n");
}

void *Commander::SocketThreadProc(void *pv)
{
    Commander *pcmdr = (Commander*)pv;

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[100];
    time_t ticks; 
    
    struct timeval timeout;      
    timeout.tv_sec = 2; // 2 second timeouts...
    timeout.tv_usec = 0;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    pthread_cleanup_push(&SocketThreadCleanupProc, pv);

    while(!pcmdr->_fCancel)
    {
        printf("Wait for Accept\n");
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("Accept\n");
        if (setsockopt (connfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
            printf("setsockopt failed\n");

        if (setsockopt (connfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
            printf("setsockopt failed\n");
        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuff, strlen(sendBuff)); 
        printf("Did Write\n");
        sleep(1);
        
        // Now lets process messages from this socket client...
        unsigned char bIn;
        unsigned char abPacket[7];      // 8 byte packets
        unsigned char iPacket;
        unsigned char bChecksum = 0;
#ifdef DEBUG_SOCKET
        unsigned char abPacketPrev[7];  
        unsigned char fChanged;
#endif        
        // Lts loop through reading in Commander Packets that start with 0xff
        iPacket = 0xff;
        while (read(connfd, &bIn, 1) > 0) {
            if (iPacket == 0xff) {
                if (bIn == 0xff) {
                    iPacket = 0;
                    bChecksum = 0;

#ifdef DEBUG_SOCKET
                    fChanged = 0;
#endif
                }
            } else {
#ifdef DEBUG_SOCKET
                if (abPacketPrev[iPacket] != bIn) {
                    fChanged = 1;
                    abPacketPrev[iPacket] = bIn;
                }
#endif                
                abPacket[iPacket++] = bIn;
                bChecksum += bIn;
                if (iPacket == 7) {
                    // received a whole packet...
                    if (bChecksum == 0xff) {
#ifdef DEBUG_SOCKET
                        if (fChanged) {
                            printf("%d %d %d %d - %x\n", 
                                (int)abPacket[0]-128,
                                (int)abPacket[1]-128,
                                (int)abPacket[2]-128,
                                (int)abPacket[3]-128,
                                abPacket[4] );
                        }
#endif
                        // Lets grab our mutex to keep things consistent
                        pthread_mutex_lock(&pcmdr->lock);
                        for (int i=0; i < 6; i++)
                            pcmdr->vals[i] = abPacket[i];
                        pcmdr->fValidPacket = true;
                        pthread_mutex_unlock(&pcmdr->lock);
                    }
                    else {
                        printf("Checksum error: %x\n", bChecksum);
                    }
                    iPacket = 0xff;    // setup for next packet
                }
            }
        }
        printf("Read timed out so close\n");
        close(connfd);
        sleep(1);
    };
    pthread_cleanup_pop(1);

    return 0;
}
#endif


bool Commander::begin(char *pszDevice,  speed_t baud)
{
    int err;
    // Create our lock to make sure we can do stuff safely
    if (pthread_mutex_init(&lock, NULL) != 0)
        return false;

    _fCancel = false;	// Flag to let our thread(s) know to abort.
#ifdef CMDR_USE_XBEE
    _pszDevice = pszDevice;
    _baud = baud;
    // Now we need to create our thread for doing the reading from the Xbee
    err = pthread_create(&tidXBee, NULL, &XBeeThreadProc, this);
    if (err != 0)
        return false;
#endif
#ifdef CMDR_USE_SOCKET
    // Now we need to create our thread for doing the reading from the Xbee
    err = pthread_create(&tidSocket, NULL, &SocketThreadProc, this);
    if (err != 0)
        return false;

#endif


    return true;
}

void Commander::end()
{
    _fCancel = true;
#ifdef CMDR_USE_SOCKET
    // See if the thread goes away.  If not, we can kill it.
    pthread_cancel(tidSocket);

#endif
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
