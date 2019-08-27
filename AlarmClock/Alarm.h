// Alarm.h

#ifndef _ALARM_h
#define _ALARM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "DaysOfWeek.h"
#include "MinutesTimeStamp.h"
#include <RTClib.h> // for datetime


//#define AlarmClass_EEPROM_record_length identifier(1B) + sizeof(TimeStampClass - jen  2 byte) + sizeof(enabled - 1 byte) + sizeof(DaysOfWeekClass - jen 1 byte (eeprom)) + sizeof(Snooze) + sizeOf(Signalization)
#define AlarmClass_EEPROM_record_length (1 + 2 + 1 + 1 + 2 + 3)
#define EEPROM_alarms_identificator 0xFE
#define AlarmClass_current_snooze_count_none 255
#define AlarmClass_current_snooze_count_value_mask 0b00001111
#define AlarmClass_current_snooze_count_snooze_mask 0b01000000
#define AlarmClass_current_snooze_count_snooze_bit 6
#define AlarmClass_current_snooze_count_beeping_mask 0b00100000
#define AlarmClass_current_snooze_count_beeping_bit 5

#define Alarm_regular_ringing_frequency 1000 // in Hz
#define Alarm_regular_ringing_period 500 // in ms

#define Alarm_last_ringing_frequency 2000 // in Hz
#define Alarm_last_ringing_period 250 // in ms

struct Snooze {
    byte time_minutes; // max 99
    byte count; // max 9
};

struct Signalization {
    byte ambient; // ambient light - dimmable LED strips
    boolean lamp;
    boolean buzzer;
};



class AlarmClass
{
protected:
    // not saved in EEPROM:
    DateTime last_alarm; // needed in case the alarm gets canceled during the same minute it started
    byte current_snooze_count; // bit 6 - currently in snooze; bit 5 - currently beeping; bit 0,1,2,3 - actual value (max 9)
    unsigned long previous_millis;

    void(*lamp)(boolean);
    void(*ambient)(byte, byte, unsigned long);
    void(*buzzerTone)(unsigned int, unsigned long); // freq, duration
    void(*buzzerNoTone)();

    // current snooze count value selected from the variable that also contains other info; counts down to 0
    byte get_current_snooze_count() const { return current_snooze_count & AlarmClass_current_snooze_count_value_mask; };

    // set the snooze count value
    void set_current_snooze_count(byte count) {
        current_snooze_count &= ~AlarmClass_current_snooze_count_value_mask; // value = 0 (~ = bitwise not)
        current_snooze_count |= count;
    };

    // currently in snooze
    boolean get_current_snooze_status() const { return current_snooze_count & AlarmClass_current_snooze_count_snooze_mask; };

    // set the snooze bit
    void set_current_snooze_status(boolean bitvalue) { bitWrite(current_snooze_count, AlarmClass_current_snooze_count_snooze_bit, bitvalue); };

    // currently beeping (for inversing buzzer)
    boolean get_current_beeping_status() const { return current_snooze_count & AlarmClass_current_snooze_count_beeping_mask; };

    // set the beeping bit (for inversing buzzer)
    void set_current_beeping_status(boolean bitvalue) { bitWrite(current_snooze_count, AlarmClass_current_snooze_count_beeping_bit, bitvalue); };

    // true --> alarm is on (ringing or snooze)
    boolean get_active() const { return current_snooze_count < AlarmClass_current_snooze_count_none; };


public:

    // saved in the EEPROM:
    MinutesTimeStampClass when;
    boolean enabled;
    DaysOfWeekClass days_of_week;
    Snooze snooze;
    Signalization signalization;

    boolean readEEPROM(byte data[]);
    byte * writeEEPROM();

    void loop(DateTime time);
    void set_hardware(void(*lamp_)(boolean), void(*ambient_)(byte, byte, unsigned long), void(*buzzerTone_)(unsigned int, unsigned long), void(*buzzerNoTone_)());
    AlarmClass();


    boolean set_enabled(boolean enabled_);
    boolean get_enabled() { return enabled; };

    boolean set_time(byte hours_, byte minutes_);
    MinutesTimeStampClass get_time() { return when; };

    boolean set_days_of_week(DaysOfWeekClass days_of_week_);
    DaysOfWeekClass get_days_of_week() { return days_of_week; };

    boolean set_snooze(byte time_minutes_, byte count_);
    Snooze get_snooze() { return snooze; };

    boolean set_signalization(byte ambient_, boolean lamp_, boolean buzzer_);
    Signalization get_signalization() { return signalization; };
};


#endif
