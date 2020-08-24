/*!
    @file
*/

#ifndef ALARMCLOCKCLI_H
#define ALARMCLOCKCLI_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Settings.h"
#include "Constants.h"
#include "SerialCLI.h"
#include "Alarm.h"
#include "PWMDimmer.h"
#include "HALbool.h"


/*!
    @brief  A command line interface for configuring the alarm.
    Only one instance should exist at a time, because a lot of stuff needs to
    be static in order to pass pointers to the commands as function pointers.
*/
class AlarmClockCLI
{
public:
    //! call me in your loop()
    void loop(DateTime now);


    /*!
        Warning: only one instance of this class should exist at a time.
        @param ser  Reference to the Stream object the CLI should be provided on.
        @param alarms   Pointer to the alarms array.
        @param rtc      Pointer to the RTC object. (Used for adjusting time.)
        @param writeEEPROM  Pointer to a function that writes all alarms to
                            the EEPROM.
        @param ambientDimmer    Pointer to the ambientDimmer object.
        @param lamp     Pointer to the lamp object.
        @param set_inhibit  Pointer to a function that sets the status of the
                            inhibit function.
        @param get_inhibit  Pointer to a function that gets the status of the
                            inhibit function.
    */
    AlarmClockCLI(Stream& ser,
                  Alarm* alarms, RTC_DS3231* rtc, void(*writeEEPROM)(),
                  PWMDimmer* ambientDimmer, HALbool* lamp,
                  void(*set_inhibit)(bool), bool(*get_inhibit)()
                 ) : CLI_(ser, commands, command_count, print_error,
                          cmd_not_found, prompt_)
    {
        ser_ = &ser;
        alarms_ = alarms;
        rtc_ = rtc;
        writeEEPROM_ = writeEEPROM;
        ambientDimmer_ = ambientDimmer;
        lamp_ = lamp;
        set_inhibit_ = set_inhibit;
        get_inhibit_ = get_inhibit;

        strcpy(prompt_, prompt_default_);
    };

protected:
    /*!
        @brief  Errors that can be returned by commands.
        The commands return a one-byte error_t value, individual bits indicate
        individual errors defined here.
        To return multiple errors, use bitwise OR:
        `return kArgument | kNothingSelected;`
        These values must be powers of 2.
    */
    enum CommandError
    {
        kOk = 0,
        kArgument = 1,
        kNothingSelected = 2,
        kUselessSave = 4,
        kLast = 8, //!< last; all errors are lower than this
    };

    //! Error strings corresponding to errors in CommandError
    static const char* error_strings[];

    static Stream* ser_;
    static const byte command_count;
    static const SerialCLI::command_t commands[];
    static void print_error(SerialCLI::error_t code);
    static void cmd_not_found();

    static Alarm* alarms_;
    static DateTime now_;
    static RTC_DS3231* rtc_;
    static void(*writeEEPROM_)();
    static PWMDimmer* ambientDimmer_;
    static HALbool* lamp_;
    static void(*set_inhibit_)(bool);
    static bool(*get_inhibit_)();

    SerialCLI CLI_;


    static bool change_; //!< for save
    static constexpr byte sel_alarm_index_none_ = 255;
    static byte sel_alarm_index_;

    static constexpr byte kSerial_prompt_length_ = 5;
    static constexpr char prompt_default_[2 + 1] = "> ";
    static char prompt_[kSerial_prompt_length_];


    // utils
    static byte strbyte_(const char* str);
    static char * find_digit_(char* str);
    static char * find_next_digit_(char* str);
    static void indent_(byte level);


    // commands
    static SerialCLI::error_t cmd_sel_(char *index);
    static SerialCLI::error_t cmd_amb_(char *duty);
    static SerialCLI::error_t cmd_lamp_(char *status);
    static SerialCLI::error_t cmd_inh_(char *status);
    static SerialCLI::error_t cmd_en_(char *type);
    static SerialCLI::error_t cmd_time_(char *time);
    static SerialCLI::error_t cmd_dow_(char *dow);
    static SerialCLI::error_t cmd_snz_(char *snooze);
    static SerialCLI::error_t cmd_sig_(char *sig);
    static SerialCLI::error_t cmd_st_(char *time);
    static SerialCLI::error_t cmd_sd_(char *date);
    static SerialCLI::error_t cmd_sav_(char *ignored);
    static SerialCLI::error_t cmd_rtc_(char *ignored);
    static SerialCLI::error_t cmd_ls_(char *ignored);

    static SerialCLI::error_t select_alarm_(byte index);
    static SerialCLI::error_t set_enabled_(AlarmEnabled status);
    static SerialCLI::error_t save_(); //!< check if something changed, save if true

};

#endif
