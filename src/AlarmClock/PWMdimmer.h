// PWMdimmer.h

#ifndef _PWMDIMMER_h
#define _PWMDIMMER_h

#if defined(ARDUINO) && ARDUINO >= 100
    #include "arduino.h"
#else
    #include "WProgram.h"
#endif

#include "Settings.h"
#include "Constants.h"

class PWMDimmerClass
{
 protected:
     unsigned long _previousChangeMillis = 0;
     byte _pin;
     int _value; // int because of overflows

     // I need to initialize these variables with values bacause start() can be
     // executed before set()
     byte _start = 0;
     byte _stop = 0;
     unsigned long _interval = 100;
     int _step = 10; // can be negative
     boolean _active = false;


 public:
     PWMDimmerClass(byte pin);

     void set(byte start, byte stop, int step, unsigned long interval);

     // Automatically calculate step and interval from duration
     void set_from_duration(byte start, byte stop, unsigned long duration);

     void start();

     // Set the output to LOW and deactivate
     void stop();

     void loop();

     byte get_value() const { return byte(_value); }

     byte get_stop() const { return byte(_stop); }

     boolean get_active() const { return _active; }

     unsigned long get_remaining() const {
         if (!_active) return 0;
         byte diff_remaining = abs(_stop - _value);
         return (unsigned long)(diff_remaining * _interval) / _step;
     }
};


#endif

