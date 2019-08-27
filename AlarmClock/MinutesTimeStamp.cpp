// 
// 
// 

#include "MinutesTimeStamp.h"

byte MinutesTimeStampClass::get_hours()
{
    return byte(timestamp / 60);
}

byte MinutesTimeStampClass::get_minutes()
{
    return byte(timestamp % 60);
}

void MinutesTimeStampClass::set_hours_minutes(byte hours, byte minutes)
{
    timestamp = (hours * 60) + minutes;
}

MinutesTimeStampClass::MinutesTimeStampClass(byte hours, byte minutes)
{
    set_hours_minutes(hours, minutes);
}

MinutesTimeStampClass::MinutesTimeStampClass()
{
    timestamp = 0;
}
