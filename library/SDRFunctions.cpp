//======================================================================================
// SDRFunctions.cpp - A way to allow multiple things register to use the sound system.
//    will relese the others if necessary.
//======================================================================================
#include "SDRFunctions.h"

static SDRFunc g_aSDRFuncs[MAX_SDRFUNCS];
unsigned char  g_cSDRFuncs = 0;
unsigned char  g_iSCRFuncsCur = 0xff;

unsigned char SDRRegisterReleaseFunction(SDRFunc func)
{
    if (g_cSDRFuncs < MAX_SDRFUNCS)
    {
        g_aSDRFuncs[g_cSDRFuncs] = func;
        return (g_cSDRFuncs++);
    }
    return 0xff;
}

void SDRSetCurrent(unsigned char uID)
{
    if (uID != g_iSCRFuncsCur) 
    {
        for (unsigned char i = 0; i < g_cSDRFuncs; i++)
        {
            if (i != uID)
                g_aSDRFuncs[i]();
        }
        
        g_iSCRFuncsCur = uID;
    }    
}