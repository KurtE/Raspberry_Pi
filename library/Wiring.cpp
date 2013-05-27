//=============================================================================
//Kurts Port support library for Raspberry Pi and BeagleBone Black
//=============================================================================
#include "ArduinoDefs.h"
#include <stdint.h>
#include <time.h>


long min(long a, long b)
{
    if (a <= b)
        return a;
    return b;
}


long max(long a, long b)
{
    if (a >= b)
        return a;
    return b;
}


unsigned long millis(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,  &ts );
    return ( ts.tv_sec * 1000 + ts.tv_nsec / 1000000L );
}


unsigned long micros(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,  &ts );
    return ( ts.tv_sec * 1000000L + ts.tv_nsec / 1000L );
}


extern "C" void __cxa_pure_virtual()
{
    while (1);
}


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

