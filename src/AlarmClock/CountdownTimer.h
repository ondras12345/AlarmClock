#ifndef COUNTDOWNTIMER_H
#define COUNTDOWNTIMER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define CountdownTimer_freq 3000 // in Hz
#define CountdownTimer_period 1000 // in ms

struct TimedEvents
{
    // the status after the timer times out - timers can be used for both turning off and on
    byte ambient;
    bool lamp;
    bool buzzer;
};

class CountdownTimerClass
{
protected:
    void(*lamp)(bool);
    void(*ambient)(byte, byte, unsigned long);
    void(*buzzerTone)(unsigned int, unsigned long); // freq, duration
    void(*buzzerNoTone)();

public:
    unsigned long prev_millis;
    unsigned int time_left; // in seconds - max >18 hours
    bool running;
    bool beeping;

    TimedEvents events;
    void start();
    void stop();
    void loop();

    void set_hardware(void(*lamp_)(bool), void(*ambient_)(byte, byte, unsigned long), void(*buzzerTone_)(unsigned int, unsigned long), void(*buzzerNoTone_)());

    CountdownTimerClass();

};


#endif

