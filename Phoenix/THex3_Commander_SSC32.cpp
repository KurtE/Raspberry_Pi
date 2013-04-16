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
extern void setup(void);
extern void loop(void);

int main()
{
    setup();

    for(;;)
    {
        loop();
    }
}
