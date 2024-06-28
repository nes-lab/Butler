/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2018 - 2019, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					gpi/msp430f16x/olf.h
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

#ifndef __GPI_MSP430F16x_OLF_H__
#define __GPI_MSP430F16x_OLF_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "../msp430_common/olf.h"

#include "gpi/olf_internal.h"
#include "gpi/interrupts.h"
#include "gpi/tools.h"

#include <msp430.h>

#include <stdint.h>

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************



//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************

// ATTENTION: influences interrupt latency
#ifndef GPI_MEMCPY_DMA_BLOCKSIZE
	#define GPI_MEMCPY_DMA_BLOCKSIZE	16
#endif

//**************************************************************************************************
//***** Forward Class and Struct Declarations ******************************************************



//**************************************************************************************************
//***** Global Typedefs and Class Declarations *****************************************************



//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Prototypes of Global Functions *************************************************************

#ifdef __cplusplus
	extern "C" {
#endif

// forward declarations needed below
void			gpi_memcpy_dma(void *dest, const void *src, size_t size);
void			gpi_memmove_dma(void *dest, const void *src, size_t size);


#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

// shift left unsigned 32 bit
static inline __attribute__((always_inline)) uint32_t gpi_slu_32(uint32_t x, unsigned int s)
{
	s &= 31;

	if (s >= 16)
	{
		x = x << 16;
		s -= 16;
    }

	if (s >= 8)
	{
		x = x << 8;
		s -= 8;
    }

	// shift/rotate unrolled, achieves speedup if number of bits to shift is constant
	switch (s)
	{
		case  7: x <<= 1;
		case  6: x <<= 1;
		case  5: x <<= 1;
		case  4: x <<= 1;
		case  3: x <<= 1;
		case  2: x <<= 1;
		case  1: x <<= 1;
    }

	return x;
}

//**************************************************************************************************

// shift left unsigned 16 bit
static inline __attribute__((always_inline)) uint16_t gpi_slu_16(uint16_t x, unsigned int s)
{
	s &= 15;

	if (s >= 8)
	{
		x = x << 8;
		s -= 8;
    }

	__asm__
	(
		"add	%1, r0	\n"		// indirect jump
		"rla	%0		\n"
		"rla	%0		\n"
		"rla	%0		\n"
		"rla	%0		\n"
		"rla	%0		\n"
		"rla	%0		\n"
		"rla	%0		\n"
		: "+r"(x)
		: "r"((7 - s) * 2)
		: "cc"
	);

	return x;

	// note: another implementation option is via the multiplier. Here, this would not be faster
	// due to the number of cycles needed to write the multiplier operand registers.
}

//**************************************************************************************************

static inline __attribute__((always_inline)) uint8_t gpi_slu_8(uint8_t x, unsigned int s)
{
	return gpi_slu_16(x, s);
}

_GPI_SIZE_DISPATCHER_FUNCTION_2_32(gpi_slu, uint32_t, unsigned int)

#define gpi_slu(x, s)	 ((typeof(x))_GPI_SIZE_DISPATCHER_2_32(gpi_slu, x, s))

//*************************************************************************************************
// mul using hardware multiplier
// TODO: hardware mul is device-specific -> distinguish

// mul unsigned 16 x 16 bit
static inline __attribute__((always_inline,pure)) uint32_t gpi_mulu_16x16(uint16_t a, uint16_t b)
{
	register uint32_t	result;
	register int		ie;

	// use barrier to keep int lock as short as possible
	COMPUTE_BARRIER(a);
	COMPUTE_BARRIER(b);

	ie = gpi_int_lock();

	__asm__
	(
	    "mov 	%[a], &0x0130	 	\n"
	    "mov 	%[b], &0x0138	 	\n"
	    "mov 	&0x013a, %A[res] 	\n"
	    "mov 	&0x013c, %B[res] 	\n"
	    : [res] "=r"(result)
	    : [a] "r"(a), [b] "r"(b)
	    :
	);

	gpi_int_unlock(ie);

	return result;
}

//*************************************************************************************************

// mul unsigned 32 x 16 bit
static inline __attribute__((pure)) uint32_t gpi_mulu_32x16(uint32_t a, uint16_t b)
{
	register uint32_t	result;
	register int		ie;

	// use barrier to keep int lock as short as possible
	COMPUTE_BARRIER(a);
	COMPUTE_BARRIER(b);

	ie = gpi_int_lock();

	__asm__
	(
		"mov	%[b], &0x0130		\n"
		"mov	%A[a], &0x0138		\n"
		"mov	&0x013a, %A[res]	\n"
		"mov	&0x013c, %B[res]	\n"
		"mov	%B[a], &0x0138		\n"
		"add	&0x013a, %B[res]	\n"
//		"adc	&0x013c, &0x013c	\n"		// can be read later if needed
	    : [res] "=&r"(result)
	    : [a] "r"(a), [b] "r"(b)
	    :
	);

	gpi_int_unlock(ie);

	return result;
}

//**************************************************************************************************

// the preferred concept for gpi_mulu() would be this:
// #define gpi_mulu(a,b)
//		!!((2 == sizeof(a)) && (2 == sizeof(b))) * gpi_mulu16x16((uint16_t)a, (uint16_t)b) +
//		!!((4 == sizeof(a)) && (2 == sizeof(b))) * gpi_mulu32x16((uint32_t)a, (uint16_t)b) + ...
// or alternatively this:
//		((2 == sizeof(a)) && (2 == sizeof(b))) ? gpi_mulu16x16((uint16_t)a, (uint16_t)b) : (
//		((4 == sizeof(a)) && (2 == sizeof(b))) ? gpi_mulu32x16((uint32_t)a, (uint16_t)b) : ( ...
//		)...)
// The consideration behind is that all but the single relevant call get optimized out due to
// constant propagation. Unfortunately, this does not happen for some reason (it seems that the
// _pure_ attribute is ignored, maybe because the functions are inline). To work around that problem,
// we use a more explicit distinction that leads to the desired effect. The disadvantage is the
// intermediate usage of explicit (fixed) types. Therefore we suggest to use this macro with care.
// See also comments on _GPI_SIZE_DISPATCHER_2_32().

#define gpi_mulu(a,b)	(								\
	gpi_mulu_(a, b,										\
	1 * !!((2 == sizeof(a)) && (2 == sizeof(b))) +		\
	2 * !!((4 == sizeof(a)) && (2 == sizeof(b))) +		\
	0) + ASSERT_CT_EVAL(								\
	((2 == sizeof(a)) && (2 == sizeof(b))) ||			\
	((4 == sizeof(a)) && (2 == sizeof(b))) ||			\
	0, gpi_mulu_unsupported_variant)	)

static inline __attribute__((always_inline)) uint32_t gpi_mulu_(uint32_t a, uint16_t b, int variant)
{
	switch (variant)
	{
		case 1:		return gpi_mulu_16x16(a, b);
		case 2: 	return gpi_mulu_32x16(a, b);
		default:	assert(0);
    }

	return 0;	// must not be reached
}

//**************************************************************************************************
//**************************************************************************************************

extern uint16_t	gpi_memcpy8_core[5];

// note: inlining is important to resolve as much tests as possible at compile time
static inline void __attribute__((always_inline)) gpi_memcpy_8(void* dest, const void* src, size_t size)
{
	if (0 == size)
		return;

	// adapt machine code to dest
	gpi_memcpy8_core[1] = ((uintptr_t)dest + size);

	// call core loop
	((void (*)(const void*, unsigned int))&gpi_memcpy8_core)(src, -size);
}

//**************************************************************************************************

#if 1
// ATTENTION: behaviour of this function is undefined if source or dest address is not aligned
// or if (size & 1). Use gpi_memcpy_dma() if you are unsure.
#else
// ATTENTION: behaviour of this function is undefined if source or dest address is not aligned
// or if size > 128K or if (size & 1). Besides this, this function doesn't account for
// GPI_MEMCPY_DMA_BLOCKSIZE. Use gpi_memcpy_dma() if you are unsure.
#endif
static inline void __attribute__((always_inline)) gpi_memcpy_dma_aligned(void *dest, const void *src, size_t size)
{
	size >>= 1;

	while (size)
	{
		register uint16_t block_size = (size > GPI_MEMCPY_DMA_BLOCKSIZE) ? GPI_MEMCPY_DMA_BLOCKSIZE : size;

		int ie = gpi_int_lock();

		DMACTL0 &= ~(0xF << MSB(DMA2TSEL0));
		DMA2SA = (uint16_t)src;
		DMA2DA = (uint16_t)dest;
		DMA2SZ = block_size;
		DMA2CTL = DMADT0 | DMADSTINCR1 | DMADSTINCR0 | DMASRCINCR1 | DMASRCINCR0 | DMAEN | DMAREQ;

		// note: CPU is blocked until transfer ends

		gpi_int_unlock(ie);

		dest = (int16_t*)dest + block_size;
		src  = (int16_t*)src + block_size;
		size -= block_size;
	}
}

//**************************************************************************************************

static inline void __attribute__((always_inline)) gpi_memcpy_dma_inline(void *dest, const void *src, size_t size)
{
	uint8_t			*pd = dest;
	const uint8_t	*ps = src;
	uint16_t		ctl;

	if (0 == size)
		return;

	if (size < 4)
	{
		pd += size;
		switch (size)
		{
			case 3:	 __asm__ volatile ("mov.b @%0+, -3(%1)" : "+&r"(ps) : "r"(pd) : "memory");	// don't break
			case 2:	 __asm__ volatile ("mov.b @%0+, -2(%1)" : "+&r"(ps) : "r"(pd) : "memory");	// don't break
			default: __asm__ volatile ("mov.b @%0,  -1(%1)" : : "r"(ps) , "r"(pd) : "memory");	// don't break
        }
//		memcpy(dest, src, size);
//		memcpy8(dest, src, size);
		return;
    }

	ASSERT_CT(GPI_MEMCPY_DMA_BLOCKSIZE <= 0x7FFF, GPI_MEMCPY_DMA_BLOCK_SIZE_too_big);

	// note: don't need to respect GPI_MEMCPY_DMA_BLOCKSIZE with global interrupts disabled
	if ((size > GPI_MEMCPY_DMA_BLOCKSIZE) && (__read_status_register() & GIE))
	{
		gpi_memcpy_dma(dest, src, size);
		return;
    }

	ctl = DMADT0 | DMADSTINCR1 | DMADSTINCR0 | DMASRCINCR1 | DMASRCINCR0 | DMAEN | DMAREQ;

	if ((((uint16_t)pd & 1) != ((uint16_t)ps & 1)) || (size < 3))
	{
		ctl |= DMADSTBYTE | DMASRCBYTE;
		size <<= 1;
    }
	else
	{
		if ((uint16_t)pd & 1)
		{
			*pd++ = *ps++;
			size--;
		}
	}

	int ie = gpi_int_lock();

	DMACTL0 &= ~(0xF << MSB(DMA2TSEL0));
	DMA2SA = (uint16_t)ps;
	DMA2DA = (uint16_t)pd;
	DMA2SZ = size >> 1;
	DMA2CTL = ctl;

	// note: CPU is blocked until transfer ends

	gpi_int_unlock(ie);

	if (size & 1)
		pd[size - 1] = ps[size - 1];
}

//**************************************************************************************************

static inline void __attribute__((always_inline)) gpi_memmove_dma_inline(void *dest, const void *src, size_t size)
{
	uint8_t			*pd = dest;
	const uint8_t	*ps = src;
	uint16_t		ctl;

	if (0 == size)
		return;

	ASSERT_CT(GPI_MEMCPY_DMA_BLOCKSIZE <= 0x7FFF, GPI_MEMCPY_DMA_BLOCK_SIZE_too_big);

	// note: don't need to respect GPI_MEMCPY_DMA_BLOCKSIZE with global interrupts disabled
	if ((size > GPI_MEMCPY_DMA_BLOCKSIZE) && (__read_status_register() & GIE))
	{
		gpi_memmove_dma(dest, src, size);
		return;
    }

	if (pd <= ps)
	{
		gpi_memcpy_dma_inline(dest, src, size);
		return;
    }

	ctl = DMADT0 | DMADSTINCR1 | DMASRCINCR1 | DMAEN | DMAREQ;

	if ((((uint16_t)pd & 1) != ((uint16_t)ps & 1)) || (size < 3))
	{
		ctl |= DMADSTBYTE | DMASRCBYTE;
		size <<= 1;
    }
	else
	{
		if ((uint16_t)&pd[size - 1] & 1)
		{
			size--;
			pd[size] = ps[size];
		}
	}

	int ie = gpi_int_lock();

	DMACTL0 &= ~(0xF << MSB(DMA2TSEL0));
	DMA2SA = (uint16_t)&ps[size - 1];
	DMA2DA = (uint16_t)&pd[size - 1];
	DMA2SZ = size >> 1;
	DMA2CTL = ctl;

	// note: CPU is blocked until transfer ends

	gpi_int_unlock(ie);

	if (size & 1)
		*pd = *ps;
}

//**************************************************************************************************
//**************************************************************************************************

#endif // __GPI_MSP430F16x_OLF_H__
