#!/usr/bin/env python3
"""Make a binary file with the data part of the E1M1 melody."""

# Use EEPROM-tool.py form PyAlarmClock to write the resulting binary file to
# the EEPROM (starting from a suitable address).
#
# A header needs to be created manually.

# The melody is based on
# https://vbstudio.hu/en/blog/20190330-Playing-DOOM-on-an-Arduino

from dataclasses import dataclass


@dataclass
class Tone:
    frequency: int  # in Hz
    amplitude: int  # 0-255
    duration: int  # in ms

    def to_bytes(self):
        if self.frequency < 0 or self.frequency > 256*32:
            raise ValueError(f"Invalid frequency: {self.frequency}")
        if self.amplitude < 0 or self.amplitude > 255:
            raise ValueError(f"Invalid amplitude: {self.amplitude}")
        if self.duration <= 0 or self.duration > 255*25:
            raise ValueError(f"Invalid duration: {self.duration}")
        return bytes([round((self.frequency-32)/32),
                      self.amplitude,
                      round(self.duration/25)])


notes = {
    0: {
        'C': 33,
        'CS': 35,
        'D': 37,
        'DS': 39,
        'E': 41,
        'F': 44,
        'FS': 46,
        'G': 49,
        'GS': 52,
        'A': 55,
        'AS': 58,
        'B': 62,
        },
    1: {
        'C': 65,
        'CS': 69,
        'D': 73,
        'DS': 78,
        'E': 82,
        'F': 87,
        'FS': 92,
        'G': 98,
        'GS': 104,
        'A': 110,
        'AS': 117,
        'B': 123,
        },
    2: {
        'C': 131,
        'CS': 139,
        'D': 147,
        'DS': 156,
        'E': 165,
        'F': 175,
        'FS': 185,
        'G': 196,
        'GS': 208,
        'A': 220,
        'AS': 233,
        'B': 247,
        },
    3: {
        'C': 262,
        'CS': 277,
        'D': 294,
        'DS': 311,
        'E': 330,
        'F': 349,
        'FS': 370,
        'G': 392,
        'GS': 415,
        'A': 440,
        'AS': 466,
        'B': 494,
        },
    4: {
        'C': 523,
        'CS': 554,
        'D': 587,
        'DS': 622,
        'E': 659,
        'F': 698,
        'FS': 740,
        'G': 784,
        'GS': 831,
        'A': 880,
        'AS': 932,
        'B': 988,
        },
    5: {
        'C': 1047,
        'CS': 1109,
        'D': 1175,
        'DS': 1245,
        'E': 1319,
        'F': 1397,
        'FS': 1480,
        'G': 1568,
        'GS': 1661,
        'A': 1760,
        'AS': 1865,
        'B': 1976,
        },
    6: {
        'C': 2093,
        'CS': 2217,
        'D': 2349,
        'DS': 2489,
        'E': 2637,
        'F': 2794,
        'FS': 2960,
        'G': 3136,
        'GS': 3322,
        'A': 3520,
        'AS': 3729,
        'B': 3951,
        },
    7: {
        'C': 4186,
        'CS': 4435,
        'D': 4699,
        'DS': 4978,
        'E': 5274,
        'F': 5588,
        'FS': 5920,
        'G': 6272,
        'GS': 6645,
        'A': 7040,
        'AS': 7459,
        'B': 7902,
        },
}


def note(octave, note, duration):
    return Tone(notes[octave][note], 255, duration)


def noteDoomBase(octave, duration):
    return [note(octave - 1, 'E', duration / 2),
            Tone(440, 0, duration/2),
            note(octave - 1, 'E', duration)
            ]


octave = 3
duration = 64

tones = [
    note(octave, 'B', duration),
    note(octave, 'G', duration),
    note(octave, 'E', duration),
    note(octave, 'C', duration),

    note(octave, 'E', duration),
    note(octave, 'G', duration),
    note(octave, 'B', duration),
    note(octave, 'G', duration),

    note(octave, 'B', duration),
    note(octave, 'G', duration),
    note(octave, 'E', duration),
    note(octave, 'G', duration),

    note(octave, 'B', duration),
    note(octave, 'G', duration),
    note(octave, 'B', duration),
    note(octave + 1, 'E', duration),

    *noteDoomBase(octave, duration*2),
    note(octave, 'E', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'D', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'C', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'AS', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'B', duration*2),
    note(octave, 'C', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'E', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'D', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave, 'C', duration*2),

    *noteDoomBase(octave, duration*2),
    note(octave - 1, 'AS', duration*2 * 2),
]


with open('e1m1.bin', 'wb') as f:
    f.write(bytes([0xFD, 0x55, 0xAA]))

    for tone in tones:
        f.write(tone.to_bytes())

    restart = False
    f.write(bytes([0x00, 0x00, 0x00, 0xFF,
                   (0x01 if restart else 0x00), 0x00, 0x00, 0x00]))
