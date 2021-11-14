/*!
    @file
*/

#ifndef PWMSINE_H
#define PWMSINE_H


// PWM period. 20 us (50 kHz) seems to be optimal for further processing.
#define timer1_us 20


/*!
    @brief  PWM sine wave tone generator.

    This uses Timer1 interrupts.
    Only one instance should exist at a time. This is because it is using
    a hardware timer.
*/
class PWMSine
{
public:
    PWMSine() {};
    static void begin();
    static void tone(uint8_t pin, uint16_t freq, uint8_t amplitude=255);
    static void noTone(uint8_t pin);
};

#endif  // PWMSINE_H
