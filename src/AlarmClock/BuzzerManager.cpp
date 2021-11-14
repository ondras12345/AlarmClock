/*!
    @file
*/

#include "BuzzerManager.h"


/*!
    @brief  Call me in your loop()
*/
void BuzzerManager::loop()
{
    if(tone_ != ringing_off && (unsigned long)(millis() - prev_millis_) >= period_)
    {
        set_buzzer(!status_);
        prev_millis_ = millis();
    }

    if (tone_ != ringing_off && volume_ < 255)
    {
        // We need to make 255 steps during Alarm_ringing_ramp_duration.
        // It won't be that smooth because we only update the amplitude at the
        // beginning of period_ though.
        if ((millis() - volume_prev_millis_) >= (Alarm_ringing_ramp_duration / 255UL))
        {
            volume_prev_millis_ = millis();
            volume_++;
        }
    }
}


/*!
    @brief  This is used by the alarms to start or stop ringing.
    @param tone The tone to use. If current tone has higher priority, this
                parameter is ignored unless it is `ringing_off`.
*/
void BuzzerManager::set_ringing(BuzzerTone tone)
{
    // This will cause the buzzer getting stuck on last_ringing if only the
    // alarm that requested it is stopped (e.g. timeout), but I would need to
    // keep the count of alarms that request each type of ringing separately to
    // solve this. At least for now, a warning will be added to the docs and
    // I'll leave it as-is.
    if (tone > tone_) tone_ = tone;

    if (tone == ringing_off)
    {
        on_count_--;
        if (on_count_ == 0)
        {
            tone_ = ringing_off;
            set_buzzer(false);
            volume_ = 0;
        }
        return;
    }

    on_count_++;
    if (tone_ == ringing_timer)
    {
        period_ = Timer_ringing_period;
        freq_ = Timer_ringing_freq;
    }
    else
    {
        period_ = (tone_ == ringing_regular) ?
            Alarm_regular_ringing_period : Alarm_last_ringing_period;
        freq_ = (tone_ == ringing_regular) ?
            Alarm_regular_ringing_freq : Alarm_last_ringing_freq;
    }

    set_buzzer(true);
}


//! Turn the buzzer on or off. Active buzzer is handled here.
void BuzzerManager::set_buzzer(bool status)
{
    status_ = status;
    if (pin_ == 255) return; // tests
    if (status)
    {
#ifdef active_buzzer
        digitalWrite(pin_buzzer, HIGH);
#else
        sine_.tone(pin_, freq_, volume_);
#endif
    }
    else
    {
#ifdef active_buzzer
        digitalWrite(pin_, LOW);
#else
        sine_.noTone(pin_);
#endif
    }
}
