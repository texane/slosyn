/*-------------------------------------------------------------------------
  interrupt_iface.c - Interruptions interface

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

/* $Id: interrupt_iface.c,v 1.3 2009-08-15 16:11:15 gaufille Exp $ */

#include "config.h"

/* Interrupt vectors */

LOW_PRIORITY_ISR_PRAGMA
void low_priority_isr(void) interrupt 2
{
}

HIGH_PRIORITY_ISR_PRAGMA
void high_priority_isr(void) interrupt 1
{
}
