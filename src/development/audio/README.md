# Audio
- 2021-11-12


## MusicWithoutDelay 3.5.2
- https://github.com/nathanRamaNoodles/Noodle-Synth


### Circuit
- Oscilloscope and/or speaker with series resistance on pin 11.
  Use the recommended RC filter (see link above)


### Program
- I tried the `Basic` and `volume` examples, use `CHA` instead of `CHB`.
- Seems to work really well, but won't fit in flash:

AC on UNO without this lib:
```
Sketch uses 26944 bytes (83%) of program storage space. Maximum is 32256 bytes.
Global variables use 1411 bytes (68%) of dynamic memory, leaving 637 bytes for local variables. Maximum is 2048 bytes.
```

with include:
```
Sketch uses 26974 bytes (83%) of program storage space. Maximum is 32256 bytes.
Global variables use 1411 bytes (68%) of dynamic memory, leaving 637 bytes for local variables. Maximum is 2048 bytes.
```

with basic example:
```
text section exceeds available space in board
Sketch uses 35652 bytes (110%) of program storage space. Maximum is 32256 bytes.
Global variables use 1815 bytes (88%) of dynamic memory, leaving 233 bytes for local variables. Maximum is 2048 bytes.
Sketch too big; see https://support.arduino.cc/hc/en-us/articles/360013825179 for tips on reducing it.
Error compiling for board Arduino Uno.
```



## Mozzi
- https://sensorium.github.io/Mozzi/
- I haven't even tested this because it seems to have an incompatible license.




## Own version
- I couldn't find anything that would be small and useful, so I wrote my own.
- Only sine waves are supported.
- Volume can be controlled.
- I was doing all of my testing with an analog amplifier based on TDA2030,
  I was feeding the signal in through this circuit:

```
                                             _______
            ____        ____                |       |
pin 9  ----|____|---*--|____|---*-------*---| amp   |
            1k      |   10k     |       |   |_______|
                   ---         | |     ---
                   --- 10n     | | 1k  --- 10n
                    |           |       |
                   ---         ---     ---
                   GND         GND     GND
```
