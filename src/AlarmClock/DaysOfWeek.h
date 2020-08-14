/*!
    @file
*/


#ifndef DAYSOFWEEK_H
#define DAYSOFWEEK_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

/*!
    @brief  A class that is used by alarms to determine which day of the week
            to activate on. Stores a one-bit boolean value for each day of the
            week.

    Saved in EEPROM as 1B.
*/
class DaysOfWeek
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

