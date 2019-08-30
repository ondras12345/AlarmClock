#ifndef _CONSTANTS_h
#define _CONSTANTS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// 1 = Monday, 7 = Sunday
static const char* days_of_the_week_names_short[] = { "", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
#endif
