#include "CountdownTimer.h"

void CountdownTimerClass::start()
{
    prev_millis = millis();
    running = true;
}

void CountdownTimerClass::stop()
{
    running = false;
    beeping = false;
    buzzerNoTone();
}

void CountdownTimerClass::loop()
{
    if ((unsigned long)(millis() - prev_millis) >= 1000 && time_left > 0) {
        time_left--; // subtract 1 second
        if (time_left == 0) {
            // Do events - can either switch on or off
            if(events.ambient == 0) ambient(255, events.ambient, 60000); // turn off, 1 minute
            else ambient(0, events.ambient, 600000); // turn on, 10 minutes
            lamp(events.lamp);
            beeping = false;
        }
        else prev_millis += 1000;
    }

    if (time_left == 0 && running && events.buzzer) { // buzzer
        if ((unsigned long)(millis() - prev_millis) >= CountdownTimer_period) {
            if (beeping) buzzerNoTone();
            else buzzerTone(CountdownTimer_frequency, 0);
            beeping = !beeping;
            prev_millis = millis();
        }
    }
}

void CountdownTimerClass::set_hardware(void(*lamp_)(bool), void(*ambient_)(byte, byte, unsigned long), void(*buzzerTone_)(unsigned int, unsigned long), void(*buzzerNoTone_)())
{
    lamp = lamp_;
    ambient = ambient_;
    buzzerTone = buzzerTone_;
    buzzerNoTone = buzzerNoTone_;
}

CountdownTimerClass::CountdownTimerClass()
{
    running = false;
    beeping = false;
}
