
// https://github.com/PaulStoffregen/TimerOne
// version 1.1.0 (git tag 1.1)
#include <TimerOne.h>

#include "PWMSine.h"
#include "sinlut.h"

uint8_t tone_pin;
uint8_t tone_amplitude;
uint16_t tone_k;  // number of sine wave points per interrupt * 64 (*64 to increase resolution)


void timer1_ISR()
{
    static uint16_t i = 0;
    i += tone_k;
    //int16_t value = int8_t(pgm_read_byte_near(sin_LUT + (uint8_t)(i/64))) * tone_amplitude / 256 * 4;
    int16_t value = int8_t(pgm_read_byte_near(sin_LUT + (uint8_t)(i/64))) * tone_amplitude / 64;
    Timer1.setPwmDuty(tone_pin, value + 512);
}


void PWMSine_setup()
{
    Timer1.initialize(timer1_us);
    Timer1.stop();
    Timer1.attachInterrupt(timer1_ISR);
}


void PWMSine_tone(uint8_t pin, uint16_t freq, uint8_t amplitude)
{
    tone_k = 64UL * timer1_us * 256UL / (1000000UL/freq);
    tone_pin = pin;
    tone_amplitude = amplitude;
    //Serial.print("k: ");
    //Serial.println(tone_k);
    //Serial.print("ampl: ");
    //Serial.println(amplitude);
    //Serial.flush();
    //Timer1.attachInterrupt(timer1_ISR);
    Timer1.pwm(tone_pin, 512);  // Timer1.pwm needs to be called at least once
    Timer1.start();
}


void PWMSine_noTone(uint8_t pin)
{
    (void)pin;
    Timer1.stop();
    //Timer1.detachInterrupt();  // TODO do we really need this?
}
