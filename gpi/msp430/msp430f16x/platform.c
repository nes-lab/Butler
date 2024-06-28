/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2018, Networked Embedded Systems Lab, TU Dresden
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions are met:
 *		* Redistributions of source code must retain the above copyright
 *		  notice, this list of conditions and the following disclaimer.
 *		* Redistributions in binary form must reproduce the above copyright
 *		  notice, this list of conditions and the following disclaimer in the
 *		  documentation and/or other materials provided with the distribution.
 *		* Neither the name of the NES Lab or TU Dresden nor the
 *		  names of its contributors may be used to endorse or promote products
 *		  derived from this software without specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************************************//**
 *
 *	@file					gpi/msp430f16x/platform.c
 *
 *	@brief					platform interface functions, specific for MSP430F16x
 *
 *	@version				$Id$
 *	@date					TODO
 *
 *	@author					Carsten Herrmann
 *
 ***************************************************************************************************

 	@details

	TODO

 **************************************************************************************************/
//***** Trace Settings *****************************************************************************

#include "gpi/trace.h"

GPI_TRACE_CONFIG(platform, GPI_TRACE_LOG_ALL);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/platform.h"
#include "gpi/interrupts.h"
#include "gpi/clocks.h"

#include <sys/crtld.h>

#include <string.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************



//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************



//**************************************************************************************************
//***** Forward Declarations ***********************************************************************



//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************



//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Local Functions ****************************************************************************



//**************************************************************************************************
//***** Global Functions ***************************************************************************

void gpi_msp430_flash_read(Gpi_Msp430_Flash_Type type, void *dest, uintptr_t src, size_t size)
{
	const uint8_t	*section;

	switch (type)
	{
		case INFO_A:	section = (const uint8_t*)&__infoa; 	break;
		case INFO_B:	section = (const uint8_t*)&__infob; 	break;
		default:
			assert(0);
			return;
    }

	if (src + size > 128)
	{
		assert(src + size <= 128);
		if (src >= 128)
			return;
		size = 128 - src;
    }

	memcpy(dest, &section[src], size);
}

//**************************************************************************************************

void gpi_msp430_flash_erase(Gpi_Msp430_Flash_Type type)
{
	int	*section;

	switch (type)
	{
		case INFO_A:	section = (int*)&__infoa; 	break;
		case INFO_B:	section = (int*)&__infob; 	break;
		default:
			assert(0);
			return;
    }

	int ie = gpi_int_lock();

	while (FCTL3 & BUSY);

	ASSERT_CT(GPI_FAST_CLOCK_RATE / 257000ul > 0);
	ASSERT_CT(GPI_FAST_CLOCK_RATE / 257000ul < 0x40);

	FCTL2 = FWKEY | FSSEL0 | (((GPI_FAST_CLOCK_RATE / 257000ul) - 1) << MSB(FN0));
	FCTL3 = FWKEY;

	FCTL1 = FWKEY | ERASE;
	section[0] = 0;
	while (FCTL3 & BUSY);

	FCTL1 = FWKEY;
	FCTL3 = FWKEY | LOCK;

	gpi_int_unlock(ie);
}

//**************************************************************************************************

void gpi_msp430_flash_write(Gpi_Msp430_Flash_Type type, uintptr_t dest, const void *src, size_t size)
{
	const uint8_t	*src_ = (const uint8_t*)src;
	uint8_t			*dest_;

	if (0 == size)
		return;

	switch (type)
	{
		case INFO_A:	dest_ = (uint8_t*)&__infoa; 	break;
		case INFO_B:	dest_ = (uint8_t*)&__infob; 	break;
		default:
			assert(0);
			return;
    }

	if (dest + size > 128)
	{
		assert(dest + size <= 128);
		if (dest >= 128)
			return;
		size = 128 - dest;
    }

	int ie = gpi_int_lock();

	while (FCTL3 & BUSY);

	ASSERT_CT(GPI_FAST_CLOCK_RATE / 257000ul > 0);
	ASSERT_CT(GPI_FAST_CLOCK_RATE / 257000ul < 0x40);

	FCTL2 = FWKEY | FSSEL0 | (((GPI_FAST_CLOCK_RATE / 257000ul) - 1) << MSB(FN0));
	FCTL3 = FWKEY;

	FCTL1 = FWKEY | WRT;

	dest_ += dest;
	while (size-- > 0)
	{
		*dest_++ = *src_++;
		while (FCTL3 & BUSY);
    }

	FCTL1 = FWKEY;
	FCTL3 = FWKEY | LOCK;

	gpi_int_unlock(ie);
}

//**************************************************************************************************
//**************************************************************************************************
