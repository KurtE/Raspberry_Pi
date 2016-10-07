//
// Quick and dirty program to clear the screen of the LCD...
//

#include "Adafruit_ILI9341B.h"
#include "Adafruit_GFX.h"
#include <signal.h>
#include "WrapperSerial.h"


//#include <math.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <termios.h>
//#include <errno.h>
//#include <fcntl.h>
//#include <string.h>
//#include <ctype.h>
//#include <sys/stat.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <stdarg.h>
//#include <time.h>
//#include <inttypes.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 29
#define TFT_CS  22//-1
#define TFT_RST 31
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


//--------------------------------------------------------------------------
// SignalHandler - Try to free up things like servos if we abort.
//--------------------------------------------------------------------------
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    tft.end();  // try to cleanup.
    
   exit(1); 

}

extern void setup(void);
extern void loop(void);

int main()
{
    // Install signal handler to allow us to do some cleanup...
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    printf("ILI9341 Clear screen Test!\n"); 
 
    tft.begin(0);
    tft.fillScreen(ILI9341_BLACK);
    printf("Done\n");
    tft.fillRect(100, 100, 100, 100, ILI9341_RED);
    tft.update();

    tft.end();

    delay(5000);
}

