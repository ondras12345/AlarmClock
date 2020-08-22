/*!
    @file
*/
#include "SerialCLI.h"


const SerialCLI::command_t SerialCLI::commands_[] = {
    {"sel",     &SerialCLI::cmd_sel_},
    {"amb",     &SerialCLI::cmd_amb_},
    {"lamp",    &SerialCLI::cmd_lamp_},
    {"inh",     &SerialCLI::cmd_inh_},
    {"en-",     &SerialCLI::cmd_en_},
    {"time",    &SerialCLI::cmd_time_},
    {"dow",     &SerialCLI::cmd_dow_},
    {"snz",     &SerialCLI::cmd_snz_},
    {"sig",     &SerialCLI::cmd_sig_},
    {"st",      &SerialCLI::cmd_st_},
    {"sd",      &SerialCLI::cmd_sd_},
    {"sav",     &SerialCLI::cmd_sav_},
    {"rtc",     &SerialCLI::cmd_rtc_},
    {"ls",      &SerialCLI::cmd_ls_}
};


/*!
    @brief  Call this function in your loop().
*/
void SerialCLI::loop(DateTime time)
{
    now_ = time;
    bool complete_message = false;

    while (ser_.available() && !complete_message)
    {
        // !complete_message - this prevents the Serial_buffer_ being rewritten
        // by new data when 2 messages are sent with very little delay (index
        // is set to 0 when complete_message is received).
        // I need to process the data before I receive the next message.

        Serial_buffer_[Serial_buffer_index_] = tolower(ser_.read());

        if (Serial_buffer_[Serial_buffer_index_] == '\n' || Serial_buffer_[Serial_buffer_index_] == '\r')
        {
            //  CR/LF
            if (Serial_buffer_index_ != 0)
            {
                // ignore if it is the first character (to avoid problems with CR+LF/LF)

                Serial_buffer_[Serial_buffer_index_] = '\0';  // rewrite CR/LF
                complete_message = true;
                Serial_buffer_index_ = 0;
                ser_.println();

#if defined(DEBUG) && defined(DEBUG_Serial_CLI)
                Serial.println();
                Serial.print(">> ");
                Serial.println(Serial_buffer_);

                char *ptr = &Serial_buffer_[0];
                while (*ptr != '\0')
                {
                    Serial.print(byte(*ptr), HEX);
                    Serial.print(' ');
                    ptr++;
                }
                Serial.println();
#endif // DEBUG
            }
        }
        else
        {
            // Character playback - this needs to be before index++ and should
            // not happen when the character is CR/LF
            ser_.print(Serial_buffer_[Serial_buffer_index_]);

            if (Serial_buffer_index_ < kSerial_buffer_length_ - 1)
            {
                Serial_buffer_index_++;
            }
            else
            {
                ser_.println();
                ser_.print(F("Cmd too long: "));
                for (byte i = 0; i <= Serial_buffer_index_; i++)
                    ser_.print(Serial_buffer_[i]);

                delay(40);  // to receive the rest of the message
                while (ser_.available())
                    ser_.print(char(ser_.read()));

                ser_.println();

                Serial_buffer_index_ = 0;
                Serial_buffer_[0] = '\0';  // this is currently not necessary
            }
        }
    }


    if (complete_message)
    {
        DEBUG_println();
        DEBUG_println(F("Processing"));

        char *cmd_ptr = nullptr;
        bool cmd_found = false;

        for (byte i = 0; i < (sizeof(commands_) / sizeof(command_t)); i++)
        {
            if ((cmd_ptr = strstr(Serial_buffer_, commands_[i].text)) != nullptr)
            {
                print_error_((this->*(commands_[i].handler))(cmd_ptr));
                cmd_found = true;
                break;  // do not process any further commands
            }
        }

        if (!cmd_found)
        {
            ser_.println(F("? SYNTAX ERROR"));
            print_help_();
        }

        prev_command_millis_ = millis();

        // Prompt will be missing after reboot, but this can't be easily fixed.
        ser_.println();
        ser_.print(prompt_);
    }

    // autosave
    if ((unsigned long)(millis() - prev_command_millis_) >= Serial_autosave_interval && change_)
    {
        ser_.println();
        ser_.println(F("Autosaving"));
        print_error_(save_());
    }
}


void SerialCLI::print_help_()
{
    ser_.println();
    ser_.println(F("Help:"));
    indent_(1);
    ser_.println(F("amb - get ambient 0-255"));
    indent_(1);
    ser_.println(F("amb{nnn} - ambient 0-255"));
    indent_(1);
    ser_.println(F("lamp - get 0|1"));
    indent_(1);
    ser_.println(F("lamp{l} - set 0|1"));
    indent_(1);
    ser_.println(F("inh - get inhibit 0|1"));
    indent_(1);
    ser_.println(F("inh{i} - set inhibit 0|1"));
    indent_(1);
    ser_.println(F("sel{i} - select alarm{i}"));
    indent_(1);
    ser_.println(F("Selected alarm:"));
    indent_(2);
    ser_.println(F("ls - list"));
    indent_(2);
    ser_.println(F("en-off/en-sgl/en-rpt/en-skp - enable - off/single/repeat/skip"));
    indent_(2);
    ser_.println(F("time{h}:{m} - set time"));
    indent_(2);
    ser_.println(F("dow{d}:{s} - set day {d}1-7 of week to {s}1|0"));
    indent_(2);
    ser_.println(F("snz{t};{c} - set snooze: time{t}min;count{c}"));
    indent_(2);
    ser_.println(F("sig{a};{l};{b} - set signalization: ambient{a}0-255;lamp{l}1|0;buzzer{b}1|0"));
    indent_(1);
    ser_.println(F("sav - save all"));
    indent_(1);
    ser_.println(F("RTC:"));
    indent_(2);
    ser_.println(F("rtc - get RTC time"));
    indent_(2);
    ser_.println(F("sd{dd}.{mm}.{yy} - set RTC date"));
    indent_(2);
    ser_.println(F("st{h}:{m} - set RTC time"));
}


byte SerialCLI::strbyte_(char *str)
{
    byte result = 0;
    while (isDigit(*str))
    {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}


char * SerialCLI::find_digit_(char * str)
{
    while (!isDigit(*str) && *str != '\0') str++;
    return str;
}


char * SerialCLI::find_next_digit_(char * str)
{
    while (isDigit(*str) && *str != '\0') str++;
    str = find_digit_(str);
    return str;
}


void SerialCLI::indent_(byte level)
{
    for (byte i = 0; i < level * Serial_indentation_width; i++)
    {
        ser_.print(' ');
    }
}


void SerialCLI::print_error_(error_t error_code)
{
    ser_.println();
    ser_.print(F("err "));
    ser_.print(error_code);
    ser_.print(F(": "));

    if (!error_code)
        ser_.println(F("OK"));

    if (error_code & kArgument)
        ser_.println(F("Invalid args"));

    if (error_code & kNothingSelected)
        ser_.println(F("Sel first"));

    if (error_code & kUselessSave)
        ser_.println(F("Nothing to save"));
}


SerialCLI::error_t SerialCLI::cmd_sel_(char *index)
{
    index = find_digit_(index);
    if (*index == '\0')
        return select_alarm_(sel_alarm_index_none_);

    byte index_num = strbyte_(index);
    return select_alarm_(index_num);
}


SerialCLI::error_t SerialCLI::cmd_amb_(char * duty)
{
    byte ambient;

    duty = find_next_digit_(duty);
    if (*duty == '\0')
    {
        ser_.print(F("amb: "));
        ser_.println(ambientDimmer_->get_value());
        return 0;
    }
    ambient = strbyte_(duty);
    ambientDimmer_->set_from_duration(ambientDimmer_->get_value(), ambient,
                                    Serial_ambient_dimming_duration);
    ambientDimmer_->start();
    return 0;
}


SerialCLI::error_t SerialCLI::cmd_lamp_(char *status)
{
    status = find_next_digit_(status);
    if (*status == '\0')
    {
        ser_.print(F("lamp: "));
        ser_.println(lamp_->get());
        return 0;
    }
    lamp_->set_manu(strbyte_(status));
    return 0;
}


SerialCLI::error_t SerialCLI::cmd_inh_(char *status)
{
    status = find_next_digit_(status);
    if (*status == '\0')
    {
        ser_.print(F("inhibit: "));
        ser_.println(get_inhibit_());
        return 0;
    }
    set_inhibit_(strbyte_(status));
    return 0;
}


SerialCLI::error_t SerialCLI::cmd_en_(char *type)
{
    // ! - strcmp returns 0 if matches
    if (!strcmp(type, "en-off"))
        return set_enabled_(Off);

    if (!strcmp(type, "en-sgl"))
        return set_enabled_(Single);

    if (!strcmp(type, "en-rpt"))
        return set_enabled_(Repeat);

    if (!strcmp(type, "en-skp"))
        return set_enabled_(Skip);

    return kArgument;
}


SerialCLI::error_t SerialCLI::cmd_time_(char *time)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte hours, minutes;
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    hours = strbyte_(time);
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    minutes = strbyte_(time);

    if ((alarms_ + sel_alarm_index_)->set_time(hours, minutes))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t SerialCLI::cmd_dow_(char *dow)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte day;
    bool status;
    dow = find_next_digit_(dow);
    if (*dow == '\0') return kArgument;
    day = strbyte_(dow);
    dow = find_next_digit_(dow);
    if (*dow == '\0') return kArgument;
    status = strbyte_(dow);

    if ((alarms_ + sel_alarm_index_)->set_day_of_week(day, status))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t SerialCLI::cmd_snz_(char *snooze)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte time, count;

    snooze = find_next_digit_(snooze);
    if (*snooze == '\0') return kArgument;
    time = strbyte_(snooze);
    snooze = find_next_digit_(snooze);
    if (*snooze == '\0') return kArgument;
    count = strbyte_(snooze);

    if ((alarms_ + sel_alarm_index_)->set_snooze(time, count))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t SerialCLI::cmd_sig_(char *sig)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte ambient;
    bool lamp, buzzer;

    sig = find_next_digit_(sig);
    if (*sig == '\0') return kArgument;
    ambient = strbyte_(sig);
    sig = find_next_digit_(sig);
    if (*sig == '\0') return kArgument;
    lamp = strbyte_(sig);
    sig = find_next_digit_(sig);
    if (*sig == '\0') return kArgument;
    buzzer = strbyte_(sig);

    if ((alarms_ + sel_alarm_index_)->set_signalization(ambient, lamp, buzzer))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t SerialCLI::cmd_st_(char *time)
{
    byte hour, minute;
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    hour = strbyte_(time);
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    minute = strbyte_(time);

    if (hour > 23 || minute > 59) return kArgument;


    now_ = rtc_->now();
    rtc_->adjust(DateTime(now_.year(), now_.month(), now_.day(), hour, minute));
    return 0;
}


SerialCLI::error_t SerialCLI::cmd_sav_(char *ignored)
{
    (void)ignored;
    return save_();
}


SerialCLI::error_t SerialCLI::cmd_rtc_(char *ignored)
{
    (void)ignored;

    ser_.print(F("Time: "));
    if (now_.dayOfTheWeek() == 0) ser_.print(days_of_the_week_names_short[7]);
    else ser_.print(days_of_the_week_names_short[now_.dayOfTheWeek()]);
    ser_.print(' ');
    ser_.print(now_.day());
    ser_.print(". ");
    ser_.print(now_.month());
    ser_.print(". ");
    ser_.print(now_.year());
    ser_.print("  ");
    ser_.print(now_.hour());
    ser_.print(':');
    ser_.print(now_.minute());
    ser_.print(':');
    ser_.println(now_.second());

    return 0;
}


SerialCLI::error_t SerialCLI::cmd_ls_(char *ignored)
{
    (void)ignored;

    if (sel_alarm_index_ == sel_alarm_index_none_)
        return kNothingSelected;

    ser_.print(F("Num: "));
    ser_.println(sel_alarm_index_);

    indent_(1);
    ser_.print(F("Enabled: "));
    switch ((alarms_ + sel_alarm_index_)->get_enabled())
    {
    case Off:
        ser_.println(F("Off"));
        break;

    case Single:
        ser_.println(F("Single"));
        break;

    case Repeat:
        ser_.println(F("Repeat"));
        break;

    case Skip:
        ser_.println(F("Skip"));
        break;
    }

    indent_(1);
    ser_.print(F("Days of week: "));
    for (byte i = 1; i <= 7; i++)
    {
        if ((alarms_ + sel_alarm_index_)->get_days_of_week().getDayOfWeek(i))
        {
            ser_.print(days_of_the_week_names_short[i]);
            ser_.print(' ');
        }
        else ser_.print("   ");
    }
    ser_.println();

    indent_(1);
    ser_.print(F("Time: "));
    ser_.print((alarms_ + sel_alarm_index_)->get_time().hours);
    ser_.print(":");
    ser_.println((alarms_ + sel_alarm_index_)->get_time().minutes);

    indent_(1);
    ser_.println(F("Snooze: "));
    indent_(2);
    ser_.print(F("Time: "));
    ser_.print((alarms_ + sel_alarm_index_)->get_snooze().time_minutes);
    ser_.println(F(" min"));
    indent_(2);
    ser_.print(F("Count: "));
    ser_.println((alarms_ + sel_alarm_index_)->get_snooze().count);

    indent_(1);
    ser_.println(F("Signalization: "));
    indent_(2);
    ser_.print(F("Ambient: "));
    ser_.println((alarms_ + sel_alarm_index_)->get_signalization().ambient);
    indent_(2);
    ser_.print(F("Lamp: "));
    ser_.println((alarms_ + sel_alarm_index_)->get_signalization().lamp);
    indent_(2);
    ser_.print(F("Buzzer: "));
    ser_.println((alarms_ + sel_alarm_index_)->get_signalization().buzzer);

    return 0;
}


SerialCLI::error_t SerialCLI::cmd_sd_(char *date)
{
    int year;
    byte month, day;

    date = find_next_digit_(date);
    if (*date == '\0') return kArgument;
    day = strbyte_(date);
    date = find_next_digit_(date);
    if (*date == '\0') return kArgument;
    month = strbyte_(date);
    date = find_next_digit_(date);
    if (*date == '\0') return kArgument;
    year = strbyte_(date);

    if (month > 12 || day > 31) return kArgument;

    now_ = rtc_->now();
    DateTime new_date(year, month, day, now_.hour(), now_.minute());
    if (!new_date.isValid()) return kArgument;
    rtc_->adjust(new_date);
    return 0;
}


SerialCLI::error_t SerialCLI::select_alarm_(byte index_)
{
    if (index_ >= alarms_count && index_ != sel_alarm_index_none_)
        return kArgument;

    sel_alarm_index_ = index_;
    if (sel_alarm_index_ == sel_alarm_index_none_) strcpy(prompt_, prompt_default_);
    else sprintf(prompt_, "A%u%s", sel_alarm_index_, prompt_default_);

    return 0;
}


SerialCLI::error_t SerialCLI::set_enabled_(AlarmEnabled status)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    (alarms_ + sel_alarm_index_)->set_enabled(status);

    change_ = true;
    return 0;
}


SerialCLI::error_t SerialCLI::save_()
{
    if (change_)
    {
        change_ = false;
        writeEEPROM_();
        return 0;
    }
    else return kUselessSave;
}
