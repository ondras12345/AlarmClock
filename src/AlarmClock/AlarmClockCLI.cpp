/*!
    @file
*/
#include "AlarmClockCLI.h"
#include <RTClib.h>
#include <avr/pgmspace.h>

// Assigned in the constructor:
Stream* AlarmClockCLI::ser_;
Alarm* AlarmClockCLI::alarms_;
DateTime AlarmClockCLI::now_;
RTC_DS3231* AlarmClockCLI::rtc_;
void(*AlarmClockCLI::writeEEPROM_)();
PWMDimmer* AlarmClockCLI::ambientDimmer_;
HALbool* AlarmClockCLI::lamp_;
CountdownTimer* AlarmClockCLI::timer_;
void(*AlarmClockCLI::set_inhibit_)(bool);
bool(*AlarmClockCLI::get_inhibit_)();

bool AlarmClockCLI::change_ = false;
byte AlarmClockCLI::sel_alarm_index_ = AlarmClockCLI::sel_alarm_index_none_;

// https://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char
char AlarmClockCLI::prompt_[];
constexpr char AlarmClockCLI::prompt_default_[];


const SerialCLI::command_t AlarmClockCLI::commands[] = {
    {"sel",     &AlarmClockCLI::cmd_sel_},
    {"amb",     &AlarmClockCLI::cmd_amb_},
    {"lamp",    &AlarmClockCLI::cmd_lamp_},
    {"inh",     &AlarmClockCLI::cmd_inh_},
    {"en-",     &AlarmClockCLI::cmd_en_},
    {"time",    &AlarmClockCLI::cmd_time_},
    {"dow",     &AlarmClockCLI::cmd_dow_},
    {"snz",     &AlarmClockCLI::cmd_snz_},
    {"sig",     &AlarmClockCLI::cmd_sig_},
    // tmr needs to be above st because of tmr-start
    {"tmr",     &AlarmClockCLI::cmd_tmr_},
    {"tme",     &AlarmClockCLI::cmd_tme_},
    {"st",      &AlarmClockCLI::cmd_st_},
    {"sd",      &AlarmClockCLI::cmd_sd_},
    {"sav",     &AlarmClockCLI::cmd_sav_},
    {"rtc",     &AlarmClockCLI::cmd_rtc_},
    {"ls",      &AlarmClockCLI::cmd_ls_}
};
const byte AlarmClockCLI::command_count =
    (sizeof(AlarmClockCLI::commands) / sizeof(SerialCLI::command_t));

const char* AlarmClockCLI::error_strings[] = {
    "OK",
    "Invalid args",
    "Sel first",
    "Nothing to save",
};


void AlarmClockCLI::loop(const DateTime& now)
{
    now_ = now;
    CLI_.loop();

    // autosave
    if (change_ &&
            (unsigned long)(millis() - CLI_.prev_command_millis()) >= Serial_autosave_interval)
    {
        ser_->println();
        ser_->println(F("Autosaving"));
        print_error(save_());
    }
}


/*!
    @brief  Convert a string of digits to a one byte unsigned integer.
            Negative numbers are not supported.
            Overflows are not handled.
    @param str  The string.
    @return The number.
            Returns 0 if the first character of the string is not a digit.
*/
byte AlarmClockCLI::strbyte_(const char* str)
{
    byte result = 0;
    while (isDigit(*str))
    {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}


/*!
    @brief  Find the first digit in a string.
    @param str  The string.
    @return Pointer to the digit.
*/
char* AlarmClockCLI::find_digit_(char* str)
{
    while (!isDigit(*str) && *str != '\0') str++;
    return str;
}


/*!
    @brief  Find the next number in a string.
            If the first character of the string is a digit, it skips all
            following characters until it finds one that is not a digit and
            then calls find_digit_.
    @param str  The string.
    @return Pointer to the first digit of the next number.
*/
char* AlarmClockCLI::find_next_digit_(char* str)
{
    while (isDigit(*str) && *str != '\0') str++;
    str = find_digit_(str);
    return str;
}


void AlarmClockCLI::indent_(byte level)
{
    for (byte i = 0; i < level * Serial_indentation_width; i++)
    {
        ser_->print(' ');
    }
}


void AlarmClockCLI::print_error(SerialCLI::error_t code)
{
    ser_->println();
    ser_->print(F("err "));
    ser_->print(code, HEX);
    ser_->print(F(": "));

    if (code == kOk)
    {
        ser_->println(error_strings[kOk]);
        return;
    }

    byte index = 1;
    for (byte bit = 0x01; bit != kLast; bit <<= 1)
    {
        if (code & bit)
        {
            ser_->println(error_strings[index]);
        }
        index++;
    }
}


void AlarmClockCLI::cmd_not_found()
{
    ser_->println(F("? SYNTAX ERROR"));

    ser_->println();
    ser_->println(F("Help:"));
    indent_(1);
    ser_->println(F("amb - get ambient 0-255"));
    indent_(1);
    ser_->println(F("amb{n} - ambient 0-255"));
    indent_(1);
    ser_->println(F("lamp - get 0|1"));
    indent_(1);
    ser_->println(F("lamp{l} - set 0|1"));
    indent_(1);
    ser_->println(F("inh - get inhibit 0|1"));
    indent_(1);
    ser_->println(F("inh{i} - set inhibit 0|1"));
    indent_(1);
    ser_->println(F("sel{i} - select alarm"));
    indent_(1);
    ser_->println(F("Selected alarm:"));
    indent_(2);
    ser_->println(F("ls - list"));
    indent_(2);
    ser_->println(F("en-off/en-sgl/en-rpt/en-skp - enable"));
    indent_(2);
    ser_->println(F("time{h}:{m}"));
    indent_(2);
    ser_->println(F("dow{d}:{s} - set day {d}1-7 of week to {s}1|0"));
    indent_(2);
    ser_->println(F("snz{t};{c} - set snooze: time{t}min;count{c}"));
    indent_(2);
    ser_->println(F("sig{a};{l};{b} - set signalization: ambient{a}0-255;lamp{l}1|0;buzzer{b}1|0"));
    indent_(1);
    ser_->println(F("sav - save all"));
    indent_(1);
    ser_->println(F("RTC:"));
    indent_(2);
    ser_->println(F("rtc - get RTC time"));
    indent_(2);
    ser_->println(F("sd{YYYY-MM-DD} - set RTC date"));
    indent_(2);
    ser_->println(F("st{hh:mm[:ss]} - set RTC time"));
    indent_(1);
    ser_->println(F("Timer:"));
    indent_(2);
    ser_->println(F("tmr - get timer time"));
    indent_(2);
    ser_->println(F("tmr{hh:mm:ss} - set timer time"));
    indent_(2);
    ser_->println(F("tme - get timer events"));
    indent_(2);
    ser_->println(F("tme{a};{l};{b} - set timer events"));
    indent_(2);
    ser_->println(F("tmr-start/tmr-stop"));
}



SerialCLI::error_t AlarmClockCLI::select_alarm_(byte index_)
{
    if (index_ >= alarms_count && index_ != sel_alarm_index_none_)
        return kArgument;

    sel_alarm_index_ = index_;
    if (sel_alarm_index_ == sel_alarm_index_none_) strcpy(prompt_, prompt_default_);
    else sprintf_P(prompt_, PSTR("A%u%s"), sel_alarm_index_, prompt_default_);

    return 0;
}


SerialCLI::error_t AlarmClockCLI::set_enabled_(AlarmEnabled status)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    alarms_[sel_alarm_index_].set_enabled(status);

    change_ = true;
    return 0;
}


SerialCLI::error_t AlarmClockCLI::save_()
{
    if (change_)
    {
        change_ = false;
        writeEEPROM_();
        return 0;
    }
    else return kUselessSave;
}



SerialCLI::error_t AlarmClockCLI::cmd_sel_(char *index)
{
    index = find_digit_(index);
    if (*index == '\0')
        return select_alarm_(sel_alarm_index_none_);

    byte index_num = strbyte_(index);
    return select_alarm_(index_num);
}


SerialCLI::error_t AlarmClockCLI::cmd_amb_(char * duty)
{
    byte ambient;

    duty = find_next_digit_(duty);
    if (*duty == '\0')
    {
        ser_->print(F("amb: "));
        ser_->println(ambientDimmer_->get_value());
        return 0;
    }
    ambient = strbyte_(duty);
    ambientDimmer_->set_from_duration(ambientDimmer_->get_value(), ambient,
                                      Serial_ambient_dimming_duration);
    ambientDimmer_->start();
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_lamp_(char *status)
{
    status = find_next_digit_(status);
    if (*status == '\0')
    {
        ser_->print(F("lamp: "));
        ser_->println(lamp_->get());
        return 0;
    }
    lamp_->set_manu(strbyte_(status));
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_inh_(char *status)
{
    status = find_next_digit_(status);
    if (*status == '\0')
    {
        ser_->print(F("inhibit: "));
        ser_->println(get_inhibit_());
        return 0;
    }
    set_inhibit_(strbyte_(status));
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_en_(char *type)
{
    // ! - strcmp returns 0 if matches
    if (!strcmp_P(type, PSTR("en-off")))
        return set_enabled_(Off);

    if (!strcmp_P(type, PSTR("en-sgl")))
        return set_enabled_(Single);

    if (!strcmp_P(type, PSTR("en-rpt")))
        return set_enabled_(Repeat);

    if (!strcmp_P(type, PSTR("en-skp")))
        return set_enabled_(Skip);

    return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_time_(char *time)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte hours, minutes;
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    hours = strbyte_(time);
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    minutes = strbyte_(time);

    if (alarms_[sel_alarm_index_].set_time(hours, minutes))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_dow_(char *dow)
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

    if (alarms_[sel_alarm_index_].set_day_of_week(day, status))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_snz_(char *snooze)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte time, count;

    snooze = find_next_digit_(snooze);
    if (*snooze == '\0') return kArgument;
    time = strbyte_(snooze);
    snooze = find_next_digit_(snooze);
    if (*snooze == '\0') return kArgument;
    count = strbyte_(snooze);

    if (alarms_[sel_alarm_index_].set_snooze(time, count))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_sig_(char *sig)
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

    if (alarms_[sel_alarm_index_].set_signalization(ambient, lamp, buzzer))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_st_(char *time)
{
    byte hour, minute, second = 0;
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    hour = strbyte_(time);
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    minute = strbyte_(time);
    time = find_next_digit_(time);
    // Second is optional.
    if (*time != '\0')
        second = strbyte_(time);

    if (hour > 23 || minute > 59 || second > 59) return kArgument;


    now_ = rtc_->now();
    rtc_->adjust(DateTime(now_.year(), now_.month(), now_.day(), hour, minute, second));
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_tmr_(char *time)
{
    if (!strcmp_P(time, PSTR("tmr-start")))
    {
        timer_->start();
        return 0;
    }

    if (!strcmp_P(time, PSTR("tmr-stop")))
    {
        timer_->stop();
        return 0;
    }

    time = find_next_digit_(time);
    if (*time == '\0')
    {
        TimeSpan time_left(timer_->time_left);
        char buff[8 + 1];
        sprintf_P(buff, PSTR("%02d:%02d:%02d"),
                  time_left.hours(), time_left.minutes(), time_left.seconds());
        ser_->println(buff);
        return 0;
    }

    byte hours, minutes, seconds;
    hours = strbyte_(time);
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    minutes = strbyte_(time);
    time = find_next_digit_(time);
    if (*time == '\0') return kArgument;
    seconds = strbyte_(time);

    // unsigned int timer_->time_left overflows:
    // 18:12:15 + 1 = 00:00:00
    if (hours > 18 || minutes > 59 || seconds > 59) return kArgument;

    if (hours > 1)
    {
        // check for overflows:
        unsigned int tmp = (hours - 1) * 3600U + minutes * 60U + seconds;
        if (65535U - tmp < 3600U) return kArgument;
        timer_->time_left = tmp + 3600;
    }
    else timer_->time_left = hours * 3600U + minutes * 60U + seconds;

    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_tme_(char *events)
{

    byte ambient;
    bool lamp, buzzer;

    events = find_next_digit_(events);
    if (*events == '\0')
    {
        ser_->print(timer_->events.ambient);
        ser_->print(';');
        ser_->print(timer_->events.lamp ? 1 : 0);
        ser_->print(';');
        ser_->println(timer_->events.buzzer ? 1 : 0);
        return 0;
    }

    ambient = strbyte_(events);
    events = find_next_digit_(events);
    if (*events == '\0') return kArgument;
    lamp = strbyte_(events);
    events = find_next_digit_(events);
    if (*events == '\0') return kArgument;
    buzzer = strbyte_(events);

    timer_->events = {ambient, lamp, buzzer};
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_sav_(char *ignored)
{
    (void)ignored;
    return save_();
}


SerialCLI::error_t AlarmClockCLI::cmd_rtc_(char *ignored)
{
    (void)ignored;

    if (now_.dayOfTheWeek() == 0) ser_->print(days_of_the_week_names_short[7]);
    else ser_->print(days_of_the_week_names_short[now_.dayOfTheWeek()]);
    char buff[] = "YYYY-MM-DD hh:mm:ss";
    ser_->print(' ');
    ser_->println(now_.toString(buff));
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_ls_(char *ignored)
{
    (void)ignored;

    if (sel_alarm_index_ == sel_alarm_index_none_)
        return kNothingSelected;

    ser_->print(F("Num: "));
    ser_->println(sel_alarm_index_);

    indent_(1);
    ser_->print(F("Enabled: "));
    switch (alarms_[sel_alarm_index_].get_enabled())
    {
    case Off:
        ser_->println(F("Off"));
        break;

    case Single:
        ser_->println(F("Single"));
        break;

    case Repeat:
        ser_->println(F("Repeat"));
        break;

    case Skip:
        ser_->println(F("Skip"));
        break;
    }

    indent_(1);
    ser_->print(F("Days of week: "));
    for (byte i = 1; i <= 7; i++)
    {
        if (alarms_[sel_alarm_index_].get_days_of_week().getDayOfWeek(i))
        {
            ser_->print(days_of_the_week_names_short[i]);
            ser_->print(' ');
        }
        else ser_->print(F("   "));
    }
    ser_->println();

    indent_(1);
    ser_->print(F("Time: "));
    ser_->print(alarms_[sel_alarm_index_].get_time().hours);
    ser_->print(':');
    ser_->println(alarms_[sel_alarm_index_].get_time().minutes);

    indent_(1);
    ser_->println(F("Snooze: "));
    indent_(2);
    ser_->print(F("Time: "));
    ser_->print(alarms_[sel_alarm_index_].get_snooze().time_minutes);
    ser_->println(F(" min"));
    indent_(2);
    ser_->print(F("Count: "));
    ser_->println(alarms_[sel_alarm_index_].get_snooze().count);

    indent_(1);
    ser_->println(F("Signalization: "));
    indent_(2);
    ser_->print(F("Ambient: "));
    ser_->println(alarms_[sel_alarm_index_].get_signalization().ambient);
    indent_(2);
    ser_->print(F("Lamp: "));
    ser_->println(alarms_[sel_alarm_index_].get_signalization().lamp);
    indent_(2);
    ser_->print(F("Buzzer: "));
    ser_->println(alarms_[sel_alarm_index_].get_signalization().buzzer);

    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_sd_(char *date)
{
    byte year, month, day;

    date = find_next_digit_(date) + 1; // +1 to avoid byte overflow
    if (*date == '\0') return kArgument;
    year = strbyte_(date);
    date = find_next_digit_(date);
    if (*date == '\0') return kArgument;
    month = strbyte_(date);
    date = find_next_digit_(date);
    if (*date == '\0') return kArgument;
    day = strbyte_(date);

    if (month > 12 || day > 31) return kArgument;

    now_ = rtc_->now();
    DateTime new_date(year, month, day, now_.hour(), now_.minute());
    if (!new_date.isValid()) return kArgument;
    rtc_->adjust(new_date);
    return 0;
}
