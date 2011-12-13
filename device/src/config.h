/*-------------------------------------------------------------------------
  config.h - Bootloader configuration

             (c) 2006-2009 Pierre Gaufillet <pierre.gaufillet@magic.fr>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
-------------------------------------------------------------------------*/

/* $Id: config.h,v 1.7 2009-08-30 17:05:57 gaufille Exp $ */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "common_types.h"

/* BOOTLOADER VERSION */
# define BOOTLOADER_VERSION 0x0120

/* EP0 buffer size */
#define EP0_BUFFER_SIZE 8

/* EP1 buffer size */
#define EP1_BUFFER_SIZE 64

/* EP2 buffer size */
#define EP2_BUFFER_SIZE 64

/* USART debug buffer size */
#define EUSART_BUFFER_SIZE 256

/* Application data address */
#ifdef _DEBUG
	#define APPLICATION_DATA_ADDRESS 0x6000
#else
	#define APPLICATION_DATA_ADDRESS 0x2000
#endif

/* Interrupt vectors */
#ifdef _DEBUG
	#define HIGH_PRIORITY_ISR_PRAGMA _Pragma ("code high_priority_isr 0x6020")
	#define LOW_PRIORITY_ISR_PRAGMA _Pragma ("code high_priority_isr 0x7000")
#else
	#define HIGH_PRIORITY_ISR_PRAGMA _Pragma ("code high_priority_isr 0x2020")
	#define LOW_PRIORITY_ISR_PRAGMA _Pragma ("code high_priority_isr 0x4000")
#endif

/* Stack size and address : see boot_main.c line 24 due to _Pragma limitations */

/* Memory sections for flash operations */
extern const uchar section_descriptor [22];

#endif /*CONFIG_H_*/
