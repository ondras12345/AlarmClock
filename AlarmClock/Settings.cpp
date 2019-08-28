/*
Compile-time options
*/
#define alarms_count 6
#define VisualStudio
//#define DEBUG

/*
Alarms
*/
#define Alarm_regular_ringing_frequency 1000 // in Hz
#define Alarm_regular_ringing_period 500 // in ms

#define Alarm_last_ringing_frequency 2000 // in Hz
#define Alarm_last_ringing_period 250 // in ms

/*
Pins
*/
// 2,3 - reserved for buttons / rotary encoder
#define pin_lamp 4
#define pin_buzzer 5 // PWM
#define pin_ambient 6 // PWM
#define pin_LCD_enable 7
// 9, 10, 11, 12, 13 - reserved for SPI (ethernet, SD card)

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
Error codes
*/
// error codes for self test
#define error_I2C_ping_DS3231 1
#define error_time_lost 2
#define error_critical_mask 0b1111111111111101 // time_lost is not critical

/*
EEPROM
*/
#define EEPROM_alarms_offset 10
#define EEPROM_AlarmClass_record_length (1 + 2 + 1 + 1 + 2 + 3)
#define EEPROM_alarms_identificator 0xFE