// GUI.h

#ifndef _GUI_h
#define _GUI_h

#if defined(ARDUINO) && ARDUINO >= 100
    #include "arduino.h"
#else
    #include "WProgram.h"
#endif

#include "Settings.h"
#include "Constants.h"
#include "Alarm.h"
#include "PWMDimmer.h"
#include <Encoder.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>

// Must be contiguous
// Don't forget to update Screens_count when changing
enum Screen {
    screen_home = 0,
    screen_alarms = 1,
    screen_RTC = 2
    //screen_LAST  // Must be the last item in the enum. Not needed yet.
};
#define Screens_count 3

struct cursor_position_t {
    byte column;
    byte row;
};

enum cursor_position_home {
    cph_alarms_button = 0,
    cph_RTC_button = 1,
    cph_inhibit_button = 2,
    cph_ambient = 3,
    cph_lamp = 4
};

enum cursor_position_alarms {
    cpa_home_button = 0,
    cpa_alarm_index = 1,
    cpa_alarm_day_1 = 2,
    /*
    cpa_alarm_day_2 = 3,
    cpa_alarm_day_3 = 4,
    cpa_alarm_day_4 = 5,
    cpa_alarm_day_5 = 6,
    cpa_alarm_day_6 = 7,
    cpa_alarm_day_7 = 8,
    */
    cpa_alarm_enabled = 9,
    cpa_alarm_time_h = 10,
    cpa_alarm_time_m = 11,
    cpa_alarm_snz_time = 12,
    cpa_alarm_snz_count = 13,
    cpa_alarm_sig_a = 14,
    cpa_alarm_sig_l = 15,
    cpa_alarm_sig_b = 16
};

enum cursor_position_RTC {
    cpr_cancel_button = 0,
    cpr_apply_button = 1,
    cpr_time_h = 2,
    cpr_time_m = 3,
    cpr_time_s = 4,
    cpr_date_d = 5,
    cpr_date_m = 6,
    cpr_date_y = 7
};

enum backlight_t {
    off = 0,
    on = 1,
    permanent = 2
};


class GUIClass
{
 protected:
     AlarmClass *_alarms;
     DateTime _now;
     RTC_DS3231 *_rtc;
     void(*_writeEEPROM)();
     Encoder *_encoder;
     Bounce *_encoder_button;
     LiquidCrystal_I2C *_lcd;
     void(*_set_inhibit)(bool);
     bool(*_get_inhibit)();
     PWMDimmerClass * _ambientDimmer;
     void(*_lamp)(bool);
     bool(*_get_lamp)();

     byte _sel_alarm_index = 0;
     AlarmClass *_sel_alarm;  // set when switching alarms
     Screen _current_screen = screen_home;
     DateTime _RTC_set;
     backlight_t _backlight = on;  // LCD backlight is turned on in setup()
     unsigned long _update_prev_millis = 0;
     unsigned long _backlight_prev_millis = 0;
     unsigned long _encoder_prev_millis = 0;
     // It is faster and causes less flicker if you send the LCD complete
     // lines instead of multiple pieces.
     char _line_buffer[LCD_width + 1];  // +1 for null termination

     const byte _selectables_count[Screens_count] = { 5, 17, 8 };
    #define Selectables_count_max 17

     byte _cursor_position = 0;
     // _current_screen, _cursor_position
     // This array translates _current_screen and _cursor_position to
     // the display's coordinates
     const cursor_position_t _cursor_positions[Screens_count][Selectables_count_max] = {
        { {0,1}, {3,1}, {7,1}, {12,1}, {14,1} },

        { {0,0}, {1,0}, {5,0}, {6,0}, {7,0}, {8,0}, {9,0}, {10,0}, {11,0}, {13,0},
          {0,1}, {3,1}, {6,1}, {9,1}, {12,1}, {14,1}, {15,1} },

        { {0,0}, {1,0}, {8,0}, {11,0}, {14,0},
          {0,1}, {3,1}, {6,1} }
     };
     bool _cursor_clicked = false;
     bool _change = false;  // for EEPROM write

     /*
     Utility functions
     */
     // Apply limits for the rotary encoder. The limits are inclusive.
     // loop - allow the user to move from limit_high to limit_low
     byte _apply_limits(byte value, int step, byte limit_low, byte limit_high,
                        bool loop = false);

     // This does not handle setting variables such as _RTC_set_time
     void _switch_screen(Screen screen);


     void _update();


 public:
     void set_backlight(backlight_t status);
     backlight_t get_backlight() const { return _backlight; };
     void loop(DateTime time);
     GUIClass(AlarmClass *alarms, void(*writeEEPROM)(), RTC_DS3231 *rtc,
              Encoder *encoder, Bounce *encoder_button,
              LiquidCrystal_I2C *lcd,
              void(*set_inhibit)(bool), bool(*get_inhibit)(),
              PWMDimmerClass *ambientDimmer, void(*lamp)(bool),
              bool(*get_lamp)());
};

#endif

