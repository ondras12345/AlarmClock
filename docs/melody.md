# Melody
## EEPROM header format
Starting from EEPROM address `0x0010` (constant
`EEPROM_melodies_header_start`), 48 bytes (constant
`EEPROM_melodies_count * 3`) of space are reserved for a header that includes
information about melodies stored in the EEPROM. Each melody has 3 bytes of
space in this header:
- A 1-byte flag word with information about the melody
- 2-byte-long (16-bit) EEPROM address of the start of the melody,
  little-endian. This address must be higher than `0x0100`
  (`EEPROM_melodies_data_start`)

The 1-byte flag word contains the following flags:
- bit 0: always 1
- bit 1: 1 if the melody is enabled (if the associated address is pointing to
  a valid melody)
- bits 2 to 7: unused


## EEPROM melody record format
Melodies are stored in the built-in EEPROM. The following section describes
the format of a single melody record.

### Start and header
Start: `0xFD 0x55 0xAA`

### Tone record
- Each tone is 3 bytes long:
1. (frequency - 32 Hz) / 32 Hz
2. amplitude
3. duration / 25 ms
   duration > 0

#### Frequency
Frequency of the tone minus 32 Hz, divided by 32 Hz. The range of achievable
frequencies is 32 Hz to 8192 Hz. A value of 0x00 translates to 32 Hz, 0xFF =
8192 Hz.


#### Amplitude
Amplitude is an unsigned 8bit number (0 to 255) that signifies how loud the
tone should be. A tone with 0 amplitude is not played, the speaker remains
silent.


#### Duration
Duration of the tone divided by 25 ms. This constant was chosen because it
allows for reasonably long tones (`255 * 25 ms = 6.375 s`), while also
providing enough resolution for short tones.

Duration must be greater than 0. This allows for the use of multiple `0x00`
bytes as a record terminator.



### Stop and footer
A stop is indicated by `0x00 0x00 0x00`. Such record would be invalid since
duration must be greater than 0, so this sequence should not occur within
valid data.

Footer stars with `0xFF`, followed by a one-byte flag word with bit 0
indicating whether the melody should start over again. The next 3 bytes are an
amplitude offset that should be added for the second, third and fourth
replay. After the fourth run, the last offset is used for all following runs.
The offset cannot be smaller than 0, so the amplitude can only be increased.
Ignored if repeat flag is not set. Offset is NOT added to tones with amplitude
of 0.

If `amplitude + amplitude_offset > 255`, it is set to 255 to avoid overflow.



## Error handling
If an error is encountered in the middle of a melody (duration = 0), it is
considered the end of the melody.

If the footer is invalid, the alarm clock switches to normal beeping. This
acts as a protection against very short invalid melodies that would cause the
alarm to not make any sound at all.

If the header is invalid, the alarm clock switches to normal beeping.
