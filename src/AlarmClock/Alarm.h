// Alarm.h

#ifndef _ALARM_h
#define _ALARM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Settings.h"
#include "Constants.h"
#include "PWMDimmer.h"
#include "DaysOfWeek.h"
#include <RTClib.h> // for datetime


//#define AlarmClass_EEPROM_length identifier(1B) + sizeof(TimeStampClass - jen  2 byte) + sizeof(AlarmsEnabled - 1 byte) + sizeof(DaysOfWeekClass - jen 1 byte (eeprom)) + sizeof(Snooze) + sizeOf(Signalization)
#define AlarmClass_current_snooze_count_none 255
#define AlarmClass_current_snooze_count_value_mask 0b00001111
#define AlarmClass_current_snooze_count_snooze_mask 0b01000000
#define AlarmClass_current_snooze_count_snooze_bit 6
#define AlarmClass_current_snooze_count_beeping_mask 0b00100000
#define AlarmClass_current_snooze_count_beeping_bit 5


#define AlarmEnabled_max 2  // for input validation
enum AlarmEnabled {
    Off = 0,
    Single = 1,
    Repeat = 2
};
struct Snooze {
    byte time_minutes; // max 99
    byte count; // max 9
};

struct hours_minutes {
    byte hours;
    byte minutes;
};

struct Signalization {
    byte ambient; // ambient light - dimmable LED strips
    bool lamp;
    bool buzzer;
};


class AlarmClass
{
protected:
    byte _EEPROM_data[EEPROM_AlarmClass_length]; // needs to be static (because of pointers)

    // not saved in EEPROM:
    DateTime last_alarm; // needed in case the alarm gets canceled during the same minute it started
    byte current_snooze_count; // bit 6 - currently in snooze; bit 5 - currently beeping; bit 0,1,2,3 - actual value (max 9)
    unsigned long prev_millis;
    bool inhibit;

    void(*lamp)(bool);
    PWMDimmerClass *ambientDimmer;
    void(*buzzerTone)(unsigned int, unsigned long); // freq, duration
    void(*buzzerNoTone)();
    void(*writeEEPROM_all)();  // write all alarms to EEPROM
    void(*activation_callback)();
    void(*stop_callback)();

    // saved in the EEPROM:
    hours_minutes _when;
    AlarmEnabled _enabled;
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
    bool get_current_snooze_status() const { return current_snooze_count & AlarmClass_current_snooze_count_snooze_mask; };

    // set the snooze bit
    void set_current_snooze_status(bool bitvalue) { bitWrite(current_snooze_count, AlarmClass_current_snooze_count_snooze_bit, bitvalue); };

    // currently beeping (for inversing buzzer)
    bool get_current_beeping_status() const { return current_snooze_count & AlarmClass_current_snooze_count_beeping_mask; };

    // set the beeping bit (for inversing buzzer)
    void set_current_beeping_status(bool bitvalue) { bitWrite(current_snooze_count, AlarmClass_current_snooze_count_beeping_bit, bitvalue); };

    // true --> alarm is on (ringing or snooze)
    bool get_active() const { return current_snooze_count < AlarmClass_current_snooze_count_none; };


public:
    bool readEEPROM(byte data[EEPROM_AlarmClass_length]);
    byte * writeEEPROM();

    void loop(DateTime time);
    void set_hardware(void(*lamp)(bool),
                      PWMDimmerClass *ambientDimmer,
                      void(*buzzerTone)(unsigned int, unsigned long),
                      void(*buzzerNoTone)(), void(*writeEEPROM)(),
                      void(*activation_callback)(), void(*stop_callback)());
    void button_snooze();
    void button_stop();
    AlarmClass();


    bool set_enabled(AlarmEnabled enabled);
    AlarmEnabled get_enabled() { return _enabled; };

    bool set_time(byte hours, byte minutes);
    hours_minutes get_time() { return _when; };

    bool set_days_of_week(DaysOfWeekClass days_of_week);
    bool set_day_of_week(byte day, bool status);
    DaysOfWeekClass get_days_of_week() { return _days_of_week; };
    bool get_day_of_week(byte day) { return _days_of_week.getDayOfWeek(day); }

    bool set_snooze(byte time_minutes, byte count);
    Snooze get_snooze() { return _snooze; };

    bool set_signalization(byte ambient, bool lamp, bool buzzer);
    Signalization get_signalization() { return _signalization; };

    bool get_inhibit() { return inhibit; };
    bool set_inhibit(bool inhibit);
};


#endif
