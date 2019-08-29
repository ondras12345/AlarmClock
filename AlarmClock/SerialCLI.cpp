// 
// 
// 

#include "SerialCLI.h"

void SerialCLIClass::loop()
{
    if (Serial.available() > Serial_buffer_length) {
        Serial.print(F("Cmd too long: "));
        while (Serial.available())
            Serial.print(Serial.read());
    }
    else {
        byte index = 0;
        while (Serial.available() > 0) {
            _Serial_buffer[index] = tolower(Serial.read());
            if (_Serial_buffer[index] != '\n' && _Serial_buffer[index] != '\r') // ignore CRLF
                if (index < Serial_buffer_length - 1) // to array prevent overflow when a new message comes during processing
                    index++;
        }
        _Serial_buffer[index] = '\0';
    }

    if (!strcmp(_Serial_buffer, "help")) { // ! - strcmp returns 0 if matches
        _printHelp();
    }
    else if (strstr(_Serial_buffer, "sel") != NULL) {
        _selected_alarm_index = _selected_alarm_index_none;
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
    else if (!strcmp(_Serial_buffer, "en") || !strcmp(_Serial_buffer, "enable")) {
        if(!_set_enabled(true)) Serial.println(F("Sel first"));
    }
    else if (!strcmp(_Serial_buffer, "dis") || !strcmp(_Serial_buffer, "disable")) {
        if (!_set_enabled(false)) Serial.println(F("Sel first"));
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
    else if (strstr(_Serial_buffer, "snooze") != NULL) {
        char *snooze = strstr(_Serial_buffer, "snooze");
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
    else {
        Serial.println(F("? SYNTAX ERROR"));
        _printHelp();
    }

    Serial.println();
    Serial.print(_prompt);
}

SerialCLIClass::SerialCLIClass(AlarmClass(*__alarms)[alarms_count])
{
    _alarms = __alarms;
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
    Serial.println(F("en/dis - enable/disable"));
    _indent(2);
    Serial.println(F("time{h}:{m} - set time"));
    _indent(2);
    Serial.println(F("dow{d}:{s} - set day {d} of week to {s} 1|0"));
    _indent(2);
    Serial.println(F("snooze{t};{c} - set snooze: time{t}min;count{c}"));
    _indent(2);
    Serial.println(F("sig{a};{l};{b} - set signalization: ambient{a};lamp{l}1|0;{buzzer}1|0"));
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

void SerialCLIClass::_indent(byte level)
{
    for (byte i = 0; i < level * Serial_indentation_width; i++) {
        Serial.print(' ');
    }
}

boolean SerialCLIClass::_select_alarm(byte __index)
{
    if (__index >= alarms_count) return false;

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
    Serial.println(_alarms[_selected_alarm_index]->get_enabled());

    _indent(1);
    Serial.print(F("Days of week:"));
    for (byte i = 1; i <= 7; i++) {
        if (_alarms[_selected_alarm_index]->get_days_of_week().getDayOfWeek(i)) {
            Serial.print(days_of_the_week_names_short[i]);
            Serial.print(' ');
        }
        else Serial.print("   ");
    }
    Serial.println();

    _indent(1);
    Serial.print(F("Time: "));
    Serial.print(_alarms[_selected_alarm_index]->get_time().get_hours());
    Serial.print(":");
    Serial.println(_alarms[_selected_alarm_index]->get_time().get_minutes());

    _indent(1);
    Serial.println(F("Snooze: "));
    _indent(2);
    Serial.print(F("Time: "));
    Serial.print(_alarms[_selected_alarm_index]->get_snooze().time_minutes);
    Serial.println(F(" min"));
    _indent(2);
    Serial.print(F("Count: "));
    Serial.println(_alarms[_selected_alarm_index]->get_snooze().count);

    _indent(1);
    Serial.println(F("Signalization: "));
    _indent(2);
    Serial.print(F("Ambient: "));
    Serial.println(_alarms[_selected_alarm_index]->get_signalization().ambient);
    _indent(2);
    Serial.print(F("Lamp: "));
    Serial.println(_alarms[_selected_alarm_index]->get_signalization().lamp);
    _indent(2);
    Serial.print(F("Buzzer: "));
    Serial.println(_alarms[_selected_alarm_index]->get_signalization().buzzer);

    return true;
}

boolean SerialCLIClass::_set_enabled(boolean __en)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    _alarms[_selected_alarm_index]->set_enabled(__en);

    return true;
}

boolean SerialCLIClass::_set_time(char *time)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte hours, minutes;
    time = _find_digit(time);
    if (*time == '\0') return false;
    hours = _strbyte(time);
    time = _find_digit(time);
    if (*time == '\0') return false;
    minutes = _strbyte(time);

    return _alarms[_selected_alarm_index]->set_time(hours, minutes);
}

boolean SerialCLIClass::_set_day_of_week(char *dow)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte day;
    boolean status;
    dow = _find_digit(dow);
    if (*dow == '\0') return false;
    day = _strbyte(dow);
    dow = _find_digit(dow);
    if (*dow == '\0') return false;
    status = _strbyte(dow);

    return _alarms[_selected_alarm_index]->set_day_of_week(day, status);
}

boolean SerialCLIClass::_set_snooze(char * snooze)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte time, count;

    snooze = _find_digit(snooze);
    if (*snooze == '\0') return false;
    time = _strbyte(snooze);
    snooze = _find_digit(snooze);
    if (*snooze == '\0') return false;
    count = _strbyte(snooze);

    return _alarms[_selected_alarm_index]->set_snooze(time, count);
}

boolean SerialCLIClass::_set_signalization(char * sig)
{
    if (_selected_alarm_index == _selected_alarm_index_none) return false;

    byte ambient;
    boolean lamp, buzzer;

    sig = _find_digit(sig);
    if (*sig == '\0') return false;
    ambient = _strbyte(sig);
    sig = _find_digit(sig);
    if (*sig == '\0') return false;
    lamp = _find_digit(sig);
    sig = _find_digit(sig);
    if (*sig == '\0') return false;
    buzzer = _strbyte(sig);

    return _alarms[_selected_alarm_index]->set_signalization(ambient, lamp, buzzer);
}
