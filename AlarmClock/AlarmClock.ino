/*!
    @file
*/

/*
Days of week in GUI and EEPROM: 1=Monday


Code directives:
 - try to follow https://google.github.io/styleguide/cppguide.html

 - all comments in English

 - TODO comments must contain 'TODO' (for automated searching)

 - constants should be #defined or const, never write the actual number to the code

 - millis() checking: https://www.baldengineer.com/arduino-how-do-you-reset-millis.html
     if ((unsigned long)(millis() - previousMillis) >= interval)

 - returning array: https://www.tutorialspoint.com/cplusplus/cpp_return_arrays_from_functions.htm

 - declaring variables in switch - case: https://forum.arduino.cc/index.php?topic=64407.0

 - custom LCD characters: https://maxpromer.github.io/LCD-Character-Creator/
*/


#if 0
// needed because of platformio, unused
#include <SPI.h>
#endif


#include "Settings.h"
#include "Constants.h"


enum SelfTest_level
{
    POST,
    time
};


#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Bounce2.h>
#include <Encoder.h>
#include <Alarm.h>
#include <CountdownTimer.h>
#include <PWMDimmer.h>
#include <AlarmClockCLI.h>
#include <GUI.h>
#include <LCDchars.h>
#include <HALbool.h>
#include <BuzzerManager.h>

#include <PWMSine.h>

#ifdef internal_WDT
#include <avr/wdt.h>
#endif

// Function prototypes
// Hardware
void lamp_set(bool status);

// Arduino IDE needs these before SerialCLI definition:
void writeEEPROM();
void set_inhibit(bool status);
bool get_inhibit();


// Global variables
LiquidCrystal_I2C lcd(I2C_LCD_address, LCD_width, LCD_height);
RTC_DS3231 rtc; // DS3231
Bounce buttons[button_count];
Encoder encoder(pin_encoder_clk, pin_encoder_dt);
Alarm * alarms[alarms_count];
PWMDimmer ambientDimmer(pin_ambient);
HALbool lamp(lamp_set);
void set_backlight_permanent(bool s);  // platformio needs this
HALbool permanent_backlight(set_backlight_permanent);
PWMSine sine;
BuzzerManager buzzer(pin_buzzer, sine);
DateTime now;
CountdownTimer countdown_timer(ambientDimmer, lamp, buzzer);
AlarmClockCLI myCLI(Serial, alarms, &rtc, writeEEPROM, &ambientDimmer, &lamp,
                    &countdown_timer, sine, buzzer, set_inhibit, get_inhibit);
GUI myGUI(alarms, writeEEPROM, rtc, encoder,
          buttons[button_index_encoder], lcd, set_inhibit, get_inhibit,
          ambientDimmer, lamp, countdown_timer);


unsigned long loop_rtc_prev_millis = 0;

bool inhibit = false;
unsigned long inhibit_prev_millis = 0;

#ifdef internal_WDT
byte WDT_prev_seconds = 61;  // 61 to avoid random behavior
#endif


void setup()
{
    pinMode(pin_buzzer_emergency, OUTPUT);
    digitalWrite(pin_buzzer_emergency, HIGH);
    pinMode(pin_ambient, OUTPUT);
    pinMode(pin_lamp, OUTPUT);
    pinMode(pin_buzzer, OUTPUT);
    pinMode(pin_LCD_BLU, OUTPUT);
    digitalWrite(pin_LCD_BLU, LOW);

    buttons[button_index_snooze].attach(pin_button_snooze, INPUT);
    buttons[button_index_stop].attach(pin_button_stop, INPUT);
    buttons[button_index_encoder].attach(pin_encoder_sw, INPUT);
    for (byte i = 0; i < button_count; i++) buttons[i].interval(button_debounce_interval);

    init_alarms();

    Wire.begin();
    rtc.begin();
    Serial.begin(9600);
    sine.begin();

    lcd_init();

    unsigned int err = SelfTest(POST);
    if (!(err & err_critical_mask))
        err |= (readEEPROM() ? 0 : err_EEPROM);
    err |= SelfTest(time);

    if (err & err_I2C_ping_DS3231)
    {
        Serial.println(F("RTC not connected. Cannot continue."));
#ifdef active_buzzer
        digitalWrite(pin_buzzer, HIGH);
#else
        tone(pin_buzzer, 1000);
#endif
        for(;;);
        // TODO handle RTC ping failed
    }

    if (err & err_time_lost)
    {
        Serial.println(F("RTC time lost"));
        // TODO handle RTC time lost
    }

    if (err & err_critical_mask)
    {
        // TODO write to EEPROM log if it is not EEPROM error,
        // then wait for the user.
        Serial.print(F("POST critical error: 0x"));
        Serial.println(err, HEX);

        if (!(err & err_I2C_ping_DS3231))
            factory_reset();
    }

    Serial.println(F("boot"));
    Serial.print(Serial_change_character);

#ifdef internal_WDT
    wdt_enable(WDTO_4S);
    DEBUG_println(F("WDT enabled"));
#endif

}


void loop()
{
    buzzer.loop();

    if ((unsigned long)(millis() - loop_rtc_prev_millis) >= 800UL)
    {
        now = rtc.now(); // # TODO + summer_time
        loop_rtc_prev_millis = millis();
    }

#ifdef internal_WDT
    // WDT depends on RTC time. This allows it to detect RTC failures.
    if (now.second() != WDT_prev_seconds)
    {
        wdt_reset();
        WDT_prev_seconds = now.second();
    }
#endif

    myCLI.loop(now);
    myGUI.loop(now);
    for (byte i = 0; i < alarms_count; i++) alarms[i]->loop(now);
    countdown_timer.loop(now);
    ambientDimmer.loop();

    // buttons
    for (byte i = 0; i < button_count; i++) buttons[i].update();
    if (buttons[button_index_snooze].fell())
    {
        switch (myGUI.get_backlight())
        {
            case GUI::off:
                myGUI.set_backlight(GUI::on_dim);
                break;
            case GUI::on_dim:
                myGUI.set_backlight(GUI::on_full);
                break;
            default:
                for (byte i = 0; i < alarms_count; i++) alarms[i]->ButtonSnooze();
                DEBUG_println(F("snooze pressed"));
                break;
        }
    }
    if (buttons[button_index_stop].fell())
    {
        if(myGUI.get_backlight() == GUI::off) myGUI.set_backlight(GUI::on_full);
        for (byte i = 0; i < alarms_count; i++) alarms[i]->ButtonStop();
        countdown_timer.ButtonStop();
        DEBUG_println(F("stop pressed"));
    }
    if (inhibit && (unsigned long)(millis() - inhibit_prev_millis) >= Alarm_inhibit_duration)
    {
        set_inhibit(false);
    }
}


/*!
    @brief  Initialize the `alarms` array.

    This uses dynamic memory allocation.
    Do not call twice without deleting the individual alarms.
*/
void init_alarms()
{
    for (byte i = 0; i < alarms_count; i++)
        alarms[i] = new Alarm(lamp, ambientDimmer, buzzer, writeEEPROM,
                              alarm_activation_callback, alarm_stop_callback);
}



void alarm_activation_callback()
{
    permanent_backlight.set(true);
}


// WARNING: stop_callback is also called when the alarm times out.
void alarm_stop_callback()
{
    permanent_backlight.set(false);
}



/*
EEPROM
*/
bool readEEPROM()
{
    bool err = false;
    // basic config:

    // alarms:
    for (byte i = 0; i < alarms_count && !err; i++)
    {
        byte data[Alarm::EEPROM_length];
        for (byte j = 0; j < Alarm::EEPROM_length; j++)
        {
            data[j] = EEPROM.read((i * Alarm::EEPROM_length) + j + EEPROM_alarms_offset);
        }
        err |= !alarms[i]->ReadEEPROM(data);
    }

    return !err;
}


void writeEEPROM()
{
    DEBUG_println(F("EEPROM write"));
    // basic config:

    // alarms:
    for (byte i = 0; i < alarms_count; i++)
    {
#if defined(DEBUG) && defined(DEBUG_EEPROM_writes)
        Serial.print(F("Alarm "));
        Serial.println(i);
#endif
        byte * data = alarms[i]->WriteEPROM();
        for (byte j = 0; j < Alarm::EEPROM_length; j++)
        {
            unsigned int address = (i * Alarm::EEPROM_length) + j + EEPROM_alarms_offset;
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


/*!
    @brief  Perform a factory reset

    Must NOT be called before init_alarms.
*/
void factory_reset()
{
    Serial.println(F("Factory reset"));

    for (byte i = 0; i < alarms_count; i++)
        delete alarms[i];

    init_alarms();

    writeEEPROM();
}



/*
LCD
*/
bool lcd_init()
{
    if (I2C_ping(I2C_LCD_address))
    {
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



/*
Self test
*/
unsigned int SelfTest(SelfTest_level level)
{
    unsigned int err = 0; // each bit signalizes an error

    if (level == POST || level == time)
    {
        if (!I2C_ping(I2C_DS3231_address))
            err |= err_I2C_ping_DS3231;
    }

    if (level == time)
    {
        if (!(err & err_I2C_ping_DS3231))
        {
            // DS3231 is responding ((POST || time) code block was executed
            // earlier)
            if (rtc.lostPower()) err |= err_time_lost;
            if (rtc.now().year() == 2000) err |= err_time_lost;
        }
    }

    if (level == POST)
    {
        //digitalWrite(..., HIGH);  // # TODO
#ifdef active_buzzer
        digitalWrite(pin_buzzer, HIGH);
#else
        tone(pin_buzzer, 1000);
#endif
        delay(400);
#ifdef active_buzzer
        digitalWrite(pin_buzzer, LOW);
#else
        noTone(pin_buzzer);
#endif
        //digitalWrite(..., LOW);
    }

    return err;
}


bool I2C_ping(byte addr)
{
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
}



/*
Hardware
Included classes can control the hardware trough these functions
*/
void lamp_set(bool status)
{
    digitalWrite(pin_lamp, status);
}



/*
Utils
*/
void set_inhibit(bool status)
{
    inhibit_prev_millis = millis();
    inhibit = status;
    for (byte i = 0; i < alarms_count; i++) alarms[i]->set_inhibit(status);
    // Removing the braces around DEBUG_println in the else statement causes a
    // compiler error if DEBUG is turned off.
    if (status) { DEBUG_println(F("inhibit enabled")); }
    else { DEBUG_println(F("inhibit disabled")); }
}


bool get_inhibit() { return inhibit; }


void set_backlight_permanent(bool s)
{
    if (s) myGUI.set_backlight(GUI::permanent);
    else myGUI.set_backlight(GUI::on_full);  // disable permanent backlight
}
