# Arduino-alarm-clock
Arduino alarm clock using a DS3231 RTC and a LCD

**WARNING**: This project is still under development.


# Features
- configurable number (default 6, max 16) of
  [configurable](./docs/manual.md#Alarms) alarms
- snooze feature with configurable time and count
- a different [ringing](./docs/manual.md#Ringing) tone when snooze cannot be
  used (last ringing)
- a LED strip that slowly lights up before the alarm sounds
  ([ambient](./docs/manual.md#Ambient-LED-strip))
- 2 separate buttons (snooze and stop). Stop can be installed further away
  from the bed, so that the user cannot reach it while still laying.
- a 16x2 character LCD with a rotary encoder for configuration
  ([GUI](./docs/manual.md#LCD))
- a serial port (UART) text-based configuration interface
  ([CLI](./docs/manual.md#Serial-CLI))

## TODO
- buzzer loudness ramp-up
- melodies for the buzzer
- up to 15 alarms - HEX in GUI


# Requirements
- https://github.com/adafruit/RTClib >=1.5.0 (RTClib PR #149)
- https://github.com/johnrickman/LiquidCrystal_I2C
- https://github.com/thomasfredericks/Bounce2
- https://github.com/PaulStoffregen/Encoder


# Docs
See the [manual][manual].


[manual]: ./docs/manual.md
