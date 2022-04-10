/*!
    @file
*/

#include "Alarm.h"


/*!
    @brief  Call this function in the loop() of your sketch.
    @param time Current date and time
    @see    `SetHardware` must run before this function is first called
*/
void Alarm::loop(DateTime time)
{
    if (get_active())
    {
        // alarm is already active
        if (snooze_status_)
        {
            // alarm is NOT ringing (snooze)
            if ((unsigned long)(millis() - prev_millis_) >= (snooze_.time_minutes * 60000UL))
            {
                snooze_status_ = false;
                if (signalization_.lamp) lamp_.set(true);
                if (signalization_.buzzer)
                {
                    if (signalization_.buzzer >= signalization_melody_start &&
                        signalization_.buzzer <= signalization_melody_end &&
                        current_snooze_count_ != 0
                        )
                    {
                        buzzer_.set_ringing((BuzzerTone)(ringing_melody0 +
                                            signalization_.buzzer -
                                            signalization_melody_start));
                    }
                    else buzzer_.set_ringing((current_snooze_count_ == 0) ?
                            ringing_last : ringing_regular);
                }
                // for Alarm_timeout - this allows snooze time to be longer than
                // Alarm_timeout
                prev_activation_millis_ = millis();
                DEBUG_println(F("Alarm waking from snooze"));
            }
        }
        else
        {
            // alarm is ringing

            // WARNING: If another alarm activated after this one, this
            // timed-out alarm would disable it's ambient (buzzer and lamp are
            // already solved).
            // This would be quite hard to fix, so a note was added to the
            // docs and I'm leaving it as is, at least for now.
            // A class derived from PWMDimmer could be used to keep track
            // of the number of alarms that want the ambient to be on.
            if ((unsigned long)(millis() - prev_activation_millis_) >= Alarm_timeout)
            {
                ButtonStop();
                return;
            }
        }

        return;  // alarm is active
    }

    // alarm is not active

    if (ShouldTriggerAmbient(time))
    {
        ambient_dimmer_.set_from_duration(
                ambient_dimmer_.get_value(),
                signalization_.ambient > ambient_dimmer_.get_stop() ?
                signalization_.ambient : ambient_dimmer_.get_stop(),
                (ambient_dimmer_.get_remaining() > 0 &&
                 ambient_dimmer_.get_remaining() <
                 Alarm_ambient_dimming_duration) ?
                ambient_dimmer_.get_remaining() :
                Alarm_ambient_dimming_duration);

        ambient_dimmer_.start();
        ambient_status_ = true;
        // I am borrowing snooze's variable here, but it shouldn't cause any
        // conflicts.
        prev_millis_ = millis();
        first_ambient_ = false;
        DEBUG_println(F("Alarm enabled ambient"));
    }


    if (ShouldTrigger(time))
    {
        prev_activation_millis_ = millis();
        first_activation_ = false;

        // Single must disable even if alarm is inhibited
        if (enabled_ == Single)
        {
            enabled_ = Off;
            write_EEPROM_all_();
        }

        if (enabled_ == Skip)
        {
            enabled_ = Repeat;
            write_EEPROM_all_();
            return;
        }

        if (inhibit_)
        {
            DEBUG_println(F("Alarm inhibited"));
            return;
        }

        current_snooze_count_ = snooze_.count;
        snooze_status_ = false;

        if (signalization_.buzzer)
        {
            if (signalization_.buzzer >= signalization_melody_start &&
                signalization_.buzzer <= signalization_melody_end &&
                current_snooze_count_ != 0
                )
            {
                buzzer_.set_ringing((BuzzerTone)(ringing_melody0 +
                        signalization_.buzzer - signalization_melody_start));
            }
            else buzzer_.set_ringing((current_snooze_count_ == 0) ?
                                        ringing_last : ringing_regular);
        }

        // Do events - can only switch on
        if (signalization_.lamp) lamp_.set(true);
        activation_callback_();
        DEBUG_println(F("Alarm activated"));
    }
}


/*!
    @brief  returns true if the alarm should trigger
    This function does NOT contain the !get_active() condition.
*/
bool Alarm::ShouldTrigger(DateTime time)
{
    // check for prev_activation_millis_ - in case the alarm gets stopped in the
    // same minute it started
    // 62 seconds - time is fetched each 800 ms in AlarmClock.ino
    if (!first_activation_ &&
            (unsigned long)(millis() - prev_activation_millis_) < 62*1000UL)
        return false;

    // time is not matching
    if (!(days_of_week_.getDayOfWeek_Adafruit(time.dayOfTheWeek()) &&
            time.hour() == when_.hours && time.minute() == when_.minutes &&
            enabled_ != Off))
        return false;

    return true;
}


bool Alarm::ShouldTriggerAmbient(DateTime time)
{
    // I do not directly make use of the prev_activation_millis_ condition
    // inside ShouldTrigger for ambient, but it shouldn't break anything.
    // It helps in case inhibit gets disabled after the normal trigger was
    // inhibited as it prevents ambient from triggering again.
    if (ambient_status_ || inhibit_)
        return false;

    if (signalization_.ambient == 0)
        return false;

    if (enabled_ == Skip)
        return false;

    if (!ShouldTrigger(time + TimeSpan(long(Alarm_ambient_dimming_duration / 1000UL))))
        return false;

    // check for prev_millis_ - in case the alarm gets stopped in the
    // same minute it started.
    // I am borrowing snooze's variable here, but it shouldn't cause any
    // conflicts.
    // 62 seconds - time is fetched each 800 ms in AlarmClock.ino
    if (!first_ambient_ &&
            (unsigned long)(millis() - prev_millis_) < 62*1000UL)
        return false;

    return true;
}


/*!
    @brief  Call this function when the snooze button is pressed.
            It has no effect during the last ringing of the alarm.
*/
void Alarm::ButtonSnooze()
{
    if (snooze_status_ || !get_active())
        return;

    if (current_snooze_count_ == 0)
        return;

    snooze_status_ = true;
    current_snooze_count_--;
    prev_millis_ = millis();

    // not changing ambient

    // otherwise it would disable other alarms' lamp
    if (signalization_.lamp) lamp_.set(false);
    if (signalization_.buzzer) buzzer_.set_ringing(ringing_off);
}


/*!
    @brief  Stops everything, even if in snooze.
            Call this function when the stop button is pressed.
 */
void Alarm::ButtonStop()
{
    if(ambient_status_)
    {
        ambient_dimmer_.set_from_duration(ambient_dimmer_.get_value(), 0,
                                          Alarm_ambient_fade_out_duration);
        ambient_dimmer_.start();

        ambient_status_ = false;
    }

    if(!get_active()) return;

    current_snooze_count_ = current_snooze_count_inactive_;

    stop_callback_();

    // do not disable buzzer and lamp if already disabled
    if (snooze_status_) return;
    if (signalization_.lamp) lamp_.set(false);
    if (signalization_.buzzer) buzzer_.set_ringing(ringing_off);
}


/*!
    @brief  Initialises the alarm to sane default values (disabled).
            The alarm will never trigger unless modified.

    @param lamp reference to a HALbool instance that control the lamp

    @param ambient_dimmer reference to an instance of `PWMDimmer` that
                          controls the ambient LED strip

    @param buzzer reference to an instance of `BuzzerManager`

    @param writeEEPROM pointer to a function that writes all the alarms to
                       the EEPROM. This is needed when a "Single" or "Skip"
                       alarms changes its Enabled state.

    @param activation_callback pointer to a function that is called when the
                               alarm activates

    @param stop_callback pointer to a function that is called when the alarm
                         is stopped.
                          WARNING: also called if the alarm times out.
*/
Alarm::Alarm(
    HALbool& lamp,
    PWMDimmer& ambient_dimmer,
    BuzzerManager& buzzer,
    void(*writeEEPROM)(),
    void(*activation_callback)(),
    void(*stop_callback)()
    ) : lamp_(lamp), ambient_dimmer_(ambient_dimmer), buzzer_(buzzer),
        write_EEPROM_all_(writeEEPROM),
        activation_callback_(activation_callback),
        stop_callback_(stop_callback)
{ }


/*!
    @brief  Configure the alarm from EEPROM data.
    @param data an array of bytes - the EEPROM record, including the id
    @return True if valid, otherwise false.
            If it returns false, the alarm may be set to incomplete (and
            probably random) data.
*/
bool Alarm::ReadEEPROM(byte data[Alarm::EEPROM_length])
{
#if defined(DEBUG) && defined(DEBUG_EEPROM_alarms)
    Serial.println();
    Serial.println(F("EEPROM alarm read:"));
    for (byte i = 0; i < EEPROM_length; i++)
    {
        Serial.print(data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
#endif // DEBUG

    if (data[0] != EEPROM_alarms_id) return false;

    if (data[1] <= 23) when_.hours = data[1];
    else return false;
    if (data[2] <= 59) when_.minutes = data[2];
    else return false;

    if (data[3] <= AlarmEnabled_max) enabled_ = AlarmEnabled(data[3]);
    else return false;

    days_of_week_.days_of_week = data[4];

    if (data[5] <= 99) snooze_.time_minutes = data[5];
    else return false;

    if (data[6] <= 9) snooze_.count = data[6];
    else return false;

    signalization_.ambient = data[7];
    signalization_.lamp = data[8];
    signalization_.buzzer = data[9];

    // not saved in the EEPROM:
    current_snooze_count_ = current_snooze_count_inactive_;
    snooze_status_ = false;
    ambient_status_ = false;
    inhibit_ = false;

    DEBUG_println(F("EEPROM alarm read OK"));
    return true;
}


/*!
    @brief  Converts the alarm to an EEPROM record.
    @param  data    Array to store the resulting data in.
*/
void Alarm::WriteEEPROM(byte data[EEPROM_length])
{
    data[0] = EEPROM_alarms_id;

    data[1] = when_.hours;
    data[2] = when_.minutes;
    data[3] = byte(enabled_);
    data[4] = days_of_week_.days_of_week;
    data[5] = snooze_.time_minutes;
    data[6] = snooze_.count;
    data[7] = signalization_.ambient;
    data[8] = signalization_.lamp;
    data[9] = signalization_.buzzer;

#if defined(DEBUG) && defined(DEBUG_EEPROM_alarms)
    Serial.println(F("EEPROM alarm write:"));
    for (byte i = 0; i < EEPROM_length; i++)
    {
        Serial.print(data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
#endif // DEBUG
}


bool Alarm::set_enabled(AlarmEnabled enabled)
{
    if (enabled > AlarmEnabled_max) return false;
    enabled_ = enabled;
    return true;
}


bool Alarm::set_time(byte hours, byte minutes)
{
    if (hours > 23 || minutes > 59) return false;
    when_ = { hours, minutes };
    return true;
}


bool Alarm::set_days_of_week(DaysOfWeek days_of_week)
{
    days_of_week_ = days_of_week;
    return true;
}


bool Alarm::set_day_of_week(byte day, bool status)
{
    return days_of_week_.setDayOfWeek(day, status);
}


bool Alarm::set_snooze(byte time_minutes, byte count)
{
    if (time_minutes > 99 || count > 9) return false;
    snooze_.time_minutes = time_minutes;
    snooze_.count = count;
    return true;
}


bool Alarm::set_signalization(byte ambient, bool lamp, byte buzzer)
{
    signalization_.ambient = ambient;
    signalization_.lamp = lamp;
    signalization_.buzzer = buzzer;
    return true;
}


bool Alarm::set_inhibit(bool inhibit)
{
    inhibit_ = inhibit;
    return true;
}
