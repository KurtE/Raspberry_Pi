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
#include <signal.h>
#include "mraa.h"

//hello.cpp
int fd;
FILE *f;

char szDevice[] = "/dev/ttyXBEE";

Commander command = Commander();

signed char rightV;                               // vertical stick movement = forward speed
signed char rightH;                               // horizontal stick movement = sideways or angular speed
signed char leftV;                                // vertical stick movement = tilt
signed char leftH;                                // horizontal stick movement = pan (when we run out of pan, turn body?)
unsigned char buttons;                            //
unsigned char ext;                                // Extended function set

uint8_t fRunning = true;

void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);
    fRunning = false;
}


int main()
{
    printf("Arbotox Commander XBee Test!\n");

    // Install signal handler to allow us to do some cleanup...
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    mraa_uart_context uart;
    mraa_init();
    uart = mraa_uart_init(0);
    if (uart == NULL) {
        printf("MRAA UART failed to setup\n");
    }

    // Lets try to open the XBee device...
    command.begin(szDevice, B38400);
    printf("After Begin!\n");

    // loop simply echo what we receive from xbee to terminal
    // Now lets try to get data from the commander.
    while (fRunning)
    {
        if (command.ReadMsgs())
        {
            // We have data.  see if anything has changed before
            if ((command.rightV != rightV) || (command.rightH != rightH) ||
                (command.leftV != leftV) || (command.leftH != leftH) ||
                (command.buttons != buttons) || (command.ext != ext))
            {
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
