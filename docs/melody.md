# Melody
## EEPROM record format
### Start and header
Start: `FD DF AA`

### Tone record
- Each tone is 3 bytes long:
1. (frequency - 32 Hz) / 32 Hz
2. amplitude
3. duration / 15 ms
   duration > 0

#### Frequency
Frequency of the tone minus 32 Hz, divided by 32 Hz. The range of achievable
frequencies is 32 Hz -- 8192 Hz. The `tone` function used cannot go lower than
31 Hz, so it wouldn't make sense to have the ability to store frequencies
lower than that. A value of 0x00 translates to 32 Hz, 0xFF = 8192 Hz.


#### Amplitude
Amplitude is just a 1:1 mapping to the amplitude pin PWM duty cycle, which is
also an 8bit value.

A tone with 0 amplitude is not played, the speaker is silent.


#### Duration
Duration of the tone divided by 15 ms. This constant was chosen because it
allows for reasonably long tones (`255 * 15 ms = 3.825 s`), while also
providing enough resolution for short tones (`1 / 15 ms = 66.6 Hz`).

Duration must be greater than 0. This allows for the use of multiple 0 as
record termination.



### Stop and footer
A stop is indicated by `00 00 00`. Such record would be invalid since duration
must be greater than 0, so this sequence should not occur within valid data.

Footer stars with FF, followed by a one-byte flag word with bit 0 indicating
whether the melody should start over again. The next 3 bytes are an offset that
should be added for the second, third and forth replays. After the forth run,
the last offset is used for all following runs. The offset cannot be smaller
than 0, so the amplitude can only be increased. Ignored if repeat flag is not
set.

If amplitude + amplitude_offset > 255, it is set to 255 to avoid overflow.



## Error handling
If an error is encountered in the middle of a melody (duration = 0), it is
considered the end of the melody.

If the footer is invalid, the alarm clock switches to normal buzzing. This
acts as a protection against very short invalid melodies that would cause the
alarm to not make any sound at all.

If the header is invalid, the alarm clock switches to normal buzzing. This
allows for the melody to be temporarily disabled just by modifying the header.
