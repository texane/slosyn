/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Thu Nov 19 20:23:44 2009 texane
** Last update Sat Nov 21 08:23:54 2009 texane
*/



#include "usb_descriptors.h"



const USB_Device_Descriptor boot_device_descriptor =
{
    sizeof(USB_Device_Descriptor),    // Size of this descriptor in bytes
    DEVICE_DESCRIPTOR,                // Device descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0xff,                   // Class Code
    0x00,                   // Subclass code
    0xff,                   // Protocol code
    EP0_BUFFER_SIZE,        // Max packet size for EP0
    0x2a2a,                 // Vendor ID
    0xf00f,                 // Product ID
    0x0100,                 // Device release number in BCD format
    1,                      // Manufacturer string index
    2,                      // Product string index
    3,                      // Device serial number string index
    2                       // Number of possible configurations
};

/* Configurations Descriptor */
const USB_Default_Composite_Descriptor boot_default_cfg = {
    // Configuration Descriptor
    {sizeof(USB_Configuration_Descriptor),    // Size of this descriptor in bytes
    CONFIGURATION_DESCRIPTOR,                 // CONFIGURATION descriptor type
    sizeof(boot_default_cfg),          // Total length of data for this configuration
    1,                      // Number of interfaces in this configuration
    1,                      // Index value of this configuration
    4,                      // Configuration string index
    DEFAULT | SELF_POWERED, // Attributes
    0},                     // Max power consumption (2X mA)

    // Boot Interface Descriptor
    {sizeof(USB_Interface_Descriptor),   // Size of this descriptor in bytes
    INTERFACE_DESCRIPTOR,                // Interface descriptor type
    0,                      // Interface Number
    0,                      // Alternate Setting Number
    0,                      // Number of endpoints in this interface
    0xff,                   // Class code
    0xff,                   // Subclass code
    0xff,                   // Protocol code
    4}                      // Interface string index
};

const USB_Flash_Composite_Descriptor boot_flash_cfg = {
    // Configuration Descriptor
    {sizeof(USB_Configuration_Descriptor),    // Size of this descriptor in bytes
    CONFIGURATION_DESCRIPTOR,                 // CONFIGURATION descriptor type
    sizeof(boot_flash_cfg),          // Total length of data for this configuration
    1,                      // Number of interfaces in this configuration
    2,                      // Index value of this configuration
    5,                      // Configuration string index
    DEFAULT | SELF_POWERED, // Attributes
    0},                     // Max power consumption (2X mA)

    // Flash Interface Descriptor
    {sizeof(USB_Interface_Descriptor),   // Size of this descriptor in bytes
    INTERFACE_DESCRIPTOR,               // Interface descriptor type
    0,                      // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this interface
    0xff,                   // Class code
    0xff,                   // Subclass code
    0xff,                   // Protocol code
    5},                     // Interface string index

    // Flash Endpoint Descriptors
    {{sizeof(USB_Endpoint_Descriptor),
      ENDPOINT_DESCRIPTOR,
      EP(1) | OUT_EP,
      BULK,
      EP1_BUFFER_SIZE,   // USB 2.0 says : 64 bytes max
      0}, // not used for full speed bulk EP

    {sizeof(USB_Endpoint_Descriptor),
     ENDPOINT_DESCRIPTOR,
     EP(2) | IN_EP,
     BULK,
     EP2_BUFFER_SIZE,   // USB 2.0 says : 64 bytes max
     0}}   // not used for full speed bulk EP
};

const uchar * const boot_configuration_descriptor[] = {
    (const uchar*) &boot_default_cfg,
    (const uchar*) &boot_flash_cfg
};

/* String descriptors */
/* Language desriptors (Unicode 3.0 (UTF-16) */
const uchar str0[] = {sizeof(str0),  STRING_DESCRIPTOR, 0x09,0x04};// french = 0x040c, english = 0x409

const uchar str1[] = {sizeof(str1),  STRING_DESCRIPTOR,
                                              'A',0x00,
                                              'c', 0x00,
                                              'o',0x00,
                                              'n',0x00,
                                              'i',0x00,
                                              't',0x00};

const uchar str2[] = {sizeof(str2),  STRING_DESCRIPTOR,
                                              'M',0x00,
                                              '6',0x00,
                                              '0',0x00,
                                              '0',0x00,
                                              'R',0x00,
                                              'e',0x00,
                                              'a',0x00,
                                              'd',0x00,
                                              'e',0x00,
                                              'r',0x00};

const uchar str3[] = {sizeof(str3), STRING_DESCRIPTOR, '1',0x00};

const uchar str4[] = {sizeof(str4),  STRING_DESCRIPTOR,
                                              'D',0x00,
                                              'e',0x00,
                                              'f',0x00,
                                              'a',0x00,
                                              'u',0x00,
                                              'l',0x00,
                                              't',0x00};

const uchar str5[] = {sizeof(str5),  STRING_DESCRIPTOR,
                                              'F',0x00,
                                              'l',0x00,
                                              'a',0x00,
                                              's',0x00,
                                              'h',0x00};

const uchar * const boot_string_descriptor[] = {str0, str1, str2, str3, str4, str5};

/******************************************************************************
 * USB Endpoints callbacks
 *****************************************************************************/
void null_function() __naked
{
    __asm
        return
    __endasm;
}

static void (* const boot_ep_init_cfg0 [])(void) =
{
  ep0_init,
  null_function,
  null_function
};


static void (* const boot_ep_init_cfg1 [])(void) =
{
  ep0_init,
  ep1_init,
  ep2_init
};


/*
 *  boot_ep_init_cfg0 is duplicated so it is used on cfg 0 (device not configured)
 * and 1 (device configured)
 */

void (** const boot_ep_init[])(void) =
{
  boot_ep_init_cfg0,
  boot_ep_init_cfg0,
  boot_ep_init_cfg1
};

static void (* const boot_ep_in_cfg0 [])(void) =
{
  ep0_in,
  null_function,
  null_function
};

static void (* const boot_ep_in_cfg1 [])(void) =
{
  ep0_in,
  null_function,
  ep2_in
};

void (** const boot_ep_in[])(void) =
{
  boot_ep_in_cfg0,
  boot_ep_in_cfg0,
  boot_ep_in_cfg1
};

static void (* const boot_ep_out_cfg0 [])(void) =
{
  ep0_init,
  null_function,
  null_function
};

static void (* const boot_ep_out_cfg1 [])(void) =
{
  ep0_init,
  ep1_out,
  null_function
};

void (** const boot_ep_out[])(void) = 
{
  boot_ep_out_cfg0,
  boot_ep_out_cfg0,
  boot_ep_out_cfg1
};

static void (* const boot_ep_setup_cfg [])(void) =
{
  ep0_setup,
  null_function,
  null_function
};


void (** const boot_ep_setup[])(void) =
{
  boot_ep_setup_cfg,
  boot_ep_setup_cfg,
  boot_ep_setup_cfg
};

