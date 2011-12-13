/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 14:00:09 2009 texane
** Last update Tue Dec 13 08:30:11 2011 fabien le mentec
*/



#include <pic18fregs.h>
#include "ep2.h"
#include "stdint.h"
#include "../../../common/slosyn_types.h"


#if 0 /* TODO_PORT_M600 */

/* m600 globals */

static m600_request_t m600_request = M600_REQ_INVALID;

static m600_reply_t m600_reply;


/* m600 pinouts */

#define M600_PORT_DATA_LOW PORTB
#define M600_TRIS_DATA_LOW TRISB

/* note: some PORTA high bits not
   available and will be read as
   0. since we need to read 12bits
   values, we only care about the
   first nibble.
 */

#define M600_PORT_DATA_HIGH PORTA
#define M600_TRIS_DATA_HIGH TRISA

#define M600_PORT_ALARMS PORTD
#define M600_TRIS_ALARMS TRISD
#define M600_PIN_ERROR PORTDbits.RD0
#define M600_PIN_HOPPER_CHECK PORTDbits.RD1
#define M600_PIN_MOTION_CHECK PORTDbits.RD2

#define M600_IS_ANY_ERROR() ((PORTD & ((1 << 3) - 1)) != (0x7))

#define M600_TRIS_PICK_CMD TRISDbits.TRISD3
#define M600_PIN_PICK_CMD LATDbits.LATD3

#define M600_TRIS_RESET_CMD TRISDbits.TRISD4
#define M600_PIN_RESET_CMD LATDbits.LATD4

#define M600_TRIS_INDEX_MARK TRISCbits.TRISC0
#define M600_PIN_INDEX_MARK PORTCbits.RC0

#define M600_TRIS_NOT_READY TRISCbits.TRISC1
#define M600_PIN_NOT_READY PORTCbits.RC1

#define M600_TRIS_BUSY TRISCbits.TRISC2
#define M600_PIN_BUSY PORTCbits.RC2


/* read alarm signals */

static m600_alarms_t m600_read_alarms(void)
{
  m600_alarms_t alarms = M600_ALARM_NONE;

#define SET_ALARM_IF_ASSERTED(E, C)		\
  do {						\
    if (M600_PIN_ ## C)				\
      M600_SET_ALARM(E, C);			\
  } while (0)

#define SET_ALARM_IF_NOT_ASSERTED(E, C)		\
  do {						\
    if ((M600_PIN_ ## C) == 0)			\
      M600_SET_ALARM(E, C);			\
  } while (0)

  SET_ALARM_IF_NOT_ASSERTED(alarms, ERROR);
  SET_ALARM_IF_NOT_ASSERTED(alarms, HOPPER_CHECK);
  SET_ALARM_IF_NOT_ASSERTED(alarms, MOTION_CHECK);
  SET_ALARM_IF_ASSERTED(alarms, NOT_READY);

  return alarms;
}


/* read the 12bits data register */

static uint16_t read_data_reg(void)
{
  return
    (((unsigned int)M600_PORT_DATA_HIGH << 8) |
     M600_PORT_DATA_LOW) & 0x0fff;
}


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


static void reset_device(void)
{
  /* reset the m600 device */

  unsigned int countdown;

  M600_TRIS_RESET_CMD = 0;
  M600_PIN_RESET_CMD = 0;

  for (countdown = 20000; countdown; --countdown)
    ;

  M600_PIN_RESET_CMD = 1;
}


/* exported */

void m600_setup(void)
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
  M600_TRIS_DATA_LOW = 1;
  M600_TRIS_DATA_HIGH = 1;
  M600_TRIS_ALARMS = 1;
  M600_TRIS_INDEX_MARK = 1;
  M600_TRIS_NOT_READY = 1;
  M600_TRIS_BUSY = 1;

  /* outputs */

  M600_TRIS_RESET_CMD = 0;
  M600_PIN_RESET_CMD = 1;

  /* note: must come after the
     alarm tris is set since this
     overlap with same register */

  M600_TRIS_PICK_CMD = 0;
  M600_PIN_PICK_CMD = 1;

  reset_device();

  /* automaton state */

  m600_request = M600_REQ_INVALID;
}


void m600_start_request(m600_request_t req)
{
  m600_request = req;
}


void m600_schedule(void)
{
  unsigned char do_reply;

  switch (m600_request)
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

  m600_request = M600_REQ_INVALID;

  if (!do_reply)
    return ;

  /* reply to the host */

  ep2_num_bytes_to_send = sizeof(m600_reply_t);
  ep2_source_data = (void*)&m600_reply;
  prepare_ep2_in();
}


#if 0
void m600_print_signals(void)
{
  print_pin(M600_PIN_ERROR);
  print_pin(M600_PIN_HOPPER_CHECK);
  print_pin(M600_PIN_MOTION_CHECK);

  print_pin(M600_PIN_INDEX_MARK);
  print_pin(M600_PIN_NOT_READY);
  print_pin(M600_PIN_BUSY);
}
#endif

#endif /* TODO_PORT_M600 */
