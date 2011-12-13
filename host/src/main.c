/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 18 15:55:52 2009 texane
** Last update Tue Dec 13 08:59:41 2011 fabien le mentec
*/



#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "slosyn.h"
#include "../../../common/slosyn_types.h"



#if 0 /* TODO_M600_PORT */

static const char* __attribute__((unused))
bitmap_to_string(m600_bitmap_t bitmap)
{
#define BIT_CASE(__bitmap, __bitname)		\
  do {						\
    if ((__bitmap) & (M600_BIT_##__bitname))	\
    {						\
      strcat(buf, ", ");			\
      strcpy(buf, #__bitname);			\
    }						\
  } while (0)

  static char buf[1024];

  buf[0] = 0;

  BIT_CASE(bitmap, UNDEF);
  BIT_CASE(bitmap, IO);
  BIT_CASE(bitmap, M600_ERROR);
  BIT_CASE(bitmap, HOPPER_CHECK);
  BIT_CASE(bitmap, MOTION_CHECK);
  BIT_CASE(bitmap, NOT_CONNECTED);
  BIT_CASE(bitmap, NOT_READY);

  return buf;
}


static const char* __attribute__((unused))
alarms_to_string(unsigned int a)
{
  const char* s = "UNKNOWN_ALARM";

  switch (a)
  {
#define ALARM_CASE(E)				\
    case M600_ALARM_ ## E:			\
      s = #E;					\
    break

    ALARM_CASE(ERROR);
    ALARM_CASE(HOPPER_CHECK);
    ALARM_CASE(MOTION_CHECK);
    ALARM_CASE(NOT_READY);

  default: break;
  }

  return s;
}


static int on_card(const uint16_t* data, m600_alarms_t alarms, void* ctx)
{
  unsigned int i;

  if (alarms)
  {
    printf("alarms: %s\n", alarms_to_string(alarms));
  }
  else if (data != NULL)
  {
    for (i = 0; i < 80; ++i)
    {
      if (!(i % 8))
	printf("\n");
      printf(" %03x", ((uint16_t)~(data[i])) & 0xfff);
    }

    for (i = 0; i < 80; ++i)
    {
      if (!(i % 10))
	printf("\n");
      printf("%c", ~(data[i]));
    }
  }

  return 0;
}


static void on_card2(const unsigned int* data)
{
  unsigned int i;

  for (i = 0; i < 80; ++i)
  {
    if (!(i % 8))
      printf("\n");
    printf(" %03x", ((unsigned int)~(data[i])) & 0xfff);
  }

  for (i = 0; i < 80; ++i)
  {
    if (!(i % 10))
      printf("\n");
    printf("%c", ~(data[i]));
  }
}


static void __attribute__((unused))
print_alarms(m600_alarms_t alarms)
{
  unsigned int a;

  printf("alarms:\n");
  for (a = 0; a < M600_ALARM_MAX; ++a)
  {
    if (alarms & (1 << a))
      printf(" %s\n", alarms_to_string(a));
  }
}


static void clear_buffer(uint16_t* buffer)
{
  unsigned int i;

  for (i = 0; i < M600_COLUMN_COUNT; ++i)
    buffer[i] = 0xffff;
}


static int check_buffer(const uint16_t* buffer)
{
  unsigned int i;

  for (i = 0; i < M600_COLUMN_COUNT; ++i)
  {
    if (buffer[i] != (uint16_t)i)
    {
      printf("@%u 0x%04x\n", i, buffer[i]);
      return -1;
    }
  }

  return 0;
}


static void print_pins(const uint8_t* buffer)
{
  unsigned int j;

#if 0 /* inline printing */

#define GET_LAST_BIT(N) (((N) & (1 << 7)) >> 7)

  unsigned int i;

  for (i = 0; i < M600_PIN_COUNT / 8; ++i, ++buffer)
  {
    uint8_t n = *buffer;

    for (j = 0; j < 8; ++j, n <<= 1)
      printf("%c", '0' + GET_LAST_BIT(n));

    printf(" ");
  }
#else /* port like printing */

#define GET_FIRST_BIT(N) ((N) & 1)

  int i;
  int k;

  for (k = 1; k >= 0; --k)
  {
    for (i = M600_PIN_COUNT / 8 - 1; i >= 0; --i)
    {
      uint8_t n = buffer[i] >> k;

      for (j = 0; j < 4; ++j, n >>= 2)
	printf("%c", '0' + GET_FIRST_BIT(n));
      printf(" ");
    }

    printf("\n");
  }
  
#endif

  printf("\n");
}

#endif /* TODO_M600_PORT */


int main(int ac, char** av)
{
  int error = -1;
  slosyn_handle_t* handle = NULL;
  const char* const opt = ac == 1 ? "test" : av[1];

  if (slosyn_initialize() != SLOSYN_ERROR_SUCCESS)
    goto on_error;

  if (slosyn_open(&handle) != SLOSYN_ERROR_SUCCESS)
    goto on_error;

#if 0 /* TODO_M600_PORT */

  if (!strcmp(opt, "card0"))
  {
    if (slosyn_read_cards(handle, 1, on_card, NULL) != SLOSYN_ERROR_SUCCESS)
      printf("error\n");
    else
      printf("succes\n");
  }
  else if (!strcmp(opt, "card1"))
  {
    slosyn_bitmap_t bitmap;
    bitmap = slosyn_read_card(handle);
    if (bitmap)
    {
      printf("%s\n", bitmap_to_string(bitmap));
    }
    else
    {
      const void* card_buffer;
      slosyn_get_card_buffer(&card_buffer);
      on_card2((const unsigned int*)card_buffer);
    }
  }
  else if (!strcmp(opt, "state"))
  {
    const slosyn_bitmap_t bitmap = slosyn_get_state(handle);
    if (bitmap)
      printf("%s\n", bitmap_to_string(bitmap));
    else
      printf("no error to signal\n");
  }
  else if (!strcmp(opt, "alarms"))
  {
    slosyn_alarms_t alarms;

    if (slosyn_read_alarms(handle, &alarms) != SLOSYN_ERROR_SUCCESS)
      goto on_error;

    print_alarms(alarms);
  }
  else if (!strcmp(opt, "test"))
  {
    uint16_t data[SLOSYN_COLUMN_COUNT];

    clear_buffer(data);

    if (slosyn_fill_data(handle, data) != SLOSYN_ERROR_SUCCESS)
      goto on_error;

    if (check_buffer(data))
      printf("invalidBuffer\n");
    else
      printf("bufferOk\n");

    clear_buffer(data);

    if (slosyn_read_pins(handle, (uint8_t*)data) != SLOSYN_ERROR_SUCCESS)
      goto on_error;

    print_pins((const uint8_t*)data);
  }
  else if (!strcmp(opt, "reset"))
  {
    slosyn_reset(handle);
    getchar();
  }
  else
  {
    printf("invalidOption: %s\n", opt);
  }

#endif /* TODO_M600_PORT */

  error = 0;

 on_error:

  if (handle != NULL)
    slosyn_close(handle);

  slosyn_cleanup();

  return error;
}
