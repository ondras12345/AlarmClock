/*!
    @file
*/

#ifndef BUZZERMANAGER_H
#define BUZZERMANAGER_H

#include "Arduino.h"

#include "Settings.h"
#include "PWMSine.h"


/*!
    @brief  Available tones and melodies.
            Higher values have higher priority.
*/
enum BuzzerTone
{
    ringing_off = 0,
    // Numbers between 0 and 100 are reserved for melodies.
    ringing_timer = 100,
    ringing_regular = 101,
    ringing_last = 102
};


/*!
    @brief  A class that manages the buzzer.
            This will also implement the melody feature TODO

    This should prevent buzzer collisions - 2 or more alarms using the buzzer
    at the same time, resulting in the buzzer being on and off for seemingly
    random periods of time with seemingly random tone frequency.

    The operation of this class is affected by the #active_buzzer
    compile-time option and the settings that define "regular" and
    "last" ringing.
*/
class BuzzerManager
{
public:

    /*!
        @brief  The constructor
        @param pin The pin the buzzer is connected to. This pin must be
                   configured as OUTPUT before loop() is first called.
                   If pin is set to 255, it is ignored and no hardware is
                   accessed. This is used for tests.
        @param sine An initialized PWMSine object. This is only used if
                    #active_buzzer is not defined.
    */
    BuzzerManager(byte pin, PWMSine& sine) : pin_(pin), sine_(sine) {};

    void loop();
    void set_ringing(BuzzerTone tone);


protected:
    byte pin_;
    PWMSine& sine_;
    bool status_ = false;
    BuzzerTone tone_ = ringing_off;
    byte on_count_ = 0;  //!< number of alarms that want the buzzer to be ringing
    unsigned long period_;  //!< duration of sound (and silence) during periodic beeping
    unsigned int freq_;
    unsigned long prev_millis_ = 0;
    unsigned long volume_prev_millis_ = 0;
    uint8_t volume_ = 0;  //!< sine wave amplitude

    void set_buzzer(bool status);

};


#endif
