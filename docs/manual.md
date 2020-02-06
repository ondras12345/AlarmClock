# AlarmClock Manual
This project's source code and electronic designs are available
on [Github][Github repo].


# Date and time format
All dates and times follow the ISO8601 (`YYYY-MM-DD hh:mm:ss`) standard.
However, due to limited screen space of the LCD, the main screen of the GUI
needs to use the forbidden format with the year omitted (`MM-DD`).


# Features
## Inhibit
This feature allows you to inhibit all alarms for a set amount time (see the
`Alarm_inhibit_duration` compile-time option). I plan to use it when I get up
before the alarm rings, so that I don't have to wait for the alarm to activate
nor disable it (I know that I'd forget to re-enable it).

If an alarm with option "Enabled: Single" is inhibited, it gets disabled
the same as if it started ringing normally.
The same thing applies for "Enabled: Skip": is gets enabled even if it was
inhibited.

This feature can by enabled by clicking on the `i` button in the GUI. The
button should change to uppercase `I`, indicating that the feature is on. It
disables automatically after the set amount time, but it can be manually
disabled by pressing the same button again.


## Ringing
There are 2 types of ringing: "regular" (used when the snooze button can be
used) and "last". These differ in frequency and period.

Please note that it can happen that the "last" ringing sounds even if
"regular" should be used, but this is only if there are multiple alarms active
at the same time and only one of them is stopped (e.g. it times out). Even
though the remaining alarm may have requested "regular" ringing, "last" is
used if the timed-out alarm requested it because it has a higher priority.
This is not considered a major problem and will likely not be fixed.


## Ambient LED strip
A LED strip can be connected to the device. It slowly lights up when the
alarm enables it. [PWM][Wikipedia PWM] is used for dimming. It can also be
controlled manually trough the [Serial CLI](#Serial-CLI) and
the LCD GUI (home screen).

Ambient is turned on `Alarm_ambient_dimming_duration` before the alarm
triggers, so that it already has full brightness when the alarm triggers.
This is not a full activation, and thus does not handle Single and Skip (will
be handled once the alarm triggers at the set time).  If you notice the
ambient is on, but the alarm hasn't triggered yet, you can use the stop button
to stop the ambient. This will not inhibit the full activation, you need to
use the inhibit function for that.
If the alarm only has "Signalization: Ambient" enabled, you can use the LCD
backlight to determine whether the alarm is active or not. If the backlight is
permanently on, the alarm is active.

If only one alarm is stopped (e.g. it times out), it disables ambient even
though other alarms may still be active. However, this shouldn't happen in
normal use and thus will likely not be fixed.


## Lamp
An output pin that can be used to control a lamp. It can be controlled from
the GUI (home screen), the CLI and by the alarms.


## Countdown timer
A timer that counts down and then does some events.
It can be configured trough the CLI or from the GUI.

It can operate in two modes:
- If the timed events do not contain buzzer, it just does the events and
  stops. The stop button does nothing, and the only way to revert the changes
  is to use manual control - CLI or GUI.
- If the timed events do contain buzzer, the stop button stops the ringing and
  turns everything else off.

Note: the remaining time updates each 800ms (only if the seconds in the
current time have changed since the last update), so it may seem imprecise,
but this only causes a few hundred milliseconds of error at most, which should
be negligible compared to the timed timespan.

The maximum time the timer can be set to is 18:12:15.



## Buzzer
### Melody
If you have a passive buzzer connected to the alarm clock and the feature is
enabled in Settings.h, you can define a melody that will be used. This melody
is stored in the EEPROM and can be read or written from the Serial CLI.

See [melody.md](melody.md) for more info.


# Configuration
## Compile-time
Edit `src/AlarmClock/Settings.h`.  
Do not modify `src/AlarmClock/Constants.h` unless you know what you are doing.


## Runtime
### RTC time
RTC (real-time clock) time can be set using the Serial CLI. Use the commands
`sd` and `st` (type `help` for more information).

Use the RTC screen to set date and time from the GUI. The changes are applied
once you press the apply button, so you can set the time to now + 10 s
and press the button on time to set it precisely.

The entered date is checked for validity when the apply button is pressed.
If the entered date is invalid, nothing happens.


## Alarms

| Option                          | Values               | Type  | Meaning                                       |
| ------------------------------- | -------------------- | ----- | --------------------------------------------- |
| enabled                         | (OFF\|SGL\|RPT\|SKP) | enum  | OFF, Single: only rings once, Repeat, Skip    |
| days                            | (1-7):(1\|0)         | bools | Days of the week the alarm is enabled for     |
| time                            | (0-23):(0-59)        | time  |                                               |
| snooze: time                    | (1-99)               | min   | How long is the alarm in snooze               |
| snooze: count                   | (0-9)                | count | How many times can the snooze feature be used |
| signalization: ambient          | (0-255)              |       | Ambient LED strips intensity (0 = disabled)   |
| signalization: lamp             | (0\|1)               | bool  | Is the 'lamp' output activated                |
| signalization: buzzer           | (0\|1)               | bool  | Is the buzzer activated                       |


**Notes:**  
bool: [boolean][Wikipedia boolean]  
days of week: Mo = 1, Su = 7  
Ambient LED strip dimming: [PWM][Wikipedia PWM]

If "Enabled" is set to "Skip", the next activation is inhibited. The alarm
returns to "Repeat" afterwards.


# LCD
The backlight of the LCD turns off automatically after `GUI_backlight_timeout`
milliseconds. To turn it on again, you need to press the rotary encoder
button, rotate the knob or press the snooze button. If the backlight is set to
permanent (e.g. an alarm is active), it won't turn off automatically. Once the
alarm is stopped, the backlight returns back to the timed mode.

Use the encoder to move the cursor under the item you want to change,
then press the encoder button. Boolean type values and buttons react
immediately, otherwise the cursor starts blinking and you should be able to set
the value by turning the encoder. To stop changing it, press the button again.

On the alarms screen, the change is applied immediately. This can lead to an
alarm activating while you are setting it. The changes are written to
the EEPROM once you return to the home screen.

See [screens.md](./screens.md) for more details.


# Serial CLI
There is a CLI (command line interface) available. To access it, connect to
the Arduino's USB port and use a terminal (eg. [PuTTY] or Arduino IDE's
Serial monitor). Select the correct COM port and baudrate (9600).
Individual commands (messages) must be terminated with CR or LF or both.

For info about available commands, type `help`.

For information on how to interact with the CLI programmatically, see
documentation of `AlarmClockCLI` class.

## Error codes
When a command is executed, it prints a hexadecimal error code.
If everything is ok, 'err 0x0: OK' should be printed.
If something fails, 'err 0x{n}: {explanation}' is printed.
The explanation matches the error code, but error codes should be easier to
decode in scripts.

Error code meanings can be found in `src\AlarmClock\AlarmClockCLI.h`,
the constants in the enum represent individual bits in the error code.
(An error code can contain multiple errors.)

## Autosave
There is an autosave feature that saves the changes after long inactivity
(the interval is a compile-time option - `Serial_autosave_interval`)

## Examples
### Edit alarm 0:
```
sel0
ls
en-rpt
dow2:1
sel
sav
```

Explanation:
- `sel0` selects the alarm
- `ls` lists the alarms configuration
- `en-rpt` enables the alarm
- `dow2:1` enables the alarm for Tuesdays
- `sel` deselects the alarm (optional)
- `sav` saves the changes (optional - see [Autosave](#autosave))


[Github repo]: https://github.com/ondras12345/AlarmClock
[Wikipedia PWM]: https://en.wikipedia.org/wiki/Pulse-width_modulation
[Wikipedia boolean]: https://en.wikipedia.org/wiki/Boolean_data_type
[PuTTY]: https://www.putty.org/
