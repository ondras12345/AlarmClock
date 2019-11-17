// 
// 
// 

#include "GUI.h"

/*
Screen GUIClass::_next_screen(Screen screen)
{
    screen = Screen(screen + 1);
    if (screen == LAST) screen = Screen(0);

    return(screen);
}
*/

void GUIClass::loop(DateTime __time)
{
    _now = __time;


    /*
    Button
    */
    //_encoder_button->update();  // Already done in loop() of AlarmClock.ino
    if (_encoder_button->fell()) {
        if (_cursor_clicked) {
            _cursor_clicked = false;
        }
        else {
            if (_current_screen == screen_alarms &&
                _cursor_position != cpa_home_button) _change = true;

            switch (_current_screen) {
                // Each screen should have its own switch (_cursor_position)
                // here. This section fully handles button presses that don't
                // result in _cursor_clicked, presses resulting in
                // _cursor_clicked should be handled by `default` of each
                // switch statement by setting _cursor_clicked = true.
            case screen_home:
                switch (_cursor_position) {
                case cph_alarms_button:
                    _current_screen = screen_alarms;
                    _cursor_position = 0;
                    _encoder->write(0);
                    break;
                }
                break;


            case screen_alarms:
                Signalization prev_sig = _selected_alarm->get_signalization();
                switch (_cursor_position) {
                case cpa_home_button:
                    // # TODO make this a function
                    _current_screen = screen_home;
                    _cursor_position = 0;
                    _encoder->write(0);

                    if (_change) {
                        // Data is written to the EEPROM when returning to home
                        // screen.
                        _change = false;
                        _writeEEPROM();
                    }
                    break;

                    // Visual Studio will complain here, but gcc can handle
                    // ranges in switch-case
                    // https://geeksforgeeks.org/using-range-switch-case-cc/
                case cpa_alarm_day_1 ... (cpa_alarm_day_1 + 6):
                    byte day = _cursor_position - cpa_alarm_day_1;
                    _selected_alarm->set_day_of_week(day,
                        !_selected_alarm->get_day_of_week(day));
                    break;

                case cpa_alarm_sig_a:
                    _selected_alarm->set_signalization(!prev_sig.ambient,
                                                       prev_sig.lamp,
                                                       prev_sig.buzzer);
                    break;

                case cpa_alarm_sig_l:
                    _selected_alarm->set_signalization(prev_sig.ambient,
                                                       !prev_sig.lamp,
                                                       prev_sig.buzzer);
                    break;

                case cpa_alarm_sig_b:
                    _selected_alarm->set_signalization(prev_sig.ambient,
                                                       prev_sig.lamp,
                                                       !prev_sig.buzzer);
                    break;

                default:
                    _cursor_clicked = true;
                    _change = true;
                    break;
                }
                break;
            }
        }
        _update();
    }


    /*
    Encoder
    */
    int encoder_position = _encoder->read();
    if (abs(encoder_position) >= encoder_step) {
        float encoder_steps = encoder_position / encoder_step;
        int encoder_full_steps = int(encoder_steps);
#if defined(DEBUG) && defined(DEBUG_GUI)
        Serial.print("enc_f_steps: ");
        Serial.println(encoder_full_steps);
#endif
        _encoder->write(encoder_position - (encoder_full_steps * 4));

        if (_cursor_clicked) {
            // This section handles encoder rotations that should change some
            // values.
            switch (_current_screen) {
            case screen_home:
                break;


            case screen_alarms:
                switch (_cursor_position) {
                case cpa_alarm_index:
                    _selected_alarm_index = _apply_limits(
                        _selected_alarm_index, encoder_full_steps, 0,
                        alarms_count - 1);
                    _selected_alarm = (_alarms + _selected_alarm_index);
                    break;

                case cpa_alarm_enabled:
                    AlarmEnabled enabled = _selected_alarm->get_enabled();
                    enabled = AlarmEnabled(_apply_limits(enabled,
                        encoder_full_steps, 0, 2));
                    _selected_alarm->set_enabled(enabled);
                    break;

                case cpa_alarm_time_h:
                    hours_minutes time = _selected_alarm->get_time();
                    time.hours = _apply_limits(time.hours, encoder_full_steps,
                                               0, 23);
                    _selected_alarm->set_time(time.hours, time.minutes);
                    break;

                case cpa_alarm_time_m:
                    hours_minutes time_2 = _selected_alarm->get_time();
                    time_2.minutes = _apply_limits(time_2.minutes,
                        encoder_full_steps, 0, 59);
                    _selected_alarm->set_time(time_2.hours, time_2.minutes);
                    break;

                case cpa_alarm_snz_time:
                    Snooze snooze = _selected_alarm->get_snooze();
                    snooze.time_minutes = _apply_limits(snooze.time_minutes,
                        encoder_full_steps, 0, 99);
                    _selected_alarm->set_snooze(snooze.time_minutes, snooze.count);
                    break;

                case cpa_alarm_snz_count:
                    Snooze snooze_2 = _selected_alarm->get_snooze();
                    snooze_2.count = _apply_limits(snooze_2.count,
                        encoder_full_steps, 0, 9);
                    _selected_alarm->set_snooze(snooze_2.time_minutes, snooze_2.count);
                    break;
                }
                break;
            }

        }
        else {
            // Move the cursor
            _cursor_position = _apply_limits(_cursor_position,
                encoder_full_steps, 0, _selectables_count[_current_screen] - 1);
#if defined(DEBUG) && defined(DEBUG_GUI)
            Serial.print("cpos: ");
            Serial.println(_cursor_position);

            Serial.print("sc: ");
            Serial.println(_selectables_count[_current_screen] - 1);

            Serial.print("cs: ");
            Serial.println(_current_screen);
#endif
        }
        _update();
    }
}

byte GUIClass::_apply_limits(byte value, int step, byte limit_low, byte limit_high)
{
    // Byte cannot be lower than 0, so I have to check before adding step to
    // the value to be able to detect limit_low
    if (step < 0 && -step > value)
        return limit_low;
    else {
        if ((value + step) > limit_high)
            return limit_high;
        else return byte(value + step);
    }
}

void GUIClass::_update()
{
    _lcd->noCursor();
    _lcd->noBlink();
    switch (_current_screen) {
    case screen_home:
        _lcd->setCursor(0, 0);
        sprintf(_line_buffer, "%02d.%02d %d %02d:%02d:%02d",
                _now.day(), _now.month(),
                _now.dayOfTheWeek() == 0 ? 7 : _now.dayOfTheWeek(),
                _now.hour(), _now.minute(), _now.second());
        _lcd->print(_line_buffer);
        _lcd->setCursor(0, 1);
        sprintf(_line_buffer, "%c               ", char(LCD_char_bell_index));
        _lcd->print(_line_buffer);

        break;


    case screen_alarms:
        _lcd->setCursor(0, 0);

        char days_of_week[8] = "";  // 7 + \0
        days_of_week[7] = '\0';
        for (byte i = 1; i <= 7; i++) {
            if (_selected_alarm->get_days_of_week().getDayOfWeek(i))
                days_of_week[i - 1] = char('0' + i);
            else days_of_week[i - 1] = ' ';
        }

        char enabled[4] = "";
        switch (_selected_alarm->get_enabled()) {
        case Off:
            strcpy(enabled, "Off");
            break;

        case Single:
            strcpy(enabled, "SGL");
            break;

        case Repeat:
            strcpy(enabled, "RPT");
            break;
#if defined(DEBUG) && defined(DEBUG_GUI)
        default:
            strcpy(enabled, "err");
            break;
#endif
        }
#if defined(DEBUG) && defined(DEBUG_GUI)
        Serial.print(F("enabled: "));
        Serial.println(enabled);
#endif

        sprintf(_line_buffer, "%c%d/%d %s %s",
                char(LCD_char_home_index), _selected_alarm_index + 1, alarms_count,
                days_of_week, enabled );
        _lcd->print(_line_buffer);

        _lcd->setCursor(0, 1);
        sprintf(_line_buffer, "%02d:%02d+%02d*%d   %c%c%c",
                _selected_alarm->get_time().hours,
                _selected_alarm->get_time().minutes, 
                _selected_alarm->get_snooze().time_minutes,
                _selected_alarm->get_snooze().count,
                _selected_alarm->get_signalization().ambient ? 'A' : ' ',
                _selected_alarm->get_signalization().lamp ? 'L' : ' ', 
                _selected_alarm->get_signalization().buzzer ? 'B' : ' ' );
        _lcd->print(_line_buffer);

        break;


    }

    _lcd->setCursor(_cursor_positions[_current_screen][_cursor_position].column,
                    _cursor_positions[_current_screen][_cursor_position].row);
    _lcd->cursor();
    if (_cursor_clicked) _lcd->blink();
}

GUIClass::GUIClass(AlarmClass *__alarms, void(*__writeEEPROM)(), RTC_DS3231 * __rtc, Encoder * __encoder, Bounce * __encoder_button, LiquidCrystal_I2C *__lcd)
{
    _alarms = __alarms;
    _writeEEPROM = __writeEEPROM;
    _rtc = __rtc;
    _encoder = __encoder;
    _encoder_button = __encoder_button;
    _lcd = __lcd;
}
