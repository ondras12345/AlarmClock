/*!
    @file DaysOfWeek.cpp
*/

#include "DaysOfWeek.h"

/*!
    @brief  Check whether the alarm should activate on a given day of the week.
    @param num day of the week (1 = Monday, 7 = Sunday)
*/
bool DaysOfWeekClass::getDayOfWeek(byte num) const
{
    return bitRead(DaysOfWeek, num);
}


/*!
    @brief  Check whether the alarm should activate on a given day of the week.
    @param num day of the week, Adafruit style (0 = Sunday, 6 = Saturday)
*/
bool DaysOfWeekClass::getDayOfWeek_Adafruit(byte num) const
{
    if (num == 0) return bitRead(DaysOfWeek, 7);
    else return bitRead(DaysOfWeek, num);
}


/*!
    @brief  Set whether the alarm should activate on a given day of the week.
    @param num day of the week (1 = Monday, 7 = Sunday)
    @param status whether the alarm should activate or not
    @return true if the input data is valid; false if it fails
*/
bool DaysOfWeekClass::setDayOfWeek(byte num, bool status)
{
    if (num > 7 || num == 0) return false;
    bitWrite(DaysOfWeek, num, status);
    return true;
}


/*!
    @brief  Set whether the alarm should activate on a given day of the week.
    @param num day of the week, Adafruit style (0 = Sunday, 6 = Saturday)
    @param status whether the alarm should activate or not
    @return true if the input data is valid; false if it fails
*/
bool DaysOfWeekClass::setDayOfWeek_Adafruit(byte num, bool status)
{
    if (num > 6) return false;
    if (num == 0) bitWrite(DaysOfWeek, 7, status);
    else bitWrite(DaysOfWeek, num, status);
    return true;
}
