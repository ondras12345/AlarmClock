# Arduino-alarm-clock
Arduino alarm clock using a DS3231 RTC and a LCD

**WARNING**: This project is still in development

# Requirements
- https://github.com/adafruit/RTClib
- https://github.com/johnrickman/LiquidCrystal_I2C
- https://github.com/thomasfredericks/Bounce2
- https://github.com/PaulStoffregen/Encoder

# Docs
See the [manual][manual].

In case of a segmentation fault error during compiling, try it again.
[See this](https://github.com/arduino/Arduino/issues/7949)
It seems to be caused by a code block in `case screen_alarms` in GUI.cpp at
line 48. New Arduino IDE (>1.8.9) should fix it.

[manual]: ./docs/manual.md
