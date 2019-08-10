// PWMfade.h

#ifndef _PWMFADE_h
#define _PWMFADE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class PWMfadeClass
{
 protected:
     unsigned long _previousChangeMillis = 0;
     byte _pin;
     int _value; // int because of overflows

     byte _start = 0;
     byte _stop = 255;
     unsigned long _interval = 100;
     int _step = 10; // can be negative
     boolean _active = false;

 public:
     PWMfadeClass(byte pin);
     void set(byte start, byte stop, int step, unsigned long interval);
     void start();
     void stop();
     void loop();
};


#endif

