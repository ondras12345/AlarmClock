#include <unity.h>
#include <ArduinoFake.h>

#include <PWMDimmer.h>

using namespace fakeit;


void test_initial_state()
{
    When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
    When(Method(ArduinoFake(), analogWrite)).AlwaysReturn();

    PWMDimmer dimmer = PWMDimmer(1);

    TEST_ASSERT_FALSE(dimmer.get_active());
    TEST_ASSERT_EQUAL_INT(0, dimmer.get_value());
    TEST_ASSERT_EQUAL_UINT32(0, dimmer.get_remaining());

    VerifyNoOtherInvocations(Method(ArduinoFake(), digitalWrite));
    VerifyNoOtherInvocations(Method(ArduinoFake(), analogWrite));
}


void test_set_from_duration()
{
    When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
    When(Method(ArduinoFake(), analogWrite)).AlwaysReturn();

    PWMDimmer dimmer = PWMDimmer(1);

    dimmer.set_from_duration(0, 100, 1000);
    TEST_ASSERT_FALSE(dimmer.get_active());
    TEST_ASSERT_EQUAL_INT(0, dimmer.get_value());
    TEST_ASSERT_EQUAL_INT(100, dimmer.get_stop());
    TEST_ASSERT_EQUAL_UINT32(0, dimmer.get_remaining());

    dimmer.start();
    TEST_ASSERT_TRUE(dimmer.get_active());
    TEST_ASSERT_EQUAL_INT(0, dimmer.get_value());
    TEST_ASSERT_EQUAL_INT(100, dimmer.get_stop());
    TEST_ASSERT_EQUAL_UINT32(1000, dimmer.get_remaining());

    VerifyNoOtherInvocations(Method(ArduinoFake(), digitalWrite));
    VerifyNoOtherInvocations(Method(ArduinoFake(), analogWrite));
}


void test_full_run()
{
    When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
    When(Method(ArduinoFake(), analogWrite)).AlwaysReturn();
    When(Method(ArduinoFake(), millis)).AlwaysDo([]()->unsigned long {static unsigned long a = 1234; a++; return a; });

    constexpr uint8_t pin = 1;
    PWMDimmer dimmer = PWMDimmer(pin);
    constexpr uint8_t start = 0;
    constexpr uint8_t stop = 100;
    constexpr unsigned long interval = 10;
    dimmer.set(start, stop, 1, interval);
    dimmer.start();
    TEST_ASSERT_TRUE(dimmer.get_active());
    for (unsigned long i = 0; dimmer.get_active() && i < 1.2*(stop-start)*interval; i++)
    {
        dimmer.loop();
        if (i == 0)
        {
            TEST_ASSERT_EQUAL_INT(1, dimmer.get_value());
            Verify(Method(ArduinoFake(), analogWrite).Using(pin, 1)).Once();
        }
    }
    Verify(Method(ArduinoFake(), analogWrite)).AtLeast(100_Times);
    TEST_ASSERT_FALSE(dimmer.get_active());
    TEST_ASSERT_EQUAL_INT(stop, dimmer.get_value());
    VerifyNoOtherInvocations(Method(ArduinoFake(), digitalWrite));

    dimmer.stop();
    TEST_ASSERT_FALSE(dimmer.get_active());
    Verify(Method(ArduinoFake(), digitalWrite).Using(pin, LOW)).Once();
}


int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_initial_state);
    RUN_TEST(test_set_from_duration);
    RUN_TEST(test_full_run);
    UNITY_END();

    return 0;
}
