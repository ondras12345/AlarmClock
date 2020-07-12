/*!
    @file   LCDchars.h
    @brief  Custom characters for the LCD.
    Indexes are defined in Constants.h
    Character editor: https://maxpromer.github.io/LCD-Character-Creator/
*/


#ifndef _LCDchars_h
#define _LCDchars_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// The custom characters cannot be const because of the library
extern byte LCD_char_home[];

extern byte LCD_char_bell[];

extern byte LCD_char_timer[];

extern byte LCD_char_apply[];

extern byte LCD_char_cancel[];

#endif
