/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 14:00:09 2009 texane
** Last update Tue Dec 13 10:47:11 2011 fabien le mentec
*/



#include <pic18fregs.h>
#include "ep2.h"
#include "stdint.h"
#include "../../../common/slosyn_types.h"


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


#if 0 /* TODO_M600_PORT */

/* initiate a read cycle and read card contents */

static m600_alarms_t m600_read_card(uint16_t* col_data)
{
  unsigned int col_count = M600_COLUMN_COUNT;
  unsigned int countdown;

#if 0 /* simple_test */

  /* wait for the ready conditions(p.38)
     ready is 4v if there is no card
     ready is 0v if there are cards
   */
  while (M600_PIN_NOT_READY)
    ;

  /* wait for previous cycle to end
     0v indicates busy is true
   */
  while (!M600_PIN_BUSY)
    ;

#endif

  /* initiate a read cycle. according
     to the documentation (p.38), it
     is better to wait for the busy
     condition than idling for 1usecs
     but practically it doesnot seem
     to work, so we wait.
   */

  M600_PIN_PICK_CMD = 0;

#if 0 /* wait for the cycle to start */
  while (!M600_PIN_BUSY)
  {
    /* an error occured. 6 attempts are made
       every 50 ms, for a total of 300ms max
    */

    if (!M600_PIN_ERROR)
    {
      M600_PIN_PICK_CMD = 1;
      return m600_read_alarms();
    }
  }
#else /* wait a bit for the cycle to start */
  for (countdown = 0x100; countdown > 0; --countdown)
    ;
#endif

  M600_PIN_PICK_CMD = 1;

  /* read the data */

  /* very approximative constant */
#define COUNTDOWN_10MS 15000
  countdown = COUNTDOWN_10MS;

  while (col_count)
  {
    /* wait for the INDEX_MARK signal to
       become true. it indicates data is
       available (ie. storage completed).
       the mark are generated periodically
       every 864 usecs on the M600.
    */

#if 0
    if ((--countdown) == 0)
      return (M600_ALARM_ERROR | m600_read_alarms());
#else
#if 0
    if (M600_IS_ANY_ERROR())
      return (M600_ALARM_ERROR | m600_read_alarms());
#endif
#endif

    if (!M600_PIN_INDEX_MARK)
    {
      /* data available. the index mark
	 signal held true for 6 usecs
      */

#if 0
      /* reload the countdown */
      countdown = COUNTDOWN_10MS;
#endif

      while (!M600_PIN_INDEX_MARK)
      {
#if 0
	if (!(--countdown))
	  return (M600_ALARM_ERROR | m600_read_alarms());
#else
#if 0
	if (M600_IS_ANY_ERROR())
	  return (M600_ALARM_ERROR | m600_read_alarms());
#endif
#endif
      }

      *col_data = read_data_reg();

#if 0
      /* reload the countdown */
      countdown = COUNTDOWN_10MS;
#endif

      /* advance position */
      ++col_data;
      --col_count;
    }
  }

  return M600_ALARM_NONE;
}

#endif /* TODO_M600_PORT */


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
  SLOSYN_TRIS_PULSE_FWD = 0;
  SLOSYN_PIN_PULSE_FWD = 0;
  SLOSYN_TRIS_PULSE_BWD = 0;
  SLOSYN_PIN_PULSE_BWD = 0;

  reset_device();

  /* automaton state */

  slosyn_request.req = SLOSYN_REQ_INVALID;
}


void slosyn_start_request(slosyn_request_t* req)
{
  slosyn_request.req = req->req;
  slosyn_request.nchars = req->nchars;
  slosyn_request.dir = req->dir;
}


void slosyn_schedule(void)
{
  unsigned char do_reply;

#if 0 /* TODO_M600_PORT */

  switch (slosyn_request.req)
    {
    case M600_REQ_READ_CARD:
      {
	m600_reply.alarms = m600_read_card(m600_reply.card_data);

	do_reply = 1;

	break;
      }

    case M600_REQ_READ_ALARMS:
      {
	m600_reply.alarms = m600_read_alarms();

	do_reply = 1;

	break;
      }

    case M600_REQ_FILL_DATA:
      {
	uint16_t i;

	for (i = 0; i < M600_COLUMN_COUNT; ++i)
	  m600_reply.card_data[i] = i;

	do_reply = 1;

	break;
      }

    case M600_REQ_READ_PINS:
      {
	uint8_t* const p = (uint8_t*)m600_reply.card_data;

	p[0] = PORTA;
	p[1] = PORTB;
	p[2] = PORTC;
	p[3] = PORTD;

	do_reply = 1;

	break;
      }

    case M600_REQ_RESET_DEV:
      {
	/* wait for the ready signal is done in userland */

	reset_device();
	do_reply = 1;
	break;
      }

    case M600_REQ_INVALID:
    default:
      {
	do_reply = 0;

	break;
      }
    }

#endif /* TODO_M600_PORT */

  slosyn_request.req = SLOSYN_REQ_INVALID;

  if (!do_reply)
    return ;

  /* reply to the host */

  ep2_num_bytes_to_send = sizeof(slosyn_reply_t);
  ep2_source_data = (void*)&slosyn_reply;
  prepare_ep2_in();
}
