/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 17:09:47 2009 texane
** Last update Tue Dec 13 10:10:27 2011 fabien le mentec
*/


#ifndef SLOSYN_H_INCLUDED
# define SLOSYN_H_INCLUDED



#include <stdint.h>
#include <sys/types.h>
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
  SLOSYN_ERROR_EINVAL,

  SLOSYN_ERROR_MAX
};

typedef enum slosyn_error slosyn_error_t;


/* interface */

slosyn_error_t slosyn_initialize(void);
void slosyn_cleanup(void);
slosyn_error_t slosyn_open(slosyn_handle_t**);
void slosyn_close(slosyn_handle_t*);
slosyn_error_t slosyn_echo(slosyn_handle_t*, uint8_t*, size_t);
slosyn_error_t slosyn_get_state(slosyn_handle_t*, slosyn_bitmap_t*);
slosyn_error_t slosyn_read_chars(slosyn_handle_t*, unsigned int, uint8_t*, size_t*);
slosyn_error_t slosyn_rewind(slosyn_handle_t*, unsigned int);



#endif /* ! SLOSYN_H_INCLUDED */
