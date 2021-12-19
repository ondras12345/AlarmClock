/*!
    @file
    @brief  Constants that are not meant to be changed by the user.
    User changeable settings are in Settings.h
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "Settings.h"

/*
DEBUG
*/
#ifdef DEBUG
#define DEBUG_print(x) Serial.print(x)
#define DEBUG_println(x) Serial.println(x)
#else
#define DEBUG_print(x)
#define DEBUG_println(x)
#endif // DEBUG

/*
WDT
*/
#ifdef use_internal_WDT
#if defined(__AVR__)
#define internal_WDT
#else
// warning is not a standard C feature, but GCC should support it.
#warning "WDT is not supported on this platform."
#endif
#endif // use_internal_WDT


/*
String constants
*/
extern const char* days_of_the_week_names_short[8];


/*
POST error codes
*/
#define err_I2C_ping_DS3231 1
#define err_time_lost 2
#define err_EEPROM 4
#define err_critical_mask 0b1111111111111101 // time_lost is not critical


/*
EEPROM
*/
#define EEPROM_size 1024
#define EEPROM_alarms_offset 0x40
#define EEPROM_alarms_id 0xFE

#define EEPROM_melodies_header_start 0x0010
#define EEPROM_melodies_count 16  // 48 bytes
// we need to leave enough space for alarms...
// 16 alarms * 10 bytes per alarm = 160 bytes
// alarms start at 0x0040
#define EEPROM_melodies_data_start 0x0100


/*
Button debounce
*/
#define button_count 3
#define button_index_snooze 0
#define button_index_stop 1
#define button_index_encoder 2


/*
LCD
*/
// The arrays are in LCDchars.h
// The numbers cannot start from 0 because '\0' has a special meaning (string
// termination)
#define LCD_char_home_index 1
#define LCD_char_bell_index 2
#define LCD_char_timer_index 3
#define LCD_char_apply_index 4
#define LCD_char_cancel_index 5


/*
LCD dimensions
*/
#define LCD_width 16
#define LCD_height 2

/*
Melodies
*/
// bit 0 is always 1
#define melody_header_flag_enabled 1  // bit 1

#define melody_footer_flag_repeat 0  // bit 0

#endif
