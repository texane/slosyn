/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 17:09:47 2009 texane
** Last update Tue Dec 13 08:58:34 2011 fabien le mentec
*/


#ifndef SLOSYN_H_INCLUDED
# define SLOSYN_H_INCLUDED



#include <stdint.h>
#include "../../common/slosyn.h"
#include "../../../common/slosyn_types.h"



/* forward decls */

typedef struct slosyn_handle slosyn_handle_t;


/* error types */

enum slosyn_error
{
  SLOSYN_ERROR_SUCCESS = 0,
    
  SLOSYN_ERROR_ALREADY_INIT,
  SLOSYN_ERROR_LIBUSB,
  SLOSYN_ERROR_NOT_INIT,
  SLOSYN_ERROR_NOT_FOUND,
  SLOSYN_ERROR_MEMORY,
  SLOSYN_ERROR_TIMEOUT,
  SLOSYN_ERROR_IO,

  SLOSYN_ERROR_MAX
};

typedef enum slosyn_error slosyn_error_t;

/* error bitmap */
typedef unsigned int slosyn_bitmap_t;

#define SLOSYN_BIT_UNDEF (1 << 0) /* undefined error */
#define SLOSYN_BIT_IO (1 << 1) /* communication (usb) error */
#define SLOSYN_BIT_SLOSYN_ERROR (1 << 2) /* slosyn generic error */
#define SLOSYN_BIT_HOPPER_CHECK (1 << 3) /* hopper is empty */
#define SLOSYN_BIT_MOTION_CHECK (1 << 4) /* */
#define SLOSYN_BIT_NOT_CONNECTED (1 << 5) /* device not connected */
#define SLOSYN_BIT_NOT_READY (1 << 6) /* device not ready */


/* card callback */

typedef int (*slosyn_cardfn_t)(const uint16_t*, slosyn_alarms_t, void*);


/* interface */

slosyn_error_t slosyn_initialize(void);
void slosyn_cleanup(void);
slosyn_error_t slosyn_open(slosyn_handle_t**);
void slosyn_close(slosyn_handle_t*);

#if 0 /* TODO_M600_PORT */

slosyn_error_t slosyn_reset(slosyn_handle_t*);
slosyn_error_t slosyn_read_alarms(slosyn_handle_t*, slosyn_alarms_t*);
slosyn_error_t slosyn_read_cards(slosyn_handle_t*, unsigned int, slosyn_cardfn_t, void*);
slosyn_bitmap_t slosyn_read_card(slosyn_handle_t*);
void slosyn_get_card_buffer(const void**);
slosyn_bitmap_t slosyn_get_state(slosyn_handle_t*);
slosyn_error_t slosyn_fill_data(slosyn_handle_t*, uint16_t*);
slosyn_error_t slosyn_read_pins(slosyn_handle_t*, uint8_t*);

#endif /* TODO_M600_PORT */


#endif /* ! SLOSYN_H_INCLUDED */
