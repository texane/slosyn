/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Wed Nov 11 15:33:43 2009 texane
** Last update Tue Dec 13 10:14:34 2011 fabien le mentec
*/



#ifndef SLOSYN_TYPES_H_INCLUDED
# define SLOSYN_TYPES_H_INCLUDED



/* include stdint before */


/* for testing, 4 ports * 8 pins */

#define SLOSYN_PIN_COUNT 32


/* protocol related types */

struct slosyn_request
{
  /* command */
#define SLOSYN_REQ_READ 0
#define SLOSYN_REQ_REWIND 1
#define SLOSYN_REQ_ECHO 2
#define SLOSYN_REQ_STATE 2
#define SLOSYN_REQ_INVALID (slosyn_request_t)-1
  uint8_t req;

  /* how many chars. 0 for inf.
     note: must not be greater than SLOSYN_NCHARS_MAX.
     bigger read request are handle by chunks.
   */
#define SLOSYN_NCHARS_INF 0
  uint8_t nchars;

  /* forward, backward direction */
#define SLOSYN_DIR_FWD 0
#define SLOSYN_DIR_BWD 1
  uint8_t dir;
}
#ifndef SDCC
__attribute__((packed))
#endif
;

typedef struct slosyn_request slosyn_request_t;

typedef uint8_t slosyn_bitmap_t;

struct slosyn_reply
{
#define SLOSYN_BIT_EOB (1 << 0) /* end of band */
  slosyn_bitmap_t status;

  uint8_t nchars;

#define SLOSYN_NCHARS_MAX 80
  uint8_t chars[SLOSYN_NCHARS_MAX];
}
#ifndef SDCC
__attribute__((packed))
#endif
;

typedef struct slosyn_reply slosyn_reply_t;



#endif /* ! SLOSYN_TYPES_H_INCLUDED */
