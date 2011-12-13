/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 13:59:45 2009 texane
** Last update Tue Dec 13 08:30:38 2011 fabien le mentec
*/



#ifndef SLOSYN_H_INCLUDED
# define SLOSYN_H_INCLUDED



#include "stdint.h"
#include "../../../common/slosyn_types.h"


void slosyn_setup(void);
void slosyn_start_request(slosyn_request_t);
void slosyn_schedule(void);
void slosyn_print_signals(void);



#endif /* ! SLOSYN_H_INCLUDED */
