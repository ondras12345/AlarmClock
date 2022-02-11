/*!
    @file
*/

#include "PWMDimmer.h"

/*!
    @brief  Initialises the PWMDimmer object
    @param pin  a pin that supports PWM.
                This pin must be configured as OUTPUT before loop() is first
                called.
                If pin is set to 255, it is ignored and no hardware is
                accessed. This is used for tests.
*/
PWMDimmer::PWMDimmer(byte pin) : pin_(pin)
{ }


/*!
    @brief  Sets the values required for operation.
            WARNING: This does not start the process.
*/
void PWMDimmer::set(byte start, byte stop, int step, unsigned long interval)
{
    start_ = start;
    stop_ = stop;
    step_ = step;
    interval_ = interval;
}


/*!
    @brief  Automatically calculate `step` and `interval` and call `set`.
    @see `set`
*/
void PWMDimmer::set_from_duration(byte start, byte stop, unsigned long duration)
{
    int step_sign = (start > stop) ? -1 : 1;
    byte diff = abs(stop - start);
    int step_ = 0;
    unsigned long interval_;
    unsigned long duration_ = duration;

    if (duration_ < 500) duration_ = 500;

    // Choose interval
    // interval >= 50 ms
    interval_ = duration_ / diff;
    if(interval_ < 50) interval_ = 50;


    step_ = step_sign * ((interval_ * diff) / duration_);
    if (step_ == 0) step_ = step_sign; // step must not be 0

#if defined(DEBUG) && defined(DEBUG_dimmer)
    DEBUG_print(F("dimmer - start: "));
    DEBUG_println(start);

    DEBUG_print(F("dimmer - stop: "));
    DEBUG_println(stop);

    DEBUG_print(F("dimmer - diff: "));
    DEBUG_println(diff);

    DEBUG_print(F("dimmer - duration: "));
    DEBUG_println(duration_);

    DEBUG_print(F("dimmer - interval: "));
    DEBUG_println(interval_);

    DEBUG_print(F("dimmer - step: "));
    DEBUG_println(step_);
#endif


    set(start, stop, step_, interval_);
}


/*!
    @brief  Start dimming

    `set` of `set_from_duration` should be called before starting the
    dimmer.
    @see    set
    @see    set_from_duration
*/
void PWMDimmer::start()
{
    active_ = true;
    value_ = start_;
}


//! Set the output to LOW and deactivate.
void PWMDimmer::stop()
{
    active_ = false;
    stop_ = 0;  // get_remaining needs it to work correctly
    value_ = 0;

    if (pin_ == 255) return; // tests
    digitalWrite(pin_, LOW);
}


//! Call this function in the loop() of your sketch.
void PWMDimmer::loop()
{
    if (active_ && ((unsigned long)(millis() - prev_change_millis_) >= interval_))
    {
        prev_change_millis_ = millis();

        if ((step_ > 0 && value_ >= stop_) || (step_ < 0 && value_ <= stop_))
        {
            value_ = stop_;
            active_ = false;


            if (pin_ == 255) return; // tests
            analogWrite(pin_, value_);

            // <10 and >250 is necessary for the GUI, because it only displays
            // the value divided by 10:
            // real ....... GUI
            // 0 - 9 ....... 0
            // 10 - 19 ..... 1
            // ...
            // 250 - 255 ... 25
            if (value_ < 10) digitalWrite(pin_, LOW);
            else if (value_ >= 250) digitalWrite(pin_, HIGH);
        }
        else
        {
            value_ += step_;
            if (value_ > 255) value_ = 255;
            else if (value_ < 0) value_ = 0;

            if (pin_ == 255) return; // tests
            analogWrite(pin_, value_);
        }
    }
}
