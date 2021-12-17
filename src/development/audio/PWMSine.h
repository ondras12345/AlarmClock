#ifndef PWMSINE_H
#define PWMSINE_H


// PWM period. 20 us (50 kHz) seems to be optimal for further processing.
#define timer1_us 20


void PWMSine_setup();
void PWMSine_tone(uint8_t pin, uint16_t freq, uint8_t amplitude=255);
void PWMSine_noTone(uint8_t pin);


#endif  // PWMSINE_H
