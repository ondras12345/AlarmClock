/*!
    @file
*/

#ifndef GUI_H
#define GUI_H

#include <Arduino.h>

#include "Settings.h"
#include "Constants.h"
#include <Alarm.h>
#include <PWMDimmer.h>
#include <HALbool.h>
#include <CountdownTimer.h>
#include <Encoder.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>


/*!
    @brief  A single-purpose GUI. It can only be used with the alarm clock.

    It handles its own LCD, rotary encoder and button.
*/
class GUI
{
public:
    //! Enumeration of all possible states of LCD backlight
    enum backlight_t
    {
        off = 0,        //!< off
        on = 1,         //!< on, will be turned off after GUI_backlight_timeout
        permanent = 2   //!< on, will NOT time out
    };


    void set_backlight(backlight_t status);
    backlight_t get_backlight() const { return backlight_; };
    void loop(const DateTime& time);

    // cppcheck-suppress uninitMemberVar symbolName=GUI::line_buffer_
    GUI(Alarm* alarms, void(&writeEEPROM)(), RTC_DS3231& rtc,
        Encoder& encoder, Bounce& encoder_button,
        LiquidCrystal_I2C& lcd,
        void(&set_inhibit)(bool), bool(&get_inhibit)(),
        PWMDimmer& ambientDimmer, HALbool& lamp,
        CountdownTimer& timer): alarms_(alarms),
                               writeEEPROM_(writeEEPROM),
                               rtc_(rtc),
                               encoder_(encoder),
                               encoder_button_(encoder_button),
                               lcd_(lcd),
                               set_inhibit_(set_inhibit),
                               get_inhibit_(get_inhibit),
                               ambientDimmer_(ambientDimmer),
                               lamp_(lamp),
                               timer_(timer)
   {
       // First alarm.
       // I can't initialize it in the definition because the compiler doesn't
       // know the address yet.
       sel_alarm_ = alarms_;
   }


protected:
    /*!
        @brief  Enum of all screens.

        Must be contiguous.
    */
    enum Screen
    {
        screen_home = 0,
        screen_alarms = 1,
        screen_timer = 2,
        screen_RTC = 3
        //screen_LAST  // Must be the last item in the enum. Not needed yet.
    };

    //! Position of the cursor on the screen
    struct cursor_position_t
    {
        byte column;
        byte row;
    };

    enum cursor_position_home
    {
        cph_alarms_button = 0,
        cph_timer_button = 1,
        cph_RTC_button = 2,
        cph_inhibit_button = 3,
        cph_ambient = 4,
        cph_lamp = 5
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

    enum cursor_position_timer
    {
        cpt_home_button = 0,
        cpt_start_stop_button = 1,
        cpt_hh = 2,
        cpt_mm= 3,
        cpt_ss= 4,
        cpt_sig_a = 5,
        cpt_sig_l = 6,
        cpt_sig_b = 7
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
    CountdownTimer& timer_;

    byte sel_alarm_index_ = 0;
    Alarm* sel_alarm_;  //!< set when switching alarms
    Screen current_screen_ = screen_home;
    DateTime RTC_set_;
    backlight_t backlight_ = on;  //!< LCD backlight is turned on in setup()
    unsigned long update_prev_millis_ = 0;
    unsigned long backlight_prev_millis_ = 0;
    unsigned long encoder_prev_millis_ = 0;
    //! It is faster and causes less flicker if you send the LCD complete
    //! lines instead of multiple pieces.
    char line_buffer_[LCD_width + 1];  // +1 for null termination

    byte cursor_position_ = 0;

    static constexpr cursor_position_t curspos_home[] = {
        // the first line is empty
        {0,1}, {1,1}, {3,1}, {7,1}, {12,1}, {14,1}
    };

    static constexpr cursor_position_t curspos_alarms[] = {
        {0,0}, {1,0}, {5,0}, {6,0}, {7,0}, {8,0}, {9,0}, {10,0}, {11,0}, {13,0},
        {0,1}, {3,1}, {6,1}, {9,1}, {12,1}, {14,1}, {15,1}
    };

    static constexpr cursor_position_t curspos_timer[] = {
        // the first line is empty
        {0,1}, {1,1}, {3,1}, {6,1}, {9,1}, {12,1}, {14,1}, {15,1}
    };

    static constexpr cursor_position_t curspos_RTC[] = {
        {0,0}, {1,0}, {8,0}, {11,0}, {14,0},
        {0,1}, {5,1}, {8,1}
    };

    //! current_screen_, cursor_position_
    //! This array translates current_screen_ and cursor_position_ to
    //! the display's coordinates
    static constexpr const cursor_position_t * cursor_positions_[] = {
        curspos_home,
        curspos_alarms,
        curspos_timer,
        curspos_RTC
    };

    //! Number of selectable items for each screen.
    static constexpr byte selectables_count_[] = {
        sizeof(curspos_home) / sizeof(curspos_home[0]),
        sizeof(curspos_alarms) / sizeof(curspos_alarms[0]),
        sizeof(curspos_timer) / sizeof(curspos_timer[0]),
        sizeof(curspos_RTC) / sizeof(curspos_RTC[0]),
    };


    bool cursor_clicked_ = false;
    bool change_ = false;  //!< for EEPROM write

    byte apply_limits_(byte value, int step, byte limit_low, byte limit_high,
                       bool loop = false);

    //! This does not handle setting variables such as RTC_set_time_
    void switch_screen_(Screen screen);

    void update_(const DateTime& now);
};

#endif

