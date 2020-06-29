#ifndef _ALARM_MOCKUPS_H
#define _ALARM_MOCKUPS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

extern bool lamp_status;

extern bool buzzer_status;
extern unsigned int buzzer_freq;
extern unsigned long buzzer_duration;

extern bool EEPROM_write;

extern bool activated;
extern bool stopped;

void reset_alarm_mockups();

void lamp(bool status);

void buzzerTone(unsigned int freq, unsigned long duration = 0);
void buzzerNoTone();

void writeEEPROM();

void activation_callback();
void stop_callback();


#endif
