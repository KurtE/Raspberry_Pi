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

//hello.cpp
int fd;
FILE *f;

char szDevice[] = "/dev/ttyXBEE";

Commander command = Commander();

    signed char rightV;      // vertical stick movement = forward speed
    signed char rightH;      // horizontal stick movement = sideways or angular speed
    signed char leftV;      // vertical stick movement = tilt    
    signed char leftH;      // horizontal stick movement = pan (when we run out of pan, turn body?)
    unsigned char buttons;  // 
    unsigned char ext;      // Extended function set


int main() {
	printf("Arbotox Commander XBee Test!\n");
	// Lets try to open the XBee device...
	command.begin(szDevice, B38400);
	printf("After Begin!\n");

	// loop simply echo what we receive from xbee to terminal
	// Now lets try to get data from the commander.
	for (;;) {
		if (command.ReadMsgs()) {
			// We have data.  see if anything has changed before 
			if ((command.rightV != rightV) || (command.rightH != rightH) ||
				(command.leftV != leftV) || (command.leftH != leftH) ||
				(command.buttons != buttons) || (command.ext != ext)) {
				// Something changed so print it out
    				rightV = command.rightV;
    				rightH = command.rightH;
    				leftV = command.leftV;
    				leftH = command.leftH;
    				buttons = command.buttons;  
    				ext = command.ext;      
				printf("%x %x - %d %d %d %d\n", buttons, ext, rightV, rightH, leftV, leftH);
			}
		}
		usleep(100);
	}
	return 0;
}
