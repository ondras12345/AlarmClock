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
        while (!isDigit(*index) && *index != '\0') *index++;
        if (*index == '\0') _select_alarm(_selected_alarm_index_none);
        else {
            byte index_num = *index - '0';
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
    Serial.println(F("ls - list selected alarm"));
    _indent(1);
    Serial.println(F("en/dis - enable/disable selected alarm"));
    // ...
    Serial.println();
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

void SerialCLIClass::_indent(byte level)
{
    for (byte i = 0; i < level * Serial_indentation_width; i++) {
        Serial.print(' ');
    }
}
