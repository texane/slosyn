/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Thu Nov 19 20:20:29 2009 texane
** Last update Tue Dec 13 10:32:56 2011 fabien le mentec
*/



#include <pic18fregs.h>
#include <delay.h>
#include "ep1.h"
#include "ep2.h"
#include "usb.h"
#include "stdint.h"
#include "slosyn.h"
#include "../../common/slosyn_types.h"



#pragma udata usb_buf ep1_OutBuffer
volatile uchar ep1_OutBuffer[EP1_BUFFER_SIZE];

void ep1_init(void)
{
    EP_OUT_BD(1).Cnt = EP1_BUFFER_SIZE;
    EP_OUT_BD(1).ADR = (uchar __data *)&ep1_OutBuffer;
    EP_OUT_BD(1).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
    UEP1 = EPHSHK_EN | EPOUTEN_EN | EPCONDIS_EN;       // Init EP1 as an OUT EP
}

void ep1_out(void)
{
  if (EP_OUT_BD(1).Cnt >= 1)
    {
      switch(((slosyn_request_t*)ep1_OutBuffer)->req)
        {
	case SLOSYN_REQ_READ:
	case SLOSYN_REQ_REWIND:
	case SLOSYN_REQ_ECHO:
	case SLOSYN_REQ_STATE:
	  {
	    slosyn_start_request((slosyn_request_t*)ep1_OutBuffer);
	    break;
	  }

	default:
	  {
	    EP_OUT_BD(1).Cnt = EP1_BUFFER_SIZE;
	    EP_OUT_BD(1).ADR = (uchar __data *)&ep1_OutBuffer;
	    EP_OUT_BD(1).Stat.uc = BDS_USIE | BDS_BSTALL;
	    break;
	  }
        }

      EP_OUT_BD(1).Cnt = EP1_BUFFER_SIZE;

      if(EP_OUT_BD(1).Stat.DTS == 0)
        {
	  EP_OUT_BD(1).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
        }
      else
        {
	  EP_OUT_BD(1).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
        }
    }
    else // Raise an error
    {
        EP_OUT_BD(1).Cnt = EP1_BUFFER_SIZE;
        EP_OUT_BD(1).ADR = (uchar __data *)&ep1_OutBuffer;
        EP_OUT_BD(1).Stat.uc = BDS_USIE | BDS_BSTALL;
    }
}
