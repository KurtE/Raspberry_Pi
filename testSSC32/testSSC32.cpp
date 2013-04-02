
/*
* Quick and dirty test of the SSC-32
*/
#include <math.h>
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
#include <stdarg.h>
#include <time.h>
#include <inttypes.h>


#include "WrapperSerial.h"

// definition of some helper functions
typedef unsigned char byte;

extern int SSCRead (byte* pb, int cb, unsigned long ulTimeout, int EOL);

char szSSC32Device[] = "/dev/ttySSC-32";
WrapperSerial SSCSerial;


#define delay(x)  usleep((x)*1000)

unsigned long millis(void) {
    struct timeval tv;
    gettimeofday ( &tv, NULL );
    return ( tv.tv_sec * 1000 + tv.tv_usec / 1000 );
}

unsigned long micros(void) {
    struct timeval tv;
    gettimeofday ( &tv, NULL );
    return ( tv.tv_sec * 1000000 + tv.tv_usec);
}

int main(void)
{
	char abT[40];        // give a nice large buffer.
	byte cbRead;
	
	printf("Start\n");
	bool f = SSCSerial.begin(szSSC32Device, B115200);	
  	if (!f) {
		printf("SSCSeerial begin failed!\n");
	  	return (-1);
  	}
	printf("Test Ver command\n");
	SSCSerial.println("VER");
  	SSCSerial.flush();
  	cbRead = SSCRead((byte*)abT, sizeof(abT), 250000, -1);
  	if (cbRead) {
    		printf("Ver: %s\n", abT);
	}

	// Instead of hard checking version numbers instead ask it for
	// status of one of the players.  If we do not get a response...
	// probably does not support 
	SSCSerial.println("QPL0");
	cbRead = SSCRead((byte*)abT, 4, 25000, -1);

	printf("Check GP Enable: %i\n", cbRead);

	// See if it will digital inputs work at all...
	SSCSerial.println("A");
	abT[0] = '?';  // Some unlikely value
	cbRead = SSCRead((byte*)abT, 1, 25000, -1);
	printf("A cb: %i c: %c\n", cbRead, abT[0]);


	// likewise check for Analog
	SSCSerial.println("VB");
	abT[0] = 1;  // Some unlikely value
	cbRead = SSCRead((byte*)abT, 1, 25000, -1);
	printf("VB cb: %i c: %x\n", cbRead, (int)abT[0]);

	printf("Exit\n");
	

	return 0;
}
//==============================================================================
// Quick and dirty helper function to read so many bytes in from the SSC with a timeout and an end of character marker...
//==============================================================================
int SSCRead (byte* pb, int cb, unsigned long ulTimeout, int EOL)
{
  int ich;
  byte* pbIn = pb;
  unsigned long ulTimeLastChar = micros();
  while (cb) {
    while ((ich = SSCSerial.read()) == -1) {
      // check for timeout
      if ((unsigned long)(micros()-ulTimeLastChar) > ulTimeout) {
        return (int)(pb-pbIn);
      }    
    }
    *pb++ = (byte)ich;
    cb--;

    if (ich == EOL)
      break;    // we matched so get out of here.
    ulTimeLastChar = micros();    // update to say we received something
  }

  return (int)(pb-pbIn);
}

