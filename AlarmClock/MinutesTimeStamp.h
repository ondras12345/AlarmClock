// MinutesTimeStamp.h
/*
Minutes since midnight
*/

#ifndef _MINUTESTIMESTAMP_h
#define _MINUTESTIMESTAMP_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class MinutesTimeStampClass
{
public:
    int timestamp;
    byte get_hours();
    byte get_minutes();
    void set_hours_minutes(byte hours, byte minutes);
    MinutesTimeStampClass(byte hours, byte minutes);
    MinutesTimeStampClass();

};

#endif

