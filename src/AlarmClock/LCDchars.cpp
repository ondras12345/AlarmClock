/*!
    @file   LCDchars.cpp
    @brief  The actual definitions of the arrays.
*/

#include "LCDchars.h"

byte LCD_char_home[] = {
    B00100,
    B01110,
    B11111,
    B10001,
    B10001,
    B10001,
    B11111,
    B00000 };

byte LCD_char_bell[] = {
    B00100,
    B01110,
    B01110,
    B01110,
    B01110,
    B11111,
    B00000,
    B00100 };

byte LCD_char_timer[] = {
    B11111,
    B11111,
    B01110,
    B00100,
    B00100,
    B01110,
    B11111,
    B11111 };

byte LCD_char_apply[] = {
    B00000,
    B00000,
    B00001,
    B00010,
    B10100,
    B01000,
    B00000,
    B00000 };

byte LCD_char_cancel[] = {
    B00000,
    B10001,
    B01010,
    B00100,
    B01010,
    B10001,
    B00000,
    B00000 };
