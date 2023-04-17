/**************************************************************************
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
 *********************************************************************/
#include "Arduino.h"
#include "bacnet/basic/sys/mstimer.h"
#include "led.h"

static struct mstimer Off_Delay_Timer_Rx;
static struct mstimer Off_Delay_Timer_Tx;
static bool Rx_State;
static bool Tx_State;
static bool LD3_State;
static constexpr int LED_TX = 12;
static constexpr int LED_RX = 13;
static constexpr int LED_D3 = 17;
static constexpr int LED_D4 = 3;
static constexpr int IN1 = 15;
static constexpr int IN2 = 2;

/*************************************************************************
 * Description: Activate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_tx_on(void)
{
    digitalWrite(LED_TX, LOW);
    mstimer_set(&Off_Delay_Timer_Tx, 0);
    Tx_State = true;
}

/*************************************************************************
 * Description: Activate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_rx_on(void)
{
    digitalWrite(LED_RX, LOW);
    mstimer_set(&Off_Delay_Timer_Rx, 0);
    Rx_State = true;
}

/*************************************************************************
 * Description: Deactivate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_tx_off(void)
{
    digitalWrite(LED_TX, HIGH);
    mstimer_set(&Off_Delay_Timer_Tx, 0);
    Tx_State = false;
}

/*************************************************************************
 * Description: Deactivate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_rx_off(void)
{
    digitalWrite(LED_RX, HIGH);
    mstimer_set(&Off_Delay_Timer_Rx, 0);
    Rx_State = false;
}

/*************************************************************************
 * Description: Get the state of the LED
 * Returns: true if on, false if off.
 * Notes: none
 *************************************************************************/
bool led_rx_state(void)
{
    return Rx_State;
}

/*************************************************************************
 * Description: Get the state of the LED
 * Returns: true if on, false if off.
 * Notes: none
 *************************************************************************/
bool led_tx_state(void)
{
    return Tx_State;
}

/*************************************************************************
 * Description: Toggle the state of the LED
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_tx_toggle(void)
{
    if (led_tx_state()) {
        led_tx_off();
    } else {
        led_tx_on();
    }
}

/*************************************************************************
 * Description: Toggle the state of the LED
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_rx_toggle(void)
{
    if (led_rx_state()) {
        led_rx_off();
    } else {
        led_rx_on();
    }
}

/*************************************************************************
 * Description: Delay before going off to give minimum brightness.
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_rx_off_delay(uint32_t delay_ms)
{
    mstimer_set(&Off_Delay_Timer_Rx, delay_ms);
}

/*************************************************************************
 * Description: Delay before going off to give minimum brightness.
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_tx_off_delay(uint32_t delay_ms)
{
    mstimer_set(&Off_Delay_Timer_Tx, delay_ms);
}

/*************************************************************************
 * Description: Turn on, and delay before going off.
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_rx_on_interval(uint16_t interval_ms)
{
    led_rx_on();
    mstimer_set(&Off_Delay_Timer_Rx, interval_ms);
}

/*************************************************************************
 * Description: Turn on, and delay before going off.
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_tx_on_interval(uint16_t interval_ms)
{
    led_tx_on();
    mstimer_set(&Off_Delay_Timer_Tx, interval_ms);
}

/*************************************************************************
 * Description: Task for blinking LED
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_task(void)
{
    if (mstimer_expired(&Off_Delay_Timer_Rx)) {
        mstimer_set(&Off_Delay_Timer_Rx, 0);
        led_rx_off();
    }
    if (mstimer_expired(&Off_Delay_Timer_Tx)) {
        mstimer_set(&Off_Delay_Timer_Tx, 0);
        led_tx_off();
    }
}

/*************************************************************************
 * Description: Activate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_ld4_on(void)
{
    digitalWrite(LED_D4, LOW);
}

/*************************************************************************
 * Description: Deactivate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_ld4_off(void)
{
    digitalWrite(LED_D4, HIGH);
}

/*************************************************************************
 * Description: Activate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_ld3_on(void)
{
    digitalWrite(LED_D3, LOW);
    LD3_State = true;
}

/*************************************************************************
 * Description: Deactivate the LED
 * Returns: nothing
 * Notes: none
 **************************************************************************/
void led_ld3_off(void)
{
    digitalWrite(LED_D3, HIGH);
    LD3_State = false;
}

/*************************************************************************
 * Description: Get the state of the LED
 * Returns: true if on, false if off.
 * Notes: none
 *************************************************************************/
bool led_ld3_state(void)
{
    return LD3_State;
}

/*************************************************************************
 * Description: Toggle the state of the LED
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_ld3_toggle(void)
{
    if (led_ld3_state()) {
        led_ld3_off();
    } else {
        led_ld3_on();
    }
}

/*************************************************************************
 * Description: Get the state of the IN
 * Returns: true if on, false if off.
 * Notes: none
 *************************************************************************/
bool led_in1_state(void)
{
    return digitalRead(IN1) and digitalRead(IN2);
}

/*************************************************************************
 * Description: Initialize the LED hardware
 * Returns: none
 * Notes: none
 *************************************************************************/
void led_init(void)
{
    pinMode(LED_TX, OUTPUT);
    pinMode(LED_RX, OUTPUT);
    pinMode(LED_D4, OUTPUT);
    pinMode(LED_D3, OUTPUT);
    pinMode(IN1, INPUT);

    led_tx_on();
    led_rx_on();
    led_ld3_on();
    led_ld4_on();
}
