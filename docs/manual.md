# Arduino-alarm-clock
This project's source code and electronic designs are available
on [Github][Github repo].

# Date and time format
All the dates and times are in the `dd.mm.yyyy hh:mm:ss` format.

# Features
## Inhibit
This feature allows you to inhibit all alarms for a set time (see the
Alarm_inhibit_duration compile-time option). I plan to use it when I get up
before the alarm rings, so that I don't have to wait for the alarm to activate
nor disable it (I know that I'd forget to re-enable it).

If an alarm with option 'Enabled: Single' is inhibited, it gets disabled
the same as if it started ringing normally.

This feature can by enabled by clicking on the `i` button in the GUI. The
button should change to uppercase `I`, indicating that the feature is on. It
disables automatically after the set time, but it can be manually disabled by
pressing the same button again.


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
| Serial_indentation_width        | 2                  | chars | Indentation width for the Serial CLI        |
| Serial_autosave_interval        | 60 * 1000UL        | ms    | Inactivity length after for autosave        |
| GUI_update_interval             | 9000               | ms    | Delay between screen updates - see note     |
| button_debounce_interval        | 25                 | ms    | Debounce interval for buttons.              |
| button_long_press               | 1000               | ms    | Unused: Minimal duration of a 'long press'  |
| encoder_step                    | 4                  |       | Number of pulses the encoder send per step  |

**Notes:**  
Individual DEBUG_* options have no effect if main DEBUG is disabled  
1000UL - normal 16bit int would overflow, so this needs to be an unsigned long  
GUI_update_interval - the screen only updates if seconds % 10 = 0, but I need
                      this parameter to avoid updating it multiple times during
                      the same second.  
Button debouncing theory: [Allaboutcircuits' article][Allaboutcircuits debounce

### Pins
There should be no need to change the pins used.
If you really need to, make sure you choose a pin that
supports all the functions used (eg. [PWM][Arduino PWM], TimerOne, ...)

## Runtime
### RTC time
RTC time can be set using the Serial CLI. Use the commands `sd` and `st` (type
`help` for more information).

Use the RTC screen to set date and time from the GUI. The changes are applied
once you press the apply button, so you can set the time to now + 10 s
and press the button on time to set it precisely.

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
Use the encoder to move the cursor under the item you want to change,
then press the encoder button. Boolean type values and buttons react
immediately, otherwise the cursor starts blinking and you should be able to set
the value by turning the encoder. To stop changing it, press the button again.

On the alarms screen, the change is applied immediately. This can lead to an
alarm activating while you are setting it. The changes are written to
the EEPROM once you return to the home screen.

From the DS3231 datasheet: "Illogical time and date entries result in undefined
operation". The software does user input checking, but I couldn't implement it
for days of the month, so please do not set dates such as 31. 11. 2019, because
November only has 30 days. This may eventually get fixed in the RTClib library,
see https://github.com/adafruit/RTClib/issues/127 . It also applies to the CLI.

See [screens.md](./screens.md) for more details.

# Serial CLI
There is a CLI (command line interface) available. To access it, connect to
the Arduino's USB port and use a terminal (eg. [PuTTY] or Arduino IDE's
Serial monitor). Select the correct COM port and baudrate (9600).
Individual commands (messages) must be terminated with CR or LF or both.

For info about available commands, type `help`.

## Error codes
When a command is executed, it prints an error code. If everything is ok,
'err 0: OK' should be printed. If something fails, 'err {n}: {explanation}'
is printed. The explanation always matches the error code, but error codes
should be easier to decode in scripts.

Error code meanings can be found in `src/AlarmClock/Constants.h` (do not
modify!), the #defines represent individual bits in the error code. (An error
code can contain multiple errors.)

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


[Github repo]: https://github.com/ondras12345/Arduino-alarm-clock
[Arduino PWM]: https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
[Allaboutcircuits debounce]: https://www.allaboutcircuits.com/technical-articles/switch-bounce-how-to-deal-with-it/
[Wikipedia PWM]: https://en.wikipedia.org/wiki/Pulse-width_modulation
[Wikipedia boolean]: https://en.wikipedia.org/wiki/Boolean_data_type
[PuTTY]: https://www.putty.org/
