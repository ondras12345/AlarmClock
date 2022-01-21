/*!
    @file
*/

#include "DaysOfWeek.h"


#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1 << (bit)))
#define bitClear(value, bit) ((value) &= ~(1 << (bit)))
#define bitToggle(value, bit) ((value) ^= (1 << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))


/*!
    @brief  Check whether the alarm should activate on a given day of the week.
    @param num day of the week (1 = Monday, 7 = Sunday)
    @see getDayOfWeek_Adafruit
*/
bool DaysOfWeek::getDayOfWeek(uint8_t num) const
{
    return bitRead(days_of_week, num);
}


/*!
    @brief  Check whether the alarm should activate on a given day of the week.
    @param num day of the week, Adafruit style (0 = Sunday, 6 = Saturday)
    @see getDayOfWeek
*/
bool DaysOfWeek::getDayOfWeek_Adafruit(uint8_t num) const
{
    if (num == 0) return bitRead(days_of_week, 7);
    else return bitRead(days_of_week, num);
}


/*!
    @brief  Set whether the alarm should activate on a given day of the week.
    @param num day of the week (1 = Monday, 7 = Sunday)
    @param status whether the alarm should activate or not
    @return true if the input data is valid; false if it fails
    @see setDayOfWeek_Adafruit
*/
bool DaysOfWeek::setDayOfWeek(uint8_t num, bool status)
{
    if (num > 7 || num == 0) return false;
    bitWrite(days_of_week, num, status);
    return true;
}


/*!
    @brief  Set whether the alarm should activate on a given day of the week.
    @param num day of the week, Adafruit style (0 = Sunday, 6 = Saturday)
    @param status whether the alarm should activate or not
    @return true if the input data is valid; false if it fails
    @see setDayOfWeek
*/
bool DaysOfWeek::setDayOfWeek_Adafruit(uint8_t num, bool status)
{
    if (num > 6) return false;
    if (num == 0) bitWrite(days_of_week, 7, status);
    else bitWrite(days_of_week, num, status);
    return true;
}
