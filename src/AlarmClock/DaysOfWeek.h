// DaysOfWeek.h
/*
1 = Monday, 7 = Sunday
Saves to EEPROM as 1B
Used by alarms to determine which day of week to ring on
*/

#ifndef _DAYSOFWEEK_h
#define _DAYSOFWEEK_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class DaysOfWeekClass
{
protected:


public:
    byte DaysOfWeek;
    bool getDayOfWeek(byte num) const;
    bool getDayOfWeek_Adafruit(byte num) const;
    bool setDayOfWeek(byte num, bool status);
    bool setDayOfWeek_Adafruit(byte num, bool status);
};

#endif

