#include "DaysOfWeek.h"

bool DaysOfWeekClass::getDayOfWeek(byte num)
{
    return bitRead(DaysOfWeek, num);
}

// day of the week using the standard of the Adafruit RTC library - 0=Sunday, 6=saturday
bool DaysOfWeekClass::getDayOfWeek_Adafruit(byte num)
{
    if (num == 0) return bitRead(DaysOfWeek, 7);
    else return bitRead(DaysOfWeek, num);
}

bool DaysOfWeekClass::setDayOfWeek(byte num, bool status)
{
    if (num > 7 || num == 0) return false;
    bitWrite(DaysOfWeek, num, status);
    return true;
}
bool DaysOfWeekClass::setDayOfWeek_Adafruit(byte num, bool status)
{
    if (num > 6) return false;
    if (num == 0) bitWrite(DaysOfWeek, 7, status);
    else bitWrite(DaysOfWeek, num, status);
    return true;
}
