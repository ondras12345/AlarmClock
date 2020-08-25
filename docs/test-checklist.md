# Checklist for tests
- [ ] Serial CLI
    - [ ] All commands listed by `help`
    - [ ] `sel` - deselect
    - [ ] `sav` (with DEBUG on) - saved
    - [ ] `sav` (with DEBUG on) - nothing to save
    - [ ] Autosave (with DEBUG on)

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

    - [ ] Timeout
        - [ ] Does the alarm time out?
        - [ ] When the alarm times out, does it break other alarms' lamp and
              ambient? (it shouldn't, but ambient is not solved yet)

    - [ ] Multiple alarm active at the same time
        - [ ] Do they fight over the buzzer? (they shouldn't)

- [ ] RTC
    - Get time - see 'Serial CLI'
    - Set time - see 'Serial CLI'

- [ ] Compile time options
    - [ ] Alarm count
    - [ ] Alarm ringing frequency and period
    - [ ] DEBUG
