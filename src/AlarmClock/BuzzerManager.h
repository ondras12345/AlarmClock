/*!
    @file
*/

#ifndef BUZZERMANAGER_H
#define BUZZERMANAGER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


#include "Settings.h"


/*!
    @brief  Available tones and melodies.
            Higher values have higher priority.
*/
enum BuzzerTone
{
    ringing_off = 0,
    // Numbers between 0 and 100 are reserved for melodies.
    ringing_regular = 100,
    ringing_last = 101
};


/*!
    @brief  A class that manages the buzzer.
            This will also implement the melody feature TODO

    This should prevent buzzer collisions - 2 or more alarms using the buzzer
    at the same time, resulting in the buzzer being on and off for seemingly
    random periods of time with seemingly random tone frequency.

    The operation of this class is affected by the `active_buzzer`
    compile-time option and the settings that define "regular" and
    "last" ringing.
*/
class BuzzerManager
{
public:
    BuzzerManager(byte pin);
    void loop();
    void set_ringing(BuzzerTone tone);


protected:
    byte pin_;
    bool status_ = false;
    BuzzerTone tone_ = ringing_off;
    byte on_count_ = 0; //!< number of alarms that want the buzzer to be ringing
    unsigned long period_;
    unsigned int freq_;
    unsigned long prev_millis_ = 0;

    void set_buzzer(bool status);

};


#endif
