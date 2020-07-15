/*!
    @file
*/

#ifndef _HALBOOL_h
#define _HALBOOL_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

/*!
    @brief  A boolean hardware resource that needs to be true unless all
            alarms want it to be false. E.g. lamp, permanent LCD backlight.
            This is a part of the HAL for alarms.
*/
class HALbool
{
public:
    HALbool(void(*set_hw_)(bool));
    void set(bool s);
    void set_manu(bool s);
    bool get() const { return status; };


protected:
    void(*set_hw)(bool);

    bool status = false;

    /*!
        @brief  Number of alarms that want the resource to be true.

        This should prevent collisions like one alarm disabling another's lamp
        (e.g. timeout).

        Manual control (e.g. GUI, CLI) sets this number to  0 (false) or
        255 (true) when it wants to set the resource.
    */
    byte true_count = 0;
    //! true_count == 255 --> manually true (GUI, CLI)
#define true_count_manu 255

};


#endif
