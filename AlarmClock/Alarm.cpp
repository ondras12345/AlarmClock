// 
// 
// 

#include "Alarm.h"


void AlarmClass::loop(DateTime time)
{

    // # TODO button handling
    // stop - stops everything (even if in snooze)
    // snooze - doesn't have any effect when last ringing

    if (get_active()) { // alarm is already active
        if (get_current_snooze_status()) { // alarm is NOT ringing (snooze)
            if ((unsigned long)(millis() - previous_millis) >= (_snooze.time_minutes * 60000)) {
                set_current_snooze_status(false); // current_snooze_count &= ~AlarmClass_current_snooze_count_snooze_mask; // flip the bit
                set_current_snooze_count(get_current_snooze_count() - 1);
            }

        }
        else if (_signalization.buzzer) { // alarm is ringing
            unsigned long period = (get_current_snooze_count() == 1) ? Alarm_last_ringing_period : Alarm_regular_ringing_period; // select either the regular or last ringing parameters
            unsigned int frequency = (get_current_snooze_count() == 1) ? Alarm_last_ringing_frequency : Alarm_regular_ringing_frequency;

            if ((unsigned long)(millis() - previous_millis) >= period) { // inverse buzzer
                previous_millis = millis();
                if (get_current_beeping_status()) buzzerNoTone();
                else buzzerTone(frequency, 0);
                set_current_beeping_status(!get_current_beeping_status());
            }
        }

    }
    else { // alarm is not active
        if (_days_of_week.getDayOfWeek_Adafruit(time.dayOfTheWeek()) && time.hour() == _when.get_hours() && time.minute() == _when.get_minutes() && _enabled) { // time is matching
            if ((time - last_alarm).totalseconds() > 60) { // check for last_alarm - in case the alarm gets canceled during the same minute it started
                last_alarm = time;
                current_snooze_count = _snooze.count; // bit 6 = 0 --> alarm is ringing (NOT in snooze), bit 5 = 0 --> buzzer is off (doesn't matter)
                // Do events - can only switch on
                if (_signalization.ambient > 0) ambient(0, _signalization.ambient, 900000); // 15 minutes
                if (_signalization.lamp) lamp(true);
            }
        }
    }
}

void AlarmClass::set_hardware(void(*lamp_)(boolean), void(*ambient_)(byte, byte, unsigned long), void(*buzzerTone_)(unsigned int, unsigned long), void(*buzzerNoTone_)())
{
    lamp = lamp_;
    ambient = ambient_;
    buzzerTone = buzzerTone_;
    buzzerNoTone = buzzerNoTone_;
}

AlarmClass::AlarmClass()
{
    _when.timestamp = 0;
    _enabled = false;
    _days_of_week.DaysOfWeek = 0;
    _snooze = { 0,0 };
    _signalization = { 0, false, false };
    last_alarm = DateTime(2000, 1, 1);
    current_snooze_count = AlarmClass_current_snooze_count_none;
}

boolean AlarmClass::readEEPROM(byte data[]) // data length must be equal to AlarmClass_EEPROM_record_length
{
    if (data[0] != EEPROM_alarms_identificator) return false;

    _when.timestamp = 0;
    _when.timestamp = data[1];
    _when.timestamp |= data[2] << 8;
    if (_when.get_hours() > 23 || _when.get_minutes() > 59) return false;

    _enabled = data[3];
    _days_of_week.DaysOfWeek = data[4];

    if (data[5] <= 99) _snooze.time_minutes = data[5];
    else return false;

    if (data[6] <= 9) _snooze.count = data[6];
    else return false;

    _signalization.ambient = data[7];
    _signalization.lamp = data[8];
    _signalization.buzzer = data[9];

    // not saved in the EEPROM:
    last_alarm = DateTime(2000, 1, 1);
    current_snooze_count = AlarmClass_current_snooze_count_none;

    return true;
}

byte * AlarmClass::writeEEPROM()
{
    static byte data[AlarmClass_EEPROM_record_length];
    data[0] = EEPROM_alarms_identificator;

    data[1] = _when.timestamp & 0xFF;
    data[2] = (_when.timestamp >> 8) & 0xFF;
    data[3] = _enabled;
    data[4] = _days_of_week.DaysOfWeek;
    data[5] = _snooze.time_minutes;
    data[6] = _snooze.count;
    data[7] = _signalization.ambient;
    data[8] = _signalization.lamp;
    data[9] = _signalization.buzzer;

    return data;
}


boolean AlarmClass::set_enabled(boolean enabled_) {
    _enabled = enabled_;
    return true;
}

boolean AlarmClass::set_time(byte hours_, byte minutes_) {
    if (hours_ > 23 || minutes_ > 59) return false;
    _when = MinutesTimeStampClass(hours_, minutes_);
    return true;
}

boolean AlarmClass::set_days_of_week(DaysOfWeekClass days_of_week_) {
    _days_of_week = days_of_week_;
    return true;
}

boolean AlarmClass::set_snooze(byte time_minutes_, byte count_) {
    if (time_minutes_ > 99 || count_ > 9) return false;
    _snooze.time_minutes = time_minutes_;
    _snooze.count = count_;
    return true;
}

boolean AlarmClass::set_signalization(byte ambient_, boolean lamp_, boolean buzzer_) {
    _signalization.ambient = ambient_;
    _signalization.lamp = lamp_;
    _signalization.buzzer = buzzer_;
    return true;
}
