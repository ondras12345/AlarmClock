#include "Alarm_mockups.h"

PWMSine sine;
HALbool lamp(set_lamp);
MockupBuzzerManager buzzer;
bool lamp_status;
bool EEPROM_write;
bool activated;
bool stopped;


void set_lamp(bool status) { lamp_status = status; }
void writeEEPROM() { EEPROM_write = true; }
void activation_callback() { activated = true; }
void stop_callback() { stopped = true; }


void reset_alarm_mockups()
{
    lamp = HALbool(set_lamp);
    buzzer.reset_mockup();
    lamp_status = false;
    EEPROM_write = false;
    activated = false;
    stopped = false;
}
