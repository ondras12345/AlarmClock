// 
// 
// 

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
        if (_value == _stop) {
            if (_value == 0) digitalWrite(_pin, LOW);
            else if (_value == 255) digitalWrite(_pin, HIGH);
            _active = false; // not fading
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
