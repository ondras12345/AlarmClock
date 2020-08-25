/*!
    @file
*/

#include "CountdownTimer.h"

//! Starts the timer.
void CountdownTimer::start()
{
    if (time_left == 0) return;
    prev_seconds_ = 61; // time_left will decrement immediately.
    running_ = true;
}


/*!
    @brief  Used to stop the timer BEFORE it ends.
*/
void CountdownTimer::stop()
{
    running_ = false;
}


/*!
    @brief  Stop a ringing timer - set everything to off.
            This only works if the timer enabled the buzzer,
            otherwise nothing happens.
    @see    documentation of CountdownTimer.
*/
void CountdownTimer::ButtonStop()
{
    if (!ringing_) return;

    ringing_ = false;
    ambient_.set_from_duration(ambient_.get_value(), 0,
            Timer_ambient_fade_out_duration);
    ambient_.start();
    lamp_.set_manu(false);
    buzzer_.set_ringing(ringing_off);
}


//! Call this function in your loop().
void CountdownTimer::loop(const DateTime& now)
{
    if (!running_) return;
    if (now.second() != prev_seconds_)
    {
        prev_seconds_ = now.second();
        time_left--; // subtract 1 second
        if (time_left == 0)
        {
            running_ = false;
            // Do events - can either switch on or off
            ambient_.set_from_duration(ambient_.get_value(),
                    events.ambient, Timer_ambient_dimming_duration);
            ambient_.start();
            lamp_.set_manu(events.lamp);
            if (events.buzzer)
            {
                ringing_ = true;
                buzzer_.set_ringing(ringing_timer);
            }
        }
    }
}
