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
            if ((unsigned long)(millis() - previous_millis) >= (snooze.time_minutes * 60000)) {
                set_current_snooze_status(false); // current_snooze_count &= ~AlarmClass_current_snooze_count_snooze_mask; // flip the bit
                set_current_snooze_count(get_current_snooze_count() - 1);
            }

        }
        else { // alarm is ringing
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
        if (days_of_week.getDayOfWeek_Adafruit(time.dayOfTheWeek()) && time.hour() == when.get_hours() && time.minute() == when.get_minutes() && enabled) { // time is matching
            if ((time - last_alarm).totalseconds() > 60) { // check for last_alarm - in case the alarm gets canceled during the same minute it started
                last_alarm = time;
                current_snooze_count = snooze.count; // bit 6 = 0 --> alarm is ringing (NOT in snooze), bit 5 = 0 --> buzzer is off (doesn't matter)
                // Do events - can only switch on
                if (signalization.ambient > 0) ambient(0, signalization.ambient, 900000); // 15 minutes
                if (signalization.lamp) lamp(true);
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
    when.timestamp = 0;
    enabled = false;
    days_of_week.DaysOfWeek = 0;
    snooze = { 0,0 };
    signalization = { 0, false, false };
    last_alarm = DateTime(2000, 1, 1);
    current_snooze_count = AlarmClass_current_snooze_count_none;
}

boolean AlarmClass::readEEPROM(byte data[]) // data length must be equal to AlarmClass_EEPROM_record_length
{
    if (data[0] != EEPROM_alarms_identificator) return false;

    when.timestamp = 0;
    when.timestamp = data[1];
    when.timestamp |= data[2] << 8;
    if (when.get_hours() > 23 || when.get_minutes() > 59) return false;

    enabled = data[3];
    days_of_week.DaysOfWeek = data[4];

    if (data[5] <= 99) snooze.time_minutes = data[5];
    else return false;

    if (data[6] <= 9) snooze.count = data[6];
    else return false;

    signalization.ambient = data[7];
    signalization.lamp = data[8];
    signalization.buzzer = data[9];

    // not saved in the EEPROM:
    last_alarm = DateTime(2000, 1, 1);
    current_snooze_count = AlarmClass_current_snooze_count_none;

    return true;
}

byte * AlarmClass::writeEEPROM()
{
    static byte data[AlarmClass_EEPROM_record_length];
    data[0] = EEPROM_alarms_identificator;

    data[1] = when.timestamp & 0xFF;
    data[2] = (when.timestamp >> 8) & 0xFF;
    data[3] = enabled;
    data[4] = days_of_week.DaysOfWeek;
    data[5] = snooze.time_minutes;
    data[6] = snooze.count;
    data[7] = signalization.ambient;
    data[8] = signalization.lamp;
    data[9] = signalization.buzzer;

    return data;
}