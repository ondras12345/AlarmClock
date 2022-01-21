/*!
    @file
*/


#ifndef DAYSOFWEEK_H
#define DAYSOFWEEK_H

#include <stdint.h>


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
    /*!
        @brief  A one byte long representation of the object.
        The value for each day of the week is represented as a single bit in
        this number.
        Bit 0 (LSB) means nothing,
        bit 1 is Monday,
        bit 7 (MSB) is Sunday.
    */
    uint8_t days_of_week;
    bool getDayOfWeek(uint8_t num) const;
    bool getDayOfWeek_Adafruit(uint8_t num) const;
    bool setDayOfWeek(uint8_t num, bool status);
    bool setDayOfWeek_Adafruit(uint8_t num, bool status);
};

#endif

