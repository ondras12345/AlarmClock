/*!
    @file
    @brief  User-modifiable settings.
            Doxygen could not be used to document these settings because some
            of them need to be commented out.

    Constants like error codes are in Constants.h

    **NOTES**:
    - 1000UL - normal 16bit int would overflow for some values, so they need to
      be unsigned long
*/

#ifndef SETTINGS_H
#define SETTINGS_H


/*
DEBUG
*/
// Enables debug messages if uncommented
// Individual DEBUG_* options have no effect if main DEBUG is disabled
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
// Number of configurable alarms. Must be <255 (byte).
// Must be <=16 because of the GUI.
// default: 6
#define alarms_count 6

// Buzzer tone frequency (regular ringing)
// This setting has no effect if active_buzzer mode is enabled
// default: 1000
#define Alarm_regular_ringing_freq 1000 // Hz

// Buzzer on (and off) time (regular ringing)
// default: 500
#define Alarm_regular_ringing_period 500 // ms


// Buzzer volume ramp-up duration
// This setting has no effect if active_buzzer mode is enabled
// default: 20 * 1000UL
#define Alarm_ringing_ramp_duration 20 * 1000UL // ms

// default: 2000
#define Alarm_last_ringing_freq 2000 // Hz

// default: 250
#define Alarm_last_ringing_period 250 // ms

// Duration of the 'inhibit' function
// default: 120 * 60UL * 1000UL
#define Alarm_inhibit_duration 120 * 60UL * 1000UL  // ms

// Time before ambient LED reaches the set value
// default: 15 * 60UL * 1000UL
#define Alarm_ambient_dimming_duration 15 * 60UL * 1000UL  // ms

// Ambient LED fade-out time
// default: 2000UL
#define Alarm_ambient_fade_out_duration 2000UL  // ms


// Alarm timeout
// default: 15 * 60UL * 1000UL
#define Alarm_timeout 15 * 60UL * 1000UL  // ms



/*
Timer
*/

// Buzzer tone frequency for timer
// This setting has no effect if active_buzzer mode is enabled
// default: 3000
#define Timer_ringing_freq 3000  // Hz

// Buzzer on (and off) time for timer
// default: 1000
#define Timer_ringing_period 1000  // ms

// Time before ambient LED reaches the set value
// default: 60UL * 1000UL
#define Timer_ambient_dimming_duration 60UL * 1000UL  // ms

// Ambient LED fade-out time
// default: 2000UL
#define Timer_ambient_fade_out_duration 2000UL  // ms

// Timer timeout
// default: 15 * 60UL * 1000UL
#define Timer_timeout 15 * 60UL * 1000UL  // ms




/*
Hardware
*/
// Active buzzer mode. pin_buzzer is only switched on or off instead of
// producing a PWM modulated sine wave at the selected beeping frequency.
//
// Active buzzer - a buzzer that makes noise on its own without the need to
// feed it with AC signal (tone) - DC power is enough.
//
// Do not use with passive buzzers or speakers.
// default: disabled
//#define active_buzzer


// Internal watchdog timer.
// It resets the board if the software gets stuck or if the RTC fails.
// This only works on the AVR platform, otherwise a warning is generated.
// Only enable this setting if you are compiling this project for an Arduino
// board with a bootloader that does not have bugs in WDT implementation.
// An example of an unsupported platform is "Arduino Nano (old bootloader)".
// https://github.com/arduino/ArduinoCore-avr/issues/150
// Boards with Optiboot bootloader should not have this problem.
#define use_internal_WDT



/*
Pins
*/
// There should be no need to change the pins used.
// If you really need to, make sure you choose a pin that
// supports all the functions used (e.g. PWM, TimerOne, ...)

// 0, 1 - Serial
#define pin_encoder_clk 2
#define pin_encoder_dt 3
#define pin_button_snooze 4
#define pin_button_stop 5
#define pin_ambient 6  // PWM
// 7, 8
#define pin_buzzer 9  // PWM, TimerOne
#define pin_lamp 10  // PWM
// 11, 12, 13, A0, A1 - reserved for 2 SPI devices (e.g. Ethernet module)
#define pin_encoder_sw A2  // rotary encoder button
// A4, A5 - I2C



/*
I2C addresses
*/
#define I2C_LCD_address 0x27
#define I2C_DS3231_address 0x68



/*
Serial CLI
*/
// Width of indentation (e.g. in help and YAML)
#define Serial_indentation_width 2  // characters

// YAML_begin and YAML_end need to be put on separate lines with no other
// characters (except for '\r' and '\n')
#define YAML_begin F("---")
#define YAML_end F("...")

// Time elapsed since last command after which the CLI writes the changed data
// to the EEPROM
// default: 60 * 1000UL
#define Serial_autosave_interval 60 * 1000UL // ms

// Time before ambient LED reaches the set value
// default: 1000UL
#define Serial_ambient_dimming_duration 1000UL  // ms



/*
GUI
*/
// Delay between screen updates
// The screen only updates if seconds % 10 = 0, but I need this parameter to
// avoid updating it multiple times during the same second.
// default: 9000UL
#define GUI_update_interval 9000UL  // ms

// Time before ambient LED reaches the set value
// default: 500UL
#define GUI_ambient_dimming_duration 500UL  // ms

// LCD backlight timeout
// default: 15000UL
#define GUI_backlight_timeout 15000UL  // ms



/*
Buttons
*/
// Switch contact debouncing:
// https://www.allaboutcircuits.com/technical-articles/switch-bounce-how-to-deal-with-it/
// default: 25
#define button_debounce_interval 25  // ms

// Duration of a long press
// Not needed right now
//#define button_long_press 1000  // ms



/*
Rotary encoder
*/
// Number of pulses the encoder creates per step
// default: 4
#define encoder_step 4

// Idle time after which the encoder "microstep" count resets to 0.
// This prevents problems with missed "microsteps" causing the first full step
// in the opposite direction to be missed because the "microstep" count just
// changes from e.g. 2 to -2 (abs(-2) < encoder_step).
// default: 1000
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
