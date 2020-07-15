#ifndef _ALARM_MOCKUPS_H
#define _ALARM_MOCKUPS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "src/AlarmClock/HALbool.h"
#include "src/AlarmClock/BuzzerManager.h"

class MockupBuzzerManager : public BuzzerManager
{
public:
    MockupBuzzerManager() : BuzzerManager(255) { };
    bool get_status() const { return status_; };
    BuzzerTone get_tone() const { return tone_; };

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
