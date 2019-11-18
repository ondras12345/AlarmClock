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
For more info about these options see docs/manual.md
*/


#define VisualStudio // support for visual studio (aditional function prototypes required)

/*
DEBUG
*/
//#define DEBUG
//#define DEBUG_EEPROM_alarms // extra EEPROM messages
//#define DEBUG_EEPROM_writes // very long messages when writing
//#define DEBUG_ambient  // very long messages when calculating
//#define DEBUG_Serial_CLI  // HEX dumps of received data, ...
//#define DEBUG_GUI
//#define DEBUG_encoder

/*
Alarms
*/
#define alarms_count 6

#define Alarm_regular_ringing_frequency 1000 // in Hz
#define Alarm_regular_ringing_period 500 // in ms

#define Alarm_last_ringing_frequency 2000 // in Hz
#define Alarm_last_ringing_period 250 // in ms

#define Alarm_inhibit_duration 60 * 60UL * 1000UL  // in ms

/*
Pins
*/
// 0, 1 - Serial
#define pin_encoder_clk 2
#define pin_encoder_dt 3
#define pin_button_snooze 4
#define pin_button_stop 5
#define pin_ambient 6  // PWM
#define pin_LED 7  // inhibit indicator
#define pin_LCD_enable 8
#define pin_buzzer 9  // PWM, TimerOne
#define pin_lamp 10  // (PWM)
// 11, 12, 13, A0, A1 - reserved for 2 SPI devices (eg. ethernet)
#define pin_encoder_sw A2  // rotary encoder button
// A4, A5 - I2C

/*
I2C
*/
#define I2C_LCD_address 0x27
#define I2C_DS3231_address 0x68

/*
Serial CLI
*/
#define Serial_indentation_width 2
#define Serial_autosave_interval 60 * 1000UL // time elapsed since last command

/*
Buttons
*/
#define button_debounce_interval 25  // ms
#define button_long_press 1000  // ms

/*
Rotary encoder
*/
#define encoder_step 4

#endif
