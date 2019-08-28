// 
// 
// 

#include "SerialCLI.h"

void SerialCLIClass::loop()
{
    if (Serial.available() > Serial_buffer_size) {
        Serial.print(F("Cmd too long: "));
        while (Serial.available())
            Serial.print(Serial.read());
    }
    else {
        byte index = 0;
        while (Serial.available() > 0) {
            _Serial_buffer[index] = tolower(Serial.read());
            if (_Serial_buffer[index] != '\n' && _Serial_buffer[index] != '\r') // ignore CRLF
                index++;
        }
        _Serial_buffer[index] = '\0';
    }

    if (!strcmp(_Serial_buffer, "help")) { // ! - strcmp returns 0 if matches
        _printHelp();
    }
}

void SerialCLIClass::_printHelp()
{
    Serial.println();
    Serial.println(F("Help:"));
    // ...
    Serial.println();
}

