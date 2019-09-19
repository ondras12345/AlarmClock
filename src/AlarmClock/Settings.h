#ifndef _SETTINGS_h
#define _SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
/*
This file contains settings that are meant to be user changable.
Constants like error codes are in Constants.h
*/


#define VisualStudio // support for visual studio (aditional function prototypes required)

/*
DEBUG
*/
//#define DEBUG
//#define DEBUG_EEPROM_alarms // extra EEPROM messages
//#define DEBUG_EEPROM_writes // very long messages when writing
//#define DEBUG_ambient  // very long messages when calculating

/*
Alarms
*/
#define alarms_count 6

#define Alarm_regular_ringing_frequency 1000 // in Hz
#define Alarm_regular_ringing_period 500 // in ms

#define Alarm_last_ringing_frequency 2000 // in Hz
#define Alarm_last_ringing_period 250 // in ms

/*
Pins
*/
// 0, 1 - Serial
// 2, 3 - reserved for buttons / rotary encoder
#define pin_button_snooze 4
#define pin_button_stop 5
#define pin_ambient 6 // PWM
#define pin_lamp 7
#define pin_LCD_enable 8
#define pin_buzzer 9 // PWM, TimerOne
// 10, 11, 12, 13, A0 - reserved for SPI (eg. ethernet)
// A4, A5 - I2C

/*
I2C
*/
#define I2C_LCD_address 0x27
#define I2C_DS3231_address 0x68

/*
LCD dimensions
*/
#define LCD_width 16
#define LCD_height 2

/*
Serial CLI
*/
#define Serial_indentation_width 2
#define Serial_autosave_interval 60 * 1000UL // time elapsed since last command

/*
Button debounce
*/
#define button_debounce_interval 25 // ms

#endif
