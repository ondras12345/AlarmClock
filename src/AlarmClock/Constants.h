#ifndef _CONSTANTS_h
#define _CONSTANTS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Settings.h"
/*
This file contains constants that are not meant to be user changable.
User changable settings are in Settings.h
*/

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
String constants
*/
// 1 = Monday, 7 = Sunday
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
#define EEPROM_alarms_offset 10
#define EEPROM_AlarmClass_length (1 + 2 + 1 + 1 + 2 + 3)
#define EEPROM_alarms_id 0xFE
#define EEPROM_DEBUG_dump_length 100


/*
Serial CLI
*/
#define Serial_buffer_length 12
#define Serial_prompt_length 5

#define Serial_err_argument 1
#define Serial_err_select 2  // nothing selected
#define Serial_err_useless_save 4  // nothing to save


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

#endif
