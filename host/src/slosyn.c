/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Tue Nov 17 04:21:01 2009 fabien le mentec
** Last update Tue Dec 13 10:24:08 2011 fabien le mentec
*/



#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CONFIG_LIBUSB_VERSION 1

#if CONFIG_LIBUSB_VERSION
# include "usb.h"
#else
# include <libusb-1.0/libusb.h>
#endif

#include "debug.h"
#include "slosyn.h"
#include "../../common/slosyn_usb.h"
#include "../../common/slosyn_types.h"



/* slosyn handle */

struct slosyn_handle
{
#if CONFIG_LIBUSB_VERSION
  usb_dev_handle* usb_handle;
#else
  libusb_device_handle* usb_handle;
  struct libusb_transfer* req_trans;
  struct libusb_transfer* rep_trans;
#endif

  unsigned int ep_req;
  unsigned int ep_rep;
};


/* utility macro */

#define GOTO_ERROR(E)	\
  do {			\
    error = E;		\
    goto on_error;	\
  } while (0)


/* slosyn command */

union slosyn_cmd
{
  slosyn_request_t req;
  slosyn_reply_t rep;
};

typedef union slosyn_cmd slosyn_cmd_t;


/* internal globals */

#if CONFIG_LIBUSB_VERSION
/* none */
#else
static libusb_context* libusb_ctx =  NULL;
#endif


/* allocation wrappers */

static slosyn_handle_t* alloc_slosyn_handle(void)
{
  slosyn_handle_t* handle = malloc(sizeof(slosyn_handle_t));
  if (handle == NULL)
    return NULL;

  handle->usb_handle = NULL;

#if CONFIG_LIBUSB_VERSION

  /* nop */

#else

  handle->req_trans = NULL;
  handle->rep_trans = NULL;

#endif

  return handle;
}


static void free_slosyn_handle(slosyn_handle_t* handle)
{
  free(handle);
}


/* translate libusb error */

#if CONFIG_LIBUSB_VERSION

static slosyn_error_t __attribute__((unused)) usb_to_slosyn_error(int ue)
{
  return SLOSYN_ERROR_LIBUSB;
}

#else

static slosyn_error_t __attribute__((unused))
usb_to_slosyn_error(enum libusb_error ue)
{
  slosyn_error_t me;

  switch (ue)
  {
  case LIBUSB_SUCCESS:
    me = SLOSYN_ERROR_SUCCESS;
    break;

  case LIBUSB_ERROR_IO:
    me = SLOSYN_ERROR_IO;
    break;

  case LIBUSB_ERROR_NO_DEVICE:
  case LIBUSB_ERROR_NOT_FOUND:
    me = SLOSYN_ERROR_NOT_FOUND;
    break;

  case LIBUSB_ERROR_TIMEOUT:
    me = SLOSYN_ERROR_TIMEOUT;
    break;

  case LIBUSB_ERROR_NO_MEM:
    me = SLOSYN_ERROR_MEMORY;
    break;

  default:
    me = SLOSYN_ERROR_LIBUSB;
    break;
  }

  return me;
}

#endif


/* is the usb device a slosyn card reader */

#if CONFIG_LIBUSB_VERSION

static unsigned int is_slosyn_device(const struct usb_device* dev)
{
  DEBUG_PRINTF("device: 0x%04x, 0x%04x\n",
	       dev->descriptor.idVendor,
	       dev->descriptor.idProduct);

  if (dev->descriptor.idVendor != SLOSYN_USB_VENDOR_ID)
    return 0;

  if (dev->descriptor.idProduct != SLOSYN_USB_PRODUCT_ID)
    return 0;

  return 1;
}

#else

static unsigned int is_slosyn_device(libusb_device* dev)
{
  struct libusb_device_descriptor desc;

  DEBUG_PRINTF("device: 0x%04x, 0x%04x\n", desc.idVendor, desc.idProduct);

  if (libusb_get_device_descriptor(dev, &desc))
    return 0;

  if (desc.idVendor != SLOSYN_USB_VENDOR_ID)
    return 0;

  if (desc.idProduct != SLOSYN_USB_PRODUCT_ID)
    return 0;

  return 1;
}

#endif


#if CONFIG_LIBUSB_VERSION

static slosyn_error_t send_recv_cmd
(
 slosyn_handle_t* handle,
 slosyn_cmd_t* cmd
)
{
  /* timeouts in ms */
#define CONFIG_DEFAULT_TIMEOUT (2000)

  int error = SLOSYN_ERROR_SUCCESS;
  int usberr;

  usberr = usb_bulk_write
    (handle->usb_handle, handle->ep_req, (void*)&cmd->req,
     (int)sizeof(cmd->req), CONFIG_DEFAULT_TIMEOUT);

  if (usberr < 0)
  {
    DEBUG_ERROR("usb_bulk_write() == %d(%s)\n", usberr, usb_strerror());
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);
  }

  usberr = usb_bulk_read
    (handle->usb_handle, handle->ep_rep, (void*)&cmd->rep,
     (int)sizeof(cmd->rep), CONFIG_DEFAULT_TIMEOUT);

  if (usberr < 0)
  {
    DEBUG_ERROR("usb_bulk_read() == %d(%s)\n", usberr, usb_strerror());
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);
  }

 on_error:
  return error;
}

#else /* ! CONFIG_LIBUSB_VERSION */

/* usb async io wrapper */

struct trans_ctx
{
#define TRANS_FLAGS_IS_DONE (1 << 0)
#define TRANS_FLAGS_HAS_ERROR (1 << 1)
  volatile unsigned long flags;
};


static void on_trans_done(struct libusb_transfer* trans)
{
  struct trans_ctx* const ctx = trans->user_data;

  if (trans->status != LIBUSB_TRANSFER_COMPLETED)
    ctx->flags |= TRANS_FLAGS_HAS_ERROR;

  ctx->flags = TRANS_FLAGS_IS_DONE;
}


static slosyn_error_t submit_wait(struct libusb_transfer* trans)
{
  struct trans_ctx trans_ctx;
  enum libusb_error error;

  trans_ctx.flags = 0;

  /* brief intrusion inside the libusb interface */
  trans->callback = on_trans_done;
  trans->user_data = &trans_ctx;

  if ((error = libusb_submit_transfer(trans)))
  {
    DEBUG_ERROR("libusb_submit_transfer(%d)\n", error);
    return SLOSYN_ERROR_LIBUSB;
  }

  while (!(trans_ctx.flags & TRANS_FLAGS_IS_DONE))
  {
    if (libusb_handle_events(NULL))
    {
      DEBUG_ERROR("libusb_handle_events()\n");
      return SLOSYN_ERROR_IO;
    }
  }

  return SLOSYN_ERROR_SUCCESS;
}


static slosyn_error_t send_recv_cmd
(
 slosyn_handle_t* handle,
 slosyn_cmd_t* cmd
)
{
  /* endpoint 0: control, not used here
     endpoint 1: interrupt mode write
     endpoint 2: bulk transfer used since
     time delivery does not matter but data
     integrity and transfer completion
  */

  slosyn_error_t error;

  /* send the command */

#if 0
  libusb_fill_interrupt_transfer
  (
   handle->req_trans,
   handle->usb_handle,
   handle->ep_req,
   (void*)&cmd->req,
   sizeof(cmd->req),
   NULL, NULL,
   0
  );
#else
  libusb_fill_bulk_transfer
  (
   handle->req_trans,
   handle->usb_handle,
   handle->ep_req,
   (void*)&cmd->req,
   sizeof(cmd->req),
   NULL, NULL,
   0
  );
#endif

  DEBUG_PRINTF("-- req transfer\n");

  error = submit_wait(handle->req_trans);
  if (error)
    GOTO_ERROR(error);

  /* read the response */

  libusb_fill_bulk_transfer
  (
   handle->rep_trans,
   handle->usb_handle,
   handle->ep_rep,
   (void*)&cmd->rep,
   sizeof(cmd->rep),
   NULL, NULL,
   0
  );

  DEBUG_PRINTF("-- rep transfer\n");

  error = submit_wait(handle->rep_trans);
  if (error)
    GOTO_ERROR(error);

  /* success */

  error = SLOSYN_ERROR_SUCCESS;

 on_error:
  return error;
}

#endif /* CONFIG_LIBUSB_VERSION */


/* forward decls */

#if CONFIG_LIBUSB_VERSION
static slosyn_error_t open_slosyn_usb_handle(usb_dev_handle**, int);
static void close_slosyn_usb_handle(usb_dev_handle*);
#else
static slosyn_error_t open_slosyn_usb_handle(libusb_dev_handle**, int);
static void close_slosyn_usb_handle(libusb_dev_handle**, int);
#endif

static int find_slosyn_device(void);

static slosyn_error_t send_recv_cmd_or_reopen
(
 slosyn_handle_t* handle,
 slosyn_cmd_t* cmd
)
{
  /* send_recv_cmd or reopen handle on libusb failure */

  slosyn_error_t error = send_recv_cmd(handle, cmd);
  if (error != SLOSYN_ERROR_SUCCESS)
  {
    if (find_slosyn_device() == -1)
      GOTO_ERROR(SLOSYN_ERROR_NOT_FOUND);

    /* reopen the usb handle */

    close_slosyn_usb_handle(handle->usb_handle);

    error = open_slosyn_usb_handle(&handle->usb_handle, 0);
    if (error != SLOSYN_ERROR_SUCCESS)
      GOTO_ERROR(error);

    /* resend the command */

    error = send_recv_cmd(handle, cmd);
  }

 on_error:
  return error;
}


/* endianness */

static int is_mach_le = 0;

static inline int get_is_mach_le(void)
{
  const uint16_t magic = 0x0102;

  if ((*(const uint8_t*)&magic) == 0x02)
    return 1;

  return 0;
}

static inline uint16_t le16_to_mach(uint16_t n)
{
  if (!is_mach_le)
    n = ((n >> 8) & 0xff) | ((n & 0xff) << 8);

  return n;
}


/* find slosyn device wrapper */

static int find_slosyn_device(void)
{
  /* return 0 if slosyn device found */

  usb_dev_handle* dummy_handle = NULL;
  int res = 0;

  const slosyn_error_t error = open_slosyn_usb_handle(&dummy_handle, 1);
  if (error == SLOSYN_ERROR_NOT_FOUND)
    res = -1;

  return res;
}


/* exported functions */

void slosyn_cleanup(void)
{
#if CONFIG_LIBUSB_VERSION

  /* nop */

#else

  if (libusb_ctx != NULL)
  {
    libusb_exit(libusb_ctx);
    libusb_ctx = NULL;
  }

#endif
}


slosyn_error_t slosyn_initialize(void)
{
  is_mach_le = get_is_mach_le();

#if CONFIG_LIBUSB_VERSION

  usb_init();

#else

  slosyn_error_t error;

  if (libusb_ctx != NULL)
    GOTO_ERROR(SLOSYN_ERROR_ALREADY_INIT);

  if (libusb_init(&libusb_ctx))
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);

 on_error:
  slosyn_cleanup();
  return error;

#endif

  return SLOSYN_ERROR_SUCCESS;
}


#if CONFIG_LIBUSB_VERSION

static void close_slosyn_usb_handle(usb_dev_handle* usb_handle)
{
  usb_close(usb_handle);
}

static slosyn_error_t open_slosyn_usb_handle
(usb_dev_handle** usb_handle, int enum_only)
{
  /* enum_only is used when we only wants to find
     the slosyn device. */

  slosyn_error_t error;
  struct usb_bus* bus;

  *usb_handle = NULL;

  usb_find_busses();
  usb_find_devices();

  for (bus = usb_busses; bus != NULL; bus = bus->next)
  {
    struct usb_device* dev;
    for (dev = bus->devices; dev != NULL; dev = dev->next)
    {
      if (is_slosyn_device(dev))
      {
	if (enum_only)
	  GOTO_ERROR(SLOSYN_ERROR_SUCCESS);

	*usb_handle = usb_open(dev);
	if (*usb_handle == NULL)
	  GOTO_ERROR(SLOSYN_ERROR_LIBUSB);

	if (usb_set_configuration(*usb_handle, 2))
	{
	  DEBUG_ERROR("libusb_set_configuration()\n");
	  GOTO_ERROR(SLOSYN_ERROR_LIBUSB);
	}

	if (usb_claim_interface(*usb_handle, 0))
	{
	  DEBUG_ERROR("libusb_claim_interface()\n");
	  GOTO_ERROR(SLOSYN_ERROR_LIBUSB);
	}

	return SLOSYN_ERROR_SUCCESS;
      }
    }
  }

  /* not found */

  error = SLOSYN_ERROR_NOT_FOUND;

 on_error:

  if (*usb_handle != NULL)
  {
    usb_close(*usb_handle);
    *usb_handle = NULL;
  }

  return error;
}

#else

static void close_slosyn_usb_handle(libusb_dev_handle* usb_handle)
{
  libusb_close(usb_handle);
}

static slosyn_error_t open_slosyn_usb_handle
(libusb_dev_handle** usb_handle, int enum_only)
{
  /* @see the above comment for enum_only meaning */

  ssize_t i;
  ssize_t count;
  libusb_device** devs = NULL;
  libusb_device* dev;
  slosyn_error_t error = SLOSYN_ERROR_SUCCESS;

  *usb_handle = NULL;

  if (libusb_ctx == NULL)
    GOTO_ERROR(SLOSYN_ERROR_NOT_INIT);

  count = libusb_get_device_list(libusb_ctx, &devs);
  if (count < 0)
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);

  for (i = 0; i < count; ++i)
  {
    dev = devs[i];

    if (is_slosyn_device(dev))
      break;
  }

  if (i == count)
    GOTO_ERROR(SLOSYN_ERROR_NOT_FOUND);

  /* open for enumeration only */
  if (enum_only)
    GOTO_ERROR(SLOSYN_ERROR_SUCCESS);

  if (libusb_open(dev, usb_handle))
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);

  if (libusb_set_configuration(*usb_handle, 2))
  {
    DEBUG_ERROR("libusb_set_configuration()\n");
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);
  }

  if (libusb_claim_interface(*usb_handle, 0))
  {
    DEBUG_ERROR("libusb_claim_interface()\n");
    GOTO_ERROR(SLOSYN_ERROR_LIBUSB);
  }

 on_error:

  if (devs != NULL)
    libusb_free_device_list(devs, 1);

  if (error == SLOSYN_ERROR_SUCCESS)
    return SLOSYN_ERROR_SUCCESS;

  if (*usb_handle != NULL)
  {
    libusb_close(*usb_handle);
    *usb_handle = NULL;
  }

  return error;
}

#endif /* CONFIG_LIBUSB_VERSION */


slosyn_error_t slosyn_open(slosyn_handle_t** slosyn_handle)
{
  slosyn_error_t error;
  usb_dev_handle* usb_handle = NULL;
  
  *slosyn_handle = NULL;

  /* open usb device */

  error = open_slosyn_usb_handle(&usb_handle, 0);
  if (error != SLOSYN_ERROR_SUCCESS)
    goto on_error;

  /* open and init the device */

  *slosyn_handle = alloc_slosyn_handle();
  if (*slosyn_handle == NULL)
    GOTO_ERROR(SLOSYN_ERROR_MEMORY);

  (*slosyn_handle)->usb_handle = usb_handle;

#if CONFIG_LIBUSB_VERSION

  (*slosyn_handle)->ep_req = SLOSYN_USB_EP_REQ | USB_ENDPOINT_OUT;
  (*slosyn_handle)->ep_rep = SLOSYN_USB_EP_REP | USB_ENDPOINT_IN;

#else

  (*slosyn_handle)->req_trans = libusb_alloc_transfer(0);
  if ((*slosyn_handle)->req_trans == NULL)
    GOTO_ERROR(SLOSYN_ERROR_MEMORY);

  (*slosyn_handle)->rep_trans = libusb_alloc_transfer(0);
  if ((*slosyn_handle)->rep_trans == NULL)
    GOTO_ERROR(SLOSYN_ERROR_MEMORY);

  (*slosyn_handle)->ep_req = SLOSYN_USB_EP_REQ | LIBUSB_ENDPOINT_OUT;
  (*slosyn_handle)->ep_rep = SLOSYN_USB_EP_REP | LIBUSB_ENDPOINT_IN;

#endif

  error = SLOSYN_ERROR_SUCCESS;
  
 on_error:

  return error;
}


void slosyn_close(slosyn_handle_t* handle)
{
#if CONFIG_LIBUSB_VERSION

  /* nothing to do */

#else

  if (handle->req_trans != NULL)
    libusb_free_transfer(handle->req_trans);

  if (handle->rep_trans != NULL)
    libusb_free_transfer(handle->rep_trans);

#endif

  if (handle->usb_handle != NULL)
    close_slosyn_usb_handle(handle->usb_handle);

  free_slosyn_handle(handle);
}


slosyn_error_t slosyn_echo
(slosyn_handle_t* handle, uint8_t* buf, size_t size)
{
  slosyn_error_t error;
  slosyn_cmd_t cmd;

  if (size > SLOSYN_NCHARS_MAX) return SLOSYN_ERROR_EINVAL;

  cmd.req.req = SLOSYN_REQ_ECHO;
  cmd.req.nchars = size;

  error = send_recv_cmd_or_reopen(handle, &cmd);
  if (error != SLOSYN_ERROR_SUCCESS) return error;

  if (cmd.rep.nchars != size) return SLOSYN_ERROR_EINVAL;

  memcpy(buf, cmd.rep.chars, size);

  return SLOSYN_ERROR_SUCCESS;
}


slosyn_error_t slosyn_get_state
(slosyn_handle_t* handle, slosyn_bitmap_t* b)
{
  slosyn_error_t error;
  slosyn_cmd_t cmd;

  cmd.req.req = SLOSYN_REQ_STATE;

  error = send_recv_cmd_or_reopen(handle, &cmd);
  if (error != SLOSYN_ERROR_SUCCESS) return error;

  *b = cmd.rep.status;

  return SLOSYN_ERROR_SUCCESS;
}


slosyn_error_t slosyn_read_chars
(slosyn_handle_t* handle, unsigned int dir, uint8_t* buf, size_t* nchars)
{
  slosyn_error_t error;
  slosyn_cmd_t cmd;
  unsigned int nloops;
  unsigned int i;
  size_t nread = 0;
  size_t npad;

  nloops = *nchars / SLOSYN_NCHARS_MAX;
  for (i = 0; i < nloops; ++i)
  {
    cmd.req.req = SLOSYN_REQ_READ;
    cmd.req.dir = dir;
    cmd.req.nchars = SLOSYN_NCHARS_MAX;

    error = send_recv_cmd_or_reopen(handle, &cmd);
    if (error != SLOSYN_ERROR_SUCCESS) return error;

    nread += cmd.rep.nchars;

    /* fewer chars may have been read */
    if (cmd.rep.nchars != SLOSYN_NCHARS_MAX) goto on_done;
  }

  npad = (*nchars) % SLOSYN_NCHARS_MAX;
  if (npad)
  {
    cmd.req.req = SLOSYN_REQ_READ;
    cmd.req.dir = dir;
    cmd.req.nchars = npad;

    error = send_recv_cmd_or_reopen(handle, &cmd);
    if (error != SLOSYN_ERROR_SUCCESS) return error;

    nread += cmd.rep.nchars;
  }

 on_done:
  *nchars = nread;
  return SLOSYN_ERROR_SUCCESS;
}


slosyn_error_t slosyn_rewind
(slosyn_handle_t* handle, unsigned int dir)
{
  slosyn_cmd_t cmd;
  cmd.req.req = SLOSYN_REQ_REWIND;
  cmd.req.dir = dir;
  return send_recv_cmd_or_reopen(handle, &cmd);
}
