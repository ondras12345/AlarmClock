# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [Unreleased]
### Added
- Support for PWM generated sine wave audio output.
- Support for melodies.
- CLI now sends the BEL character (`0x07`) every time the state of the
  hardware (ambient, lamp) or settings (inhibit) changes.

### Changed
- EEPROM data storage format. The section that contains alarms has been moved.
  All preexisting alarm configuration will be lost after upgrading to this
  version.  **WARNING** braking changes in EEPROM handling.


## [0.5.0] - 2021-11-10
### Added
- Support for backspace key in Serial CLI
- Buzzer should beep when RTC failure is detected

### Fixed
- Incompatibility with new RTClib

### Changed
- CLI rework. It should be much easier to parse. Commands now output YAML.  
  **WARNING**: breaking changes in CLI


## [0.4.1] - 2021-03-11
### Fixed
- Alarm could be missed if it's first scheduled activation is within a
  1 minute wide window at around 43 days of runtime


## [0.4.0] - 2021-03-09
### Added
- Watchdog timer (WDT)
- Ambient starts `Alarm_ambient_dimming_duration` before the alarm triggers,
  so that it already has full brightness when the alarm triggers.
- Countdown timer feature.

### Changed
- `dis` command in CLI changed to `en-off` for consistency
  WARNING: breaking API change
- CLI command recognition optimized
- Alarm indexes in GUI now start from 0 instead of 1 (to be consistent with
  CLI).
- Up to 16 alarms
- Date and time format changed to ISO8601. WARNING: CLI commands changed.
- Default `Alarm_timeout` changed from 30 minutes to 15 minutes


## [0.3.1] - 2020-07-18
### Fixed
- Alarm disables buzzer even if it did not enable it.
- Alarm disables buzzer and lamp twice when stopped in snooze.


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
