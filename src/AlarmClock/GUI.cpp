/*!
    @file
*/

#include "GUI.h"

void GUI::set_backlight(backlight_t status)
{
    backlight_ = status;
    if (backlight_ == off) lcd_->noBacklight();
    else lcd_->backlight();
    backlight_prev_millis_ = millis();
}


void GUI::loop(DateTime time)
{
    now_ = time;

    /*
    LCD Backlight
    */
    if (backlight_ == on &&
        (unsigned long)(millis() - backlight_prev_millis_) >= GUI_backlight_timeout)
    {
        set_backlight(off);
    }

    if (encoder_button_->fell() || abs(encoder_->read()) >= encoder_step)
    {
        backlight_prev_millis_ = millis();
        if (backlight_ == off)
        {
            set_backlight(on);
            encoder_->write(encoder_->read() % encoder_step);
        }
        else
        {
            /*
            Button
            */
            //encoder_button_->update();  // Already done in loop() of AlarmClock.ino
            if (encoder_button_->fell())
            {
                if (cursor_clicked_)
                {
                    cursor_clicked_ = false;
                }
                else
                {
                    if (current_screen_ == screen_alarms &&
                        cursor_position_ != cpa_home_button &&
                        cursor_position_ != cpa_alarm_index)
                    {
                        change_ = true;
                    }

                    switch (current_screen_)
                    {
                        // Each screen should have its own switch
                        // (cursor_position_) here. This section fully handles
                        // button presses that don't result in cursor_clicked_,
                        // presses resulting in cursor_clicked_ should be
                        // handled by `default` of each switch statement by
                        // setting cursor_clicked_ = true.
                    case screen_home:
                        switch (cursor_position_)
                        {
                        case cph_alarms_button:
                            switch_screen_(screen_alarms);
                            break;

                        case cph_RTC_button:
                            switch_screen_(screen_RTC);
                            RTC_set_ = now_;
                            break;

                        case cph_inhibit_button:
                            set_inhibit_(!get_inhibit_());
                            break;

                        case cph_lamp:
                            lamp_->set_manu(!lamp_->get());
                            break;

                        default:
                            cursor_clicked_ = true;
                            break;
                        }
                        break;


                    case screen_alarms:
                    {
                        Signalization prev_sig = sel_alarm_->get_signalization();

                        switch (cursor_position_)
                        {
                        case cpa_home_button:
                            switch_screen_(screen_home);
                            if (change_)
                            {
                                // Data is written to the EEPROM when returning
                                // to home screen.
                                change_ = false;
                                writeEEPROM_();
                            }
                            break;

                        // Visual Studio will complain here, but gcc should
                        // handle ranges in switch-case
                        // https://geeksforgeeks.org/using-range-switch-case-cc/
                        case cpa_alarm_day_1 ... (cpa_alarm_day_1 + 6):
                            // 1 = Monday; 7 = Sunday
                            // I need a code block here (see
                            // https://forum.arduino.cc/index.php?topic=64407.0 )
                            //
                            // It was causing all the other cases not to
                            // execute without the block.
                            //
                            // Code block is necessary if local variables are
                            // declared within a case
                        {
                            byte day = (cursor_position_ - cpa_alarm_day_1) + 1;
                            sel_alarm_->set_day_of_week(day,
                                !sel_alarm_->get_day_of_week(day));
                        }
                            break;

                        //case cpa_alarm_sig_a: // multiple steps --> encoder

                        case cpa_alarm_sig_l:
                            sel_alarm_->set_signalization(prev_sig.ambient,
                                                          !prev_sig.lamp,
                                                          prev_sig.buzzer);
                            break;

                        case cpa_alarm_sig_b:
                            sel_alarm_->set_signalization(prev_sig.ambient,
                                                          prev_sig.lamp,
                                                          !prev_sig.buzzer);
                            break;

                        default:
                            cursor_clicked_ = true;
                            break;
                        }
                    }
                        break;


                    case screen_RTC:
                        switch (cursor_position_)
                        {
                        case cpr_cancel_button:
                            switch_screen_(screen_home);
                            break;

                        case cpr_apply_button:
                            if (RTC_set_.isValid())
                            {
                                rtc_->adjust(RTC_set_);
                                switch_screen_(screen_home);
                            }
                            break;

                        default:
                            cursor_clicked_ = true;
                            break;
                        }
                        break;
                    }
                }
#if defined(DEBUG) && defined(DEBUG_encoder)
                Serial.print("cc: ");
                Serial.println(cursor_clicked_);
#endif

                update_();
            }


            /*
            Encoder
            */
            int encoder_position = encoder_->read();
            if (abs(encoder_position) >= encoder_step)
            {
                encoder_prev_millis_ = millis();
                int encoder_full_steps = encoder_position / encoder_step;
#if defined(DEBUG) && defined(DEBUG_encoder)
                Serial.print(F("enc_pos: "));
                Serial.println(encoder_position);
                Serial.print(F("enc_f_steps: "));
                Serial.println(encoder_full_steps);
#endif
                encoder_->write(encoder_position - (encoder_full_steps * encoder_step));

                if (cursor_clicked_)
                {
                    // Handle encoder rotations that should change some values.
                    switch (current_screen_)
                    {
                    case screen_home:
                        switch (cursor_position_)
                        {
                        case cph_ambient:
                            ambientDimmer_->set_from_duration(
                                    ambientDimmer_->get_value(),
                                    apply_limits_(ambientDimmer_->get_stop(),
                                        encoder_full_steps * 10, 0, 255,
                                        encoder_loop_ambient),
                                    GUI_ambient_dimming_duration);
                            ambientDimmer_->start();
                            break;

                        }
                        break;


                    case screen_alarms:
                        switch (cursor_position_)
                        {
                        case cpa_alarm_index:
                            sel_alarm_index_ = apply_limits_(
                                sel_alarm_index_, encoder_full_steps, 0,
                                alarms_count - 1, encoder_loop_alarm);
                            sel_alarm_ = (alarms_ + sel_alarm_index_);
                            break;

                        case cpa_alarm_enabled:
                        {
                            AlarmEnabled enabled = sel_alarm_->get_enabled();
                            enabled = AlarmEnabled(apply_limits_(enabled,
                                encoder_full_steps, 0, 3));
                            sel_alarm_->set_enabled(enabled);
                        }
                            break;

                        case cpa_alarm_time_h:
                        {
                            HoursMinutes time = sel_alarm_->get_time();
                            time.hours = apply_limits_(time.hours, encoder_full_steps,
                                                       0, 23, encoder_loop_time);
                            sel_alarm_->set_time(time.hours, time.minutes);
                        }
                            break;

                        case cpa_alarm_time_m:
                        {
                            HoursMinutes time = sel_alarm_->get_time();
                            time.minutes = apply_limits_(time.minutes,
                                                         encoder_full_steps,
                                                         0, 59, encoder_loop_time);
                            sel_alarm_->set_time(time.hours, time.minutes);
                        }
                            break;

                        case cpa_alarm_snz_time:
                        {
                            Snooze snooze = sel_alarm_->get_snooze();
                            snooze.time_minutes = apply_limits_(snooze.time_minutes,
                                encoder_full_steps, 0, 99, encoder_loop_snooze);
                            sel_alarm_->set_snooze(snooze.time_minutes,
                                                   snooze.count);
                        }
                            break;

                        case cpa_alarm_snz_count:
                        {
                            Snooze snooze = sel_alarm_->get_snooze();
                            snooze.count = apply_limits_(snooze.count,
                                encoder_full_steps, 0, 9);
                            sel_alarm_->set_snooze(snooze.time_minutes,
                                                   snooze.count);
                        }
                            break;

                        case cpa_alarm_sig_a:
                        {
                            Signalization prev_sig = sel_alarm_->get_signalization();
                            prev_sig.ambient = apply_limits_(prev_sig.ambient,
                                encoder_full_steps * 10, 0, 255, encoder_loop_ambient);
                            sel_alarm_->set_signalization(prev_sig.ambient,
                                                          prev_sig.lamp,
                                                          prev_sig.buzzer);
                        }
                            break;

                        }
                        break;


                    case screen_RTC:
                        switch (cursor_position_)
                        {
                        case cpr_time_h:
                            RTC_set_ = DateTime(RTC_set_.year(), RTC_set_.month(),
                                RTC_set_.day(),
                                apply_limits_(RTC_set_.hour(),
                                              encoder_full_steps, 0, 23,
                                              encoder_loop_time),
                                RTC_set_.minute(), RTC_set_.second());
                            break;

                        case cpr_time_m:
                            RTC_set_ = DateTime(RTC_set_.year(), RTC_set_.month(),
                                RTC_set_.day(), RTC_set_.hour(),
                                apply_limits_(RTC_set_.minute(),
                                              encoder_full_steps, 0, 59,
                                              encoder_loop_time),
                                RTC_set_.second());
                            break;

                        case cpr_time_s:
                            RTC_set_ = DateTime(RTC_set_.year(), RTC_set_.month(),
                                RTC_set_.day(), RTC_set_.hour(), RTC_set_.minute(),
                                apply_limits_(RTC_set_.second(),
                                              encoder_full_steps, 0, 59,
                                              encoder_loop_time));
                            break;

                        case cpr_date_d:
                            // RTC_set_ validity is checked when the apply button is pressed.
                            RTC_set_ = DateTime(RTC_set_.year(),
                                RTC_set_.month(),
                                apply_limits_(RTC_set_.day(),
                                    encoder_full_steps, 1, 31),
                                RTC_set_.hour(), RTC_set_.minute(),
                                RTC_set_.second());

                            break;

                        case cpr_date_m:
                            RTC_set_ = DateTime(RTC_set_.year(),
                                apply_limits_(RTC_set_.month(),
                                    encoder_full_steps, 1, 12),
                                RTC_set_.day(),
                                RTC_set_.hour(), RTC_set_.minute(),
                                RTC_set_.second());
                            break;

                        case cpr_date_y:
                            RTC_set_ = DateTime(
                                apply_limits_(RTC_set_.year() - 2000,
                                    encoder_full_steps, 0, 99),
                                RTC_set_.month(),
                                RTC_set_.day(),
                                RTC_set_.hour(), RTC_set_.minute(),
                                RTC_set_.second());
                            break;

                        }
                        break;
                    }

                }
                else
                {
                    // Move the cursor
                    cursor_position_ = apply_limits_(cursor_position_,
                        encoder_full_steps,
                        0, selectables_count_[current_screen_] - 1,
                        encoder_loop_cursor);
#if defined(DEBUG) && defined(DEBUG_encoder)
                    Serial.print("cpos: ");
                    Serial.println(cursor_position_);
                    //Serial.print("sc: ");
                    //Serial.println(selectables_count_[current_screen_] - 1);
                    //Serial.print("cs: ");
                    //Serial.println(current_screen_);
#endif
                }
                update_();
            }
        }
    }
    // Encoder missed microsteps correction
    else if ((unsigned long)(millis() - encoder_prev_millis_) >=
             encoder_reset_interval)
    {
        encoder_->write(0);
        encoder_prev_millis_ = millis();
    }

    if ((unsigned long)(millis() - update_prev_millis_) >=
        GUI_update_interval &&
        now_.second() % 10 == 0)
    {
        update_prev_millis_ = millis();
        update_();
    }
}

byte GUI::apply_limits_(byte value, int step, byte limit_low,
                        byte limit_high, bool loop)
{
    // Byte cannot be lower than 0, so I have to check before adding step to
    // the value to be able to detect limit_low
    if (value < limit_low)
        return limit_low;

    if (step < 0 && -step > (value - limit_low))
    {
        if (loop)
            return limit_high - (-step - (value - limit_low) - 1);
        else return limit_low;
    }

    if (step > (limit_high - value))
    {
        if (loop)
            return limit_low + ((value + step) - limit_high - 1);
        else return limit_high;
    }

    return byte(value + step);
}

void GUI::switch_screen_(Screen screen)
{
    current_screen_ = screen;
    cursor_position_ = 0;
    encoder_->write(0);
}

void GUI::update_()
{
    lcd_->noCursor();
    lcd_->noBlink();
    lcd_->setCursor(0, 0);
    switch (current_screen_)
    {
    case screen_home:
        sprintf(line_buffer_, "%02d.%02d %d %02d:%02d:%02d",
                now_.day(), now_.month(),
                now_.dayOfTheWeek() == 0 ? 7 : now_.dayOfTheWeek(),
                now_.hour(), now_.minute(), now_.second());
        lcd_->print(line_buffer_);
        lcd_->setCursor(0, 1);
        sprintf(line_buffer_, "%c  RTC %c    %02d%c ",
                char(LCD_char_bell_index), get_inhibit_() ? 'I' : 'i',
                ambientDimmer_->get_stop() / 10, lamp_->get() ? 'L' : 'l');
        lcd_->print(line_buffer_);

        break;


    case screen_alarms:
    {
        char days_of_week[8] = "";  // 7 + \0
        days_of_week[7] = '\0';
        for (byte i = 1; i <= 7; i++)
        {
            if (sel_alarm_->get_days_of_week().getDayOfWeek(i))
                days_of_week[i - 1] = char('0' + i);
            else days_of_week[i - 1] = ' ';
        }

        char enabled[4] = "";
        switch (sel_alarm_->get_enabled())
        {
        case Off:
            strcpy(enabled, "Off");
            break;

        case Single:
            strcpy(enabled, "SGL");
            break;

        case Repeat:
            strcpy(enabled, "RPT");
            break;

        case Skip:
            strcpy(enabled, "SKP");
            break;
        }

        sprintf(line_buffer_, "%c%d/%d %s %s",
                char(LCD_char_home_index), sel_alarm_index_ + 1, alarms_count,
                days_of_week, enabled );
        lcd_->print(line_buffer_);

        lcd_->setCursor(0, 1);
        sprintf(line_buffer_, "%02d:%02d+%02d*%d  %02d%c%c",
                sel_alarm_->get_time().hours,
                sel_alarm_->get_time().minutes,
                sel_alarm_->get_snooze().time_minutes,
                sel_alarm_->get_snooze().count,
                sel_alarm_->get_signalization().ambient / 10,
                sel_alarm_->get_signalization().lamp ? 'L' : 'l',
                sel_alarm_->get_signalization().buzzer ? 'B' : 'b' );
        lcd_->print(line_buffer_);
    }
        break;


    case screen_RTC:
        sprintf(line_buffer_, "%c%cRTC %d %02d:%02d:%02d",
                LCD_char_cancel_index, LCD_char_apply_index,
                RTC_set_.dayOfTheWeek() == 0 ? 7 : RTC_set_.dayOfTheWeek(),
                RTC_set_.hour(), RTC_set_.minute(), RTC_set_.second());
        lcd_->print(line_buffer_);

        lcd_->setCursor(0, 1);
        sprintf(line_buffer_, "%02d.%02d.%04d      ",
                RTC_set_.day(), RTC_set_.month(), RTC_set_.year());
        lcd_->print(line_buffer_);
    break;


    }

    lcd_->setCursor(cursor_positions_[current_screen_][cursor_position_].column,
                    cursor_positions_[current_screen_][cursor_position_].row);
    lcd_->cursor();
    if (cursor_clicked_) lcd_->blink();
}

GUI::GUI(Alarm *alarms, void(*writeEEPROM)(),
         RTC_DS3231 * rtc, Encoder * encoder, Bounce * encoder_button,
         LiquidCrystal_I2C *lcd,
         void(*set_inhibit)(bool), bool(*get_inhibit)(),
         PWMDimmer *ambientDimmer, HALbool *lamp)
{
    alarms_ = alarms;
    writeEEPROM_ = writeEEPROM;
    rtc_ = rtc;
    encoder_ = encoder;
    encoder_button_ = encoder_button;
    lcd_ = lcd;
    set_inhibit_ = set_inhibit;
    get_inhibit_ = get_inhibit;
    ambientDimmer_ = ambientDimmer;
    lamp_ = lamp;

    // First alarm. I can't initialize it in the header because the compiler
    // doesn't know the address yet.
    sel_alarm_ = alarms_;
}
