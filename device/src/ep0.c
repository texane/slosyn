/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Thu Nov 19 20:07:01 2009 texane
** Last update Sat Nov 21 08:08:50 2009 texane
*/


#include "ep0.h"
#include "usb_descriptors.h"
#include "usb_std_req.h"
#include "usb.h"
#include <pic18fregs.h>

/* Control Transfer States */
#define WAIT_SETUP          0
#define WAIT_IN             1
#define WAIT_OUT            2

#pragma udata usb_buf SetupBuffer
volatile far StandardRequest SetupBuffer;

#pragma udata usb_buf InBuffer
volatile far uchar InBuffer[EP0_BUFFER_SIZE];

static uchar ep0_state;
static uint  num_bytes_to_be_send;
static uchar *sourceData;
static uchar coming_cfg;
static uint  ret_status;

uchar ep0_usb_std_request(void)
{
    // hack to avoid register allocation bug in sdcc
    static uchar unknown_request;

    unknown_request = FALSE;

    if(SetupBuffer.request_type != STANDARD)
    {
        return FALSE;
    }

    switch(SetupBuffer.bRequest)
    {
        case CLEAR_FEATURE:
            // TODO not implemented
            break;
        case GET_CONFIGURATION:
            sourceData = &GET_ACTIVE_CONFIGURATION();
            num_bytes_to_be_send = 1;
            break;
        case GET_DESCRIPTOR:
            switch(SetupBuffer.bDescType)
            {
                case DEVICE_DESCRIPTOR:
                    sourceData = (uchar *) device_descriptor;
                    num_bytes_to_be_send = device_descriptor->bLength;
                    break;
                case CONFIGURATION_DESCRIPTOR:
                    sourceData = configuration_descriptor[SetupBuffer.bDescIndex];
                    num_bytes_to_be_send = ((USB_Configuration_Descriptor*)sourceData)->wTotalLength;
                    break;
                case STRING_DESCRIPTOR:
                    sourceData = string_descriptor[SetupBuffer.bDescIndex];
                    num_bytes_to_be_send = sourceData[0];
                    break;
                default:
                    // This is required to stall the DEVICE_QUALIFIER request
                    unknown_request = TRUE;
                    break;
            }
            break;
        case GET_INTERFACE:
            // TODO not implemented
            break;
        case GET_STATUS:
            // TODO To be completed
            ret_status = 0;
            switch(SetupBuffer.recipient)
            {
                case RECIPIENT_DEVICE:
                case RECIPIENT_INTERFACE:
                case RECIPIENT_ENDPOINT:
                    sourceData = (uchar *) &ret_status;
                    num_bytes_to_be_send = sizeof(ret_status);
                    break;
                default:
                    break;
            }
            break;
        case SET_ADDRESS:
            SET_DEVICE_STATE(ADDRESS_PENDING_STATE);
            break;
        case SET_CONFIGURATION:
            // is this configuration valid ?
            if(device_descriptor->bNumConfigurations >= SetupBuffer.bConfigurationValue)
            {
                coming_cfg = SetupBuffer.bConfigurationValue;
                SET_DEVICE_STATE(CONFIGURATION_PENDING_STATE);
            }
            else // invalid configuration
            {
                unknown_request = TRUE;
            }
            break;
        case SET_FEATURE:
            // TODO not implemented
            break;
        case SET_INTERFACE:
            // TODO not implemented
            break;
//        case SYNCH_FRAME:
// only for isochronous synchronization
//            break;
        default:
        	unknown_request = TRUE;
            break;
    }
    return !unknown_request;
}


void ep0_init(void)
{
    ep0_state = WAIT_SETUP;
    EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
    EP_OUT_BD(0).ADR = (uchar __data *)&SetupBuffer;
    EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
    EP_IN_BD(0).Stat.uc = BDS_UCPU;
    UEP0 = EPINEN_EN | EPOUTEN_EN | EPHSHK_EN;
}

void ep0_in(void)
{
    if(GET_DEVICE_STATE() == ADDRESS_PENDING_STATE)
    {
        UADDR = SetupBuffer.bAddress;
        if(UADDR != 0)
        {
            SET_DEVICE_STATE(ADDRESS_STATE);
        }
        else
        {
            SET_DEVICE_STATE(DEFAULT_STATE);
        }
    }

    if(ep0_state == WAIT_IN)
    {
        fill_in_buffer(0, &sourceData, EP0_BUFFER_SIZE, &num_bytes_to_be_send);

        if(EP_IN_BD(0).Stat.DTS == 0)
        {
            EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
        }
        else
        {
            EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
        }
    }
    else
    {
        ep0_init();
    }

   if(GET_DEVICE_STATE() == CONFIGURATION_PENDING_STATE)
    {
        // First, disable all endpoints.
        // UEP0 is never disabled
        UEP1  = 0; UEP2  = 0; UEP3  = 0; UEP4  = 0;
        UEP5  = 0; UEP6  = 0; UEP7  = 0; UEP8  = 0;
        UEP9  = 0; UEP10 = 0; UEP11 = 0; UEP12 = 0;
        UEP13 = 0; UEP14 = 0; UEP15 = 0;

        // switch the functions vectors
	ep_init = boot_ep_init;
	ep_in = boot_ep_in;
	ep_out = boot_ep_out;
	ep_setup = boot_ep_setup;

        SET_ACTIVE_CONFIGURATION(coming_cfg);

        if(coming_cfg == 0)
        {
            SET_DEVICE_STATE(ADDRESS_STATE);
        }
        else
        {
            static uchar i;

            // Switch to decrement loop because of a sdcc bug
            for(i = USB_MAX_ENDPOINTS - 1; i > 0; i--)
            {
                ep_init[coming_cfg][i]();
            }

            SET_DEVICE_STATE(CONFIGURED_STATE);
        }
    }
}

void ep0_setup(void)
{
    ep0_state = WAIT_SETUP;
    num_bytes_to_be_send = 0;

    if(ep0_usb_std_request())
    {
        UCONbits.PKTDIS = 0;
        if(SetupBuffer.data_transfer_direction == DEVICE_TO_HOST)
        {
            ep0_state = WAIT_IN;

            EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
            EP_OUT_BD(0).ADR = (uchar __data *)&SetupBuffer;
            EP_OUT_BD(0).Stat.uc = BDS_USIE;

            EP_IN_BD(0).ADR = (uchar __data *)InBuffer;
            if(SetupBuffer.wLength < num_bytes_to_be_send)
                {
                    num_bytes_to_be_send = SetupBuffer.wLength;
                }
            fill_in_buffer(0, &sourceData, EP0_BUFFER_SIZE, &num_bytes_to_be_send);
            EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

        }
        else // HOST_TO_DEVICE
        {
            ep0_state = WAIT_OUT;

            EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
            EP_OUT_BD(0).ADR = (uchar __data *)InBuffer;
            EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

            EP_IN_BD(0).Cnt = 0;
            EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
        }
    }
    else
    {
        UCONbits.PKTDIS = 0;
        EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
        EP_OUT_BD(0).ADR = (uchar __data *)&SetupBuffer;
        EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_BSTALL;

        EP_IN_BD(0).Stat.uc  = BDS_USIE | BDS_BSTALL;
    }
}
