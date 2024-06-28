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
 *	@file					mixer/tmote/memxor.c
 *
 *	@brief					memxor implementation, optimized for MSP430
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

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
//#define TRACE_GROUP1		0x00000001
//#define TRACE_GROUP2		0x00000002

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION	GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(memxor, GPI_TRACE_BASE_SELECTION | GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "memxor.h"

#include "gpi/tools.h"
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

// optimized memxor() implementation

// .data part of the core loop (running from RAM), code is dynamically modified at runtime
// (i.e. memxor() altogether uses self-modifying code)
//
// The best way to store the function in RAM would be to declare it with __attribute__((section(".data"))).
// Unfortunately this causes an annoying warning message. To avoid that, we copy the code to RAM
// manually by using a constructor function.
//
// ATTENTION: One might think that setting the block size based on sizeof(memxor_core_text)
// is the best way. I would agree, but for some reason this resolves to 0 without any warning(!).
// So we set it manually -> take care to be consistent.

uint16_t	memxor_core[5];

static void __attribute__((naked)) memxor_core_text(const void *src, unsigned int offset)
{
	__asm__ volatile
	(
		"1:								\n"
		"xor		@r15+, 0(r14)		\n"		// [0]
		"incd		r14					\n"		// [2]
		"jnz		1b					\n"		// [3]
		"ret							\n"		// [4]
		:
		:
		: "cc", "memory"
	);
}

// ATTENTION: as a constructor, this function is called before main; so take care what you do
// (e.g. hardware is not initialized at this point and watchdog timer is running)
static void __attribute__((constructor)) memxor_init()
{
	gpi_memcpy_dma_aligned(&memxor_core, &memxor_core_text, sizeof(memxor_core));
}

//**************************************************************************************************
// optimized memxor_block() implementation

// .text part of the core loop (running from flash)
void __attribute__((naked)) memxor_block_core(unsigned int minus_size, /*const*/ void *src[], int skip)
{
	__asm__ volatile
	(
		// r15 = -size
		// r14 = src
		// r13 = skip, p0
		// r13...r4 = p0...p9

		"add	r13, r0				\n"		// indirect jump
		"push	r4					\n"
		"mov	@r14+, r4			\n"
		"push	r5					\n"
		"mov	@r14+, r5			\n"
		"push	r6					\n"
		"mov	@r14+, r6			\n"
		"push	r7					\n"
		"mov	@r14+, r7			\n"
		"push	r8					\n"
		"mov	@r14+, r8			\n"
		"push	r9					\n"
		"mov	@r14+, r9			\n"
		"push	r10					\n"
		"mov	@r14+, r10			\n"
		"push	r11					\n"
		"mov	@r14+, r11			\n"
		"nop						\n"
		"mov	@r14+, r12			\n"
		"mov	@r14+, r13			\n"
		"br		%[loop]				\n"
		:
		: [loop] "i"(&memxor_block_core_loop[13])
		: "cc", "memory"
	);
}

//**************************************************************************************************

// .data part of the core loop (running from RAM), code is dynamically modified at runtime
// (i.e. memxor_block() altogether uses self-modifying code)
//
// The best way to store the function in RAM would be to declare it with __attribute__((section(".data"))).
// Unfortunately this causes an annoying warning message. To avoid that, we copy the code to RAM
// manually by using a constructor function.
//
// ATTENTION: One might think that setting the block size based on sizeof(memxor_block_core_loop_text)
// is the best way. I would agree, but for some reason this resolves to 0 without any warning(!).
// So we set it manually -> take care to be consistent.

static void		memxor_block_core_loop_text();
uint16_t		memxor_block_core_loop[24];

// ATTENTION: as a constructor, this function is called before main; so take care what you do
// (e.g. hardware is not initialized at this point and watchdog timer is running)
static void __attribute__((constructor)) memxor_block_init()
{
	gpi_memcpy_dma_aligned(&memxor_block_core_loop, &memxor_block_core_loop_text, sizeof(memxor_block_core_loop));
}

static void __attribute__((naked)) memxor_block_core_loop_text()
{
	__asm__ volatile
	(
		// r15 = -size
		// r14 = tmp
		// r13...r4 = p0...p9

		"1:							\n"
		"mov	@r13+, r14			\n"		// memxor_block_core_loop[0]
		"xor	@r4+, r14			\n"
		"xor	@r5+, r14			\n"
		"xor	@r6+, r14			\n"
		"xor	@r7+, r14			\n"
		"xor	@r8+, r14			\n"
		"xor	@r9+, r14			\n"
		"xor	@r10+, r14			\n"
		"xor	@r11+, r14			\n"
		"xor	@r12+, r14			\n"
		"xor	r14, 0(r15)			\n"		// memxor_block_core_loop[10,11]
		"incd	r15					\n"
		"jnz	1b					\n"		// memxor_block_core_loop[13]
		"nop						\n"
		"pop	r11					\n"
		"pop	r10					\n"
		"pop	r9					\n"
		"pop	r8					\n"
		"pop	r7					\n"
		"pop	r6					\n"
		"pop	r5					\n"
		"pop	r4					\n"
		"ret						\n"		// memxor_block_core_loop[23]
		:
		:
		: "cc", "memory"
	);
}

//**************************************************************************************************
//**************************************************************************************************
#if 0
void memxor_test()
{
	uint8_t		dest[8];
	uint8_t		src[8];
	int			i;

	for (i = 0; i < NUM_ELEMENTS(src); ++i)
		src[i] = i + 1;

	for (i = 0; i < NUM_ELEMENTS(dest); ++i)
		dest[i] = (i + 1) << 4;

	GPI_TRACE_MSG(1, "test full block size...");
	memxor(dest, src, sizeof(dest));
//	TRACE_DUMP(1, "result:", dest, sizeof(dest));

	GPI_TRACE_MSG(1, "test half block size...");
	memxor(dest, src, sizeof(dest) / 2);
//	TRACE_DUMP(1, "result:", dest, sizeof(dest));

	GPI_TRACE_MSG(1, "done");
}
#endif
//**************************************************************************************************
#if 0
void memxor_block_test()
{
	uint8_t		dest[16];
	uint8_t		src[MEMXOR_BLOCKSIZE][sizeof(dest)];
	void*		pp[MEMXOR_BLOCKSIZE];
	int			i, k;

	for (i = 0; i < NUM_ELEMENTS(pp); ++i)
		pp[i] = &src[i][0];

	for (i = 0; i < sizeof(src); ++i)
		((uint8_t*)src)[i] = i;

	for (k = 1; k <= MEMXOR_BLOCKSIZE; ++k)
	{
		for (i = 0; i < NUM_ELEMENTS(dest); ++i)
			dest[i] = i << 4;

		GPI_TRACE_MSG(1, "test full block size with %u sources ...", k);
		memxor_block(dest, pp, sizeof(dest), k);
//		TRACE_DUMP(1, "result:", dest, sizeof(dest));

		for (i = 0; i < NUM_ELEMENTS(dest); ++i)
			dest[i] = i << 4;

		GPI_TRACE_MSG(1, "test half block size with %u sources ...", k);
		memxor_block(dest, pp, sizeof(dest) / 2, k);
//		TRACE_DUMP(1, "result:", dest, sizeof(dest));
	}

	GPI_TRACE_MSG(1, "done");
}
#endif
//**************************************************************************************************
//**************************************************************************************************
