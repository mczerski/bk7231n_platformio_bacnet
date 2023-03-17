/************************************************************************
 *
 * Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *************************************************************************/

#include "Arduino.h"
#include "bacnet/basic/sys/mstimer.h"
#include "bacnet/basic/object/ai.h"
#include "led.h"
#include "bacnet.h"
#include "BL0942.h"

struct mstimer Blink_Timer;
struct mstimer BL0942_Timer;
BL0942 bl0942(Serial1);

void setup()
{
    mstimer_init();
    led_init();
    bacnet_init();
    mstimer_set(&Blink_Timer, 125);
    mstimer_set(&BL0942_Timer, 1000);
    bl0942.begin();
    Analog_Input_COV_Increment_Set(0, 0.01);
    Analog_Input_COV_Increment_Set(1, 1);
    Analog_Input_COV_Increment_Set(2, 0.1);
}

void loop()
{
    for (;;) {
        if (mstimer_expired(&Blink_Timer)) {
            mstimer_reset(&Blink_Timer);
            led_ld3_toggle();
        }
        if (mstimer_expired(&BL0942_Timer)) {
            mstimer_reset(&BL0942_Timer);
            bl0942.startMeasurement();
        }
        led_task();
        bacnet_task();
        bl0942.update();
        Analog_Input_Present_Value_Set(0, bl0942.getIRms());
        Analog_Input_Present_Value_Set(1, bl0942.getVRms());
        Analog_Input_Present_Value_Set(2, bl0942.getPRms());
    }
}
