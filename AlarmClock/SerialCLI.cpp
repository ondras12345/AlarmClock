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
            Serial_buffer[index] = Serial.read();
            index++;
        }
        Serial_buffer[index] = '\0';
    }

    if (!strcmp(Serial_buffer, "help")) { // ! - strcmp returns 0 if matches
        printHelp();
    }
}

void SerialCLIClass::printHelp()
{
    Serial.println();
    Serial.println(F("Help:"));
    // ...
    Serial.println();
}

