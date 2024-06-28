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
 *	@file					gpi/msp430f16x/olf.c
 *
 *	@brief					optimized low-level functions, tuned for MSP430F16x
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



//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/olf.h"

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************

// TI changed the ABI in later GCC versions -> catch that
// TODO: adapt code to support new ABI
#if ((__GNUC__ != 4) || (__GNUC_MINOR__ != 6) || (__GNUC_PATCHLEVEL__ != 3))
	#error unsupported GCC version
#endif

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

// .data part of the gpi_memcpy8 core loop (running from RAM), code is dynamically modified at runtime
// (i.e. gpi_memcpy8() altogether uses self-modifying code)
//
// The best way to store the function in RAM would be to declare it with __attribute__((section(".data"))).
// Unfortunately this causes an annoying warning message. To avoid that, we copy the code to RAM
// manually by using a constructor function.
//
// ATTENTION: One might think that setting the block size based on sizeof(gpi_memcpy8_core_text)
// is the best way. I would agree, but for some reason this resolves to 0 without any warning(!).
// So we set it manually -> take care to be consistent.

uint16_t	gpi_memcpy8_core[5];

static void __attribute__((naked)) gpi_memcpy8_core_text(const void *src, unsigned int offset)
{
	__asm__ volatile
	(
		"1:								\n"
		"mov.b		@r15+, 0(r14)		\n"		// [0]
		"inc		r14					\n"		// [2]
		"jnz		1b					\n"		// [3]
		"ret							\n"		// [4]
		:
		:
		: "cc", "memory"
	);
}

// ATTENTION: as a constructor, this function is called before main; so take care what you do
// (e.g. hardware is not initialized at this point and watchdog timer is running)
static void __attribute__((constructor)) gpi_memcpy8_init()
{
	gpi_memcpy_dma_aligned(&gpi_memcpy8_core, &gpi_memcpy8_core_text, sizeof(gpi_memcpy8_core));
}

//**************************************************************************************************

void gpi_memcpy_dma(void *dest, const void *src, size_t size)
{
	uint8_t			*pd = dest;
	const uint8_t	*ps = src;

	while (size > 0)
	{
		uint16_t block_size = (size > GPI_MEMCPY_DMA_BLOCKSIZE) ? GPI_MEMCPY_DMA_BLOCKSIZE : size;
		if (!(__read_status_register() & GIE))
			block_size = size;

		gpi_memcpy_dma_inline(pd, ps, block_size);
		pd += block_size;
		ps += block_size;
		size -= block_size;
    }
}

//**************************************************************************************************

void gpi_memmove_dma(void *dest, const void *src, size_t size)
{
	uint8_t			*pd = dest;
	const uint8_t	*ps = src;

	if (pd <= ps)
	{
		gpi_memcpy_dma(dest, src, size);
		return;
    }

	while (size > 0)
	{
		uint16_t block_size = (size > GPI_MEMCPY_DMA_BLOCKSIZE) ? GPI_MEMCPY_DMA_BLOCKSIZE : size;
		if (!(__read_status_register() & GIE))
			block_size = size;

		gpi_memmove_dma_inline(pd, ps, block_size);
		pd += block_size;
		ps += block_size;
		size -= block_size;
    }
}

//**************************************************************************************************
//**************************************************************************************************
