/*!
    @file
*/
#include "AlarmClockCLI.h"
#include <RTClib.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "version.h"

// Assigned in the constructor:
Stream* AlarmClockCLI::ser_;
Alarm** AlarmClockCLI::alarms_;
DateTime AlarmClockCLI::now_;
RTC_DS3231* AlarmClockCLI::rtc_;
void(*AlarmClockCLI::writeEEPROM_)();
PWMDimmer* AlarmClockCLI::ambientDimmer_;
HALbool* AlarmClockCLI::lamp_;
CountdownTimer* AlarmClockCLI::timer_;
PWMSine* AlarmClockCLI::sine_;
BuzzerManager* AlarmClockCLI::buzzer_;
void(*AlarmClockCLI::set_inhibit_)(bool);
bool(*AlarmClockCLI::get_inhibit_)();

bool AlarmClockCLI::buzzer_playing_ = false;

bool AlarmClockCLI::change_ = false;
byte AlarmClockCLI::sel_alarm_index_ = AlarmClockCLI::sel_alarm_index_none_;

// https://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char
char AlarmClockCLI::prompt_[];
constexpr char AlarmClockCLI::prompt_default_[];


const SerialCLI::command_t AlarmClockCLI::commands[] = {
    {"sync",    &AlarmClockCLI::cmd_sync_},
    {"sel",     &AlarmClockCLI::cmd_sel_},
    {"amb",     &AlarmClockCLI::cmd_amb_},
    {"lamp",    &AlarmClockCLI::cmd_lamp_},
    {"inh",     &AlarmClockCLI::cmd_inh_},
    {"en-",     &AlarmClockCLI::cmd_en_},
    {"time",    &AlarmClockCLI::cmd_time_},
    {"dow",     &AlarmClockCLI::cmd_dow_},
    {"snz",     &AlarmClockCLI::cmd_snz_},
    {"sig",     &AlarmClockCLI::cmd_sig_},
    {"stop",    &AlarmClockCLI::cmd_stop_},
    {"tmr",     &AlarmClockCLI::cmd_tmr_},
    {"tme",     &AlarmClockCLI::cmd_tme_},
    {"st",      &AlarmClockCLI::cmd_st_},
    {"sd",      &AlarmClockCLI::cmd_sd_},
    {"sav",     &AlarmClockCLI::cmd_sav_},
    {"rtc",     &AlarmClockCLI::cmd_rtc_},
    {"ls",      &AlarmClockCLI::cmd_ls_},
    {"la",      &AlarmClockCLI::cmd_la_},
    {"ver",     &AlarmClockCLI::cmd_ver_},
    {"tone",    &AlarmClockCLI::cmd_tone_},
    {"silence", &AlarmClockCLI::cmd_silence_},
    {"notone",  &AlarmClockCLI::cmd_notone_},
    {"melody",  &AlarmClockCLI::cmd_melody_},
    {"eer",     &AlarmClockCLI::cmd_eer_},
    {"eew",     &AlarmClockCLI::cmd_eew_},
    {"active",  &AlarmClockCLI::cmd_active_},
};
const byte AlarmClockCLI::command_count =
    (sizeof(AlarmClockCLI::commands) / sizeof(SerialCLI::command_t));

// TODO make this PROGMEM
const char* AlarmClockCLI::error_strings[] = {
    "OK",
    "Invalid args",
    "Sel first",
    "Nothing to save",
    "? SYNTAX ERROR",
    "Unsupported"
};


// TODO make this progmem
//! 1 = Monday, 7 = Sunday; 0 = two spaces (same length as other elements)
const char* days_of_the_week_names_short[] = {
    "  ", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"
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
        save_();
    }

    // send Serial_change_character (BEL) on change of state
    static bool prev_inhibit = false;
    static byte prev_ambient = 0;
    static bool prev_lamp = false;
    if (get_inhibit_() != prev_inhibit ||
        ambientDimmer_->get_stop() != prev_ambient ||
        lamp_->get() != prev_lamp
       )
    {
        prev_inhibit = get_inhibit_();
        prev_ambient = ambientDimmer_->get_stop();
        prev_lamp = lamp_->get();
        ser_->print(Serial_change_character);
    }
}


/*!
    @brief  Convert a string of digits to an 8-bit unsigned integer.

    Negative numbers are not supported. Overflows are not handled.
    @param str  The string.
    @return The number.
            Returns 0 if the first character of the string is not a digit.
    @see struint16_
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
    @brief  Convert a string of digits to a 16-bit unsigned integer.

    Negative numbers are not supported. Overflows are not handled.
    @param str  The string.
    @return The number.
            Returns 0 if the first character of the string is not a digit.
    @see strbyte_
*/
uint16_t AlarmClockCLI::struint16_(const char* str)
{
    uint16_t result = 0;
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

    If the first character of the string is a digit, it skips all following
    characters until it finds one that is not a digit and then calls
    find_digit_.
    @param str  The string.
    @return Pointer to the first digit of the next number.
    @see find_digit_
*/
char* AlarmClockCLI::find_next_digit_(char* str)
{
    while (isDigit(*str) && *str != '\0') str++;
    str = find_digit_(str);
    return str;
}


void AlarmClockCLI::indent_(byte level)
{
    // No tabs in YAML!
    for (byte i = 0; i < level * Serial_indentation_width; i++)
    {
        ser_->print(' ');
    }
}


void AlarmClockCLI::yaml_time_(byte hours, byte minutes)
{
    ser_->print('\'');
    ser_->print(hours);
    ser_->print(':');
    if (minutes < 10) ser_->print('0');
    ser_->print(minutes);
    ser_->println('\'');
}


void AlarmClockCLI::yaml_time_(byte hours, byte minutes, byte seconds)
{
    ser_->print('\'');
    ser_->print(hours);
    ser_->print(':');
    if (minutes < 10) ser_->print('0');
    ser_->print(minutes);
    ser_->print(':');
    if (seconds < 10) ser_->print('0');
    ser_->print(seconds);
    ser_->println('\'');
}


void AlarmClockCLI::yaml_time_(HoursMinutes time)
{
    yaml_time_(time.hours, time.minutes);
}


//! Print out an alarm as YAML.
//! A valid index must be passed to this function.
//! This function does not print YAML_begin nor YAML_end
void AlarmClockCLI::yaml_alarm_(byte index, bool comments)
{
    ser_->print(F("alarm"));
    ser_->print(index);
    ser_->println(':');

    indent_(1);
    ser_->print(F("enabled: "));
    switch (alarms_[index]->get_enabled())
    {
        case Off:
            ser_->println(F("'OFF'"));
            break;

        case Single:
            ser_->println(F("SGL"));
            break;

        case Repeat:
            ser_->println(F("RPT"));
            break;

        case Skip:
            ser_->println(F("SKP"));
            break;
    }

    indent_(1);
    ser_->print(F("dow: 0x"));
    // Filter out bit 0. It has no meaning and should always be zero.
    ser_->print(alarms_[index]->get_days_of_week().days_of_week & 0xFE, HEX);
    if (comments)
    {
        ser_->print(F("  #"));
        for (byte i = 1; i <= 7; i++)
        {
            ser_->print(' ');
            ser_->print(days_of_the_week_names_short[
                    alarms_[index]->get_days_of_week().getDayOfWeek(i) ?
                    i : 0]);  // 0 - two spaces
        }
    }
    ser_->println();

    indent_(1);
    ser_->print(F("time: "));
    yaml_time_(alarms_[index]->get_time());

    indent_(1);
    ser_->println(F("snz:"));
    indent_(2);
    ser_->print(F("time: "));
    ser_->print(alarms_[index]->get_snooze().time_minutes);
    if (comments) ser_->println(F("  # min"));
    else ser_->println();
    indent_(2);
    ser_->print(F("count: "));
    ser_->println(alarms_[index]->get_snooze().count);

    indent_(1);
    ser_->println(F("sig:"));
    indent_(2);
    ser_->print(F("ambient: "));
    ser_->println(alarms_[index]->get_signalization().ambient);
    indent_(2);
    ser_->print(F("lamp: "));
    ser_->println(alarms_[index]->get_signalization().lamp);
    indent_(2);
    ser_->print(F("buzzer: "));
    ser_->print(alarms_[index]->get_signalization().buzzer);
    if (comments)
    {
        ser_->print(F("  # "));
        ser_->print(signalization_melody_start);
        ser_->print('-');
        ser_->print(signalization_melody_end);
        ser_->print(F(" are melodies"));
    }
    ser_->println();
}


//! Print out state of the CountdownTimer as YAML.
//! This function does not print YAML_begin nor YAML_end
void AlarmClockCLI::yaml_timer_()
{
    ser_->println(F("timer:"));
    indent_(1);
    ser_->print(F("running: "));
    ser_->println(timer_->get_running() ? F("true") : F("false"));
    indent_(1);
    ser_->print(F("time left: "));
    TimeSpan time_left(timer_->time_left);
    yaml_time_(time_left.hours(), time_left.minutes(), time_left.seconds());
    indent_(1);
    ser_->print(F("ambient: "));
    ser_->println(timer_->events.ambient);
    indent_(1);
    ser_->print(F("lamp: "));
    ser_->println(timer_->events.lamp ? 1 : 0);
    indent_(1);
    ser_->print(F("buzzer: "));
    ser_->println(timer_->events.buzzer ? 1 : 0);
}


void AlarmClockCLI::print_error(SerialCLI::error_t code)
{
    ser_->println();
    ser_->print(F("err 0x"));
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
    ser_->println();
    ser_->print(F(
        "Help:\r\n"
        "  ver - static info\r\n"
        "  amb - get ambient 0-255\r\n"
        "  amb{n} - ambient 0-255\r\n"
        "  lamp - get 0|1\r\n"
        "  lamp{l} - set 0|1\r\n"
        "  inh - get inhibit 0|1\r\n"
        "  inh{i} - set inhibit 0|1\r\n"
        "  sel{i} - select alarm\r\n"
        "  la - list all alarms\r\n"
        "  stop - stop button\r\n"
        "  active - print active alarms\r\n"
        "  Selected alarm:\r\n"
        "    ls - list\r\n"
        "    en-off/en-sgl/en-rpt/en-skp - enable\r\n"
        "    time{h}:{m}\r\n"
        "    dow{d}:{s} - set day {d}1-7 of week to {s}1|0\r\n"
        "    snz{t};{c} - set snooze: time{t}min;count{c}\r\n"
        "    sig{a};{l};{b} - set signalization: ambient{a}0-255;lamp{l}0|1;buzzer{b}0|1|"
    ));
    ser_->print(signalization_melody_start);
    ser_->print('-');
    ser_->println(signalization_melody_end);

    ser_->print(F(
        "  sav - save all\r\n"
        "  RTC:\r\n"
        "    rtc - get RTC time\r\n"
        "    sd{YYYY-MM-DD} - set RTC date\r\n"
        "    st{hh:mm[:ss]} - set RTC time\r\n"
        "  Timer:\r\n"
        "    tmr - get timer time\r\n"
        "    tmr{hh:mm:ss} - set timer time\r\n"
        "    tme - get timer events\r\n"
        "    tme{a};{l};{b} - set timer events\r\n"
        "    tmr-start/tmr-stop\r\n"
        "  Sound (testing only):\r\n"
        // The `tone` command would not fit in the buffer if frequency wasn't
        // divided by 10.
        "    tone{f/10};{a}\r\n"
        "    silence\r\n"
        "    notone\r\n"
        "  Melodies:\r\n"
        "    melody{i} - play melody 0-15\r\n"
        "  EEPROM:\r\n"
        "    eer{aaaa} - read data from address\r\n"
        "    eew{aaaa};{ddd} - write data to address\r\n"
    ));

    print_error(kNotFound);
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

    alarms_[sel_alarm_index_]->set_enabled(status);

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



/*!
    @brief  Does nothing.
    This is useful when a program communicating with this device needs to get
    a prompt as quickly as possible.

    This command is not documented in CLI `help`, because it is useless for a
    human user.
*/
SerialCLI::error_t AlarmClockCLI::cmd_sync_(char *ignored)
{
    (void)ignored;
    return 0;
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
        ser_->println(YAML_begin);
        ser_->println(F("ambient:"));
        indent_(1);
        ser_->print(F("current: "));
        ser_->println(ambientDimmer_->get_value());
        indent_(1);
        ser_->print(F("target: "));
        ser_->println(ambientDimmer_->get_stop());
        ser_->println(YAML_end);
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
        ser_->println(YAML_begin);
        ser_->print(F("lamp: "));
        ser_->println(lamp_->get());
        ser_->println(YAML_end);
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
        ser_->println(YAML_begin);
        ser_->print(F("inhibit: "));
        ser_->println(get_inhibit_());
        ser_->println(YAML_end);
        return 0;
    }
    set_inhibit_(strbyte_(status));
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_en_(char *type)
{
    if (strcmp_P(type, PSTR("en-off")) == 0)
        return set_enabled_(Off);

    if (strcmp_P(type, PSTR("en-sgl")) == 0)
        return set_enabled_(Single);

    if (strcmp_P(type, PSTR("en-rpt")) == 0)
        return set_enabled_(Repeat);

    if (strcmp_P(type, PSTR("en-skp")) == 0)
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

    if (alarms_[sel_alarm_index_]->set_time(hours, minutes))
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

    if (alarms_[sel_alarm_index_]->set_day_of_week(day, status))
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

    if (alarms_[sel_alarm_index_]->set_snooze(time, count))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_sig_(char *sig)
{
    if (sel_alarm_index_ == sel_alarm_index_none_) return kNothingSelected;

    byte ambient, buzzer;
    bool lamp;

    sig = find_next_digit_(sig);
    if (*sig == '\0') return kArgument;
    ambient = strbyte_(sig);
    sig = find_next_digit_(sig);
    if (*sig == '\0') return kArgument;
    lamp = strbyte_(sig);
    sig = find_next_digit_(sig);
    if (*sig == '\0') return kArgument;
    buzzer = strbyte_(sig);

    if (alarms_[sel_alarm_index_]->set_signalization(ambient, lamp, buzzer))
    {
        change_ = true;
        return 0;
    }
    else return kArgument;
}


SerialCLI::error_t AlarmClockCLI::cmd_stop_(char *ignored)
{
    (void)ignored;
    for (byte i = 0; i < alarms_count; i++) alarms_[i]->ButtonStop();
    timer_->ButtonStop();
    return 0;
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
        ser_->println(YAML_begin);
        yaml_timer_();
        ser_->println(YAML_end);
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
        ser_->println(YAML_begin);
        yaml_timer_();
        ser_->println(YAML_end);
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

    ser_->println(YAML_begin);
    ser_->println(F("rtc:"));
    indent_(1);
    ser_->print(F("time: "));
    char buff[] = "YYYY-MM-DD hh:mm:ss";
    ser_->println(now_.toString(buff));

    indent_(1);
    ser_->print(F("dow: "));
    ser_->println(days_of_the_week_names_short[
            (now_.dayOfTheWeek() == 0) ? 7 : now_.dayOfTheWeek()
            ]);

    ser_->println(YAML_end);
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_ls_(char *ignored)
{
    (void)ignored;

    if (sel_alarm_index_ == sel_alarm_index_none_)
        return kNothingSelected;

    ser_->println(YAML_begin);
    yaml_alarm_(sel_alarm_index_, true);
    ser_->println(YAML_end);

    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_la_(char *ignored)
{
    (void)ignored;
    ser_->println(YAML_begin);

    // Comments are disabled to make the message shorter. Otherwise, printing
    // all the alarms could take way too long and interfere with normal
    // operation of the alarm clock.
    for (byte i = 0; i < alarms_count; i++)
        yaml_alarm_(i, false);

    ser_->println(YAML_end);

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
    DateTime new_date(year, month, day,
                      now_.hour(), now_.minute(), now_.second());
    if (!new_date.isValid()) return kArgument;
    rtc_->adjust(new_date);
    return 0;
}



SerialCLI::error_t AlarmClockCLI::cmd_ver_(char *ignored)
{
    (void)ignored;
    ser_->println(YAML_begin);
    ser_->println(F("ver:"));
    indent_(1);
    ser_->print(F("number of alarms: "));
    ser_->println(alarms_count);
    indent_(1);
    ser_->print(F("version: "));
    ser_->println(F(AlarmClock_version));
    indent_(1);
    ser_->print(F("build time: "));
    ser_->println(F(AlarmClock_build_time));
    ser_->println(YAML_end);

    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_tone_(char *args)
{
#ifdef active_buzzer
    (void)args;
    return kUnsupported;
#else
    uint16_t frequency;
    uint8_t amplitude;

    args = find_next_digit_(args);
    if (*args == '\0') return kArgument;
    frequency = struint16_(args) * 10;
    if (frequency == 0 || frequency > 20000) return kArgument;
    args = find_next_digit_(args);
    if (*args == '\0') return kArgument;
    amplitude = strbyte_(args);

    sine_->tone(pin_buzzer, frequency, amplitude);
    return 0;
#endif
}


SerialCLI::error_t AlarmClockCLI::cmd_silence_(char *ignored)
{
    (void)ignored;
#ifdef active_buzzer
    return kUnsupported;
#else
    sine_->silence(pin_buzzer);
    return 0;
#endif
}


SerialCLI::error_t AlarmClockCLI::cmd_notone_(char *ignored)
{
    (void)ignored;
#ifdef active_buzzer
    return kUnsupported;
#else
    sine_->noTone(pin_buzzer);
    return 0;
#endif
}


SerialCLI::error_t AlarmClockCLI::cmd_melody_(char *id)
{
#ifdef active_buzzer
    return kUnsupported;
#else
    id = find_digit_(id);
    if (*id == '\0')
    {
        if (buzzer_playing_)
        {
            buzzer_->set_ringing(ringing_off);
            buzzer_playing_ = false;
        }
        return 0;
    }

    byte id_num = strbyte_(id);
    // We must keep BuzzerManager's on_count_ correct.
    if (buzzer_playing_) buzzer_->set_ringing(ringing_off);
    buzzer_->set_ringing((BuzzerTone)(ringing_melody0 + id_num));
    buzzer_playing_ = true;
    return 0;
#endif
}


SerialCLI::error_t AlarmClockCLI::cmd_eer_(char *args)
{
    args = find_next_digit_(args);
    if (*args == '\0') return kArgument;
    uint16_t address = struint16_(args);
    if (address >= EEPROM_size) return kArgument;

    ser_->println(YAML_begin);
    ser_->println(F("EEPROM:"));
    indent_(1);
    ser_->print(address);
    ser_->print(": ");
    ser_->println(EEPROM.read(address));
    ser_->println(YAML_end);
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_eew_(char *args)
{
    args = find_next_digit_(args);
    if (*args == '\0') return kArgument;
    uint16_t address = struint16_(args);
    if (address >= EEPROM_size) return kArgument;
    args = find_next_digit_(args);
    if (*args == '\0') return kArgument;
    byte data = strbyte_(args);
    EEPROM.update(address, data);
    return 0;
}


SerialCLI::error_t AlarmClockCLI::cmd_active_(char *ignored)
{
    (void)ignored;
    ser_->println(YAML_begin);
    ser_->println(F("active alarms:"));
    for (uint8_t i = 0; i < alarms_count; i++)
    {
        if (alarms_[i]->get_active())
        {
            indent_(1);
            ser_->print(F("- "));
            ser_->println(i);
        }
    }
    ser_->println(YAML_end);
    return 0;
}
