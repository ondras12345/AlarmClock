/*!
    @file
*/

#include "HALbool.h"


/*!
    @param  set_hw_ a pointer to the function that operates the hardware
                    resource.
*/
HALbool::HALbool(void(*set_hw_)(bool))
{
    set_hw = set_hw_;
}


/*!
    @brief  Manually set the resource. Normal control cannot set it to
            false if manually set to true.

    @param s what should the resource be set to
*/
void HALbool::set_manu(bool s)
{
    // I cannot just add 1 to true_count because repeated set_manu(true) could
    // cause an overflow.
    if (s) true_count = true_count_manu;
    else true_count = 0;

    set(s);
}


/*!
    @brief  Normal control of the resource.
    @param s what should the resource be set to
*/
void HALbool::set(bool s)
{
    if (true_count != true_count_manu)
    {
        if (s) true_count++;
        else if (true_count != 0) true_count--;
    }

    status = !(true_count == 0);
    set_hw(status);
}
