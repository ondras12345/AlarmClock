#include "Alarm_mockups.h"


bool lamp_status;
bool buzzer_status;
unsigned int buzzer_freq;
unsigned long buzzer_duration;
bool EEPROM_write;
bool activated;
bool stopped;


void lamp(bool status) { lamp_status = status; }
void writeEEPROM() { EEPROM_write = true; }
void activation_callback() { activated = true; }
void stop_callback() { stopped = true; }

void buzzerTone(unsigned int freq, unsigned long duration)
{
    buzzer_status = true;
    buzzer_freq = freq;
    buzzer_duration = duration;
}

void buzzerNoTone()
{
    buzzer_status = false;
}

void reset_alarm_mockups()
{
    lamp_status = false;
    buzzer_status = false;
    buzzer_freq = 0;
    buzzer_duration = 0;
    EEPROM_write = false;
    activated = false;
    stopped = false;
}
