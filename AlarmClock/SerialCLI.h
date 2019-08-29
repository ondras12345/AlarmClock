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
     AlarmClass(*_alarms)[alarms_count];
     const char _prompt_default[2 + 1] = "> ";
     char _Serial_buffer[Serial_buffer_length + 1]; // +1 for termination
     char _prompt[Serial_prompt_length + 1] = "> ";

     const byte _selected_alarm_index_none = 255;
     byte _selected_alarm_index = _selected_alarm_index_none;

     void _printHelp();
     byte _strbyte(char *str);
     char * _find_digit(char *str);
     void _indent(byte level);

     boolean _select_alarm(byte index);
     boolean _list_selected_alarm();
     boolean _set_enabled(boolean __en);
     boolean _set_time(char *time);
     boolean _set_day_of_week(char *dow);

 public:
     void loop();
     SerialCLIClass(AlarmClass (*alarms)[alarms_count]);
};

#endif
