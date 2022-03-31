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


## Building
Use [PlatformIO][PlatformIO] to build the firmware. It will handle the
dependencies automatically. Just run
```sh
# update libraries
make update

# compile
make

# upload
make upload
```

For more information, type `make help`.

Building with Arduino IDE should be possible, but you might need to create a
few symlinks. I use PlatformIO exclusively for my testing, so Arduino IDE
should be considered unsupported.


## Testing
Static code analysis is performed on the computer used for development using
`cppcheck`.

Native unit tests run natively on the computer used for development.

Not everything can be tested that way, so additional tests were written that
need to run on the target microcontroller (embedded device). A new firmware is
uploaded to the device and test results are received through UART.
```
# run static code analysis:
make check

# run native tests:
make test

# run tests on embedded device:
make test_embedded
```


## Documentation
See the [manual][manual].

You can generate documentation for the source code using `doxygen`.
Just type `make docs`.


[manual]: ./docs/manual.md
[PyAlarmClock]: https://github.com/ondras12345/PyAlarmClock
[PlatformIO]: https://platformio.org/
