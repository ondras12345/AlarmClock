// 
// 
// 

#include "SerialCLI.h"

void SerialCLIClass::loop(DateTime __time)
{
    _now = __time;
    boolean complete_message = false;

    while (Serial.available() && !complete_message) {
        // !complete_message - this prevents the _Serial_buffer being rewritten
        // by new data when 2 messages are sent with very little delay (index
        // is set to 0 when complete_message is received).
        // I need to process the data before I receive the next message.

        _Serial_buffer[_Serial_buffer_index] = tolower(Serial.read());

        if (_Serial_buffer[_Serial_buffer_index] == '\n' || _Serial_buffer[_Serial_buffer_index] == '\r') {
            //  CR/LF
            if (_Serial_buffer_index != 0) {
                // ignore if it is the first character (to avoid problems with CR+LF/LF)

                _Serial_buffer[_Serial_buffer_index] = '\0';  // rewrite CR/LF
                complete_message = true;
                _Serial_buffer_index = 0;
                Serial.println();

#if defined(DEBUG) && defined(DEBUG_Serial_CLI)
                Serial.println();
                Serial.print(">> ");
                Serial.println(_Serial_buffer);

                char *ptr = &_Serial_buffer[0];
                while (*ptr != '\0') {
                    Serial.print(byte(*ptr), HEX);
                    Serial.print(' ');
                    ptr++;
                }
                Serial.println();
#endif // DEBUG
            }
        }
        else {
            // Character playback - this needs to be before index++ and should
            // not happen when the character is CR/LF
            Serial.print(_Serial_buffer[_Serial_buffer_index]);

            if (_Serial_buffer_index < Serial_buffer_length - 1) {
                _Serial_buffer_index++;
            }
            else {
                Serial.println();
                Serial.print(F("Cmd too long: "));
                for (byte i = 0; i <= _Serial_buffer_index; i++)
                    Serial.print(_Serial_buffer[i]);

                delay(40);  // to receive the rest of the message
                while (Serial.available())
                    Serial.print(char(Serial.read()));

                Serial.println();

                _Serial_buffer_index = 0;
                _Serial_buffer[0] = '\0';  // this is currently not necessary
            }
        }
    }


    if (complete_message) {
        DEBUG_println();
        DEBUG_println(F("Processing"));
        if (!strcmp(_Serial_buffer, "help")) { // ! - strcmp returns 0 if matches
            _printHelp();
        }
        else if (strstr(_Serial_buffer, "sel") != NULL) {
            char *index = strstr(_Serial_buffer, "sel");
            index = _find_digit(index);
            if (*index == '\0') _select_alarm(_selected_alarm_index_none);
            else {
                byte index_num = _strbyte(index);
                if (!_select_alarm(index_num)) {
                    Serial.print(F("Invalid i: "));
                    Serial.println(index_num);
                }
            }

        }
        else if (!strcmp(_Serial_buffer, "ls") || !strcmp(_Serial_buffer, "list")) {
            if (!_list_selected_alarm()) Serial.println(F("Sel first"));
        }
        else if (!strcmp(_Serial_buffer, "en-sgl")) {
            if (!_set_enabled(Single)) Serial.println(F("Sel first"));
        }
        else if (!strcmp(_Serial_buffer, "en-rpt")) {
            if (!_set_enabled(Repeat)) Serial.println(F("Sel first"));
        }
        else if (!strcmp(_Serial_buffer, "dis") || !strcmp(_Serial_buffer, "disable")) {
            if (!_set_enabled(Off)) Serial.println(F("Sel first"));
        }
        else if (strstr(_Serial_buffer, "time") != NULL) {
            char *time = strstr(_Serial_buffer, "time");
            if (!_set_time(time)) {
                Serial.println(F("Sel first"));
                Serial.println(F("Enter valid params"));
            }
        }
        else if (strstr(_Serial_buffer, "dow") != NULL) {
            char *dow = strstr(_Serial_buffer, "dow");
            if (!_set_day_of_week(dow)) {
                Serial.println(F("Sel first"));
                Serial.println(F("Enter valid params"));
            }
        }
        else if (strstr(_Serial_buffer, "snz") != NULL) {
            char *snooze = strstr(_Serial_buffer, "snz");
            if (!_set_snooze(snooze)) {
                Serial.println(F("Sel first"));
                Serial.println(F("Enter valid params"));
            }
        }
        else if (strstr(_Serial_buffer, "sig") != NULL) {
            char *sig = strstr(_Serial_buffer, "sig");
            if (!_set_signalization(sig)) {
                Serial.println(F("Sel first"));
                Serial.println(F("Enter valid params"));
            }
        }
        else if (strstr(_Serial_buffer, "sav") != NULL) {
            Serial.println(_save() ? F("Saved") : F("Nothing to save"));
        }
        else if (strstr(_Serial_buffer, "rtc") != NULL) {
            _rtc_get();
        }
        else if (strstr(_Serial_buffer, "sd") != NULL) {
            char *date = strstr(_Serial_buffer, "sd");
            if (!_rtc_date(date)) {
                Serial.println(F("Enter valid params"));
            }
        }
        else if (strstr(_Serial_buffer, "st") != NULL) {
            char *time = strstr(_Serial_buffer, "st");
            if (!_rtc_time(time)) {
                Serial.println(F("Enter valid params"));
            }
        }
        else {
            Serial.println(F("? SYNTAX ERROR"));
            _printHelp();
        }

        _previous_command_millis = millis();

        // Prompt will be missing after reboot, but this can't be easily fixed.
        Serial.println();
        Serial.print(_prompt);
    }

    // autosave
    if ((unsigned long)(millis() - _previous_command_millis) >= Serial_autosave_interval) _save();
}

SerialCLIClass::SerialCLIClass(AlarmClass *__alarms, void(*__writeEEPROM)(), RTC_DS3231 *__rtc)
{
    _alarms = __alarms;
    _writeEEPROM = __writeEEPROM;
    _rtc = __rtc;
    strcpy(_prompt, _prompt_default);
}

void SerialCLIClass::_printHelp()
{
    Serial.println();
    Serial.println(F("Help:"));
    _indent(1);
    Serial.println(F("sel{i} - select alarm{i}"));
    _indent(1);
    Serial.println(F("Selected alarm:"));
    _indent(2);
    Serial.println(F("ls - list"));
    _indent(2);
    Serial.println(F("en-sgl/en-rpt - enable - single/repeat"));
    _indent(2);
    Serial.println(F("dis - disable"));
    _indent(2);
    Serial.println(F("time{h}:{m} - set time"));
    _indent(2);
    Serial.println(F("dow{d}:{s} - set day {d} of week to {s} 1|0"));
    _indent(2);
    Serial.println(F("snz{t};{c} - set snooze: time{t}min;count{c}"));
    _indent(2);
    Serial.println(F("sig{a};{l};{b} - set signalization: ambient{a};lamp{l}1|0;{buzzer}1|0"));
    _indent(1);
    Serial.println(F("sav - save all"));
    _indent(1);
    Serial.println(F("RTC:"));
    _indent(2);
    Serial.println(F("rtc - get RTC time"));
    _indent(2);
    Serial.println(F("sd{dd}.{mm}.{yy} - set RTC date"));
    _indent(2);
    Serial.println(F("st{h}:{m} - set RTC time"));
}

byte SerialCLIClass::_strbyte(char *str)
{
    byte result = 0;
    while (isDigit(*str)) {
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
    for (byte i = 0; i < level * Serial_indentation_width; i++) {
        Serial.print(' ');
    }
}

boolean SerialCLIClass::_select_alarm(byte __index)
{
    if (__index >= alarms_count && __index != _selected_alarm_index_none) return false;

    _selected_alarm_index = __index;
    if (_selected_alarm_index == _selected_alarm_index_none) strcpy(_prompt, _prompt_default);
    else sprintf(_prompt, "A%u%s", _selected_alarm_index, _prompt_default);

    return true;
}

boolean SerialCLIClass::_list_selected_alarm()
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    Serial.print(F("Num: "));
    Serial.println(_selected_alarm_index);

    _indent(1);
    Serial.print(F("Enabled: "));
    switch ((_alarms + _selected_alarm_index)->get_enabled()) {
    case Off:
        Serial.println(F("Off"));
        break;

    case Single:
        Serial.println(F("Single"));
        break;

    case Repeat:
        Serial.println(F("Repeat"));
        break;
    }

    _indent(1);
    Serial.print(F("Days of week: "));
    for (byte i = 1; i <= 7; i++) {
        if ((_alarms + _selected_alarm_index)->get_days_of_week().getDayOfWeek(i)) {
            Serial.print(days_of_the_week_names_short[i]);
            Serial.print(' ');
        }
        else Serial.print("   ");
    }
    Serial.println();

    _indent(1);
    Serial.print(F("Time: "));
    Serial.print((_alarms + _selected_alarm_index)->get_time().hours);
    Serial.print(":");
    Serial.println((_alarms + _selected_alarm_index)->get_time().minutes);

    _indent(1);
    Serial.println(F("Snooze: "));
    _indent(2);
    Serial.print(F("Time: "));
    Serial.print((_alarms + _selected_alarm_index)->get_snooze().time_minutes);
    Serial.println(F(" min"));
    _indent(2);
    Serial.print(F("Count: "));
    Serial.println((_alarms + _selected_alarm_index)->get_snooze().count);

    _indent(1);
    Serial.println(F("Signalization: "));
    _indent(2);
    Serial.print(F("Ambient: "));
    Serial.println((_alarms + _selected_alarm_index)->get_signalization().ambient);
    _indent(2);
    Serial.print(F("Lamp: "));
    Serial.println((_alarms + _selected_alarm_index)->get_signalization().lamp);
    _indent(2);
    Serial.print(F("Buzzer: "));
    Serial.println((_alarms + _selected_alarm_index)->get_signalization().buzzer);

    return true;
}

boolean SerialCLIClass::_set_enabled(AlarmEnabled __en)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    (_alarms + _selected_alarm_index)->set_enabled(__en);

    _change = true;
    return true;
}

boolean SerialCLIClass::_set_time(char *time)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte hours, minutes;
    time = _find_next_digit(time);
    if (*time == '\0') return false;
    hours = _strbyte(time);
    time = _find_next_digit(time);
    if (*time == '\0') return false;
    minutes = _strbyte(time);

    if ((_alarms + _selected_alarm_index)->set_time(hours, minutes)) {
        _change = true;
        return true;
    }
    else return false;
}

boolean SerialCLIClass::_set_day_of_week(char *dow)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte day;
    boolean status;
    dow = _find_next_digit(dow);
    if (*dow == '\0') return false;
    day = _strbyte(dow);
    dow = _find_next_digit(dow);
    if (*dow == '\0') return false;
    status = _strbyte(dow);

    if ((_alarms + _selected_alarm_index)->set_day_of_week(day, status)) {
        _change = true;
        return true;
    }
    else return false;
}

boolean SerialCLIClass::_set_snooze(char * snooze)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte time, count;

    snooze = _find_next_digit(snooze);
    if (*snooze == '\0') return false;
    time = _strbyte(snooze);
    snooze = _find_next_digit(snooze);
    if (*snooze == '\0') return false;
    count = _strbyte(snooze);

    if ((_alarms + _selected_alarm_index)->set_snooze(time, count)) {
        _change = true;
        return true;
    }
    else return false;
}

boolean SerialCLIClass::_set_signalization(char * sig)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte ambient;
    boolean lamp, buzzer;

    sig = _find_next_digit(sig);
    if (*sig == '\0') return false;
    ambient = _strbyte(sig);
    sig = _find_next_digit(sig);
    if (*sig == '\0') return false;
    lamp = _find_next_digit(sig);
    sig = _find_next_digit(sig);
    if (*sig == '\0') return false;
    buzzer = _strbyte(sig);

    if ((_alarms + _selected_alarm_index)->set_signalization(ambient, lamp, buzzer)) {
        _change = true;
        return true;
    }
    else return false;
}

boolean SerialCLIClass::_save()
{
    if (_change) {
        _change = false;
        _writeEEPROM();
        return true;
    }
    else return false;
}

boolean SerialCLIClass::_rtc_time(char * time)
{
    byte hour, minute;
    time = _find_next_digit(time);
    if (*time == '\0') return false;
    hour = _strbyte(time);
    time = _find_next_digit(time);
    if (*time == '\0') return false;
    minute = _strbyte(time);

    if (hour > 23 || minute > 59) return false;


    _now = _rtc->now();
    _rtc->adjust(DateTime(_now.year(), _now.month(), _now.day(), hour, minute));
    return true;
}

boolean SerialCLIClass::_rtc_date(char * date)
{
    int year;
    byte month, day;

    date = _find_next_digit(date);
    if (*date == '\0') return false;
    day = _strbyte(date);
    date = _find_next_digit(date);
    if (*date == '\0') return false;
    month = _strbyte(date);
    date = _find_next_digit(date);
    if (*date == '\0') return false;
    year = _strbyte(date);

    if (month > 12 || day > 31) return false;

    _now = _rtc->now();
    _rtc->adjust(DateTime(year, month, day, _now.hour(), _now.minute()));
    return true;
}

void SerialCLIClass::_rtc_get()
{
    Serial.print(F("Time: "));
    if (_now.dayOfTheWeek() == 0) Serial.print(days_of_the_week_names_short[7]);
    else Serial.print(days_of_the_week_names_short[_now.dayOfTheWeek()]);
    Serial.print(' ');
    Serial.print(_now.day());
    Serial.print(". ");
    Serial.print(_now.month());
    Serial.print(". ");
    Serial.print(_now.year());
    Serial.print("  ");
    Serial.print(_now.hour());
    Serial.print(':');
    Serial.print(_now.minute());
    Serial.print(':');
    Serial.println(_now.second());
}
