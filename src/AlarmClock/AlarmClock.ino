/*
 Name:		AlarmClock.ino
 Created:	6/12/2019 5:26:39 PM
 Author:	Ondra

Days of week in GUI and EEPROM: 1=Monday

# TODO implement watchdog - external or internal (bootloader problems - flash optiboot)

Code directives:
 - all comments in English

 - Todo comments must contain '# TODO' (for automated searching)

 - constants should be #defined or const, never write te actual number to the code

 - millis() checking: https://www.baldengineer.com/arduino-how-do-you-reset-millis.html
     if ((unsigned long)(millis() - previousMillis) >= interval)

 - returning array: https://www.tutorialspoint.com/cplusplus/cpp_return_arrays_from_functions.htm

 - declaring variables in switch - case: https://forum.arduino.cc/index.php?topic=64407.0

 - custom lcd characters: https://maxpromer.github.io/LCD-Character-Creator/
*/


#include "Settings.h"
#include "Constants.h"


enum SelfTest_level {
    POST,
    time
};


#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Bounce2.h>
#include <Encoder.h>
#include "Alarm.h"
//#include "CountdownTimer.h"  // # TODO implement CountdownTimer
#include "PWMDimmer.h"
#include "SerialCLI.h"
#include "GUI.h"
#include "LCDchars.h"

// function prototypes
#ifdef VisualStudio
unsigned int SelfTest(SelfTest_level level);
#endif
// Hardware
void lamp(boolean status);
boolean get_lamp();
void buzzerTone(unsigned int freq, unsigned long duration = 0); // specifies default duration=0
//void buzzerNoTone();

// Arduino IDE needs these before SerialCLI definition:
void writeEEPROM();
void set_inhibit(boolean status);
boolean get_inhibit();


// Global variables
LiquidCrystal_I2C lcd(I2C_LCD_address, LCD_width, LCD_height);
RTC_DS3231 rtc; // DS3231
Bounce buttons[button_count];
Encoder encoder(pin_encoder_clk, pin_encoder_dt);
AlarmClass alarms[alarms_count];
//CountdownTimerClass countdownTimer;  // # TODO implement CountdownTimer
PWMDimmerClass ambientDimmer(pin_ambient);
DateTime now;
SerialCLIClass CLI(alarms, writeEEPROM, &rtc, &ambientDimmer, lamp, get_lamp);
GUIClass GUI(alarms, writeEEPROM, &rtc, &encoder,
             &buttons[button_index_encoder], &lcd, set_inhibit, get_inhibit,
             &ambientDimmer, lamp, get_lamp);


unsigned long loop_rtc_previous_millis = 0;

boolean inhibit = false;
unsigned long inhibit_previous_millis = 0;

boolean lamp_status = false;

#ifdef active_buzzer
unsigned long active_buzzer_previous_millis = 0;
unsigned long active_buzzer_duration = 0;
boolean active_buzzer_status = false;
#endif


void setup() {
    pinMode(pin_ambient, OUTPUT);
    pinMode(pin_lamp, OUTPUT);
    pinMode(pin_buzzer, OUTPUT);
    pinMode(pin_LCD_enable, OUTPUT);

    buttons[button_index_snooze].attach(pin_button_snooze, INPUT_PULLUP); // # TODO DEBUG only, then switch to external pull-ups
    buttons[button_index_stop].attach(pin_button_stop, INPUT_PULLUP);
    buttons[button_index_encoder].attach(pin_encoder_sw);  // The module already has its own pull-up
    for (byte i = 0; i < button_count; i++) buttons[i].interval(button_debounce_interval);

    init_hardware();

    Wire.begin();
    Serial.begin(9600);

    lcd_on();

    unsigned int error = SelfTest(POST);
    if ((error & error_critical_mask) == 0) error |= (readEEPROM() ? 0 : error_EEPROM);
    error |= SelfTest(time); // rtc.begin() can be omited (only calls Wire.begin())

    if (error & error_time_lost) {
        Serial.println(F("RTC time lost"));
        // # TODO
    }

    if ((error & error_critical_mask) != 0) {
        factory_reset(); // # TODO show the error first, write to log if it is not EEPROM error, the wait for the user
    }

    Serial.println(F("boot"));
}

void loop() {
    if ((unsigned long)(millis() - loop_rtc_previous_millis) >= 800UL) {
        now = rtc.now(); // # TODO + summer_time
        loop_rtc_previous_millis = millis();
    }
    CLI.loop(now);
    GUI.loop(now);
    for (byte i = 0; i < alarms_count; i++) alarms[i].loop(now);
    //countdownTimer.loop();  // # TODO implement CountdownTimer
    ambientDimmer.loop();

    // buttons
    for (byte i = 0; i < button_count; i++) buttons[i].update();
    if (buttons[button_index_snooze].fell()) {
        for (byte i = 0; i < alarms_count; i++) alarms[i].button_snooze();
        DEBUG_println(F("snooze pressed"));
    }
    if (buttons[button_index_stop].fell()) {
        for (byte i = 0; i < alarms_count; i++) alarms[i].button_stop();
        DEBUG_println(F("stop pressed"));
    }
    if (inhibit && (unsigned long)(millis() - inhibit_previous_millis) >= Alarm_inhibit_duration) {
        set_inhibit(false);
    }

    // active buzzer
#ifdef active_buzzer
    if (active_buzzer_status && active_buzzer_duration > 0 &&
        (unsigned long)(millis() - active_buzzer_previous_millis) >= active_buzzer_duration) {
        buzzerNoTone();
    }
#endif
}

void init_hardware() {
    for (byte i = 0; i < alarms_count; i++)
        alarms[i].set_hardware(lamp, &ambientDimmer, buzzerTone, buzzerNoTone, writeEEPROM);

    // # TODO implement CountdownTimer WARNING: ambient implementation changed.
    //countdownTimer.set_hardware(lamp, ambient, buzzerTone, buzzerNoTone);
}


/*
EEPROM
*/
boolean readEEPROM() {
#ifdef DEBUG
    Serial.print(F("EEPROM dump"));
    byte val;
    for (byte i = 0; i < EEPROM_DEBUG_dump_length; i++) {
        if (i % 16 == 0) {
            Serial.println();
            if (i < 16) Serial.print('0');
            Serial.print(i, HEX); // address
            Serial.print("  ");
        }
        val = EEPROM.read(i);
        if (val < 16) Serial.print('0');
        Serial.print(val, HEX);
        Serial.print(' ');
    }
    Serial.println();
#endif // DEBUG


    boolean error = false;
    // basic config:

    // alarms:
    for (byte i = 0; i < alarms_count && !error; i++) {
        byte data[EEPROM_AlarmClass_record_length];
        for (byte j = 0; j < EEPROM_AlarmClass_record_length; j++) {
            data[j] = EEPROM.read((i * EEPROM_AlarmClass_record_length) + j + EEPROM_alarms_offset);
        }
        error |= !alarms[i].readEEPROM(data);
    }

    return !error;
}

void writeEEPROM() {
    DEBUG_println(F("EEPROM write"));
    // basic config:

    // alarms:
    for (byte i = 0; i < alarms_count; i++) {
#if defined(DEBUG) && defined(DEBUG_EEPROM_writes)
        Serial.print(F("Alarm "));
        Serial.println(i);
#endif
        byte * data = alarms[i].writeEEPROM();
        for (byte j = 0; j < EEPROM_AlarmClass_record_length; j++) {
            unsigned int address = (i * EEPROM_AlarmClass_record_length) + j + EEPROM_alarms_offset;
#if defined(DEBUG) && defined(DEBUG_EEPROM_writes)
            Serial.print(F("Saving "));
            if (data[j] < 16) Serial.print('0');
            Serial.print(data[j], HEX);
            Serial.print(F(" to "));
            if (address < 16) Serial.print('0');
            Serial.println(address, HEX);
#endif // DEBUG
            EEPROM.update(address, data[j]);
        }
    }
}

/*
Factory reset
*/
void factory_reset() {
    Serial.println(F("Factory reset"));
    for (byte i = 0; i < alarms_count; i++) alarms[i] = AlarmClass();
    //countdownTimer = CountdownTimerClass();  // # TODO implement CountdownTimer

    writeEEPROM();
}


/*
LCD
*/
boolean lcd_on() {
    digitalWrite(pin_LCD_enable, HIGH);
    delay(100);
    if (I2C_ping(I2C_LCD_address)) {
        lcd.init();
        lcd.backlight();
        // Add custom characters
        lcd.createChar(LCD_char_home_index, LCD_char_home);
        lcd.createChar(LCD_char_bell_index, LCD_char_bell);
        lcd.createChar(LCD_char_timer_index, LCD_char_timer);
        lcd.createChar(LCD_char_apply_index, LCD_char_apply);
        lcd.createChar(LCD_char_cancel_index, LCD_char_cancel);

        return true;
    }
    else return false;
}

void lcd_off() {
    digitalWrite(pin_LCD_enable, LOW);
}


/*
Self test
*/
unsigned int SelfTest(SelfTest_level level) {
    unsigned int error = 0; // each bit signalizes an error

    if (level == POST || level == time) {
        if (!I2C_ping(I2C_DS3231_address)) error |= error_I2C_ping_DS3231;

    }

    if (level == time) {
        if ((error & error_I2C_ping_DS3231) == 0) { // DS3231 is responding ((POST || time) code block was executed earlier)
            if (rtc.lostPower()) error |= error_time_lost;
            if (rtc.now().year() == 2000) error |= error_time_lost;
        }
    }

    if (level == POST) {
        //digitalWrite(..., HIGH);  // # TODO
        delay(400);
        //digitalWrite(..., LOW);
        buzzerTone(1000, 100);
    }

    return error;
}

boolean I2C_ping(byte addr) {
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
}

/*
Hardware
Included classes can control the hardware trough these functions
*/
void lamp(boolean status)
{
    digitalWrite(pin_lamp, status);
    lamp_status = status;
}

boolean get_lamp() { return lamp_status; }

void buzzerTone(unsigned int freq, unsigned long duration)
{
    // default value duration=0 specified in prototype
#ifdef active_buzzer
    active_buzzer_duration = duration;
    active_buzzer_previous_millis = millis();
    active_buzzer_status = true;
    digitalWrite(pin_buzzer, HIGH);

#else
    tone(pin_buzzer, freq, duration);
#endif
}

void buzzerNoTone()
{
#ifdef active_buzzer
    active_buzzer_status = false;
    digitalWrite(pin_buzzer, LOW);

#else
    noTone(pin_buzzer);
#endif
}


/*
Utils
*/
void set_inhibit(boolean status) {
    inhibit_previous_millis = millis();
    inhibit = status;
    for (byte i = 0; i < alarms_count; i++) alarms[i].set_inhibit(status);
    if (status) DEBUG_println(F("inhibit enabled"));
    else DEBUG_println(F("inhibit disabled"));
}
boolean get_inhibit() { return inhibit; }
