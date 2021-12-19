# AlarmClock
An Arduino-compatible alarm clock with multiple configurable alarms

**WARNING**: This project is still under development.


## Features
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
- an easy to parse serial port (UART) text-based configuration interface
  ([CLI](./docs/manual.md#Serial-CLI))
- a [Python library][PyAlarmClock] for easy control from a PC and automation


## Requirements
- https://github.com/adafruit/RTClib >=1.5.0 (RTClib PR #149)
- https://github.com/johnrickman/LiquidCrystal_I2C
- https://github.com/thomasfredericks/Bounce2
- https://github.com/PaulStoffregen/Encoder


## Documentation
See the [manual][manual].

You can generate documentation for the source code using `doxygen`.


[manual]: ./docs/manual.md
[PyAlarmClock]: https://github.com/ondras12345/PyAlarmClock
