/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 14:00:09 2009 texane
** Last update Tue Dec 13 11:18:02 2011 fabien le mentec
*/



#include <pic18fregs.h>
#include "ep2.h"
#include "stdint.h"
#include "../../common/slosyn_types.h"


/* globals */

static slosyn_request_t slosyn_request;
static slosyn_reply_t slosyn_reply;


/* pinouts */

#define SLOSYN_PORT_DATA PORTB
#define SLOSYN_TRIS_DATA TRISB

#define SLOSYN_PIN_PULSE_FWD LATAbits.LATA0
#define SLOSYN_TRIS_PULSE_FWD TRISAbits.TRISA0

#define SLOSYN_PIN_PULSE_BWD LATAbits.LATA1
#define SLOSYN_TRIS_PULSE_BWD TRISAbits.TRISA1


/* which one... have to look at schems */
#define IS_EOB(__c) (((__c) == 0) || ((__c) == 0xff))


static uint8_t is_eob(void)
{
  /* is end of band */
  uint8_t c = SLOSYN_PORT_DATA;
  return IS_EOB(c);
}


static void wait_pulse(void)
{
  /* wait more than 50 usecs */
  volatile uint16_t i;
  for (i = 0; i < 1000; ++i) ;
}


static uint8_t read_nchars
(uint8_t* buf, uint8_t nchars, uint8_t dir)
{
  uint8_t i;

  for (i = 0; i < nchars; ++i)
  {
    /* pulse for more than 50us */
    if (dir == SLOSYN_DIR_FWD)
    {
      SLOSYN_PIN_PULSE_FWD = 1;
      wait_pulse();
      SLOSYN_PIN_PULSE_FWD = 0;
    }
    else
    {
      SLOSYN_PIN_PULSE_BWD = 1;
      wait_pulse();
      SLOSYN_PIN_PULSE_BWD = 0;
    }

    buf[i] = SLOSYN_PORT_DATA;
    if (IS_EOB(buf[i])) break ;
  }

  return i;
}


static void rewind(uint8_t dir)
{
  /* TODO: counter or timeout to limit looping if
     end of band not detected */

  uint8_t c;

  while (1)
  {
    /* pulse for more than 50us */
    if (dir == SLOSYN_DIR_FWD)
    {
      SLOSYN_PIN_PULSE_FWD = 1;
      wait_pulse();
      SLOSYN_PIN_PULSE_FWD = 0;
    }
    else
    {
      SLOSYN_PIN_PULSE_BWD = 1;
      wait_pulse();
      SLOSYN_PIN_PULSE_BWD = 0;
    }

    c = SLOSYN_PORT_DATA;
    if (IS_EOB(c)) break ;
  }
}


/* exported */

void slosyn_setup(void)
{
  /* have to do configure the ports as digital
     io, otherwise they will be in analog mode
     esp. PORTA on my own pic18f...
   */

  ADCON0 = 0;
  ADCON1 = 0xf;
  ADCON2 = 0;

  SPPCON = 0;

  /* inputs */
  SLOSYN_TRIS_DATA = 0xff;

  /* outputs */
  SLOSYN_PIN_PULSE_FWD = 0;
  SLOSYN_TRIS_PULSE_FWD = 0;
  SLOSYN_PIN_PULSE_BWD = 0;
  SLOSYN_TRIS_PULSE_BWD = 0;

  /* automaton state */
  slosyn_request.req = SLOSYN_REQ_INVALID;
}


void slosyn_start_request(slosyn_request_t* req)
{
  slosyn_request.req = req->req;
  slosyn_request.nchars = req->nchars;
  slosyn_request.dir = req->dir;

  if (req->req == SLOSYN_REQ_ECHO)
  {
    uint8_t i;
    uint8_t nchars = SLOSYN_NCHARS_MAX;
    if (req->nchars < SLOSYN_NCHARS_MAX) nchars = req->nchars;
    for (i = 0; i < nchars; ++i) slosyn_request.chars[i] = req->chars[i];
  }
}


void slosyn_schedule(void)
{
  unsigned char do_reply = 0;

  switch (slosyn_request.req)
  {
  case SLOSYN_REQ_READ:
    {
      slosyn_reply.nchars = read_nchars
	(slosyn_reply.chars, slosyn_request.nchars, slosyn_request.dir);
      do_reply = 1;
      break ;
    }

  case SLOSYN_REQ_REWIND:
    {
      rewind(slosyn_request.dir);
      do_reply = 1;
      break ;
    }

  case SLOSYN_REQ_ECHO:
    {
      unsigned int i;
      unsigned int nchars;

      if (slosyn_request.nchars < SLOSYN_NCHARS_MAX)
	nchars = slosyn_request.nchars;
      else
	nchars = SLOSYN_NCHARS_MAX;

      for (i = 0; i < nchars; ++i)
	slosyn_reply.chars[i] = slosyn_request.chars[nchars - i - 1];
      slosyn_reply.nchars = nchars;

      do_reply = 1;

      break ;
    }

  case SLOSYN_REQ_STATE:
    {
      slosyn_reply.status = 0;
      if (is_eob()) slosyn_reply.status |= SLOSYN_BIT_EOB;
      do_reply = 1;
      break ;
    }

  default:
    {
      /* TODO */
      do_reply = 0;
      break ;
    }
  }

  slosyn_request.req = SLOSYN_REQ_INVALID;

  if (!do_reply) return ;

  /* reply to the host */

  ep2_num_bytes_to_send = sizeof(slosyn_reply_t);
  ep2_source_data = (void*)&slosyn_reply;
  prepare_ep2_in();
}
