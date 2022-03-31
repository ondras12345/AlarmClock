/*!
    @file
*/

#ifndef COUNTDOWNTIMER_H
#define COUNTDOWNTIMER_H

#include <Arduino.h>

#include "Settings.h"
#include <BuzzerManager.h>
#include <HALbool.h>
#include <PWMDimmer.h>
#include <RTClib.h>



/*!
    @brief  A timer that counts down and then does some events.

    It can operate in two modes:
    - If the timed events do not contain buzzer, it just does the events
      and stops. buttons_stop does nothing, and the only way to revert the
      changes is to use manual control outside of this class.

    - If the timed events do contain buzzer, button_stop stops the ringing and
      turns everything else off.
*/
class CountdownTimer
{
public:
    /*!
        @brief  The status after the timer times out.
                Timers can be used for both turning stuff off and on.
                For lamp, this works the same way as manual control.
                Buzzer can only be turned on.
    */
    struct TimedEvents
    {
        byte ambient;
        bool lamp;
        bool buzzer;
    };

    void start();
    void stop();
    void ButtonStop();
    bool get_running() const { return running_; };
    void loop(const DateTime& now);

    /*!
        @brief  The remaining time in seconds.
    */
    unsigned int time_left = 0;

    //! The actions the timer should execute when done.
    TimedEvents events = { 0, false, false };

    CountdownTimer(PWMDimmer& ambient, HALbool& lamp, BuzzerManager& buzzer) :
        ambient_(ambient), lamp_(lamp), buzzer_(buzzer) { };


protected:
    byte prev_seconds_;
    bool running_ = false;
    bool ringing_ = false;
    unsigned long timeout_millis_ = 0;

    PWMDimmer& ambient_;
    HALbool& lamp_;
    BuzzerManager& buzzer_;
};


#endif
