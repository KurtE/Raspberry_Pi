//=============================================================================
//Project Lynxmotion Phoenix
//Description: Phoenix software
//Software version: V2.0
//Date: 29-10-2009
//Programmer: Jeroen Janssen [aka Xan]
//         Kurt Eckhardt(KurtE) converted to C and Arduino
//   Kåre Halvorsen aka Zenta - Makes everything work correctly!
//
// This version of the Phoenix code was ported over to the Arduino Environement
// and is specifically configured for the Arbotix Robocontroller board
//
//=============================================================================
//
//KNOWN BUGS:
//    - Lots ;)
//
//=============================================================================
// Header Files
//=============================================================================

#define DEFINE_HEX_GLOBALS
#if ARDUINO>99
#include <Arduino.h>
#else
#endif

#include "Hex_Cfg.h"
#include "Phoenix.h"
#include "speak.h"
#include <signal.h>



extern void setup(void);
extern void loop(void);
extern void cleanup(void);

//--------------------------------------------------------------------------
// SignalHandler - Try to free up things like servos if we abort.
//--------------------------------------------------------------------------
volatile uint8_t g_fSignaled = false;

void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);
    g_fSignaled = true;
}


int main()
{
    // Install signal handler to allow us to do some cleanup...
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGQUIT, &sigIntHandler, NULL);


    setup();

    while (!g_fSignaled)
    {
        loop();
    }
    // Call cleanup function and exit;
    cleanup();
}
