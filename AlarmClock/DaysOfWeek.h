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
    boolean getDayOfWeek(byte num);
    boolean getDayOfWeek_Adafruit(byte num);
    void setDayOfWeek(byte num, boolean status);
    void setDayOfWeek_Adafruit(byte num, boolean status);
};

#endif

