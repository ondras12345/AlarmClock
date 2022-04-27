/*!
    @file
*/

// https://github.com/PaulStoffregen/TimerOne
// version 1.1.0 (git tag 1.1)
#include "Arduino.h"
#include <TimerOne.h>

#include "PWMSine.h"
#include "sinlut.h"

volatile uint16_t tone_ramp_value;
uint8_t tone_pin;
uint8_t tone_amplitude;
//! Number of sine wave points per interrupt * 64 (*64 to increase resolution)
uint16_t tone_k;
//! If true, the output is sitting at 512 duty or generating sine wave signal.
//! Otherwise, the output  is off.
bool output_enabled;
//! True if sine wave should be generated after ramp-up.
bool ramp_next_sine;

volatile uint16_t i = 0;

//! Wrapper around Timer1.pwm, which is always_inline and wastes memory.
void Timer1_pwm(char pin, unsigned int duty)
{
    Timer1.pwm(pin, duty);
}

void timer1_ISR_sine()
{
    i += tone_k;
    //int16_t value = int8_t(pgm_read_byte_near(sin_LUT + (uint8_t)(i/64))) * tone_amplitude / 256 * 4;
    int16_t value = int8_t(pgm_read_byte_near(sin_LUT + (uint8_t)(i/64))) * tone_amplitude / 64;
    // setPwmDuty should be faster that pwm.
    Timer1.setPwmDuty(tone_pin, value + 512);
}


void timer1_ISR_ramp()
{
    tone_ramp_value++;
    if (tone_ramp_value % 4 == 0)
        Timer1.setPwmDuty(tone_pin, tone_ramp_value / 4);
    // Total ramp time is 2048 * timer1_us.
    if (tone_ramp_value >= 2048)
    {
        if (ramp_next_sine)
            Timer1.attachInterrupt(timer1_ISR_sine);
        else
            Timer1.detachInterrupt();
    }
}


//! Initialize the timer.
void PWMSine::begin()
{
    Timer1.initialize(timer1_us);
    Timer1.stop();
}


/*!
    @brief  Start producing a tone with specified frequency.

    If noTone was called previously (as opposed to silence), there is a
    duty cycle ramp-up section, which is needed to eliminate audible clicking.
    It takes timer1_us * 2048 microseconds (40 ms). During this time, no sound
    is generated. This means that you cannot generate tones this short.

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
    i = 0;  // sync start to sine zero crossing

    // Timer1.pwm needs to be called at least once before setPwmDuty can be
    // used.
    if (output_enabled)
    {
        Timer1_pwm(tone_pin, 512);
        Timer1.attachInterrupt(timer1_ISR_sine);
    }
    else
    {
        Timer1_pwm(pin, 0);
        tone_ramp_value = 0;
        ramp_next_sine = true;
        Timer1.attachInterrupt(timer1_ISR_ramp);
    }
    output_enabled = true;
    Timer1.start();
}


/*!
    @brief  Produce signal with zero amplitude on specified pin.

    This is not the same as noTone because noTone will completely stop PWM
    generation and thus cause the speaker to click.
    @param pin  Output pin.
    @see noTone
*/
void PWMSine::silence(uint8_t pin)
{
    if (output_enabled)
    {
        Timer1.detachInterrupt();
        Timer1_pwm(pin, 512);
    }
    else
    {
        tone_pin = pin;
        Timer1_pwm(pin, 0);
        tone_ramp_value = 0;
        ramp_next_sine = false;
        Timer1.attachInterrupt(timer1_ISR_ramp);
    }
    output_enabled = true;
    Timer1.start();
}


/*!
    @brief  Stop producing a tone previously started by `tone`.
    @param pin  Output pin the tone is produced on.
    @see tone
*/
void PWMSine::noTone(uint8_t pin)
{
    output_enabled = false;
    Timer1.stop();
    Timer1.disablePwm(pin);
    Timer1.detachInterrupt();
    digitalWrite(pin, LOW);
}
