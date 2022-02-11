#include <Arduino.h>
#include <unity.h>

#include <RTClib.h>
#include "Alarm_mockups.h"
#include <Alarm.h>
#include <DaysOfWeek.h>
#include <CountdownTimer.h>


void test_Alarm_trigger()
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
        // alarm initialization needs to be here to reset
        // prev_activation_millis
        TestAlarm alarm(lamp, ambientDimmer, buzzer,
                        writeEEPROM, activation_callback, stop_callback);
        TEST_ASSERT_TRUE(alarm.set_time(12, 13));
        TEST_ASSERT_TRUE(alarm.set_enabled(Single));
        DaysOfWeek dow(0xFE);
        TEST_ASSERT_TRUE(alarm.set_days_of_week(dow));
        TEST_ASSERT_TRUE(alarm.set_snooze(1, 2));
        TEST_ASSERT_TRUE(alarm.set_signalization(80, true, true));
        TEST_ASSERT_TRUE(alarm.set_inhibit(false));

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

                    TEST_ASSERT_FALSE(activated);
                    TEST_ASSERT_FALSE(lamp_status);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        TEST_ASSERT_EQUAL_INT(Off, alarm.get_enabled());  // single
                        activation_time = true;
                    }

                    reset_alarm_mockups();
                    time = time + TimeSpan(30);
                }
                TEST_ASSERT_TRUE(activation_time);
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
                        TEST_ASSERT_TRUE(activated);
                        TEST_ASSERT_TRUE(lamp_status);
                        TEST_ASSERT_TRUE(buzzer.get_status());
                        TEST_ASSERT_EQUAL_INT(Off, alarm.get_enabled());  // single

                        activation_time = true;
                    }
                    else
                    {
                        TEST_ASSERT_FALSE(activated);
                        TEST_ASSERT_FALSE(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.ButtonStop();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                TEST_ASSERT_TRUE(activation_time);
                }
                break;

            case skip:
                {
                TEST_ASSERT_TRUE(alarm.set_enabled(Skip));

                bool activation_time = false;
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        TEST_ASSERT_FALSE(activated);
                        TEST_ASSERT_FALSE(lamp_status);
                        TEST_ASSERT_FALSE(buzzer.get_status());
                        TEST_ASSERT_EQUAL_INT(Repeat, alarm.get_enabled());  // Skip

                        activation_time = true;
                    }
                    else
                    {
                        TEST_ASSERT_FALSE(activated);
                        TEST_ASSERT_FALSE(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.ButtonStop();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                TEST_ASSERT_TRUE(activation_time);
                }
                break;
        }

        myTest = (test)(myTest + 1);
    }
}


void test_Alarm_snooze()
{
    MockupPWMDimmer ambientDimmer;

    TestAlarm alarm(lamp, ambientDimmer, buzzer,
                    writeEEPROM, activation_callback, stop_callback);

    TEST_ASSERT_TRUE(alarm.set_time(12, 13));
    TEST_ASSERT_TRUE(alarm.set_enabled(Single));
    DaysOfWeek dow(0xFE);
    TEST_ASSERT_TRUE(alarm.set_days_of_week(dow));
    TEST_ASSERT_TRUE(alarm.set_snooze(0, 2));
    TEST_ASSERT_TRUE(alarm.set_signalization(80, true, true));

    reset_alarm_mockups();

    DateTime time(2020, 1, 1, 12, 13, 0);  // activate immediately
    for (byte snooze_remaining = 2; snooze_remaining != 255; snooze_remaining--)
    {
        // cannot do this because activated is not set when waking from snooze
        //activated = false;
        alarm.loop(time);

        TEST_ASSERT_TRUE(activated);
        TEST_ASSERT_TRUE(lamp_status);
        TEST_ASSERT_TRUE(buzzer.get_status());

        alarm.ButtonSnooze();
        if (snooze_remaining != 0)
        {
            TEST_ASSERT_FALSE(lamp_status);
        }
        else
        {
            TEST_ASSERT_TRUE(lamp_status);
        }
    }
}


void test_Alarm_ambient()
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
        // needs to be here to reset prev_activation_millis
        TestAlarm alarm(lamp, ambientDimmer, buzzer,
                        writeEEPROM, activation_callback, stop_callback);
        TEST_ASSERT_TRUE(alarm.set_time(12, 13));
        TEST_ASSERT_TRUE(alarm.set_enabled(Repeat));
        DaysOfWeek dow(0xFE);
        TEST_ASSERT_TRUE(alarm.set_days_of_week(dow));
        TEST_ASSERT_TRUE(alarm.set_snooze(1, 2));
        TEST_ASSERT_TRUE(alarm.set_signalization(80, true, true));
        TEST_ASSERT_TRUE(alarm.set_inhibit(false));

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

                    TEST_ASSERT_FALSE(activated);
                    TEST_ASSERT_FALSE(lamp_status);
                    TEST_ASSERT_FALSE(alarm.test_get_ambient_status());
                    TEST_ASSERT_EQUAL_INT(0, ambientDimmer.get_stop());

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
                TEST_ASSERT_TRUE(activation_time);
                TEST_ASSERT_TRUE(ambient_activation_time);
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
                        TEST_ASSERT_EQUAL_INT(80, ambientDimmer.get_stop());
                        TEST_ASSERT_TRUE(alarm.test_get_ambient_status());
                        ambient_activation_time = true;
                    }
                    else
                    {
                        TEST_ASSERT_EQUAL_INT(0, ambientDimmer.get_stop());
                        TEST_ASSERT_FALSE(alarm.test_get_ambient_status());
                    }

                    if (time.hour() == 12 && time.minute() == 13 && time.second() < 30)
                    {
                        TEST_ASSERT_TRUE(activated);
                        TEST_ASSERT_TRUE(lamp_status);
                        TEST_ASSERT_TRUE(buzzer.get_status());

                        activation_time = true;
                    }
                    else
                    {
                        TEST_ASSERT_FALSE(activated);
                        TEST_ASSERT_FALSE(lamp_status);
                    }

                    reset_alarm_mockups();
                    alarm.ButtonStop();
                    time = time + TimeSpan(30);
                }
                // In case the test code was wrong
                TEST_ASSERT_TRUE(activation_time);
                TEST_ASSERT_TRUE(ambient_activation_time);
                }
                break;

            case skip:
                {
                bool activation_time = false;
                bool ambient_activation_time = false;
                TEST_ASSERT_TRUE(alarm.set_enabled(Skip));
                while (time < DateTime(2020, 1, 1, 13, 15, 0))
                {
                    alarm.loop(time);

                    TEST_ASSERT_FALSE(activated);
                    TEST_ASSERT_FALSE(lamp_status);
                    TEST_ASSERT_FALSE(alarm.test_get_ambient_status());
                    TEST_ASSERT_EQUAL_INT(0, ambientDimmer.get_stop());

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
                TEST_ASSERT_TRUE(activation_time);
                TEST_ASSERT_TRUE(ambient_activation_time);
                }
                break;

        }
        myTest = (ambientTest)(myTest + 1);
    }
}


void test_Alarm_EEPROM_read()
{
    MockupPWMDimmer ambientDimmer;
    TestAlarm alarm(lamp, ambientDimmer, buzzer,
                    writeEEPROM, activation_callback, stop_callback);

    // Bad id
    byte data1[Alarm::EEPROM_length] = {
        EEPROM_alarms_id + 1, 23, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    TEST_ASSERT_FALSE(alarm.ReadEEPROM(data1));


    // Bad hours
    byte data2[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 24, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    TEST_ASSERT_FALSE(alarm.ReadEEPROM(data2));

    // Bad minutes
    byte data3[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 60, 3, 0x08, 99, 9, 80, 1, 0
    };
    TEST_ASSERT_FALSE(alarm.ReadEEPROM(data3));

    // Bad enabled
    byte data4[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 4, 0x08, 99, 9, 80, 1, 0
    };
    TEST_ASSERT_FALSE(alarm.ReadEEPROM(data4));

    // Bad snooze time
    byte data5[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 100, 9, 80, 1, 0
    };
    TEST_ASSERT_FALSE(alarm.ReadEEPROM(data5));

    // Bad snooze count
    byte data6[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 99, 10, 80, 1, 0
    };
    TEST_ASSERT_FALSE(alarm.ReadEEPROM(data6));

    // Valid alarm
    byte data7[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 3, 0x08, 99, 9, 80, 1, 0
    };
    TEST_ASSERT_TRUE(alarm.ReadEEPROM(data7));
    TEST_ASSERT_EQUAL_INT(23, alarm.get_time().hours);
    TEST_ASSERT_EQUAL_INT(59, alarm.get_time().minutes);
    TEST_ASSERT_EQUAL_INT(Skip, alarm.get_enabled());
    TEST_ASSERT_EQUAL_INT(99, alarm.get_snooze().time_minutes);
    TEST_ASSERT_EQUAL_INT(9, alarm.get_snooze().count);
    TEST_ASSERT_EQUAL_INT(80, alarm.get_signalization().ambient);
    TEST_ASSERT_TRUE(alarm.get_signalization().lamp);
    TEST_ASSERT_FALSE(alarm.get_signalization().buzzer);
}


void test_Alarm_EEPROM_write()
{
    MockupPWMDimmer ambientDimmer;
    TestAlarm alarm(lamp, ambientDimmer, buzzer,
                    writeEEPROM, activation_callback, stop_callback);

    TEST_ASSERT_TRUE(alarm.set_time(23, 59));
    TEST_ASSERT_TRUE(alarm.set_enabled(Repeat));
    DaysOfWeek dow(0x08);
    TEST_ASSERT_TRUE(alarm.set_days_of_week(dow));
    TEST_ASSERT_TRUE(alarm.set_snooze(99, 9));
    TEST_ASSERT_TRUE(alarm.set_signalization(80, true, false));
    TEST_ASSERT_TRUE(alarm.set_inhibit(false));

    byte * data = alarm.WriteEPROM();

    byte correct_data[Alarm::EEPROM_length] = {
        EEPROM_alarms_id, 23, 59, 2, 0x08, 99, 9, 80, 1, 0
    };

    for (byte i = 0; i < Alarm::EEPROM_length; i++)
    {
        TEST_ASSERT_EQUAL_INT(correct_data[i], data[i]);
    }
}


void test_Timer()
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
            TEST_ASSERT_TRUE(lamp_status);
            TEST_ASSERT_TRUE(buzzer.get_status());
            TEST_ASSERT_EQUAL_INT(120, ambientDimmer.get_stop());

            timer.ButtonStop();

            TEST_ASSERT_EQUAL_INT(0, ambientDimmer.get_stop());
            TEST_ASSERT_FALSE(lamp_status);
            TEST_ASSERT_FALSE(buzzer.get_status());

            activation_time = true;
        }
        else
        {
            if (!activation_time)
            {
                TEST_ASSERT_TRUE(timer.get_running());
            }
            else
            {
                TEST_ASSERT_FALSE(timer.get_running());
            }
            TEST_ASSERT_FALSE(lamp_status);
            TEST_ASSERT_FALSE(buzzer.get_status());
            TEST_ASSERT_EQUAL_INT(0, ambientDimmer.get_stop());
        }

        reset_alarm_mockups();
        time = time + TimeSpan(1);
    }
    // In case the test code was wrong
    TEST_ASSERT_TRUE(activation_time);

}


void setup()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(test_Alarm_trigger);
    RUN_TEST(test_Alarm_snooze);
    RUN_TEST(test_Alarm_ambient);
    RUN_TEST(test_Alarm_EEPROM_read);
    RUN_TEST(test_Alarm_EEPROM_write);
    RUN_TEST(test_Timer);
    UNITY_END();
}


void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}

