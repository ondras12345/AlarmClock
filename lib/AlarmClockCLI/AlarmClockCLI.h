/*!
    @file
*/

#ifndef ALARMCLOCKCLI_H
#define ALARMCLOCKCLI_H

#include <Arduino.h>

#include "Settings.h"
#include "Constants.h"
#include <SerialCLI.h>
#include <Alarm.h>
#include <PWMDimmer.h>
#include <HALbool.h>
#include <CountdownTimer.h>
#include <PWMSine.h>


/*!
    @brief  A command line interface for configuring the alarm clock.
    Only one instance should exist at a time, because a lot of stuff
    needs to be static in order to be able to pass commands as
    function pointers.

    Any program parsing this CLI needs to support both LF and CRLF line
    endings. In incoming data (i.e. commands), CR, LF or any combination of
    them can be used to end a message.

    The CLI aims to be easily parsable. Any entry except for an empty one
    ("\r\n" is considered empty, but " \r\n" is not) results in an error
    message that matches this regex: "^err 0x[0-9]{,2}: .*$". Before this
    message is the command's output, if any.

    Because DEBUG stuff is printed to the same serial port, it is necessary to
    mark the command output's beginning and end. For commands with YAML output,
    this is done with `YAML_begin` and `YAML_end` macros from Settings.h
    (should match "^---$" and "^\.\.\.$", but may contain CR at the end).

    No command shall be sent unless a prompt matching "^A?[0-9]{,3}> " was
    received. Note that anything can get appended to the prompt, DEBUG messages
    are a common example of this behavior.
*/
class AlarmClockCLI
{
public:
    //! call me in your loop()
    void loop(const DateTime& now);


    /*!
        Warning: only one instance of this class should exist at a time.
        @param ser  Reference to the Stream object the CLI should be provided on.
        @param alarms   The alarms array.
        @param rtc      Pointer to the RTC object. (Used for adjusting time.)
        @param writeEEPROM  Pointer to a function that writes all alarms to
                            the EEPROM.
        @param ambientDimmer    Pointer to the ambientDimmer object.
        @param sine Reference to the PWMSine object that is used to produce
                    sound.
        @param buzzer   Reference to a BuzzerManager object capable of playing
                        melodies.
        @param lamp     Pointer to the lamp object.
        @param timer    Pointer to the timer object.
        @param set_inhibit  Pointer to a function that sets the status of the
                            inhibit function.
        @param get_inhibit  Pointer to a function that gets the status of the
                            inhibit function.
    */
    AlarmClockCLI(Stream& ser,
                  Alarm* alarms[alarms_count], RTC_DS3231* rtc,
                  void(*writeEEPROM)(), PWMDimmer* ambientDimmer,
                  HALbool* lamp, CountdownTimer* timer, PWMSine& sine,
                  BuzzerManager& buzzer, void(*set_inhibit)(bool),
                  bool(*get_inhibit)()
                 ) : CLI_(ser, commands, command_count, print_error,
                          cmd_not_found, prompt_)
    {
        ser_ = &ser;
        alarms_ = alarms;
        rtc_ = rtc;
        writeEEPROM_ = writeEEPROM;
        ambientDimmer_ = ambientDimmer;
        lamp_ = lamp;
        timer_ = timer;
        sine_ = &sine;
        buzzer_ = &buzzer;
        set_inhibit_ = set_inhibit;
        get_inhibit_ = get_inhibit;

        strcpy(prompt_, prompt_default_);
    };

    void notify_alarms_changed();
    void report_display_backlight(uint8_t);

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
        kNotFound = 8,
        kUnsupported = 16,  //!< command not supported in this build
        kLast = 32,  //!< last; all errors are lower than this
    };

    void notify_change_();

    unsigned long last_BEL_change_ = 0;
    bool BEL_change_ = false;
    /// Set to true by notify_alarms changed and back to false by cmd_la_.
    /// Used to prevent unnecessary cmd_la_ after other state changes.
    /// Defaults to true.
    static bool alarms_changed_;
    static uint8_t display_backlight_status_;

    //! Error strings corresponding to errors in CommandError
    static const char* error_strings[];

    static Stream* ser_;
    static const byte command_count;
    static const SerialCLI::command_t commands[];
    static void print_error(SerialCLI::error_t code);
    static void cmd_not_found();

    static Alarm** alarms_;
    static DateTime now_;
    static RTC_DS3231* rtc_;
    static void(*writeEEPROM_)();
    static PWMDimmer* ambientDimmer_;
    static HALbool* lamp_;
    static CountdownTimer* timer_;
    static PWMSine* sine_;
    static BuzzerManager* buzzer_;
    static void(*set_inhibit_)(bool);
    static bool(*get_inhibit_)();

    //! This keeps BuzzerManager's on_count_ correct.
    static bool buzzer_playing_;

    SerialCLI CLI_;


    static bool change_; //!< for save
    static constexpr byte sel_alarm_index_none_ = 255;
    static byte sel_alarm_index_;

    static constexpr byte kSerial_prompt_length_ = 5;
    static constexpr char prompt_default_[2 + 1] = "> ";
    static char prompt_[kSerial_prompt_length_];


    // utils
    static byte strbyte_(const char* str);
    static uint16_t struint16_(const char* str);
    static char * find_digit_(char* str);
    static char * find_next_digit_(char* str);
    static void yaml_time_(byte hours, byte minutes);
    static void yaml_time_(byte hours, byte minutes, byte seconds);
    static void yaml_time_(HoursMinutes time);
    static void yaml_alarm_(byte index, bool comments);
    static void yaml_timer_();
    static void yaml_ambient_();
    static void yaml_lamp_();
    static void yaml_inhibit_();


    // commands
    static SerialCLI::error_t cmd_sync_(char *ignored);
    static SerialCLI::error_t cmd_sel_(char *index);
    static SerialCLI::error_t cmd_amb_(char *duty);
    static SerialCLI::error_t cmd_lamp_(char *status);
    static SerialCLI::error_t cmd_inh_(char *status);
    static SerialCLI::error_t cmd_en_(char *type);
    static SerialCLI::error_t cmd_time_(char *time);
    static SerialCLI::error_t cmd_dow_(char *dow);
    static SerialCLI::error_t cmd_snz_(char *snooze);
    static SerialCLI::error_t cmd_sig_(char *sig);
    static SerialCLI::error_t cmd_stop_(char *ignored);
    static SerialCLI::error_t cmd_st_(char *time);
    static SerialCLI::error_t cmd_sd_(char *date);
    static SerialCLI::error_t cmd_tmr_(char *time);
    static SerialCLI::error_t cmd_tme_(char *events);
    static SerialCLI::error_t cmd_sav_(char *ignored);
    static SerialCLI::error_t cmd_rtc_(char *ignored);
    static SerialCLI::error_t cmd_ls_(char *ignored);
    static SerialCLI::error_t cmd_la_(char *ignored);
    static SerialCLI::error_t cmd_ver_(char *ignored);
    static SerialCLI::error_t cmd_tone_(char *args);
    static SerialCLI::error_t cmd_silence_(char *ignored);
    static SerialCLI::error_t cmd_notone_(char *ignored);
    static SerialCLI::error_t cmd_melody_(char *id);
    static SerialCLI::error_t cmd_eer_(char *args);
    static SerialCLI::error_t cmd_eew_(char *args);
    static SerialCLI::error_t cmd_status_(char *ignored);

    static SerialCLI::error_t select_alarm_(byte index);
    static SerialCLI::error_t set_enabled_(AlarmEnabled status);
    static SerialCLI::error_t save_(); //!< check if something changed, save if true

};

#endif
