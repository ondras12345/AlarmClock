/*!
    @file
*/

#ifndef GUI_H
#define GUI_H

#if defined(ARDUINO) && ARDUINO >= 100
    #include "arduino.h"
#else
    #include "WProgram.h"
#endif

#include "Settings.h"
#include "Constants.h"
#include "Alarm.h"
#include "PWMDimmer.h"
#include "HALbool.h"
#include <Encoder.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>

// Must be contiguous
// Don't forget to update Screens_count when changing
enum Screen
{
    screen_home = 0,
    screen_alarms = 1,
    screen_RTC = 2
    //screen_LAST  // Must be the last item in the enum. Not needed yet.
};
#define Screens_count 3

struct cursor_position_t
{
    byte column;
    byte row;
};

enum cursor_position_home
{
    cph_alarms_button = 0,
    cph_RTC_button = 1,
    cph_inhibit_button = 2,
    cph_ambient = 3,
    cph_lamp = 4
};

enum cursor_position_alarms
{
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

enum cursor_position_RTC
{
    cpr_cancel_button = 0,
    cpr_apply_button = 1,
    cpr_time_h = 2,
    cpr_time_m = 3,
    cpr_time_s = 4,
    cpr_date_y = 5,
    cpr_date_m = 6,
    cpr_date_d = 7
};

enum backlight_t
{
    off = 0,
    on = 1,
    permanent = 2
};


class GUI
{
protected:
    Alarm* alarms_;
    void(&writeEEPROM_)();
    RTC_DS3231& rtc_;
    Encoder& encoder_;
    Bounce& encoder_button_;
    LiquidCrystal_I2C& lcd_;
    void(&set_inhibit_)(bool);
    bool(&get_inhibit_)();
    PWMDimmer& ambientDimmer_;
    HALbool& lamp_;

    DateTime now_;

    byte sel_alarm_index_ = 0;
    Alarm* sel_alarm_;  // set when switching alarms
    Screen current_screen_ = screen_home;
    DateTime RTC_set_;
    backlight_t backlight_ = on;  // LCD backlight is turned on in setup()
    unsigned long update_prev_millis_ = 0;
    unsigned long backlight_prev_millis_ = 0;
    unsigned long encoder_prev_millis_ = 0;
    // It is faster and causes less flicker if you send the LCD complete
    // lines instead of multiple pieces.
    char line_buffer_[LCD_width + 1];  // +1 for null termination

    const byte selectables_count_[Screens_count] = { 5, 17, 8 };
    static constexpr byte selectables_count_max = 17;

    byte cursor_position_ = 0;
    // current_screen_, cursor_position_
    // This array translates current_screen_ and cursor_position_ to
    // the display's coordinates
    const cursor_position_t cursor_positions_[Screens_count][selectables_count_max] = {
       { {0,1}, {3,1}, {7,1}, {12,1}, {14,1} },

       { {0,0}, {1,0}, {5,0}, {6,0}, {7,0}, {8,0}, {9,0}, {10,0}, {11,0}, {13,0},
         {0,1}, {3,1}, {6,1}, {9,1}, {12,1}, {14,1}, {15,1} },

       { {0,0}, {1,0}, {8,0}, {11,0}, {14,0},
         {0,1}, {5,1}, {8,1} }
    };
    bool cursor_clicked_ = false;
    bool change_ = false;  // for EEPROM write

    /*
    Utility functions
    */
    // Apply limits for the rotary encoder. The limits are inclusive.
    // loop - allow the user to move from limit_high to limit_low
    byte apply_limits_(byte value, int step, byte limit_low, byte limit_high,
                       bool loop = false);

    // This does not handle setting variables such as RTC_set_time_
    void switch_screen_(Screen screen);


    void update_();


public:
    void set_backlight(backlight_t status);
    backlight_t get_backlight() const { return backlight_; };
    void loop(DateTime time);
    GUI(Alarm* alarms, void(&writeEEPROM)(), RTC_DS3231& rtc,
        Encoder& encoder, Bounce& encoder_button,
        LiquidCrystal_I2C& lcd,
        void(&set_inhibit)(bool), bool(&get_inhibit)(),
        PWMDimmer& ambientDimmer, HALbool& lamp
        ):                     alarms_(alarms),
                               writeEEPROM_(writeEEPROM),
                               rtc_(rtc),
                               encoder_(encoder),
                               encoder_button_(encoder_button),
                               lcd_(lcd),
                               set_inhibit_(set_inhibit),
                               get_inhibit_(get_inhibit),
                               ambientDimmer_(ambientDimmer),
                               lamp_(lamp)
   {
       // First alarm.
       // I can't initialize it in the definition because the compiler doesn't
       // know the address yet.
       sel_alarm_ = alarms_;
   };
};

#endif

