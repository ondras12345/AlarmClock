// SerialCLI.h

#ifndef _SERIALCLI_h
#define _SERIALCLI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Settings.cpp"
#include "Constants.cpp"
#include "Alarm.h"

class SerialCLIClass
{
 protected:
     AlarmClass(*_alarms)[alarms_count];
     const char _prompt_default[2 + 1] = "> ";
     char _Serial_buffer[Serial_buffer_length + 1]; // +1 for termination
     char _prompt[Serial_prompt_length + 1] = "> ";

     const byte _selected_alarm_index_none = 255;
     byte _selected_alarm_index = _selected_alarm_index_none;

     void _printHelp();
     boolean _select_alarm(byte index);
     boolean _list_selected_alarm();

     void _indent(byte level);
 public:
     void loop();
     SerialCLIClass(AlarmClass (*alarms)[alarms_count]);
};

#endif
