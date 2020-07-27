// SerialCLI.h

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

class SerialCLIClass
{
protected:
    typedef byte error_t;  // error codes are defined in Constants.h

    struct command_t
    {
        const char* text;
        error_t (SerialCLIClass::*handler)(char* cmdptr);
    };


    static const command_t commands[];

    AlarmClass *_alarms;
    DateTime _now;
    RTC_DS3231 *_rtc;
    void(*_writeEEPROM)();
    PWMDimmerClass * _ambientDimmer;
    HALbool * _lamp;
    void(*_set_inhibit)(bool);
    bool(*_get_inhibit)();

    const char _prompt_default[2 + 1] = "> ";
    char _Serial_buffer[Serial_buffer_length + 1]; // +1 for termination
    byte _Serial_buffer_index;
    char _prompt[Serial_prompt_length + 1];
    bool _change = false; // for save
    unsigned long _prev_command_millis = 0; // for autosave

    const byte _sel_alarm_index_none = 255;
    byte _sel_alarm_index = _sel_alarm_index_none;


    // utils
    void _print_help();
    byte _strbyte(char *str);
    char * _find_digit(char *str);
    char * _find_next_digit(char *str);
    void _indent(byte level);
    void _print_error(error_t error_code);


    // commands
    error_t _cmd_sel(char *index);
    error_t _cmd_amb(char *duty);
    error_t _cmd_lamp(char *status);
    error_t _cmd_inh(char *status);
    error_t _cmd_en(char *type);
    error_t _cmd_time(char *time);
    error_t _cmd_dow(char *dow);
    error_t _cmd_snz(char *snooze);
    error_t _cmd_sig(char *sig);
    error_t _cmd_st(char *time);
    error_t _cmd_sd(char *date);
    error_t _cmd_sav(char *ignored);
    error_t _cmd_rtc(char *ignored);
    error_t _cmd_ls(char *ignored);

    error_t _select_alarm(byte index);
    error_t _set_enabled(AlarmEnabled status);
    error_t _save(); // check if something changed, save if true

public:
    void loop(DateTime time);
    SerialCLIClass(AlarmClass *alarms, void(*writeEEPROM)(), RTC_DS3231 *rtc,
                   PWMDimmerClass *ambientDimmer, HALbool *lamp,
                   void(*set_inhibit)(bool), bool(*get_inhibit)());
};

#endif
