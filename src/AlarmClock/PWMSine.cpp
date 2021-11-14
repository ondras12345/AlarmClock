/*!
    @file
*/

// https://github.com/PaulStoffregen/TimerOne
// version 1.1.0 (git tag 1.1)
#include "Arduino.h"
#include <TimerOne.h>

#include "PWMSine.h"
#include "sinlut.h"

uint8_t tone_pin;
uint8_t tone_amplitude;
//! Number of sine wave points per interrupt * 64 (*64 to increase resolution)
uint16_t tone_k;


void timer1_ISR()
{
    static uint16_t i = 0;
    i += tone_k;
    //int16_t value = int8_t(pgm_read_byte_near(sin_LUT + (uint8_t)(i/64))) * tone_amplitude / 256 * 4;
    int16_t value = int8_t(pgm_read_byte_near(sin_LUT + (uint8_t)(i/64))) * tone_amplitude / 64;
    // setPwmDuty should be faster that pwm.
    Timer1.setPwmDuty(tone_pin, value + 512);
}


//! Initialize the timer.
void PWMSine::begin()
{
    Timer1.initialize(timer1_us);
    Timer1.stop();
    Timer1.attachInterrupt(timer1_ISR);
}


/*!
    @brief  Start producing a tone with specified frequency.

    @param pin  Output pin to produce the tone on. This pin needs to be
                supported by Timer1 - either 9 or 10 on Arduino UNO.
                WARNING: E.g. Arduino Mega uses different pins for Timer1.
                Only one of the pins can be used at a time.
    @param freq     Tone frequency. Must not be 0.
    @param amplitude    Amplitude of the output signal. 0 to 255.
*/
void PWMSine::tone(uint8_t pin, uint16_t freq, uint8_t amplitude)
{
    if (freq == 0) return;

    tone_k = 64UL * timer1_us * 256UL / (1000000UL/freq);
    tone_pin = pin;
    tone_amplitude = amplitude;

    // Timer1.pwm needs to be called at least once before setPwmDuty can be
    // used.
    Timer1.pwm(tone_pin, 512);
    Timer1.start();
}


/*!
    @brief  Stop producing a tone previously started by `tone`.
    @param pin  Output pin the tone is produced on.
    @see tone
*/
void PWMSine::noTone(uint8_t pin)
{
    Timer1.stop();
    Timer1.disablePwm(pin);
    digitalWrite(pin, LOW);
}
