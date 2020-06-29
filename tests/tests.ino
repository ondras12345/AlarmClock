#line 2 "tests.ino"

// Unit tests
// https://github.com/bxparks/AUnit

#include <AUnit.h>

// Symlink src to ../src
#include "src/AlarmClock/Settings.h"
#include "src/AlarmClock/Constants.h"
#include "src/AlarmClock/Alarm.h"
#include "src/AlarmClock/DaysOfWeek.h"
#include "src/AlarmClock/PWMDimmer.h"

#include "Alarm_mockups.h"
#include <RTClib.h>


void setup()
{
    delay(1000); // wait for stability on some boards to prevent garbage Serial
    Serial.begin(115200);
    while(!Serial);
    Serial.println();
    Serial.println();
}


void loop()
{
    aunit::TestRunner::run();
}


test(Alarm_trigger)
{
    AlarmClass alarm;

    pinMode(pin_ambient, OUTPUT);
    PWMDimmerClass ambientDimmer(pin_ambient);
    alarm.set_hardware(lamp, &ambientDimmer, buzzerTone, buzzerNoTone,
                       writeEEPROM, activation_callback, stop_callback);

    enum test {
        inhibit = 0,
        activate = 1,
        last = 2
    };

    test myTest = inhibit;

    while (myTest != last)
    {
        alarm.button_stop();
        assertTrue(alarm.set_time(12, 13));
        assertTrue(alarm.set_enabled(Single));
        DaysOfWeekClass dow;
        dow.DaysOfWeek = 0xFE;
        assertTrue(alarm.set_days_of_week(dow));
        assertTrue(alarm.set_snooze(1, 2));
        assertTrue(alarm.set_signalization(80, true, true));
        assertTrue(alarm.set_inhibit(false));

        reset_alarm_mockups();

        // The alarm would not trigger again if the day was the same
        DateTime time(2020, 1, 1 + byte(myTest), 11, 10, 00);

        switch (myTest)
        {
            case inhibit:
                alarm.set_inhibit(true);
                while (time < DateTime(2020, 1, 1 + byte(myTest), 13, 15, 00))
                {
                    alarm.loop(time);

                    assertFalse(activated);
                    assertFalse(lamp_status);

                    reset_alarm_mockups();
                    time = time + TimeSpan(30);
                }
                break;

            case activate:
                {
                bool activation_time = false;
                while (time < DateTime(2020, 1, 1 + byte(myTest), 13, 15, 00))
                {
                    alarm.loop(time);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        assertTrue(activated);
                        assertTrue(lamp_status);
                        assertTrue(buzzer_status);
                        assertEqual(alarm.get_enabled(), Off);  // single

                        activation_time = true;
                    }
                    else
                    {
                        assertFalse(activated);
                        assertFalse(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.button_stop();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                assertTrue(activation_time);
                }
                break;
        }

        myTest = (test)(myTest + 1);
    }

}


test(Alarm_snooze)
{
    AlarmClass alarm;

    pinMode(pin_ambient, OUTPUT);
    PWMDimmerClass ambientDimmer(pin_ambient);
    alarm.set_hardware(lamp, &ambientDimmer, buzzerTone, buzzerNoTone,
                       writeEEPROM, activation_callback, stop_callback);

    assertTrue(alarm.set_time(12, 13));
    assertTrue(alarm.set_enabled(Single));
    DaysOfWeekClass dow;
    dow.DaysOfWeek = 0xFE;
    assertTrue(alarm.set_days_of_week(dow));
    assertTrue(alarm.set_snooze(0, 2));
    assertTrue(alarm.set_signalization(80, true, true));

    reset_alarm_mockups();

    DateTime time(2020, 1, 1, 12, 13, 00);  // activate immediately
    for (byte snooze_remaining = 2; snooze_remaining != 255; snooze_remaining--)
    {
        // cannot do this because activated is not set when waking from snooze
        //activated = false;
        alarm.loop(time);

        assertTrue(activated);
        assertTrue(lamp_status);
        //assertTrue(buzzer_status);  // depends on millis()

        alarm.button_snooze();
        if (snooze_remaining != 0) assertFalse(lamp_status);
        else assertTrue(lamp_status);
    }
}


test(Alarm_EEPROM_read)
{
    AlarmClass alarm;
    pinMode(pin_ambient, OUTPUT);
    PWMDimmerClass ambientDimmer(pin_ambient);
    alarm.set_hardware(lamp, &ambientDimmer, buzzerTone, buzzerNoTone,
                       writeEEPROM, activation_callback, stop_callback);

    // Bad id
    byte data1[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id + 1, 23, 59, 2, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.readEEPROM(data1));


    // Bad hours
    byte data2[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 24, 59, 2, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.readEEPROM(data2));

    // Bad minutes
    byte data3[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 23, 60, 2, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.readEEPROM(data3));

    // Bad enabled
    byte data4[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.readEEPROM(data4));

    // Bad snooze time
    byte data5[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 23, 59, 2, 0x08, 100, 9, 80, 1, 0
    };
    assertFalse(alarm.readEEPROM(data5));

    // Bad snooze count
    byte data6[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 23, 59, 2, 0x08, 99, 10, 80, 1, 0
    };
    assertFalse(alarm.readEEPROM(data6));

    // Valid alarm
    byte data7[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 23, 59, 2, 0x08, 99, 9, 80, 1, 0
    };
    assertTrue(alarm.readEEPROM(data7));
    assertEqual(alarm.get_time().hours, 23);
    assertEqual(alarm.get_time().minutes, 59);
    assertEqual(alarm.get_enabled(), Repeat);
    assertEqual(alarm.get_snooze().time_minutes, 99);
    assertEqual(alarm.get_snooze().count, 9);
    assertEqual(alarm.get_signalization().ambient, 80);
    assertEqual(alarm.get_signalization().lamp, true);
    assertEqual(alarm.get_signalization().buzzer, false);
}


test(Alarm_EEPROM_write)
{
    AlarmClass alarm;
    pinMode(pin_ambient, OUTPUT);
    PWMDimmerClass ambientDimmer(pin_ambient);
    alarm.set_hardware(lamp, &ambientDimmer, buzzerTone, buzzerNoTone,
                       writeEEPROM, activation_callback, stop_callback);

    assertTrue(alarm.set_time(23, 59));
    assertTrue(alarm.set_enabled(Repeat));
    DaysOfWeekClass dow;
    dow.DaysOfWeek = 0x08;
    assertTrue(alarm.set_days_of_week(dow));
    assertTrue(alarm.set_snooze(99, 9));
    assertTrue(alarm.set_signalization(80, true, false));
    assertTrue(alarm.set_inhibit(false));

    byte * data = alarm.writeEEPROM();

    byte correct_data[EEPROM_AlarmClass_length] = {
        EEPROM_alarms_id, 23, 59, 2, 0x08, 99, 9, 80, 1, 0
    };

    for (byte i = 0; i < EEPROM_AlarmClass_length; i++)
    {
        assertEqual(data[i], correct_data[i]);
    }
}
