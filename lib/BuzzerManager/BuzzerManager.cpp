/*!
    @file
*/

#include "BuzzerManager.h"
#include "Constants.h"
#include <EEPROM.h>


//! Call me in your loop()
void BuzzerManager::loop()
{
    if (melody_)
    {
        if ((millis() - melody_prev_millis_) >= melody_tone_duration_)
        {
            melody_prev_millis_ = millis();

            if (next_tone_address_ + 3 >= EEPROM_size) melody_fail();
            uint8_t frequency = EEPROM.read(next_tone_address_++);
            uint8_t amplitude = EEPROM.read(next_tone_address_++);
            uint8_t duration = EEPROM.read(next_tone_address_++);
            if (duration == 0)
            {
                if (frequency == 0 && amplitude == 0)
                {
                    // stop detected, read footer
                    DEBUG_println(F("reading melody footer"));
                    if (next_tone_address_ + 5 >= EEPROM_size) melody_fail();
                    if (EEPROM.read(next_tone_address_++) != 0xFF) melody_fail();
                    uint8_t flags = EEPROM.read(next_tone_address_);
                    if (!(flags & (1<<melody_footer_flag_repeat)))
                        set_tone(ringing_regular);
                    if (melody_replay_ < 3)
                        melody_replay_++;
                    melody_amplitude_offset_ = EEPROM.read(next_tone_address_ + melody_replay_);
                    next_tone_address_ = melody_start_address_;
                }
                else melody_fail();
            }

            melody_tone_duration_ = duration * 25;
            if (amplitude == 0) sine_.silence(pin_);
            else
            {
                if (amplitude > 255 - melody_amplitude_offset_)
                    amplitude = 255;
                else
                    amplitude = amplitude + melody_amplitude_offset_;
                sine_.tone(pin_, frequency * 32 + 32, amplitude);
            }
        }
        return;
    }

    if (tone_ != ringing_off && (unsigned long)(millis() - prev_millis_) >= period_)
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
    if (tone == ringing_off)
    {
        on_count_--;
        if (on_count_ == 0)
            set_tone(ringing_off);
        return;
    }

    on_count_++;

    // This will cause the buzzer to get stuck on ringing_last instead of going
    // back to ringing_regular if only the alarm that requested it is stopped
    // (e.g. timeout), but I would need to keep the count of alarms that
    // request each type of ringing separately to solve this. At least for now,
    // a warning will be added to the docs and I'll leave it as-is.
    if (tone > tone_)
        set_tone(tone);

}


/*!
    @brief  Set the internal tone_ and the corresponding freq_ and period_.

    This does NOT handle BuzzerTone priorities, that's what set_ringing is for.
    @param tone     The tone to be assigned to tone_.
    @see set_ringing
*/
void BuzzerManager::set_tone(BuzzerTone tone)
{
    tone_ = tone;

    if (tone_ >= ringing_melody0 && tone_ <= ringing_melody15)
    {
#ifdef active_buzzer
        tone_ = ringing_regular;
#else
        if (play_melody(tone_ - ringing_melody0))
        {
            melody_ = true;
            melody_tone_duration_ = 0;
            melody_amplitude_offset_ = 0;
            melody_replay_ = 0;
        }
        else tone_ = ringing_regular;
#endif
    }

    // this needs to handle everything except for melodies
    switch (tone_)
    {
        case ringing_off:
            melody_ = false;
            set_buzzer(false, true);
            volume_ = 0;
            break;

        case ringing_timer:
            melody_ = false;
            period_ = Timer_ringing_period;
            freq_ = Timer_ringing_freq;
            break;

        case ringing_regular:
            melody_ = false;
            freq_ = Alarm_regular_ringing_freq;
            period_ = Alarm_regular_ringing_period;
            break;

        case ringing_last:
            melody_ = false;
            freq_ = Alarm_last_ringing_freq;
            period_ = Alarm_last_ringing_period;
            break;

        case ringing_melody0 ... ringing_melody15:
            // This is here just to prevent compiler warnings.
            break;
    }

    if (!melody_ && tone_ != ringing_off)
    {
        set_buzzer(true);
        prev_millis_ = millis();
    }
}


/*!
    @brief  Turn the buzzer on or off. Active buzzer is handled here.
    @param status True if the buzzer should be beeping.
    @param amp_off  If set to true, the sound output will be completely turned
                    off (as opposed to just outputting PWM modulated silence).
*/
void BuzzerManager::set_buzzer(bool status, bool amp_off)
{
    status_ = status;
    if (pin_ == 255) return; // tests
    if (status)
    {
#ifdef active_buzzer
        digitalWrite(pin_, HIGH);
#else
        sine_.tone(pin_, freq_, volume_);
#endif
    }
    else
    {
#ifdef active_buzzer
        digitalWrite(pin_, LOW);
#else
        if (amp_off)
            sine_.noTone(pin_);
        else
            sine_.silence(pin_);
#endif
    }
}


/*!
    @brief  Prepare for playing a melody from EEPROM.

    This reads the header and sets the melody EEPROM address variable to
    the correct address. It does NOT start playback (does not set the correct
    tone_).
    @param id   ID of the melody (0 to 15).
    @return false on error. Otherwise true.
*/
bool BuzzerManager::play_melody(byte id)
{
    DEBUG_print(F("reading melody "));
    DEBUG_println(id);
    if (id >= EEPROM_melodies_count)
        return false;
    int header_start = EEPROM_melodies_header_start + 3*id;
    byte flags = EEPROM.read(header_start);
    if (!(flags & (1<<melody_header_flag_enabled) && flags & 1))
        return false;
    next_tone_address_ =
        EEPROM.read(header_start + 1) | (EEPROM.read(header_start + 2) << 8);
    if (next_tone_address_ < EEPROM_melodies_data_start ||
            next_tone_address_ >= EEPROM_size)
        return false;
    if (next_tone_address_ + 3 >= EEPROM_size) return false;
    if (EEPROM.read(next_tone_address_++) != 0xFD) return false;
    if (EEPROM.read(next_tone_address_++) != 0x55) return false;
    if (EEPROM.read(next_tone_address_++) != 0xAA) return false;
    melody_start_address_ = next_tone_address_;
    DEBUG_println(F("valid melody header"));
    return true;
}


/*!
    @brief  Handle error in melody data.

    This will switch back to regular ringing if an error is encountered during
    playing a melody.
*/
void BuzzerManager::melody_fail()
{
    DEBUG_println(F("melody_fail"));
    set_tone(ringing_regular);
}
