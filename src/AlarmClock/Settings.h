#ifndef _SETTINGS_h
#define _SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
/*
This file contains settings that are meant to be user changeable.
Constants like error codes are in Constants.h
For more info about these options see docs/manual.md
*/


/*
DEBUG
*/
//#define DEBUG
//#define DEBUG_EEPROM_alarms // extra EEPROM messages
//#define DEBUG_EEPROM_writes // very long messages when writing
//#define DEBUG_dimmer  // very long messages when calculating
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

#define Alarm_inhibit_duration 120 * 60UL * 1000UL  // in ms
#define Alarm_ambient_dimming_duration 15 * 60UL * 1000UL  // in ms
#define Alarm_ambient_fade_out_duration 2000UL  // in ms

/*
Hardware
*/
// Active buzzer mode. pin_buzzer is only switched on or off instead of
// being switched at the beeping frequency.
// Do not use with passive buzzers or speakers.
//#define active_buzzer


/*
Pins
*/
// 0, 1 - Serial
#define pin_encoder_clk 2
#define pin_encoder_dt 3
#define pin_button_snooze 4
#define pin_button_stop 5
#define pin_ambient 6  // PWM
// 7, 8
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
#define Serial_autosave_interval 60 * 1000UL // in ms; time elapsed since last command
#define Serial_ambient_dimming_duration 1000UL  // in ms
/*
GUI
*/
#define GUI_update_interval 9000UL  // in ms
#define GUI_ambient_dimming_duration 500UL  // in ms
#define GUI_backlight_timeout 15000UL  // in ms

/*
Buttons
*/
#define button_debounce_interval 25  // ms
//#define button_long_press 1000  // ms, not needed right now

/*
Rotary encoder
*/
#define encoder_step 4
// Idle time after which the encoder "microstep" count resets to 0.
// This prevents problems with missed "microsteps" causing the first full step
// in the opposite direction to be missed because the "microstep" count just
// changes from e.g. 2 to -2 (abs(-2) < encoder_step).
#define encoder_reset_interval 1000  // ms

// Encoder loop
// When a value reaches its max, next step takes it to the min and vice versa
#define encoder_loop_time true
//#define encoder_loop_date  // waits for date user input checking to be implemented
#define encoder_loop_cursor false  // cursor movement
#define encoder_loop_ambient false
#define encoder_loop_alarm false  // selected alarm
#define encoder_loop_snooze true  // snooze time

#endif
