#line 2 "tests.ino"

// Unit tests
// https://github.com/bxparks/AUnit


#include <AUnit.h>

// Symlink src to ../src
#include "src/AlarmClock/Settings.h"
#include "src/AlarmClock/Constants.h"
#include "src/AlarmClock/Alarm.h"
#include "src/AlarmClock/DaysOfWeek.h"
#include "src/AlarmClock/CountdownTimer.h"

#include "Alarm_mockups.h"
#include <RTClib.h>


void setup()
{
    delay(500); // wait for stability on some boards to prevent garbage Serial
    Serial.begin(115200);
    while(!Serial);
    Serial.println();
    Serial.println();
    Serial.println("Arduino-Alarm-Clock tests");
    Serial.print("Compiled on ");
    Serial.print(__DATE__);
    Serial.print(" at ");
    Serial.println(__TIME__);
    Serial.println();
}


void loop()
{
    aunit::TestRunner::run();
}


test(Alarm_trigger)
{
    MockupPWMDimmer ambientDimmer;

    enum test {
        inhibit = 0,
        activate = 1,
        skip = 2,
        last = 3
    };

    test myTest = inhibit;

    while (myTest != last)
    {
        TestAlarm alarm;  // needs to be here to reset prev_activation_millis
        alarm.SetHardware(&lamp, &ambientDimmer, &buzzer,
                          writeEEPROM, activation_callback, stop_callback);
        assertTrue(alarm.set_time(12, 13));
        assertTrue(alarm.set_enabled(Single));
        DaysOfWeek dow;
        dow.DaysOfWeek = 0xFE;
        assertTrue(alarm.set_days_of_week(dow));
        assertTrue(alarm.set_snooze(1, 2));
        assertTrue(alarm.set_signalization(80, true, true));
        assertTrue(alarm.set_inhibit(false));

        reset_alarm_mockups();

        DateTime time(2020, 1, 1, 11, 10, 0);

        switch (myTest)
        {
            case inhibit:
                {
                bool activation_time = false;
                alarm.set_inhibit(true);
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    assertFalse(activated);
                    assertFalse(lamp_status);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        assertEqual(alarm.get_enabled(), Off);  // single
                        activation_time = true;
                    }

                    reset_alarm_mockups();
                    time = time + TimeSpan(30);
                }
                assertTrue(activation_time);
                }
                break;

            case activate:
                {
                bool activation_time = false;
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        assertTrue(activated);
                        assertTrue(lamp_status);
                        assertTrue(buzzer.get_status());
                        assertEqual(alarm.get_enabled(), Off);  // single

                        activation_time = true;
                    }
                    else
                    {
                        assertFalse(activated);
                        assertFalse(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.ButtonStop();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                assertTrue(activation_time);
                }
                break;

            case skip:
                {
                assertTrue(alarm.set_enabled(Skip));

                bool activation_time = false;
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        assertFalse(activated);
                        assertFalse(lamp_status);
                        assertFalse(buzzer.get_status());
                        assertEqual(alarm.get_enabled(), Repeat);  // Skip

                        activation_time = true;
                    }
                    else
                    {
                        assertFalse(activated);
                        assertFalse(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.ButtonStop();
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
    TestAlarm alarm;

    MockupPWMDimmer ambientDimmer;
    alarm.SetHardware(&lamp, &ambientDimmer, &buzzer,
                      writeEEPROM, activation_callback, stop_callback);

    assertTrue(alarm.set_time(12, 13));
    assertTrue(alarm.set_enabled(Single));
    DaysOfWeek dow;
    dow.DaysOfWeek = 0xFE;
    assertTrue(alarm.set_days_of_week(dow));
    assertTrue(alarm.set_snooze(0, 2));
    assertTrue(alarm.set_signalization(80, true, true));

    reset_alarm_mockups();

    DateTime time(2020, 1, 1, 12, 13, 0);  // activate immediately
    for (byte snooze_remaining = 2; snooze_remaining != 255; snooze_remaining--)
    {
        // cannot do this because activated is not set when waking from snooze
        //activated = false;
        alarm.loop(time);

        assertTrue(activated);
        assertTrue(lamp_status);
        assertTrue(buzzer.get_status());

        alarm.ButtonSnooze();
        if (snooze_remaining != 0) assertFalse(lamp_status);
        else assertTrue(lamp_status);
    }
}


test(Alarm_ambient)
{
    enum ambientTest {
        inhibit = 0,
        activate = 1,
        skip = 2,
        // cannot check the next activation because of millis
        last = 3
    };

    ambientTest myTest = inhibit;

    while (myTest != last)
    {
        MockupPWMDimmer ambientDimmer;
        TestAlarm alarm;  // needs to be here to reset prev_activation_millis
        alarm.SetHardware(&lamp, &ambientDimmer, &buzzer,
                          writeEEPROM, activation_callback, stop_callback);
        assertTrue(alarm.set_time(12, 13));
        assertTrue(alarm.set_enabled(Repeat));
        DaysOfWeek dow;
        dow.DaysOfWeek = 0xFE;
        assertTrue(alarm.set_days_of_week(dow));
        assertTrue(alarm.set_snooze(1, 2));
        assertTrue(alarm.set_signalization(80, true, true));
        assertTrue(alarm.set_inhibit(false));

        reset_alarm_mockups();

        DateTime time(2020, 1, 1, 11, 10, 0);

        switch (myTest)
        {
            case inhibit:
                {
                bool activation_time = false;
                bool ambient_activation_time = false;
                alarm.set_inhibit(true);
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    assertFalse(activated);
                    assertFalse(lamp_status);
                    assertFalse(alarm.test_get_ambient_status());
                    assertEqual(ambientDimmer.get_stop(), 0);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        activation_time = true;
                    }

                    DateTime temptime = time + TimeSpan(long(Alarm_ambient_dimming_duration / 1000UL));
                    if (temptime.hour() == 12 && temptime.minute() == 13 && temptime.second() < 30)
                    {
                        ambient_activation_time = true;
                    }

                    reset_alarm_mockups();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                assertTrue(activation_time);
                assertTrue(ambient_activation_time);
                }
                break;

            case activate:
                {
                bool activation_time = false;
                bool ambient_activation_time = false;
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);


                    DateTime temptime = time + TimeSpan(long(Alarm_ambient_dimming_duration / 1000UL));
                    if (temptime.hour() == 12 && temptime.minute() == 13 && temptime.second() < 30)
                    {
                        assertEqual(ambientDimmer.get_stop(), 80);
                        assertTrue(alarm.test_get_ambient_status());
                        ambient_activation_time = true;
                    }
                    else
                    {
                        assertEqual(ambientDimmer.get_stop(), 0);
                        assertFalse(alarm.test_get_ambient_status());
                    }

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        assertTrue(activated);
                        assertTrue(lamp_status);
                        assertTrue(buzzer.get_status());

                        activation_time = true;
                    }
                    else
                    {
                        assertFalse(activated);
                        assertFalse(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.ButtonStop();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                assertTrue(activation_time);
                assertTrue(ambient_activation_time);
                }
                break;

            case skip:
                {
                bool activation_time = false;
                bool ambient_activation_time = false;
                assertTrue(alarm.set_enabled(Skip));
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    assertFalse(activated);
                    assertFalse(lamp_status);
                    assertFalse(alarm.test_get_ambient_status());
                    assertEqual(ambientDimmer.get_stop(), 0);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        activation_time = true;
                    }

                    DateTime temptime = time + TimeSpan(long(Alarm_ambient_dimming_duration / 1000UL));
                    if (temptime.hour() == 12 && temptime.minute() == 13 && temptime.second() < 30)
                    {
                        ambient_activation_time = true;
                    }

                    reset_alarm_mockups();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                assertTrue(activation_time);
                assertTrue(ambient_activation_time);
                }
                break;

        }
        myTest = (ambientTest)(myTest + 1);
    }
}


test(Alarm_EEPROM_read)
{
    TestAlarm alarm;
    MockupPWMDimmer ambientDimmer;
    alarm.SetHardware(&lamp, &ambientDimmer, &buzzer,
                      writeEEPROM, activation_callback, stop_callback);

    // Bad id
    byte data1[Alarm::EEPROM_length] = {
        EEPROM_alarms_id + 1, 23, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.ReadEEPROM(data1));


    // Bad hours
    byte data2[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 24, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.ReadEEPROM(data2));

    // Bad minutes
    byte data3[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 60, 3, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.ReadEEPROM(data3));

    // Bad enabled
    byte data4[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 4, 0x08, 99, 9, 80, 1, 0
    };
    assertFalse(alarm.ReadEEPROM(data4));

    // Bad snooze time
    byte data5[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 100, 9, 80, 1, 0
    };
    assertFalse(alarm.ReadEEPROM(data5));

    // Bad snooze count
    byte data6[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 99, 10, 80, 1, 0
    };
    assertFalse(alarm.ReadEEPROM(data6));

    // Valid alarm
    byte data7[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    assertTrue(alarm.ReadEEPROM(data7));
    assertEqual(alarm.get_time().hours, 23);
    assertEqual(alarm.get_time().minutes, 59);
    assertEqual(alarm.get_enabled(), Skip);
    assertEqual(alarm.get_snooze().time_minutes, 99);
    assertEqual(alarm.get_snooze().count, 9);
    assertEqual(alarm.get_signalization().ambient, 80);
    assertEqual(alarm.get_signalization().lamp, true);
    assertEqual(alarm.get_signalization().buzzer, false);
}


test(Alarm_EEPROM_write)
{
    TestAlarm alarm;
    MockupPWMDimmer ambientDimmer;
    alarm.SetHardware(&lamp, &ambientDimmer, &buzzer,
                      writeEEPROM, activation_callback, stop_callback);

    assertTrue(alarm.set_time(23, 59));
    assertTrue(alarm.set_enabled(Repeat));
    DaysOfWeek dow;
    dow.DaysOfWeek = 0x08;
    assertTrue(alarm.set_days_of_week(dow));
    assertTrue(alarm.set_snooze(99, 9));
    assertTrue(alarm.set_signalization(80, true, false));
    assertTrue(alarm.set_inhibit(false));

    byte * data = alarm.WriteEPROM();

    byte correct_data[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 2, 0x08, 99, 9, 80, 1, 0
    };

    for (byte i = 0; i < Alarm::EEPROM_length; i++)
    {
        assertEqual(data[i], correct_data[i]);
    }
}


test(Timer)
{
    MockupPWMDimmer ambientDimmer;
    CountdownTimer timer(ambientDimmer, lamp, buzzer);
    timer.time_left = 30*60 + 33;
    timer.events = { 120, true, true };
    timer.start();

    reset_alarm_mockups();

    DateTime time(2020, 1, 1, 11, 1, 0);

    bool activation_time = false;
    while (time < DateTime(2020, 1, 1, 11, 32, 0))
    {
        timer.loop(time);

        // 11:31:32 - timer.start() causes time_left to immediately decrement.
        if (time.hour() == 11 && time.minute() == 31 && time.second() == 32)
        {
            assertTrue(lamp_status);
            assertTrue(buzzer.get_status());
            assertEqual(ambientDimmer.get_stop(), 120);

            timer.ButtonStop();

            assertEqual(ambientDimmer.get_stop(), 0);
            assertFalse(lamp_status);
            assertFalse(buzzer.get_status());

            activation_time = true;
        }
        else
        {
            if (!activation_time) assertTrue(timer.get_running());
            else assertFalse(timer.get_running());
            assertFalse(lamp_status);
            assertFalse(buzzer.get_status());
            assertEqual(ambientDimmer.get_stop(), 0);
        }

        reset_alarm_mockups();
        time = time + TimeSpan(1);
    }
    // In case the test code was wrong
    assertTrue(activation_time);

}
