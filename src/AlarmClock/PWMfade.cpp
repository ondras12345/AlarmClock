#include "PWMfade.h"

PWMfadeClass::PWMfadeClass(byte pin)
{
    _pin = pin;
}


void PWMfadeClass::set(byte start, byte stop, int step, unsigned long interval)
{
    _start = start;
    _stop = stop;
    _step = step;
    _interval = interval;
}


void PWMfadeClass::set_from_duration(byte start, byte stop, unsigned long duration)
{
    int step_sign = (start > stop) ? -1 : 1;
    byte diff = abs(stop - start);
    int _step = 0;
    unsigned long _interval;
    unsigned long _duration = duration;

    if (_duration < 500) _duration = 500;

    // Choose interval
    // interval >= 50 ms
    _interval = _duration / diff;
    if(_interval < 50) _interval = 50;


    _step = step_sign * ((_interval * diff) / _duration);
    if (_step == 0) _step = step_sign; // step must not be 0

#if defined(DEBUG) && defined(DEBUG_fade)
    DEBUG_print("fade - diff: ");
    DEBUG_println(diff);

    DEBUG_print("fade - duration: ");
    DEBUG_println(_duration);

    DEBUG_print("fade - interval: ");
    DEBUG_println(_interval);

    DEBUG_print("fade - step: ");
    DEBUG_println(_step);
#endif


    set(start, stop, _step, _interval);
}


void PWMfadeClass::start()
{
    _active = true;
    _value = _start;
}


void PWMfadeClass::stop()
{
    _active = false;
    _value = 0;
    digitalWrite(_pin, LOW);
}


void PWMfadeClass::loop()
{
    if (_active && ((unsigned long)(millis() - _previousChangeMillis) >= _interval)) {
        if ((_step > 0 && _value >= _stop) || (_step < 0 && _value <= _stop)) {
            _value = _stop;
            analogWrite(_pin, _value);

            // <10 and >250 is necessary for the GUI, because it only displays
            // the value divided by 10:
            // real ....... GUI
            // 0 - 9 ....... 0
            // 10 - 19 ..... 1
            // ...
            // 250 - 255 ... 25
            if (_value < 10) digitalWrite(_pin, LOW);
            else if (_value >= 250) digitalWrite(_pin, HIGH);
            _active = false;
        }
        else {
            _value += _step;
            if (_value > 255) _value = 255;
            else if (_value < 0) _value = 0;

            analogWrite(_pin, _value);
        }

        _previousChangeMillis = millis();
    }
}
