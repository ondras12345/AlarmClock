/*!
    @file
*/

#ifndef PWMDIMMER_H
#define PWMDIMMER_H

#include <Arduino.h>

#include "Settings.h"
#include "Constants.h"


/*!
    @brief  A general purpose class that implements PWM duty cycle ramp-up and
            ramp-down
*/
class PWMDimmer
{
protected:
    unsigned long prev_change_millis_ = 0;
    byte pin_;
    int value_ = 0; // int because of overflows

    // I need to initialize these variables with values because start() could
    // be called before set().
    byte start_ = 0;
    byte stop_ = 0;
    unsigned long interval_ = 100;
    int step_ = 10; // can be negative
    bool active_ = false;


public:
    explicit PWMDimmer(byte pin);

    void set(byte start, byte stop, int step, unsigned long interval);

    void set_from_duration(byte start, byte stop, unsigned long duration);

    void start();

    void stop();

    void loop();

    //! Get current value
    byte get_value() const { return byte(value_); }

    //! Get target value
    byte get_stop() const { return stop_; }

    /*!
        @brief  Check if the dimmer is currently in the process of dimming.
        @return true if the dimming process is not complete yet (target
                value hasn't been reached and `stop` hasn't been called),
                otherwise false.
    */
    bool get_active() const { return active_; }

    /*!
        @brief  Get time remaining before the target value is reached.
        @return time in ms
    */
    unsigned long get_remaining() const
    {
        if (!active_) return 0;
        byte diff_remaining = abs(stop_ - value_);
        return (unsigned long)(diff_remaining * interval_) / step_;
    }
};


#endif

