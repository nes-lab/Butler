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
 *	@file					mixer/tmote/memxor.h
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

#ifndef __MEMXOR_MSP430_H__
#define __MEMXOR_MSP430_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "gpi/tools.h"

#include <assert.h>
#include <stdint.h>		// uintptr_t

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************

#define MEMXOR_BLOCKSIZE	10

//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************



//**************************************************************************************************
//***** Forward Class and Struct Declarations ******************************************************



//**************************************************************************************************
//***** Global Typedefs and Class Declarations *****************************************************



//**************************************************************************************************
//***** Global Variables ***************************************************************************

// internal data
extern uint16_t	memxor_core[];
extern uint16_t	memxor_block_core_loop[];

//**************************************************************************************************
//***** Prototypes of Global Functions *************************************************************

#ifdef __cplusplus
	extern "C" {
#endif

static inline void	memxor(void *dest, const void *src, unsigned int size);

// internal forward declarations
void memxor_block_core(unsigned int minus_size, /*const*/ void *src[], int skip);

#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

// ATTENTION: dest, src and size have to be aligned on word size
// note: inlining is important to resolve as much as possible tests at compile time
static inline void __attribute__((always_inline)) memxor(void *dest, const void *src, unsigned int size)
{
	if (0 == size)
		return;

	// align size
	size = ((size + 1) / 2) * 2;

	// adapt machine code to dest
	memxor_core[1] = ((uintptr_t)dest + size);

	// call core loop
	((void (*)(const void*, unsigned int))&memxor_core)(src, -size);
}

//**************************************************************************************************
// optimized memxor_block() implementation

// straight version for cases where size is small
// note: inlining saves some spill code
// note: the function seems large for inlining, but all instructions are single-word instructions,
// so it doesn't hurt as much as it looks. If code memory is short, the always_inline attribute
// can be removed.
static inline void __attribute__((always_inline)) memxor_block_straight(void *dest, /*const*/ void *src[], unsigned int size, int num_src)
{
	register int tmp;

	__asm__ volatile
	(
		"10:						\n"
		"mov	@%[pd]+, %[tmp]		\n"
		"add	%[ns], r0			\n"		// indirect jump into jump table
		"jmp	10f					\n"		// num_src = 0 is invalid
		"jmp	1f					\n"
		"jmp	2f					\n"
		"jmp	3f					\n"
		"jmp	4f					\n"
		"jmp	5f					\n"
		"jmp	6f					\n"
		"jmp	7f					\n"
		"jmp	8f					\n"
		"jmp	9f					\n"

		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"9:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"8:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"7:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"6:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"5:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"4:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"3:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"2:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"
		"1:							\n"
		"mov	@%[src]+, %[p]		\n"
		"add	%[i], %[p]			\n"
		"xor	@%[p], %[tmp]		\n"

		"mov	%[tmp], -2(%[pd])	\n"
		"sub	%[ns], %[src]		\n"		// reset src to its original value
		"incd	%[i]				\n"
		"cmp	%[pd], %[pd_end]	\n"
		"jnz	10b					\n"
		"10:						\n"

		: [pd] "+&r"(dest), [p] "=&r"(tmp), [tmp] "=&r"(tmp)
		: [src] "r"(src), [ns] "r"(num_src * 2), [i] "r"(0), [pd_end] "r"(dest + size)
		: "cc", "memory"
	);
}

//**************************************************************************************************

// ATTENTION: dest, src and size have to be aligned on word size
// note: inlining is important to resolve as much as possible tests at compile time
static inline void __attribute__((always_inline)) memxor_block(void *dest, /*const*/ void *src[], unsigned int size, int num_src)
{
	uint16_t	orig1, orig2, orig3;

	assert(num_src <= MEMXOR_BLOCKSIZE);

	if (0 == num_src || 0 == size)
		return;

	// align size
	size = ((size + 1) / 2) * 2;

	// if size - i.e. number of needed loop iterations - is small, the overhead for adapting
	// the machine code exceeds the savings. Therefore we use a more straight-forward version
	// in these situations which is slower per iteration but comes with appropriate less overhead.
	if ((size <= 4) || ((size <= 8) && (num_src < MEMXOR_BLOCKSIZE)))
	{
		memxor_block_straight(dest, src, size, num_src);
		return;
    }

	// adapt machine code to dest: set destination offset
	memxor_block_core_loop[11] = (uintptr_t)dest + size;

	// adapt machine code to num_src
	if (num_src < MEMXOR_BLOCKSIZE)
	{
		// inject load instruction at the right place
		orig1 = memxor_block_core_loop[MEMXOR_BLOCKSIZE - num_src];
		memxor_block_core_loop[MEMXOR_BLOCKSIZE - num_src] = memxor_block_core_loop[0];

		// shorten loop
		orig2 = memxor_block_core_loop[13];
		memxor_block_core_loop[13] = 0x2000 | (-(4 + num_src) & 0x03FF);

		// inject ret into pop sequence
		orig3 = memxor_block_core_loop[13 + num_src];
		memxor_block_core_loop[13 + num_src] = 0x4130;
	}

	// call core loop
	memxor_block_core(-size, src, (MEMXOR_BLOCKSIZE - num_src) * 4);

	// restore original machine code
	if (num_src < MEMXOR_BLOCKSIZE)
	{
		memxor_block_core_loop[MEMXOR_BLOCKSIZE - num_src] = orig1;
		memxor_block_core_loop[13] = orig2;
		memxor_block_core_loop[13 + num_src] = orig3;
    }
}

//**************************************************************************************************
//**************************************************************************************************

#endif // __MEMXOR_MSP430_H__
