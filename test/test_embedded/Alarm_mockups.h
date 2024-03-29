#ifndef _ALARM_MOCKUPS_H
#define _ALARM_MOCKUPS_H

#include <Arduino.h>

#include <HALbool.h>
#include <PWMSine.h>
#include <BuzzerManager.h>
#include <Alarm.h>
#include <PWMDimmer.h>


extern PWMSine sine;

class MockupBuzzerManager : public BuzzerManager
{
public:
    MockupBuzzerManager() : BuzzerManager(255, sine) { };
    bool get_status() const { return status_; };
    BuzzerTone get_tone() const { return tone_; };
    void reset_mockup() { while (on_count_ > 0) set_ringing(ringing_off); }
};

class TestAlarm : public Alarm
{
public:
    TestAlarm(
        HALbool& lamp,
        PWMDimmer& ambient_dimmer,
        BuzzerManager& buzzer,
        void(*writeEEPROM)(),
        void(*activation_callback)(),
        void(*stop_callback)()
        ) : Alarm(lamp, ambient_dimmer, buzzer,
                  writeEEPROM, activation_callback, stop_callback) { };

    byte test_get_current_snooze_count() { return current_snooze_count_; };
    bool test_get_snooze_status() { return snooze_status_; };
    bool test_get_ambient_status() { return ambient_status_; };
};


class MockupPWMDimmer : public PWMDimmer
{
public:
    MockupPWMDimmer() : PWMDimmer(255) { };
};


extern HALbool lamp;
extern MockupBuzzerManager buzzer;
extern bool lamp_status;

extern bool buzzer_status;

extern bool EEPROM_write;

extern bool activated;
extern bool stopped;

void reset_alarm_mockups();

void set_lamp(bool status);

void writeEEPROM();

void activation_callback();
void stop_callback();


#endif
