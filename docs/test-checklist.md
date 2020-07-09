# Checklist for tests
- [ ] Serial CLI
    - [ ] `lamp` - get lamp pin status
    - [ ] `lamp{l}` - set lamp pin status
    - [ ] `amb` - get ambient LED
    - [ ] `amb{nn}` - set ambient LED
    - [ ] `inh{i}` (with DEBUG on) - inhibit enabled|disabled
    - [ ] `inh` - inhibit: {i}
    - [ ] `sel{i}` - select
    - [ ] `sel` - deselect
    - [ ] `ls`
    - [ ] `en-sgl`
    - [ ] `en-rpt`
    - [ ] `en-skp`
    - [ ] `dis`
    - [ ] `time{h}:{m}`
    - [ ] `dow{d}:{s}`
    - [ ] `snz{t};{c}`
    - [ ] `sig{a};{l};{b}`
    - [ ] `sav` (with DEBUG on) - saved
    - [ ] `sav` (with DEBUG on) - nothing to save
    - [ ] Autosave (with DEBUG on)
    - [ ] `rtc` - get time
    - [ ] `sd{dd}.{mm}.{yy}` - set RTC date (year - 2000 !!)
    - [ ] `st{h}:{m}` - set RTC time

- [ ] Buttons
    - [ ] 'snooze' button (with DEBUG on)
    - [ ] 'stop' button (with DEBUG on)

- [ ] Alarms
    - [ ] Trigger
        - `sel{i}`
        - `rtc` - get current time
        - `time{h}:{m}` - one minute to the future
        - `dow{d}:{s}` - enable this day
        - `snz1:1`
        - `sig200;1;1`
        - `en-sgl`
        - Wait for the alarm to trigger

    - [ ] Snooze
        - Do the 'Trigger' steps
        - Wait for the alarm to trigger
        - Press 'snooze' button
        - Wait 1 minute, alarm should retrigger
        - Ringing sound should be different (last ringing)
        - Press 'stop' button - alarm should stop

    - [ ] Inhibit
        - `inh1`
        - Do the 'Trigger' steps. The alarm shouldn't trigger

- [ ] RTC
    - Get time - see 'Serial CLI'
    - Set time - see 'Serial CLI'

- [ ] Compile time options
    - [ ] Alarm count
    - [ ] Alarm ringing frequency and period
    - [ ] DEBUG
