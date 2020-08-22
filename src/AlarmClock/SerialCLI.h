/*!
    @file
*/

#ifndef SERIALCLI_H
#define SERIALCLI_H

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

/*!
    @brief  A command line interface provided on a Stream object used to
            configure the alarm clock.
*/
class SerialCLI
{
protected:
    typedef byte error_t;

    /*!
        @brief  Errors that can be returned by commands.
        The commands return a one-byte error_t value, individual bits indicate
        individual errors defined here.
    */
    enum CommandError
    {
        kOk = 0,
        kArgument = 1,
        kNothingSelected = 2,
        kUselessSave = 4,
    };

    struct command_t
    {
        const char* text;
        error_t (SerialCLI::*handler)(char* cmdptr);
    };

    static const command_t commands_[];

    Stream& ser_;

    Alarm *alarms_;
    DateTime now_;
    RTC_DS3231 *rtc_;
    void(*writeEEPROM_)();
    PWMDimmer * ambientDimmer_;
    HALbool * lamp_;
    void(*set_inhibit_)(bool);
    bool(*get_inhibit_)();

    static constexpr byte kSerial_buffer_length_ = 12;
    char Serial_buffer_[kSerial_buffer_length_ + 1]; // +1 for termination
    byte Serial_buffer_index_;

    static constexpr byte kSerial_prompt_length_ = 5;
    char prompt_[kSerial_prompt_length_ + 1];
    const char prompt_default_[2 + 1] = "> ";

    bool change_ = false; //! for save
    unsigned long prev_command_millis_ = 0; //! for autosave
    static constexpr byte sel_alarm_index_none_ = 255;
    byte sel_alarm_index_ = sel_alarm_index_none_;


    // utils
    void print_help_();
    byte strbyte_(char *str);
    char * find_digit_(char *str);
    char * find_next_digit_(char *str);
    void indent_(byte level);
    void print_error_(error_t error_code);


    // commands
    error_t cmd_sel_(char *index);
    error_t cmd_amb_(char *duty);
    error_t cmd_lamp_(char *status);
    error_t cmd_inh_(char *status);
    error_t cmd_en_(char *type);
    error_t cmd_time_(char *time);
    error_t cmd_dow_(char *dow);
    error_t cmd_snz_(char *snooze);
    error_t cmd_sig_(char *sig);
    error_t cmd_st_(char *time);
    error_t cmd_sd_(char *date);
    error_t cmd_sav_(char *ignored);
    error_t cmd_rtc_(char *ignored);
    error_t cmd_ls_(char *ignored);

    error_t select_alarm_(byte index);
    error_t set_enabled_(AlarmEnabled status);
    error_t save_(); //!< check if something changed, save if true

public:
    void loop(DateTime time);
    SerialCLI(Stream& ser,
              Alarm *alarms, void(*writeEEPROM)(), RTC_DS3231 *rtc,
              PWMDimmer *ambientDimmer, HALbool *lamp,
              void(*set_inhibit)(bool), bool(*get_inhibit)()) : ser_(ser)
{
    alarms_ = alarms;
    writeEEPROM_ = writeEEPROM;
    rtc_ = rtc;
    ambientDimmer_ = ambientDimmer;
    lamp_ = lamp;
    set_inhibit_ = set_inhibit;
    get_inhibit_ = get_inhibit;

    strcpy(prompt_, prompt_default_);
}

};

#endif
