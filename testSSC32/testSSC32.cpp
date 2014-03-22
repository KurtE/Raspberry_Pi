
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

#include "ArduinoDefs.h"
#include "WrapperSerial.h"

// definition of some helper functions

extern int SSCRead (byte* pb, int cb, unsigned long ulTimeout, int EOL);

char szSSC32Device[] = "/dev/ttySSC-32";
WrapperSerial SSCSerial;

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
	SSCSerial.println(" VER");
  	SSCSerial.flush();
  	cbRead = SSCRead((byte*)abT, sizeof(abT), 250000, 13);
  	if (cbRead) {
            abT[cbRead-1] = 0;  // clear out CR
    		printf("Ver: %s\n", abT);
	}

	// Instead of hard checking version numbers instead ask it for
	// status of one of the players.  If we do not get a response...
	// probably does not support 
	SSCSerial.println("QPL0");
	cbRead = SSCRead((byte*)abT, 4, 25000, -1);

	printf("Check GP Enable: %i\n", cbRead);

	// See if it will digital inputs work at all...
    for (char c = 'A'; c <= 'H'; c++) {
        SSCSerial.println(c);
        abT[0] = '?';  // Some unlikely value
        cbRead = SSCRead((byte*)abT, 1, 25000, -1);
        printf("%c cb: %i c: %c\n", c, cbRead, abT[0]);
    }

	// likewise check for Analog
    for (char c = 'A'; c <= 'H'; c++) {
        SSCSerial.print("V");
        SSCSerial.println(c);
        abT[0] = 1;  // Some unlikely value
        cbRead = SSCRead((byte*)abT, 1, 25000, -1);
        printf("V%c cb: %i c: %x\n", c, cbRead, (int)abT[0]);
    }
    
    // Try to read registers...
    printf("\nRegisters\n");
    for (int i = 0; i< 96; i++) {
        SSCSerial.print("R");
        SSCSerial.println(i, DEC);
        cbRead = SSCRead((byte*)abT, sizeof(abT), 250000, 13);
        if (cbRead > 0) {
            abT[cbRead-1] = 0;  // Get rid of CR
            printf(" %s", abT);
        } else 
            printf(" ???");
        if ((i & 0xf) == 0xf)
            printf("\n");
    }        
    
    printf("\nCheck EEPROM\n");
    // How about just the first 32 bytes which should be used for sequences
    SSCSerial.println("EER 0;32");
    cbRead = SSCRead((byte*)abT, 32, 250000, -1);
    for (int i = 0; i< 32; i+=2) {
        unsigned int w = (byte)abT[i] + ((unsigned int)(byte)abT[i] >> 8);
        printf("%x ", w);
    }
    printf("\n");

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

