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
 *
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
/* hardware layer includes */
#include "hardware.h"
#include "bacnet/datalink/dlmstp.h"
#include "bacnet/basic/sys/mstimer.h"
#include "dlmstp.h"
#include "rs485.h"
#include "led.h"
/* BACnet Stack includes */
#include "bacnet/datalink/datalink.h"
#include "bacnet/npdu.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/dcc.h"
#include "bacnet/iam.h"
/* BACnet objects */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"
/* me */
#include "bacnet.h"

/* timer for device communications control */
static struct mstimer DCC_Timer;
static struct mstimer IN_Timer;
#define DCC_CYCLE_SECONDS 1

static object_functions_t My_Object_Table[] = {
    { OBJECT_DEVICE, NULL /* Init - don't init Device or it will recourse! */,
        Device_Count, Device_Index_To_Instance,
        Device_Valid_Object_Instance_Number, Device_Object_Name,
        Device_Read_Property_Local, Device_Write_Property_Local,
        Device_Property_Lists, DeviceGetRRInfo, NULL /* Iterator */,
        NULL /* Value_Lists */, NULL /* COV */, NULL /* COV Clear */,
        NULL /* Intrinsic Reporting */ },
    { OBJECT_BINARY_INPUT, Binary_Input_Init, Binary_Input_Count,
        Binary_Input_Index_To_Instance, Binary_Input_Valid_Instance,
        Binary_Input_Object_Name, Binary_Input_Read_Property,
        Binary_Input_Write_Property, Binary_Input_Property_Lists,
        NULL /* ReadRangeInfo */, NULL /* Iterator */,
        Binary_Input_Encode_Value_List, Binary_Input_Change_Of_Value,
        Binary_Input_Change_Of_Value_Clear, NULL /* Intrinsic Reporting */ },
    { OBJECT_BINARY_OUTPUT, Binary_Output_Init, Binary_Output_Count,
        Binary_Output_Index_To_Instance, Binary_Output_Valid_Instance,
        Binary_Output_Object_Name, Binary_Output_Read_Property,
        Binary_Output_Write_Property, Binary_Output_Property_Lists,
        NULL /* ReadRangeInfo */, NULL /* Iterator */,
        Binary_Output_Encode_Value_List, Binary_Output_Change_Of_Value,
        Binary_Output_Change_Of_Value_Clear, NULL /* Intrinsic Reporting */ },
    { MAX_BACNET_OBJECT_TYPE, NULL /* Init */, NULL /* Count */,
        NULL /* Index_To_Instance */, NULL /* Valid_Instance */,
        NULL /* Object_Name */, NULL /* Read_Property */,
        NULL /* Write_Property */, NULL /* Property_Lists */,
        NULL /* ReadRangeInfo */, NULL /* Iterator */, NULL /* Value_Lists */,
        NULL /* COV */, NULL /* COV Clear */,
        NULL /* Intrinsic Reporting */ }
};

void binary_output_callback(
    uint32_t object_instance, BACNET_BINARY_PV old_value,
    BACNET_BINARY_PV value)
{
    if (value == BINARY_ACTIVE) {
        if (object_instance == 0) {
            led_ld4_on();
        }
    } else {
        if (object_instance == 0) {
            led_ld4_off();
        }
    }
}

void bacnet_init(void)
{
    dlmstp_set_mac_address(1);
    dlmstp_set_max_master(1);
    /* initialize datalink layer */
    dlmstp_init(NULL);
    handler_cov_init();
    /* initialize objects */
    Device_Init(My_Object_Table);
    for (int i = 0; i < MAX_BINARY_OUTPUTS; i++) {
        Binary_Output_Create(i);
    }
    Binary_Output_Write_Present_Value_Callback_Set(binary_output_callback);

    /* set up our confirmed service unrecognized service handler - required! */
    apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_PROP_MULTIPLE, handler_read_property_multiple);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_REINITIALIZE_DEVICE, handler_reinitialize_device);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_WRITE_PROPERTY, handler_write_property);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_SUBSCRIBE_COV, handler_cov_subscribe);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
    /* start the cyclic 1 second timer for DCC */
    mstimer_set(&DCC_Timer, DCC_CYCLE_SECONDS * 1000);
    mstimer_set(&IN_Timer, 50);
    /* Hello World! */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
}

/** Static receive buffer, initialized with zeros by the C Library Startup Code. */

static uint8_t PDUBuffer[MAX_MPDU + 16 /* Add a little safety margin to the buffer,
                                        * so that in the rare case, the message
                                        * would be filled up to MAX_MPDU and some
                                        * decoding functions would overrun, these
                                        * decoding functions will just end up in
                                        * a safe field of static zeros. */];

/** BACnet task handling receiving and transmitting of messages.  */
int prev = 0;
void bacnet_task(void)
{
    uint16_t pdu_len;
    BACNET_ADDRESS src; /* source address */
    uint8_t i;
    bool out_of_service;

    /* Binary Input */
    for (i = 0; i < MAX_BINARY_INPUTS; i++) {
        out_of_service = Binary_Input_Out_Of_Service(i);
        if (!out_of_service) {
            if (i == 0) {
                Binary_Input_Present_Value_Set(i, led_in1_state());
            }
            else {
                /* led_in2_state() */
            }
        }
    }
    /* Binary Output */
    /* handle the communication timer */
    if (mstimer_expired(&DCC_Timer)) {
        mstimer_reset(&DCC_Timer);
        dcc_timer_seconds(DCC_CYCLE_SECONDS);
        handler_cov_timer_seconds(DCC_CYCLE_SECONDS);
        tsm_timer_milliseconds(DCC_CYCLE_SECONDS * 1000);
    }
    if (mstimer_expired(&IN_Timer)) {
        mstimer_reset(&IN_Timer);
        int curr = led_in1_state();
        if (prev == 0 && curr == 1) {
            int old_value = Binary_Output_Present_Value(0);
            Binary_Output_Present_Value_Set(0, !old_value, 16);
            binary_output_callback(0, old_value, !old_value); 
        }
        prev = curr;
    }
    /* handle the messaging */
    pdu_len = datalink_receive(&src, &PDUBuffer[0], MAX_MPDU, 0);
    if (pdu_len) {
        npdu_handler(&src, &PDUBuffer[0], pdu_len);
    }

    handler_cov_task();
}

void datetime_init(void)
{
}

bool datetime_local(
    BACNET_DATE * bdate,
    BACNET_TIME * btime,
    int16_t * utc_offset_minutes,
    bool * dst_active)
{
    return true;
}
