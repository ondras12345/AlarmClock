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
static const char* days_of_the_week_names_short[] = { "", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };


/*
POST error codes
*/
#define error_I2C_ping_DS3231 1
#define error_time_lost 2
#define error_EEPROM 4
#define error_critical_mask 0b1111111111111101 // time_lost is not critical


/*
EEPROM
*/
#define EEPROM_alarms_offset 10
#define EEPROM_AlarmClass_record_length (1 + 2 + 1 + 1 + 2 + 3)
#define EEPROM_alarms_identificator 0xFE
#define EEPROM_DEBUG_dump_length 100


/*
Serial CLI
*/
#define Serial_buffer_length 12
#define Serial_prompt_length 5

#define Serial_error_argument 1
#define Serial_error_select 2  // nothing selected
#define Serial_error_useless_save 4  // nothing to save


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


/*
LCD dimensions
*/
#define LCD_width 16
#define LCD_height 2

#endif
