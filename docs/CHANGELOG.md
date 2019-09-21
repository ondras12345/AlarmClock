# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- CHANGELOG.md
- Checklist for releasing
- manual.md

### Changed
- Repository tree (source is now in `src` directory, ...)
- Checklist for tests
- Constants are now separated from user-configurable settings
- Alarm option 'Enabled' can now be Single\|Repeat\|Off.
  This unlocks the ability to configure an alarm to automatically disable after
  it rings once.  
  __WARNING__: Old enabled alarms written in EEPROM will now
  become 'Single' enabled.

### Removed

### Fixed
- Snooze count being offset by 1 (2 meant 1,
  0 wasn't handled properly ('last ringing' feature did not trigger))

## [0.0.1] - 2019-09-02
First working version
### Added
- Configurable alarms (default = 6, can be changed in `Settings.h`)
- Command line interface (CLI) for configuration (over UART)
- Compile time option for DEBUG output (printed over UART)
- Support for dimmable LED strip controlled by alarms (called 'ambient', dimming using PWM)
- Support for On/Off output controlled by alarms (called 'lamp')
- Checklist for tests (used during development)
