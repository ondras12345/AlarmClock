# Screens
Lowercase characters usually indicate disabled functions.

## Home
```
Date: 11. 6. 2019
Time: 23:14:11
Day of the week: 2 - Tuesday
Ambient LED: 25x (range 0-255 --> 0-25 on this screen). This indicates the
    stop value of PWMDimmer instead of the current value to avoid confusion
    when setting.
Buttons:
- b - (bell symbol) alarms
- t - (timer symbol) timer (not implemented yet)
- RTC
- i - inhibit (Off: i; On: I)
- L - lamp (Off: l; On: L)
```

|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__|1|1|.|0|6| |2| |2|3|:|1|4|:|1|1|
|__1__|b|t| |R|T|C| |i| | | | |2|5|L| |


## Alarm
```
Alarm index: 0 (0x00)
Last alarm index: 15 (0x0F)
Enabled for all days of the week
Only rings once, then disables itself (SGL). Other possible values: Off, RPT
Rings at 6:20
Snooze: 5 * 5 minutes
Signalization: ambient: 25x (range 0-255 --> 0-25 on this screen);
               lamp: 1, buzzer: 1
Buttons:
- h - (home symbol) - home screen
```

|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__|h|0|/|F| |1|2|3|4|5|6|7| |S|G|L|
|__1__|0|6|:|2|0|+|0|5|*|5| | |2|5|L|B|


## RTC
```
Date: 11. 6. 2019
Time: 23:14:11
Day of the week: 2 - Tuesday
Buttons:
- a - (apply symbol) - apply
- c - (cancel symbol) - cancel
```
|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__|c|a|R|T|C| |2| |2|3|:|1|4|:|1|1|
|__1__|1|1|.|0|6|.|2|0|1|9| | | | | | |

