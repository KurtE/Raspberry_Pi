//======================================================================================
// SDRFunctions.h - A way to allow multiple things register to use the sound system.
//    will relese the others if necessary.
//======================================================================================
#ifndef _SDRFUNCTIONS_H_
#define _SDRFUNCTIONS_H_

#define MAX_SDRFUNCS   3    // define how big our array should be 2 is all I know of...

typedef void (*SDRFunc)(void);
extern unsigned char SDRRegisterReleaseFunction(SDRFunc func);
extern void SDRSetCurrent(unsigned char uID);

#endif
