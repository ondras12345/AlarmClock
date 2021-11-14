#include "PWMSine.h"


#define pin_speaker 9  // needs to be supported by TimerOne


void setup()
{
    PWMSine_setup();
    Serial.begin(9600);
}


void loop()
{
    PWMSine_tone(pin_speaker, 440, 128);
    delay(1000);
    PWMSine_tone(pin_speaker, 440, 255);
    delay(1000);
    PWMSine_noTone(pin_speaker);
    delay(2000);
}
