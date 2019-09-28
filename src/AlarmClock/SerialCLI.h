// SerialCLI.h

#ifndef _SERIALCLI_h
#define _SERIALCLI_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Settings.h"
#include "Constants.h"
#include "Alarm.h"

class SerialCLIClass
{
protected:
    typedef byte error_t;  // error codes are defined in Constants.h

    AlarmClass *_alarms;
    DateTime _now;
    RTC_DS3231 *_rtc;
    void(*_writeEEPROM)();
    const char _prompt_default[2 + 1] = "> ";
    char _Serial_buffer[Serial_buffer_length + 1]; // +1 for termination
    byte _Serial_buffer_index;
    char _prompt[Serial_prompt_length + 1];
    boolean _change = false; // for save
    unsigned long _previous_command_millis = 0; // for autosave

    const byte _selected_alarm_index_none = 255;
    byte _selected_alarm_index = _selected_alarm_index_none;


    // utils
    void _printHelp();
    byte _strbyte(char *str);
    char * _find_digit(char *str);
    char * _find_next_digit(char *str);
    void _indent(byte level);
    void _print_error(error_t error_code);


    // commands
    error_t _select_alarm(byte index);
    error_t _list_selected_alarm();
    error_t _set_enabled(AlarmEnabled __en);
    error_t _set_time(char *time);
    error_t _set_day_of_week(char *dow);
    error_t _set_snooze(char *snooze);
    error_t _set_signalization(char *sig);
    error_t _save(); // check if something changed, save if true
    error_t _rtc_time(char *time);
    error_t _rtc_date(char *date);
    error_t _rtc_get();

public:
    void loop(DateTime time);
    SerialCLIClass(AlarmClass *alarms, void(*__writeEEPROM)(), RTC_DS3231 *__rtc);
};

#endif
