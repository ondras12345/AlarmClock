#include <unity.h>
#include <stdint.h>

#include <DaysOfWeek.h>


struct DowTestCase
{
    uint8_t code;
    bool days[7];
};

DowTestCase Cases[] = {
    {0x00, {false,  false,  false,  false,  false,  false,  false}},
    {0x01, {false,  false,  false,  false,  false,  false,  false}},
    {0x02, {true,   false,  false,  false,  false,  false,  false}},
    {0x04, {false,  true,   false,  false,  false,  false,  false}},
    {0x08, {false,  false,  true,   false,  false,  false,  false}},
    {0x10, {false,  false,  false,  true,   false,  false,  false}},
    {0x20, {false,  false,  false,  false,  true,   false,  false}},
    {0x40, {false,  false,  false,  false,  false,  true,   false}},
    {0x80, {false,  false,  false,  false,  false,  false,  true}},
    {0x20, {false,  false,  false,  false,  true,   false,  false}},
    {0xC0, {false,  false,  false,  false,  false,  true,   true}},
    {0x1E, {true,   true,   true,   true,   false,  false,  false}},
    {0xFE, {true,   true,   true,   true,   true,   true,   true}}
};


void test_decode()
{
    for (size_t i = 0; i < sizeof(Cases)/sizeof(Cases[0]); i++)
    {
        DaysOfWeek dow;
        dow.days_of_week = Cases[i].code;
        for (size_t j = 0; j < sizeof(Cases[i].days)/sizeof(Cases[i].days[0]); j++)
        {
            if (Cases[i].days[j])
            {
                TEST_ASSERT_TRUE(dow.getDayOfWeek(j+1));
            }
            else
            {
                TEST_ASSERT_FALSE(dow.getDayOfWeek(j+1));
            }
        }
    }
}


void test_encode()
{
    for (size_t i = 0; i < sizeof(Cases)/sizeof(Cases[0]); i++)
    {
        uint8_t code = Cases[i].code;
        if (code == 0x01) code = 0x00;
        DaysOfWeek dow;
        for (size_t j = 0; j < sizeof(Cases[i].days)/sizeof(Cases[i].days[0]); j++)
        {
            dow.setDayOfWeek(j+1, Cases[i].days[j]);
        }
        TEST_ASSERT_EQUAL_INT(code, dow.days_of_week);
    }
}


int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_decode);
    RUN_TEST(test_encode);
    UNITY_END();

    return 0;
}
