/*!
    @file
*/

#ifndef BUZZERMANAGER_H
#define BUZZERMANAGER_H

#include <Arduino.h>

#include "Settings.h"
#include <PWMSine.h>


/*!
    @brief  Available tones and melodies.
            Higher values have higher priority.
*/
enum BuzzerTone
{
    ringing_off = 0,  //!< Not ringing.

    //! Numbers between 0 and 100, exclusive, are reserved for melodies.
    //! ringing_melody0 to ringing_melody15 must be contiguous. This is to make
    //! sure that arithmetics can be used (e.g. ringing_melody0 + melody_id).
    ringing_melody0 = 10,
    ringing_melody1 = 11,
    ringing_melody2 = 12,
    ringing_melody3 = 13,
    ringing_melody4 = 14,
    ringing_melody5 = 15,
    ringing_melody6 = 16,
    ringing_melody7 = 17,
    ringing_melody8 = 18,
    ringing_melody9 = 19,
    ringing_melody10 = 20,
    ringing_melody11 = 21,
    ringing_melody12 = 22,
    ringing_melody13 = 23,
    ringing_melody14 = 24,
    ringing_melody15 = 25,

    ringing_timer = 100,  //!< Ringing requested by the countdown timer.
    ringing_regular = 101,  //!< Regular ringing.
    ringing_last = 102  //!< Last ringing.
};


/*!
    @brief  A class that manages the buzzer and plays melodies.

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
    bool status_ = false;  //!< state of the buzzer (used in beeping mode)
    BuzzerTone tone_ = ringing_off;
    //! Number of alarms that want the buzzer to be ringing.
    byte on_count_ = 0;
    //! Duration of sound (and silence) during periodic beeping.
    unsigned long period_;
    unsigned int freq_;
    unsigned long prev_millis_ = 0;
    unsigned long volume_prev_millis_ = 0;
    uint8_t volume_ = 0;  //!< Sine wave amplitude. Ignored by melodies.
    bool melody_ = false;  //!< currently playing a melody
    unsigned long melody_prev_millis_ = 0;
    //! EEPROM address of the next tone to be read.
    //! Only used (and only valid) if tone_ is a melody.
    //! int (signed) because that's what Arduino uses for EEPROM addresses.
    int next_tone_address_ = 0;
    //! Address of the first tone after the melody data's start/header.
    int melody_start_address_ = 0;
    //! How long the current tone should be played.
    unsigned long melody_tone_duration_ = 0;
    //! Amplitude offset added during this replay.
    //! Successive replays of the melody can have different amplitudes.
    uint8_t melody_amplitude_offset_ = 0;
    //! Number of times the melody has been replayed.
    //! This will always be less than or equal to 3, because
    //! melody_amplitude_offset_ is only defined for the first four runs.
    //! The value during the first play is 0.
    uint8_t melody_replay_ = 0;

    void set_buzzer(bool status, bool amp_off=false);
    bool play_melody(byte id);
    void set_tone(BuzzerTone tone);
    void melody_fail();
};


#endif
