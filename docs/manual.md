# Arduino-alarm-clock
This project's source code and electronic designs are available on [Github][Github repo].


# Features
TODO this should be in README
## Inhibit
This feature allows you to inhibit all alarms for a set time (see the
Alarm_inhibit_duration compile-time option). I plan to use it when I get up
before the alarm rings, so that I don't have to wait for the alarm to activate
nor disable it (I know that I'd forget to re-enable it).

If an alarm with option 'Enabled: Single' is inhibited, it gets disabled
the same as if it started ringing normally.

This feature can by enabled by holding the 'stop' button (long press duration
is defined by button_long_press compile-time option). It disables automatically
after the set time, but it can be manually disabled by holding the 'stop' button
4x longer than for a normal
It is indicated by the LED.

# Configuration
## Compile-time
Edit the file `src/AlarmClock/Settings.h`  
Do not modify `src/AlarmClock/Constants.h`

| Option                          | Default            | Unit  | Meaning                                     |
| ------------------------------- | ------------------ | ----- | ------------------------------------------- |
| DEBUG                           | disabled           |       | Enables debug messages if uncommented       |
| alarms_count                    | 6                  |       | Number of configurable alarms. Must be <255 |
| Alarm_regular_ringing_frequency | 1000               | Hz    | Buzzer tone frequency (regular ringing)     |
| Alarm_regular_ringing_period    | 500                | ms    | Buzzer on/off time length (regular ringing) |
| Alarm_last_ringing_frequency    | 2000               | Hz    | Buzzer tone frequency (last ringing)        |
| Alarm_last_ringing_period       | 250                | ms    | Buzzer on/off time length (last ringing)    |
| Alarm_inhibit_duration          | 60 * 60UL * 1000UL | ms    | Duration of the 'inhibit' function          |
| I2C_LCD_address                 | 0x27               |       | I2C address of the LCD                      |
| LCD_width                       | 16                 | cols  | Number of chars per line                    |
| LCD_height                      | 2                  | rows  | Number of lines                             |
| Serial_indentation_width        | 2                  | chars | Indentation width for the Serial CLI        |
| Serial_autosave_interval        | 60 * 1000UL        | ms    | Inactivity length after for autosave        |
| button_debounce_interval        | 25                 | ms    | Debounce interval for buttons.              |
| button_long_press               | 1000               | ms    | Minimal duration of a 'long press'          |

**Notes:**  
1000UL - normal 16bit int would overflow, so this needs to be an unsigned long  
Button debouncing theory: [Allaboutcircuits' article][Allaboutcircuits debounce]

### Pins
There should be no need to change the pins used.
If you really need to, make sure you choose a pin that
supports all the functions used (eg. [PWM][Arduino PWM], TimerOne, ...)

## Runtime
### RTC time
RTC time can be set using the Serial CLI. Use the commands `sd` and `st` (type
`help` for more information).

TODO LCD


## Alarms

| Option                          | Values          | Type  | Meaning                                       |
| ------------------------------- | --------------- | ----- | --------------------------------------------- |
| Enabled                         | (Off\|SGL\|RPT) | enum  | Off: disabled, Single: only rings once        |
| Days of week                    | (1-7):(1\|0)    | bools | Days of the week the alarm is enabled for     |
| Time                            | (0-23):(0-59)   | time  |                                               |
| Snooze: time                    | (1-99)          | min   | How long is the alarm in snooze               |
| Snooze: count                   | (0-9)           | count | How many times can the snooze feature be used |
| Signalization: Ambient          | (0-255)         |       | Ambient LED strips intensity (0 = disabled)   |
| Signalization: Lamp             | (0\|1)          | bool  | Is the 'lamp' output activated                |
| Signalization: Buzzer           | (0\|1)          | bool  | Is the buzzer activated                       |


**Notes:**  
bool: [boolean][Wikipedia boolean]  
days of week: Mo = 1, Su = 7  
Ambient LED strip dimming: [PWM][Wikipedia PWM]


# LCD
TODO implement


# Serial CLI
There is a CLI (command line interface) available. To access it, connect to the Arduino's USB
port and use a terminal (eg. [PuTTY] or Arduino IDE's Serial monitor). Select the correct
COM port and baudrate (9600).

For info about available commands, type `help`.

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


TODO PuTTY support

[Github repo]: https://github.com/ondras12345/Arduino-alarm-clock
[Arduino PWM]: https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
[Allaboutcircuits debounce]: https://www.allaboutcircuits.com/technical-articles/switch-bounce-how-to-deal-with-it/
[Wikipedia PWM]: https://en.wikipedia.org/wiki/Pulse-width_modulation
[Wikipedia boolean]: https://en.wikipedia.org/wiki/Boolean_data_type
[PuTTY]: https://www.putty.org/
