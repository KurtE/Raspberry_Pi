
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
#include "RoboClaw.h"

// definition of some helper functions
typedef unsigned char byte;


char szRCDevice[] ="/dev/ttyRCLAW";

RoboClaw RClaw;

#define delay(x)  usleep((x)*1000)
#define address 0x80
char	szBuff[128];

int main(int argc, char *argv[])
{
	char abT[40];        // give a nice large buffer.
	byte cbRead;

	printf("Start\n");
    	if (argc > 1)
    	{
       	for (int i=1; i < argc; i++) 
        	{
            		printf("%d - %s\n", i, argv[i]);
        	}
    	}
	bool f = RClaw.begin(argc > 1? argv[1] : szRCDevice, B38400);
  	if (!f) {
		printf("RClaw begin failed!\n");
	  	return (-1);
  	}
	printf("Test Version command\n");
	f = RClaw.ReadVersion(address, szBuff);
	printf("Ver %i : %s\n", f? 1 : 0, szBuff);

	printf("Exit\n");
	

	return 0;
}
