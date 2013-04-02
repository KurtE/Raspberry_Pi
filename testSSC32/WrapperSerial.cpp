/*
  WrapperSerial.cpp - Hardware serial library for Wiring
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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "WrapperSerial.h"

// Constructors ////////////////////////////////////////////////////////////////

WrapperSerial::WrapperSerial()
{
	_fdOut = -1;	// Say that it is not open...
	_fdIn = -1;
	_pfileOut = NULL;
	_pfileIn = NULL; 
	_peek = -1;
}

// Public Methods //////////////////////////////////////////////////////////////

bool WrapperSerial::begin(char *pszDevice, speed_t baud, bool fRtsCts)
{
	struct termios tc;
	
	// If the file is already open, call the end function to close the file...
	if (_fdOut != -1)
		end();	// first close it

	if ((_fdOut = open(pszDevice, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK)) == -1) {
		printf("Open Failed\n");
		return false;
	}

	if ((_pfileOut = fdopen(_fdOut, "r+")) == NULL) {
		return false;
	}

	// For normal files _pfileIn will simply be _pfileOut likewise for file descriptors
	_pfileIn = _pfileOut;
	_fdIn = _fdOut;

	setvbuf(_pfileOut, NULL, _IONBF, BUFSIZ);
	fflush(_pfileOut);

	if (tcgetattr(_fdOut, &tc)) {
		return false;
	}

	/* input flags */
	tc.c_iflag &= ~ IGNBRK;           /* enable ignoring break */
	tc.c_iflag &= ~(IGNPAR | PARMRK); /* disable parity checks */
	tc.c_iflag &= ~ INPCK;            /* disable parity checking */
	tc.c_iflag &= ~ ISTRIP;           /* disable stripping 8th bit */
	tc.c_iflag &= ~(INLCR | ICRNL);   /* disable translating NL <-> CR */
	tc.c_iflag &= ~ IGNCR;            /* disable ignoring CR */
	tc.c_iflag &= ~(IXON | IXOFF);    /* disable XON/XOFF flow control */
	/* output flags */
	tc.c_oflag &= ~ OPOST;            /* disable output processing */
	tc.c_oflag &= ~(ONLCR | OCRNL);   /* disable translating NL <-> CR */
	/* not for FreeBSD */
	tc.c_oflag &= ~ OFILL;            /* disable fill characters */
	/* control flags */
	tc.c_cflag |=   CLOCAL;           /* prevent changing ownership */
	tc.c_cflag |=   CREAD;            /* enable reciever */
	tc.c_cflag &= ~ PARENB;           /* disable parity */
	tc.c_cflag &= ~ CSTOPB;         /* disable 2 stop bits */
	tc.c_cflag &= ~ CSIZE;            /* remove size flag... */
	tc.c_cflag |=   CS8;              /* ...enable 8 bit characters */
	tc.c_cflag |=   HUPCL;            /* enable lower control lines on close - hang up */
	if (fRtsCts) 
		tc.c_cflag |=   CRTSCTS;          /* enable hardware CTS/RTS flow control */
	else	
		tc.c_cflag &= ~ CRTSCTS;          /* disable hardware CTS/RTS flow control */
	/* local flags */
	tc.c_lflag &= ~ ISIG;             /* disable generating signals */
	tc.c_lflag &= ~ ICANON;           /* disable canonical mode - line by line */
	tc.c_lflag &= ~ ECHO;             /* disable echoing characters */
	tc.c_lflag &= ~ ECHONL;           /* ??? */
	tc.c_lflag &= ~ NOFLSH;           /* disable flushing on SIGINT */
	tc.c_lflag &= ~ IEXTEN;           /* disable input processing */

	/* control characters */
	memset(tc.c_cc,0,sizeof(tc.c_cc));
	
	/* set i/o baud rate */
	cfsetspeed(&tc, baud);
	
	tcsetattr(_fdOut, TCSAFLUSH, &tc);

	/* enable input & output transmission */
	tcflow(_fdOut, TCOON | TCION);
	
	/* purge buffer */
	{
		char buf[1024];
		int n;
		do {
			usleep(5000); /* 5ms */
			n = ::read(_fdOut, buf, sizeof(buf));
		} while (n > 0);
	}
	fcntl(_fdOut, F_SETFL, 0); /* disable blocking */
	return true;	// it succeeded
}
	
bool WrapperSerial::begin()
{
	// Special version for wrapping stdin/stdout... My make a version for
	// any fds...
	// Go out to standard file stuff
	_fdOut = STDOUT_FILENO;
	_fdIn = STDIN_FILENO;
	_pfileOut = stdout;
	_pfileIn = stdin;
	return true;	// it succeeded
}



void WrapperSerial::end()
{
	// Close our file
	if (_fdOut != -1) {
		close(_fdOut);
		_fdOut = -1;
		if (_pfileOut != NULL) {
			fclose(_pfileOut);
			_pfileOut = NULL;
		}	
	}
	
}

int WrapperSerial::available(void)
{
	int bytes;

	// BUGBUG:: handle standard terminal better
	ioctl(_fdIn, FIONREAD, &bytes);
	return bytes;
}

int WrapperSerial::peek(void)
{
	if (_peek == -1) 
		_peek = read();
	
	return _peek;		
}

int WrapperSerial::read(void)
{
	if (_peek != -1) {
		int iRet = _peek;
		_peek = -1;
		return iRet;
	}
	
	unsigned char b;
	if (fread( &b, 1, 1, _pfileIn))
		return b;
	return -1;
}

void WrapperSerial::flush()
{
	fflush(_pfileOut);
}

size_t WrapperSerial::write(uint8_t c)
{
	fwrite(&c, 1, 1, _pfileOut);
  return 1;
}

