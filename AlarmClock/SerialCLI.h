// SerialCLI.h

#ifndef _SERIALCLI_h
#define _SERIALCLI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Alarm.h"

#define Serial_buffer_size 10

class SerialCLIClass
{
 protected:
     char Serial_buffer[Serial_buffer_size + 1]; // +1 for termination

     void printHelp();

 public:
     void loop();
};

#endif
