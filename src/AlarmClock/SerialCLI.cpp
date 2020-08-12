#include "SerialCLI.h"


const SerialCLIClass::command_t SerialCLIClass::commands[] = {
    {"sel", &SerialCLIClass::_cmd_sel},
    {"amb", &SerialCLIClass::_cmd_amb},
    {"lamp", &SerialCLIClass::_cmd_lamp},
    {"inh", &SerialCLIClass::_cmd_inh},
    {"en-", &SerialCLIClass::_cmd_en},
    {"time", &SerialCLIClass::_cmd_time},
    {"dow", &SerialCLIClass::_cmd_dow},
    {"snz", &SerialCLIClass::_cmd_snz},
    {"sig", &SerialCLIClass::_cmd_sig},
    {"st", &SerialCLIClass::_cmd_st},
    {"sd", &SerialCLIClass::_cmd_sd},
    {"sav", &SerialCLIClass::_cmd_sav},
    {"rtc", &SerialCLIClass::_cmd_rtc},
    {"ls", &SerialCLIClass::_cmd_ls}
};


void SerialCLIClass::loop(DateTime time_)
{
    _now = time_;
    bool complete_message = false;

    while (ser.available() && !complete_message)
    {
        // !complete_message - this prevents the _Serial_buffer being rewritten
        // by new data when 2 messages are sent with very little delay (index
        // is set to 0 when complete_message is received).
        // I need to process the data before I receive the next message.

        _Serial_buffer[_Serial_buffer_index] = tolower(ser.read());

        if (_Serial_buffer[_Serial_buffer_index] == '\n' || _Serial_buffer[_Serial_buffer_index] == '\r')
        {
            //  CR/LF
            if (_Serial_buffer_index != 0)
            {
                // ignore if it is the first character (to avoid problems with CR+LF/LF)

                _Serial_buffer[_Serial_buffer_index] = '\0';  // rewrite CR/LF
                complete_message = true;
                _Serial_buffer_index = 0;
                ser.println();

#if defined(DEBUG) && defined(DEBUG_Serial_CLI)
                Serial.println();
                Serial.print(">> ");
                Serial.println(_Serial_buffer);

                char *ptr = &_Serial_buffer[0];
                while (*ptr != '\0')
                {
                    Serial.print(byte(*ptr), HEX);
                    Serial.print(' ');
                    ptr++;
                }
                Serial.println();
#endif // DEBUG
            }
        }
        else
        {
            // Character playback - this needs to be before index++ and should
            // not happen when the character is CR/LF
            ser.print(_Serial_buffer[_Serial_buffer_index]);

            if (_Serial_buffer_index < Serial_buffer_length - 1)
            {
                _Serial_buffer_index++;
            }
            else
            {
                ser.println();
                ser.print(F("Cmd too long: "));
                for (byte i = 0; i <= _Serial_buffer_index; i++)
                    ser.print(_Serial_buffer[i]);

                delay(40);  // to receive the rest of the message
                while (ser.available())
                    ser.print(char(ser.read()));

                ser.println();

                _Serial_buffer_index = 0;
                _Serial_buffer[0] = '\0';  // this is currently not necessary
            }
        }
    }


    if (complete_message)
    {
        DEBUG_println();
        DEBUG_println(F("Processing"));

        char *cmd_ptr = nullptr;
        bool cmd_found = false;

        for (byte i = 0; i < (sizeof(commands) / sizeof(command_t)); i++)
        {
            if ((cmd_ptr = strstr(_Serial_buffer, commands[i].text)) != nullptr)
            {
                _print_error((this->*(commands[i].handler))(cmd_ptr));
                cmd_found = true;
                break;  // do not process any further commands
            }
        }

        if (!cmd_found)
        {
            ser.println(F("? SYNTAX ERROR"));
            _print_help();
        }

        _prev_command_millis = millis();

        // Prompt will be missing after reboot, but this can't be easily fixed.
        ser.println();
        ser.print(_prompt);
    }

    // autosave
    if ((unsigned long)(millis() - _prev_command_millis) >= Serial_autosave_interval && _change)
    {
        ser.println();
        ser.println(F("Autosaving"));
        _print_error(_save());
    }
}


void SerialCLIClass::_print_help()
{
    ser.println();
    ser.println(F("Help:"));
    _indent(1);
    ser.println(F("amb - get ambient 0-255"));
    _indent(1);
    ser.println(F("amb{nnn} - ambient 0-255"));
    _indent(1);
    ser.println(F("lamp - get 0|1"));
    _indent(1);
    ser.println(F("lamp{l} - set 0|1"));
    _indent(1);
    ser.println(F("inh - get inhibit 0|1"));
    _indent(1);
    ser.println(F("inh{i} - set inhibit 0|1"));
    _indent(1);
    ser.println(F("sel{i} - select alarm{i}"));
    _indent(1);
    ser.println(F("Selected alarm:"));
    _indent(2);
    ser.println(F("ls - list"));
    _indent(2);
    ser.println(F("en-off/en-sgl/en-rpt/en-skp - enable - off/single/repeat/skip"));
    _indent(2);
    ser.println(F("time{h}:{m} - set time"));
    _indent(2);
    ser.println(F("dow{d}:{s} - set day {d}1-7 of week to {s}1|0"));
    _indent(2);
    ser.println(F("snz{t};{c} - set snooze: time{t}min;count{c}"));
    _indent(2);
    ser.println(F("sig{a};{l};{b} - set signalization: ambient{a}0-255;lamp{l}1|0;buzzer{b}1|0"));
    _indent(1);
    ser.println(F("sav - save all"));
    _indent(1);
    ser.println(F("RTC:"));
    _indent(2);
    ser.println(F("rtc - get RTC time"));
    _indent(2);
    ser.println(F("sd{dd}.{mm}.{yy} - set RTC date"));
    _indent(2);
    ser.println(F("st{h}:{m} - set RTC time"));
}


byte SerialCLIClass::_strbyte(char *str)
{
    byte result = 0;
    while (isDigit(*str))
    {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}


char * SerialCLIClass::_find_digit(char * str)
{
    while (!isDigit(*str) && *str != '\0') str++;
    return str;
}


char * SerialCLIClass::_find_next_digit(char * str)
{
    while (isDigit(*str) && *str != '\0') str++;
    str = _find_digit(str);
    return str;
}


void SerialCLIClass::_indent(byte level)
{
    for (byte i = 0; i < level * Serial_indentation_width; i++)
    {
        ser.print(' ');
    }
}


void SerialCLIClass::_print_error(error_t error_code)
{
    ser.println();
    ser.print(F("err "));
    ser.print(error_code);
    ser.print(F(": "));

    if (!error_code)
        ser.println(F("OK"));

    if (error_code & Serial_err_argument)
        ser.println(F("Invalid args"));

    if (error_code & Serial_err_select)
        ser.println(F("Sel first"));

    if (error_code & Serial_err_useless_save)
        ser.println(F("Nothing to save"));
}


SerialCLIClass::error_t SerialCLIClass::_cmd_sel(char *index)
{
    index = _find_digit(index);
    if (*index == '\0')
        return _select_alarm(_sel_alarm_index_none);

    byte index_num = _strbyte(index);
    return _select_alarm(index_num);
}


SerialCLIClass::error_t SerialCLIClass::_cmd_amb(char * duty)
{
    byte ambient;

    duty = _find_next_digit(duty);
    if (*duty == '\0')
    {
        ser.print(F("amb: "));
        ser.println(_ambientDimmer->get_value());
        return 0;
    }
    ambient = _strbyte(duty);
    _ambientDimmer->set_from_duration(_ambientDimmer->get_value(), ambient,
                                    Serial_ambient_dimming_duration);
    _ambientDimmer->start();
    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_lamp(char *status)
{
    status = _find_next_digit(status);
    if (*status == '\0')
    {
        ser.print(F("lamp: "));
        ser.println(_lamp->get());
        return 0;
    }
    _lamp->set_manu(_strbyte(status));
    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_inh(char *status)
{
    status = _find_next_digit(status);
    if (*status == '\0')
    {
        ser.print(F("inhibit: "));
        ser.println(_get_inhibit());
        return 0;
    }
    _set_inhibit(_strbyte(status));
    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_en(char *type)
{
    // ! - strcmp returns 0 if matches
    if (!strcmp(type, "en-off"))
        return _set_enabled(Off);

    if (!strcmp(type, "en-sgl"))
        return _set_enabled(Single);

    if (!strcmp(type, "en-rpt"))
        return _set_enabled(Repeat);

    if (!strcmp(type, "en-skp"))
        return _set_enabled(Skip);

    return Serial_err_argument;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_time(char *time)
{
    if (_sel_alarm_index == _sel_alarm_index_none) return Serial_err_select;

    byte hours, minutes;
    time = _find_next_digit(time);
    if (*time == '\0') return Serial_err_argument;
    hours = _strbyte(time);
    time = _find_next_digit(time);
    if (*time == '\0') return Serial_err_argument;
    minutes = _strbyte(time);

    if ((_alarms + _sel_alarm_index)->set_time(hours, minutes))
    {
        _change = true;
        return 0;
    }
    else return Serial_err_argument;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_dow(char *dow)
{
    if (_sel_alarm_index == _sel_alarm_index_none) return Serial_err_select;

    byte day;
    bool status;
    dow = _find_next_digit(dow);
    if (*dow == '\0') return Serial_err_argument;
    day = _strbyte(dow);
    dow = _find_next_digit(dow);
    if (*dow == '\0') return Serial_err_argument;
    status = _strbyte(dow);

    if ((_alarms + _sel_alarm_index)->set_day_of_week(day, status))
    {
        _change = true;
        return 0;
    }
    else return Serial_err_argument;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_snz(char *snooze)
{
    if (_sel_alarm_index == _sel_alarm_index_none) return Serial_err_select;

    byte time, count;

    snooze = _find_next_digit(snooze);
    if (*snooze == '\0') return Serial_err_argument;
    time = _strbyte(snooze);
    snooze = _find_next_digit(snooze);
    if (*snooze == '\0') return Serial_err_argument;
    count = _strbyte(snooze);

    if ((_alarms + _sel_alarm_index)->set_snooze(time, count))
    {
        _change = true;
        return 0;
    }
    else return Serial_err_argument;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_sig(char *sig)
{
    if (_sel_alarm_index == _sel_alarm_index_none) return Serial_err_select;

    byte ambient;
    bool lamp, buzzer;

    sig = _find_next_digit(sig);
    if (*sig == '\0') return Serial_err_argument;
    ambient = _strbyte(sig);
    sig = _find_next_digit(sig);
    if (*sig == '\0') return Serial_err_argument;
    lamp = _strbyte(sig);
    sig = _find_next_digit(sig);
    if (*sig == '\0') return Serial_err_argument;
    buzzer = _strbyte(sig);

    if ((_alarms + _sel_alarm_index)->set_signalization(ambient, lamp, buzzer))
    {
        _change = true;
        return 0;
    }
    else return Serial_err_argument;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_st(char *time)
{
    byte hour, minute;
    time = _find_next_digit(time);
    if (*time == '\0') return Serial_err_argument;
    hour = _strbyte(time);
    time = _find_next_digit(time);
    if (*time == '\0') return Serial_err_argument;
    minute = _strbyte(time);

    if (hour > 23 || minute > 59) return Serial_err_argument;


    _now = _rtc->now();
    _rtc->adjust(DateTime(_now.year(), _now.month(), _now.day(), hour, minute));
    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_sav(char *ignored)
{
    (void)ignored;
    return _save();
}


SerialCLIClass::error_t SerialCLIClass::_cmd_rtc(char *ignored)
{
    (void)ignored;

    ser.print(F("Time: "));
    if (_now.dayOfTheWeek() == 0) ser.print(days_of_the_week_names_short[7]);
    else ser.print(days_of_the_week_names_short[_now.dayOfTheWeek()]);
    ser.print(' ');
    ser.print(_now.day());
    ser.print(". ");
    ser.print(_now.month());
    ser.print(". ");
    ser.print(_now.year());
    ser.print("  ");
    ser.print(_now.hour());
    ser.print(':');
    ser.print(_now.minute());
    ser.print(':');
    ser.println(_now.second());

    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_ls(char *ignored)
{
    (void)ignored;

    if (_sel_alarm_index == _sel_alarm_index_none)
        return Serial_err_select;

    ser.print(F("Num: "));
    ser.println(_sel_alarm_index);

    _indent(1);
    ser.print(F("Enabled: "));
    switch ((_alarms + _sel_alarm_index)->get_enabled())
    {
    case Off:
        ser.println(F("Off"));
        break;

    case Single:
        ser.println(F("Single"));
        break;

    case Repeat:
        ser.println(F("Repeat"));
        break;

    case Skip:
        ser.println(F("Skip"));
        break;
    }

    _indent(1);
    ser.print(F("Days of week: "));
    for (byte i = 1; i <= 7; i++)
    {
        if ((_alarms + _sel_alarm_index)->get_days_of_week().getDayOfWeek(i))
        {
            ser.print(days_of_the_week_names_short[i]);
            ser.print(' ');
        }
        else ser.print("   ");
    }
    ser.println();

    _indent(1);
    ser.print(F("Time: "));
    ser.print((_alarms + _sel_alarm_index)->get_time().hours);
    ser.print(":");
    ser.println((_alarms + _sel_alarm_index)->get_time().minutes);

    _indent(1);
    ser.println(F("Snooze: "));
    _indent(2);
    ser.print(F("Time: "));
    ser.print((_alarms + _sel_alarm_index)->get_snooze().time_minutes);
    ser.println(F(" min"));
    _indent(2);
    ser.print(F("Count: "));
    ser.println((_alarms + _sel_alarm_index)->get_snooze().count);

    _indent(1);
    ser.println(F("Signalization: "));
    _indent(2);
    ser.print(F("Ambient: "));
    ser.println((_alarms + _sel_alarm_index)->get_signalization().ambient);
    _indent(2);
    ser.print(F("Lamp: "));
    ser.println((_alarms + _sel_alarm_index)->get_signalization().lamp);
    _indent(2);
    ser.print(F("Buzzer: "));
    ser.println((_alarms + _sel_alarm_index)->get_signalization().buzzer);

    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_cmd_sd(char *date)
{
    int year;
    byte month, day;

    date = _find_next_digit(date);
    if (*date == '\0') return Serial_err_argument;
    day = _strbyte(date);
    date = _find_next_digit(date);
    if (*date == '\0') return Serial_err_argument;
    month = _strbyte(date);
    date = _find_next_digit(date);
    if (*date == '\0') return Serial_err_argument;
    year = _strbyte(date);

    if (month > 12 || day > 31) return Serial_err_argument;

    _now = _rtc->now();
    DateTime new_date(year, month, day, _now.hour(), _now.minute());
    if (!new_date.isValid()) return Serial_err_argument;
    _rtc->adjust(new_date);
    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_select_alarm(byte index_)
{
    if (index_ >= alarms_count && index_ != _sel_alarm_index_none)
        return Serial_err_argument;

    _sel_alarm_index = index_;
    if (_sel_alarm_index == _sel_alarm_index_none) strcpy(_prompt, _prompt_default);
    else sprintf(_prompt, "A%u%s", _sel_alarm_index, _prompt_default);

    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_set_enabled(AlarmEnabled status)
{
    if (_sel_alarm_index == _sel_alarm_index_none) return Serial_err_select;

    (_alarms + _sel_alarm_index)->set_enabled(status);

    _change = true;
    return 0;
}


SerialCLIClass::error_t SerialCLIClass::_save()
{
    if (_change)
    {
        _change = false;
        _writeEEPROM();
        return 0;
    }
    else return Serial_err_useless_save;
}
