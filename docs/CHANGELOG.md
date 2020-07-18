# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [Unreleased]
### Fixed
- Alarm disables buzzer even if it did not enable it.


## [0.3.0] - 2020-07-16
### Added
- Unit tests
- Doxygen documentation (incomplete)
- "Skip" added to Alarm "Enabled" options
- Timeout for alarms - the alarm stops if it is active for too long
- User input validation for the date supplied to the `sd` command and in the
  RTC section of the GUI.

### Changed
- Better LCD backlight control

### Fixed
- Buzzer collisions - multiple alarms active at the same time caused
  unpredictable behavior of the buzzer


## [0.2.1] - 2020-02-14
### Fixed
- Alarm signalization option 'lamp' could not be unset due to a command
  processing error


## [0.2.0] - 2020-02-14
### Added
- CHANGELOG.md
- Checklist for releasing
- manual.md
- GUI - 16x2 LCD display and rotary encoder  
  __WARNING__: New pinout: 2, 3, A2 - encoder; lamp is now on pin 10
    - Inhibit function (see manual)
- Mode for active buzzers
- CLI commands for controlling the ambient LED, fading reworked
- GUI interface for controlling the ambient LED
- CLI commands for controlling the lamp pin
- GUI button for controlling the lamp pin
- CLI commands for controlling alarm inhibit

### Changed
- Repository tree (source is now in `src` directory, ...)
- Checklist for tests
- Constants are now separated from user-configurable settings
- Alarm option 'Enabled' can now be Single\|Repeat\|Off.
  This unlocks the ability to configure an alarm to automatically disable after
  it rings once.  
  __WARNING__: Old enabled alarms written in EEPROM will now
  become 'Single' enabled.
- Alarm record format in the EEPROM  
  __WARNING__: This breaks old EEPROM records and may perform a factory reset
  when first used, because minutes and/or hours of the old record may exceed
  the limits (23 h 59 min) when interpreted as the new format.
- Serial CLI now requires commands (messages) to be terminated by CR, LF or
  CRLF
- Serial CLI error codes reworked
- Setting descriptions are now in `src/AlarmClock/Settings.h`

### Removed
- Long forms of some commands in Serial CLI
- Visual Studio support (I don't use it anymore, so I can't update the files)

### Fixed
- Snooze count being offset by 1 (2 meant 1,
  0 wasn't handled properly ('last ringing' feature did not trigger))
- `dow0;1` not failing although it should
- Arduino requesting current time from the DS3231 with very small intervals
- Comparison of results of floating point math using `==` in PWMfade
- PWMfade (ambient LED dimmer) should now be more fluent
- PWMfade renamed to PWMDimmer


## [0.1.0] - 2019-09-02
First working version
### Added
- Configurable alarms (default = 6, can be changed in `Settings.h`)
- Command line interface (CLI) for configuration (over UART)
- Compile time option for DEBUG output (printed over UART)
- Support for dimmable LED strip controlled by alarms (called 'ambient',
  dimming using PWM)
- Support for On/Off output controlled by alarms (called 'lamp')
- Checklist for tests (used during development)
