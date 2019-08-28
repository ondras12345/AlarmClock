// SerialCLI.h

#ifndef _SERIALCLI_h
#define _SERIALCLI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Settings.cpp"
#include "Alarm.h"

class SerialCLIClass
{
 protected:
     const char _prompt_default[2 + 1] = "> ";
     char _Serial_buffer[Serial_buffer_length + 1]; // +1 for termination
     char _prompt[Serial_prompt_length + 1] = "> ";

     const byte _select_alarm_none = 255;
     byte _select_alarm = _select_alarm_none;

     void _printHelp();
     void _select(byte index);

 public:
     void loop();
};

#endif
