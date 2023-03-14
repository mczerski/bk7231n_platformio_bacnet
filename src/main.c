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

#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "bacnet/basic/sys/mstimer.h"
#include "bacnet/basic/sys/mstimer.h"
#include "rs485.h"
#include "led.h"
#include "bacnet.h"
#include "FreeRTOS.h"
#include "task.h"

/* local version override */
char *BACnet_Version = "1.0";

void BACnetTask(void *pvParameters)
{
    struct mstimer Blink_Timer;
    mstimer_init();
    led_init();
    bacnet_init();
    mstimer_set(&Blink_Timer, 125);
    for (;;) {
        if (mstimer_expired(&Blink_Timer)) {
            mstimer_reset(&Blink_Timer);
            led_ld3_toggle();
        }
        led_task();
        bacnet_task();
    }
}

/* Entry point */
int main()
{
    // Cannot run BACnet code here, the default stack size is to small : 4096
    // byte
    xTaskCreate(BACnetTask, /* Function to implement the task */
        "BACnetTask", /* Name of the task */
        10000, /* Stack size in words */
        NULL, /* Task input parameter */
        20, /* Priority of the task */
        NULL); /* Task handle. */
}

