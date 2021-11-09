/*!
    @file
    @brief  Custom characters for the LCD.
    Indexes are defined in Constants.h
    Character editor: https://maxpromer.github.io/LCD-Character-Creator/
*/


#ifndef LCDCHARS_H
#define LCDCHARS_H

#include "Arduino.h"

// The custom characters cannot be const because of LiquidCrystal_I2C.h

extern byte LCD_char_home[];

extern byte LCD_char_bell[];

extern byte LCD_char_timer[];

extern byte LCD_char_apply[];

extern byte LCD_char_cancel[];

#endif
