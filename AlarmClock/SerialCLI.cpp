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
                if(index < Serial_buffer_length - 1) // to array prevent overflow when a new message comes during processing
                    index++;
        }
        _Serial_buffer[index] = '\0';
    }

    if (!strcmp(_Serial_buffer, "help")) { // ! - strcmp returns 0 if matches
        _printHelp();
    }
    else if (strstr(_Serial_buffer, "sel") != NULL) {
        _selected_alarm = _selected_alarm_none;
        char *index = strstr(_Serial_buffer, "sel");
        while (!isDigit(*index) && *index != '\0') *index++;
        if (*index == '\0') _select_alarm(_selected_alarm_none);
        else {
            byte index_num = *index - '0';
            if (index_num < alarms_count) _select_alarm(index_num);
            else {
                Serial.print(F("Invalid i: "));
                Serial.println(index_num);
            }

        }

    }
    else {
        Serial.println(F("? SYNTAX ERROR"));
        _printHelp();
    }

    Serial.println();
    Serial.print(_prompt);
}

void SerialCLIClass::_printHelp()
{
    Serial.println();
    Serial.println(F("Help:"));
    Serial.println(F("sel{i} - select alarm{i}"));
    // ...
    Serial.println();
}

void SerialCLIClass::_select_alarm(byte __index)
{
    _selected_alarm = __index;
    if (_selected_alarm == _selected_alarm_none) strcpy(_prompt, _prompt_default);
    else sprintf(_prompt, "A%u%s", _selected_alarm, _prompt_default);
}

