/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 18 15:55:52 2009 texane
** Last update Tue Dec 13 10:11:08 2011 fabien le mentec
*/



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "slosyn.h"
#include "../../common/slosyn_types.h"



int main(int ac, char** av)
{
  int error = -1;
  slosyn_handle_t* handle = NULL;
  const char* const opt = ac == 1 ? "echo" : av[1];
  slosyn_error_t serr;

  if (slosyn_initialize() != SLOSYN_ERROR_SUCCESS)
    goto on_error;

  if (slosyn_open(&handle) != SLOSYN_ERROR_SUCCESS)
    goto on_error;

  if (!strcmp(opt, "echo"))
  {
    const char* s = "ping";
    uint8_t buffer[32];
    size_t size;
    size_t i;

    if (ac > 2) s = av[2];
    size = strlen(s);

    if (size > (sizeof(buffer) - 1))
    {
      printf("invalid echo argument\n");
      goto on_error;
    }

    memcpy((char*)buffer, s, size);

    serr = slosyn_echo(handle, buffer, size);
    if (serr != SLOSYN_ERROR_SUCCESS)
    {
      printf("[!] slosyn_get_state() == %d\n", serr);
      goto on_error;
    }

    for (i = 0; i < size; ++i) printf("%c", buffer[i]);
    printf("\n");
  }
  else if (!strcmp(opt, "state"))
  {
    slosyn_bitmap_t bitmap;

    serr = slosyn_get_state(handle, &bitmap);
    if (serr != SLOSYN_ERROR_SUCCESS)
    {
      printf("[!] slosyn_get_state() == %d\n", serr);
      goto on_error;
    }

    printf("state == 0x%08x\n", bitmap);
  }
  else if (!strcmp(opt, "read"))
  {
    unsigned int nchars = SLOSYN_NCHARS_INF;
    unsigned int dir = SLOSYN_DIR_BWD;
    uint8_t* buffer;

    if ((ac > 2) && (!strcmp(av[2], "fwd"))) dir = SLOSYN_DIR_FWD;
    if (ac > 3) nchars = atoi(av[3]);

    buffer = malloc(nchars * sizeof(uint8_t));
    if (buffer == NULL)
    {
      perror("malloc");
      goto on_error;
    }

    serr = slosyn_read_chars(handle, dir, buffer, &nchars);
    if (serr == SLOSYN_ERROR_SUCCESS)
    {
      unsigned int i;
      for (i = 0; i < nchars; ++i)
	printf(" %02x", buffer[i]);
      printf("\n");
    }
    free(buffer);

    if (serr != SLOSYN_ERROR_SUCCESS)
    {
      printf("[!] slosyn_read_chars() == %d\n", serr);
      goto on_error;
    }
  }
  else if (!strcmp(opt, "rewind"))
  {
    unsigned int dir = SLOSYN_DIR_BWD;
    if ((ac > 2) && (!strcmp(av[2], "fwd"))) dir = SLOSYN_DIR_FWD;
    serr = slosyn_rewind(handle, dir);
    if (serr != SLOSYN_ERROR_SUCCESS)
    {
      printf("[!] slosyn_rewind() == %d\n", serr);
      goto on_error;
    }
  }

  error = 0;

 on_error:

  if (handle != NULL)
    slosyn_close(handle);

  slosyn_cleanup();

  return error;
}
