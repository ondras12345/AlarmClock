/*!
    @file Alarm.cpp
*/

#include "Alarm.h"


/*!
    @brief  Call this function in the loop() of your sketch.
    @param time Current date and time
    @see    `set_hardware` must run before this function is first called
*/
void AlarmClass::loop(DateTime time)
{
    if (get_active()) {
        // alarm is already active
        if (snooze_status) {
            // alarm is NOT ringing (snooze)
            if ((unsigned long)(millis() - prev_millis) >= (_snooze.time_minutes * 60000UL)) {
                snooze_status = false;
                if (_signalization.lamp) lamp->set(true);
                // for Alarm_timeout - this allows snooze time to be longer than
                // Alarm_timeout
                prev_activation_millis = millis();
                DEBUG_println(F("Alarm waking from snooze"));
            }
        }
        else {
            // alarm is ringing

            // WARNING: If another alarm activated after this one, it would
            // disable it's ambient (buzzer should enable again; lamp is
            // already solved)
            // # TODO
            if ((unsigned long)(millis() - prev_activation_millis) >= Alarm_timeout) {
                button_stop();
                return;
            }

            if (!_signalization.buzzer) return;

            // select either the regular or last ringing parameters
            unsigned long period = (current_snooze_count == 0) ?
                Alarm_last_ringing_period : Alarm_regular_ringing_period;
            unsigned int freq = (current_snooze_count == 0) ?
                Alarm_last_ringing_freq : Alarm_regular_ringing_freq;

            if ((unsigned long)(millis() - prev_millis) >= period) {
                // invert buzzer
                prev_millis = millis();
                if (beeping) buzzerNoTone();
                else buzzerTone(freq, 0);
                beeping = !beeping;
            }
        }

        return;  // alarm is active
    }

    // alarm is not active
    if (should_trigger(time)) {
        prev_activation_millis = millis();

        // Single must disable even if alarm is inhibited
        if (_enabled == Single) {
            _enabled = Off;
            writeEEPROM_all();
        }

        if (_enabled == Skip) {
            _enabled = Repeat;
            writeEEPROM_all();
            return;
        }

        if (inhibit) {
            DEBUG_println(F("Alarm inhibited"));
            return;
        }

        current_snooze_count = _snooze.count;
        snooze_status = false;

        // safety feature - in case ambientDimmer got stuck:
        if (_signalization.buzzer) buzzerTone(Alarm_regular_ringing_freq, 0);

        // Do events - can only switch on
        ambientDimmer->set_from_duration(
                ambientDimmer->get_value(),
                _signalization.ambient > ambientDimmer->get_stop() ?
                _signalization.ambient : ambientDimmer->get_stop(),
                (ambientDimmer->get_remaining() > 0 &&
                 ambientDimmer->get_remaining() <
                 Alarm_ambient_dimming_duration) ?
                ambientDimmer->get_remaining() :
                Alarm_ambient_dimming_duration);
        ambientDimmer->start();

        if (_signalization.lamp) lamp->set(true);
        activation_callback();
        DEBUG_println(F("Alarm activated"));
    }
}


// returns true if the alarm should trigger
// This function does NOT contain the !get_active() condition.
bool AlarmClass::should_trigger(DateTime time)
{
    // check for prev_activation_millis - in case the alarm gets stopped in the
    // same minute it started
    // 62 seconds - time is fetched each 800 ms in AlarmClock.ino
    if ((unsigned long)(millis() - prev_activation_millis) < 62*1000UL)
        return false;

    // time is not matching
    if (!(_days_of_week.getDayOfWeek_Adafruit(time.dayOfTheWeek()) &&
            time.hour() == _when.hours && time.minute() == _when.minutes &&
            _enabled != Off))
        return false;

    return true;
}


/*!
    @brief  Give the alarm access to the hardware.
            This function must run before the first execution of loop().
            This is not part of the constructor to make it easier to initialise
            an array of alarms.

    @param lamp_  pointer to a HALbool instance that control the lamp

    @param ambientDimmer_ pointer to an instance of `PWMDimmerClass` that
                          controls the ambient LED strip

    @param buzzerTone_ pointer to a function that turns the buzzer on and sets
                       the frequency and duration

    @param buzzerNoTone_ pointer to a function that turns the buzzer off

    @param writeEEPROM_ pointer to a function that writes all the alarms to the
                        EEPROM. This is needed when a "Single" or "Skip" alarms
                        changes its Enabled state.

    @param activation_callback_ pointer to a function that is called when the
                                alarm activates

    @param stop_callback_ pointer to a function that is called when the alarm
                          is stopped.
                          WARNING: also called if the alarm times out.
*/
void AlarmClass::set_hardware(HALbool *lamp_,
                              PWMDimmerClass *ambientDimmer_,
                              void(*buzzerTone_)(unsigned int, unsigned long),
                              void(*buzzerNoTone_)(), void(*writeEEPROM_)(),
                              void(*activation_callback_)(),
                              void(*stop_callback_)())
{
    lamp = lamp_;
    ambientDimmer = ambientDimmer_;
    buzzerTone = buzzerTone_;
    buzzerNoTone = buzzerNoTone_;
    writeEEPROM_all = writeEEPROM_;
    activation_callback = activation_callback_;
    stop_callback = stop_callback_;
}


/*!
    @brief  Call this function when the snooze button is pressed.
            It has no effect during the last ringing of the alarm.
*/
void AlarmClass::button_snooze()
{
    if (snooze_status || !get_active())
        return;

    if (current_snooze_count == 0)
        return;

    snooze_status = true;
    current_snooze_count--;
    prev_millis = millis();

    // not changing ambient

    // otherwise it would disable other alarms' lamp
    if (_signalization.lamp) lamp->set(false);
    buzzerNoTone();
    beeping = false;
}


/*!
    @brief  Stops everything, even if in snooze.
            Call this function when the stop button is pressed.
 */
void AlarmClass::button_stop()
{
    if(!get_active()) return;

    current_snooze_count = current_snooze_count_inactive;

    ambientDimmer->set_from_duration(ambientDimmer->get_value(), 0,
                                     Alarm_ambient_fade_out_duration);
    ambientDimmer->start();
    // otherwise it would disable other alarms' lamp
    if (_signalization.lamp) lamp->set(false);
    buzzerNoTone();
    beeping = false;
    stop_callback();
}


/*!
    @brief  Initialises the alarm to sane default values (disabled).
            The alarm will never trigger unless modified.
*/
AlarmClass::AlarmClass()
{
    _when = { 0, 0 };
    _enabled = Off;
    _days_of_week.DaysOfWeek = 0;
    _snooze = { 0, 0 };
    _signalization = { 0, false, false };
    prev_activation_millis = prev_activation_millis_init;
    current_snooze_count = current_snooze_count_inactive;
    beeping = false;
    snooze_status = false;
    inhibit = false;
}


/*!
    @brief  Configure the alarm from EEPROM data.
    @param data an array of bytes - the EEPROM record, including the id
    @return True if valid, otherwise false.
            If it returns false, the alarm may be set to incomplete (and
            probably random) data.
*/
bool AlarmClass::readEEPROM(byte data[EEPROM_AlarmClass_length])
{
#if defined(DEBUG) && defined(DEBUG_EEPROM_alarms)
    Serial.println();
    Serial.println(F("EEPROM alarm read:"));
    for (byte i = 0; i < EEPROM_AlarmClass_length; i++) {
        Serial.print(data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
#endif // DEBUG

    if (data[0] != EEPROM_alarms_id) return false;

    if (data[1] <= 23) _when.hours = data[1];
    else return false;
    if (data[2] <= 59) _when.minutes = data[2];
    else return false;

    if (data[3] <= AlarmEnabled_max) _enabled = AlarmEnabled(data[3]);
    else return false;

    _days_of_week.DaysOfWeek = data[4];

    if (data[5] <= 99) _snooze.time_minutes = data[5];
    else return false;

    if (data[6] <= 9) _snooze.count = data[6];
    else return false;

    _signalization.ambient = data[7];
    _signalization.lamp = data[8];
    _signalization.buzzer = data[9];

    // not saved in the EEPROM:
    prev_activation_millis = prev_activation_millis_init;
    current_snooze_count = current_snooze_count_inactive;
    beeping = false;
    snooze_status = false;
    inhibit = false;

    DEBUG_println(F("EEPROM alarm read OK"));
    return true;
}


/*!
    @brief  Converts the alarm to an EEPROM record.
    @return A pointer to the first byte of the data.
*/
byte * AlarmClass::writeEEPROM()
{
    _EEPROM_data[0] = EEPROM_alarms_id;

    _EEPROM_data[1] = _when.hours;
    _EEPROM_data[2] = _when.minutes;
    _EEPROM_data[3] = byte(_enabled);
    _EEPROM_data[4] = _days_of_week.DaysOfWeek;
    _EEPROM_data[5] = _snooze.time_minutes;
    _EEPROM_data[6] = _snooze.count;
    _EEPROM_data[7] = _signalization.ambient;
    _EEPROM_data[8] = _signalization.lamp;
    _EEPROM_data[9] = _signalization.buzzer;

#if defined(DEBUG) && defined(DEBUG_EEPROM_alarms)
    Serial.println(F("EEPROM alarm write:"));
    for (byte i = 0; i < EEPROM_AlarmClass_length; i++) {
        Serial.print(_EEPROM_data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
#endif // DEBUG

    return _EEPROM_data;
}


bool AlarmClass::set_enabled(AlarmEnabled __enabled)
{
    if (__enabled > AlarmEnabled_max) return false;
    _enabled = __enabled;
    return true;
}


bool AlarmClass::set_time(byte __hours, byte __minutes)
{
    if (__hours > 23 || __minutes > 59) return false;
    _when = { __hours, __minutes };
    return true;
}


bool AlarmClass::set_days_of_week(DaysOfWeekClass __days_of_week)
{
    _days_of_week = __days_of_week;
    return true;
}


bool AlarmClass::set_day_of_week(byte __day, bool __status)
{
    return _days_of_week.setDayOfWeek(__day, __status);
}


bool AlarmClass::set_snooze(byte __time_minutes, byte __count)
{
    if (__time_minutes > 99 || __count > 9) return false;
    _snooze.time_minutes = __time_minutes;
    _snooze.count = __count;
    return true;
}


bool AlarmClass::set_signalization(byte __ambient, bool __lamp, bool __buzzer)
{
    _signalization.ambient = __ambient;
    _signalization.lamp = __lamp;
    _signalization.buzzer = __buzzer;
    return true;
}


bool AlarmClass::set_inhibit(bool __inhibit)
{
    inhibit = __inhibit;
    return true;
}
