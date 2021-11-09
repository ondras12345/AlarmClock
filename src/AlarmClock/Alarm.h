/*!
    @file
*/

#ifndef ALARM_H
#define ALARM_H

#include "Arduino.h"

#include "Settings.h"
#include "Constants.h"
#include "PWMDimmer.h"
#include "HALbool.h"
#include "BuzzerManager.h"
#include "DaysOfWeek.h"
#include <RTClib.h> // for datetime


#define AlarmEnabled_max 3  // for input validation
enum AlarmEnabled
{
    Off = 0,
    Single = 1,
    Repeat = 2,
    Skip = 3
};


struct Snooze
{
    byte time_minutes; //!< max 99
    byte count; //!< max 9
};


struct HoursMinutes
{
    byte hours;
    byte minutes;
};


/*!
    @brief  Stores information about the means that should be used to wake the
            user up when the alarm activates.
*/
struct Signalization
{
    byte ambient; //!< ambient light intensity - dimmable LED strips
    bool lamp;
    bool buzzer;
};


/*!
    @brief  An alarm. In normal use, there is an array of multiple instances of
            this class.
*/
class Alarm
{
public:
    /*!
        @brief  EEPROM record length in bytes.
        EEPROM_alarms_id                1
        HoursMinutes when_              2
        AlarmEnabled enabled_           1
        DaysOfWeek days_of_week_        1
        Snooze snooze_                  2
        Signalization signalization_    3
    */
    static constexpr byte EEPROM_length = 1 + 2 + 1 + 1 + 2 + 3;
    bool ReadEEPROM(byte data[EEPROM_length]);
    byte * WriteEPROM();

    void loop(DateTime time);
    void SetHardware(HALbool *lamp,
                     PWMDimmer *ambient_dimmer,
                     BuzzerManager *buzzer,
                     void(*writeEEPROM)(),
                     void(*activation_callback)(),
                     void(*stop_callback)());
    void ButtonSnooze();
    void ButtonStop();
    Alarm();


    bool set_enabled(AlarmEnabled enabled);
    AlarmEnabled get_enabled() const { return enabled_; };

    bool set_time(byte hours, byte minutes);
    HoursMinutes get_time() const { return when_; };

    bool set_days_of_week(DaysOfWeek days_of_week);
    bool set_day_of_week(byte day, bool status);
    DaysOfWeek get_days_of_week() const { return days_of_week_; };
    bool get_day_of_week(byte day) const { return days_of_week_.getDayOfWeek(day); }

    bool set_snooze(byte time_minutes, byte count);
    Snooze get_snooze() const { return snooze_; };

    bool set_signalization(byte ambient, bool lamp, bool buzzer);
    Signalization get_signalization() const { return signalization_; };

    bool get_inhibit() const { return inhibit_; };
    bool set_inhibit(bool inhibit);


protected:
    //! This variable needs to exist all the time because a function is
    //! returning a pointer to it.
    byte EEPROM_data_[EEPROM_length];

    /*
    not saved in EEPROM:
    */
    /*!
        Needed in case the alarm gets canceled during the same minute it started.
        Also used for Alarm_timeout.
    */
    unsigned long prev_activation_millis_ = 0;

    /*!
        This prevents alarms starting during the first minute of runtime from
        being ignored
    */
    bool first_activation_ = true;
    //! Does the same thing as `first_activation_` for ambient (`prev_millis_`)
    bool first_ambient_ = true;

    static constexpr byte current_snooze_count_inactive_ = 255;
    byte current_snooze_count_;  //!< max 9; 255 has special meaning defined above

    /*!
        Used to time snooze and as prev_activation_millis_ for ambient.
        Also needs to be initialised to prev_activation_millis_init_ to prevent
        alarms starting in the first minute of runtime being ignored.
    */
    unsigned long prev_millis_ = 0;
    bool inhibit_;
    bool snooze_status_;  //!< currently in snooze
    bool ambient_status_;  //!< ambient is active

    HALbool *lamp_;
    PWMDimmer *ambient_dimmer_;
    BuzzerManager *buzzer_;
    void(*write_EEPROM_all_)();  //!< write all alarms to EEPROM
    void(*activation_callback_)();
    void(*stop_callback_)();

    /*
    saved in the EEPROM:
    */
    HoursMinutes when_;
    AlarmEnabled enabled_;
    DaysOfWeek days_of_week_;
    Snooze snooze_;
    Signalization signalization_;

    //! Returns true if alarm is on (ringing or snooze).
    bool get_active() const
    {
        return current_snooze_count_ < current_snooze_count_inactive_;
    };


    bool ShouldTrigger(DateTime time);
    bool ShouldTriggerAmbient(DateTime time);
};


#endif
