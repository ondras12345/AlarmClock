// Alarm.h

#ifndef _ALARM_h
#define _ALARM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Settings.h"
#include "DaysOfWeek.h"
#include "MinutesTimeStamp.h"
#include <RTClib.h> // for datetime


//#define AlarmClass_EEPROM_record_length identifier(1B) + sizeof(TimeStampClass - jen  2 byte) + sizeof(enabled - 1 byte) + sizeof(DaysOfWeekClass - jen 1 byte (eeprom)) + sizeof(Snooze) + sizeOf(Signalization)
#define AlarmClass_current_snooze_count_none 255
#define AlarmClass_current_snooze_count_value_mask 0b00001111
#define AlarmClass_current_snooze_count_snooze_mask 0b01000000
#define AlarmClass_current_snooze_count_snooze_bit 6
#define AlarmClass_current_snooze_count_beeping_mask 0b00100000
#define AlarmClass_current_snooze_count_beeping_bit 5

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

    // saved in the EEPROM:
    MinutesTimeStampClass _when;
    boolean _enabled;
    DaysOfWeekClass _days_of_week;
    Snooze _snooze;
    Signalization _signalization;

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
    boolean readEEPROM(byte data[]);
    byte * writeEEPROM();

    void loop(DateTime time);
    void set_hardware(void(*lamp_)(boolean), void(*ambient_)(byte, byte, unsigned long), void(*buzzerTone_)(unsigned int, unsigned long), void(*buzzerNoTone_)());
    AlarmClass();


    boolean set_enabled(boolean __enabled);
    boolean get_enabled() { return _enabled; };

    boolean set_time(byte __hours, byte __minutes);
    MinutesTimeStampClass get_time() { return _when; };

    boolean set_days_of_week(DaysOfWeekClass __days_of_week);
    boolean set_day_of_week(byte __day, boolean __status);
    DaysOfWeekClass get_days_of_week() { return _days_of_week; };

    boolean set_snooze(byte __time_minutes, byte __count);
    Snooze get_snooze() { return _snooze; };

    boolean set_signalization(byte __ambient, boolean __lamp, boolean __buzzer);
    Signalization get_signalization() { return _signalization; };
};


#endif
