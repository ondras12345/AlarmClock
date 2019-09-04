# Arduino-alarm-clock
This project's source code and electronic designs are available on [Github][Github repo].

# Features
TODO this should be in README

# Configuration
## Compile-time
Edit the file `src/AlarmClock/Settings.h`  
Do not modify `src/AlarmClock/Constants.h`

| Option                          | Default     | Unit | Meaning                                     |
| ------------------------------- | ----------- | ---- | ------------------------------------------- |
| DEBUG                           | disabled    |      | Enables debug messages if uncommented       |
| alarms_count                    | 6           |      | Number of configurable alarms. Must be <255 |
| Alarm_regular_ringing_frequency | 1000        | Hz   | Buzzer tone frequency (regular ringing)     |
| Alarm_regular_ringing_period    | 500         | ms   | Buzzer on/off time length (regular ringing) |
| Alarm_last_ringing_frequency    | 2000        | Hz   | Buzzer tone frequency (last ringing)        |
| Alarm_last_ringing_period       | 250         | ms   | Buzzer on/off time length (last ringing)    |
| I2C_LCD_address                 | 0x27        |      | I2C address of the LCD                      |
| LCD_width                       | 16          | Cols | Number of chars per line                    |
| LCD_height                      | 2           | Rows | Number of lines                             |
| Serial_autosave_interval        | 60 * 1000UL | ms   | Inactivity length after for autosave        |

Notes:  
1000UL - normal 16bit int would overflow, so this needs to be an unsigned long

## Runtime
There are currently no runtime configurable parameters (except for alarms).

## Alarms
TODO Enabled - SIG, REP

| Option                          | Values        | Type  | Meaning                                       |
| ------------------------------- | ------------- | ----- | --------------------------------------------- |
| Enabled                         | (1\|0)        | bool  |                                               |
| Days of week                    | (1-7):(1\|0)  | bools | Days of the week the alarm is enabled for     |
| Time                            | (0-23):(0-59) | time  |                                               |
| Snooze: time                    | (1-99)        | min   | How long is the alarm in snooze               |
| Snooze: count                   | (0-9)         | count | How many times can the snooze feature be used |
| Signalization: Ambient          | (0-255)       |       | Ambient LED strips intensity (0 = disabled)   |
| Signalization: Lamp             | (0\|1)        | bool  | Is the 'lamp' output activated                |
| Signalization: Buzzer           | (0\|1)        | bool  | Is the buzzer activated                       |


Notes:  
bool: [boolean](https://en.wikipedia.org/wiki/Boolean_data_type)  
days of week: Mo = 1, Su = 7  
Ambient LED strip dimming: [PWM](https://en.wikipedia.org/wiki/Pulse-width_modulation)

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
en
dow2:1
sel
sav
```

Explanation:
- `sel0` selects the alarm
- `ls` lists the alarms configuration
- `en` enables the alarm
- `dow2:1` enables the alarm for Tuesdays
- `sel` deselects the alarm (optional)
- `sav` saves the changes (optional - see [Autosave](#autosave))


TODO PuTTY support

[Github repo]: https://github.com/ondras12345/Arduino-alarm-clock
[PuTTY]: https://www.putty.org/
