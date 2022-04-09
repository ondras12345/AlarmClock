# Screens
Lowercase characters usually indicate disabled functions.

## Home
```
Date: 2019-06-13
Note: ISO8601 forbids omitting the year, but the date would otherwise not fit
on the display and I would still be using the old, non-standard date format,
so I think it is better this way.
Time: 23:14:11
Day of the week: 2 - Tuesday
Ambient LED: 25x (range 0-255 --> 0-25 on this screen). This indicates the
    stop value of PWMDimmer instead of the current value to avoid confusion
    when setting.
Buttons:
- b - (bell symbol) alarms
- t - (timer symbol) timer
- RTC
- i - inhibit (Off: i; On: I)
- L - lamp (Off: l; On: L)
```

|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__|0|6|-|1|3| |2| |2|3|:|1|4|:|1|1|
|__1__|b|t| |R|T|C| |i| | | | |2|5|L| |


## Alarm
```
Alarm index: 0 (0x00)
Last alarm index: 15 (0x0F)
Enabled for all days of the week, except for Saturday.
Only rings once, then disables itself (SGL). Other possible values: OFF, RPT, SKP
Rings at 6:20
Snooze: 5 minutes * 5
Signalization: ambient: 25x (range 0-255 --> 0-25 on this screen);
               lamp: 1, buzzer: standard beeping
    Buzzer can be either one of these:
    s: off
    S: standard beeping
    0: melody0
    ...
    9: melody9
    A: melody10
    ...
    F: melody15
Buttons:
- h - (home symbol) - home screen
```

|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__|h|0|/|F| |1|2|3|4|5|-|7| |S|G|L|
|__1__|0|6|:|2|0|+|0|5|*|5| | |2|5|L|B|


## Timer
```
Time remaining: 16:52:14
Events: ambient: 25x (range 0-255 --> 0-25 on this screen);
        lamp: 1; buzzer: 1
Buttons:
- h - (home symbol) - home screen
- t - (timer symbol) - start/stop the timer
```

|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__| | | | | |T|i|m|e|r| | | | | | |
|__1__|h|t| |1|6|:|5|2|:|1|4| |2|5|L|B|



## RTC
```
Date: 2019-06-13
Time: 23:14:11
Day of the week: 2 - Tuesday
Buttons:
- a - (apply symbol) - apply
- c - (cancel symbol) - cancel
```
|     |0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
|-----|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|
|__0__|c|a|R|T|C| |2| |2|3|:|1|4|:|1|1|
|__1__|2|0|1|9|-|0|6|-|1|3| | | | | | |

