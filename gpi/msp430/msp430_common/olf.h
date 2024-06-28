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
 *	@file					gpi/msp430/olf.h
 *
 *	@brief					optimized low-level functions, tuned for MSP430
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

#ifndef __GPI_MSP430_OLF_H__
#define __GPI_MSP430_OLF_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "gpi/tools.h"
#include "gpi/olf_internal.h"

#include <stdint.h>

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************



//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************



//**************************************************************************************************
//***** Forward Class and Struct Declarations ******************************************************



//**************************************************************************************************
//***** Global Typedefs and Class Declarations *****************************************************



//**************************************************************************************************
//***** Global Variables ***************************************************************************

extern const int8_t 	gpi_get_msb_lut[256];
extern const uint8_t	gpi_popcnt_lut[256];
extern const uint8_t	gpi_bitswap_lut[256];
extern const uint16_t	gpi_div_lut[256];

//**************************************************************************************************
//***** Prototypes of Global Functions *************************************************************

#ifdef __cplusplus
	extern "C" {
#endif

// forward declarations needed below
static uint32_t 		gpi_mulu_16x16(uint16_t a, uint16_t b);

#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

static inline int_fast8_t __attribute__((always_inline)) gpi_get_msb_8(uint8_t x)
{
	return gpi_get_msb_lut[x];
}

static inline int_fast8_t __attribute__((always_inline)) gpi_get_msb_16(uint16_t x)
{
	return (x >= 0x100) ? 8 + gpi_get_msb_8(x >> 8) : gpi_get_msb_8(x);
}

static inline int_fast8_t __attribute__((always_inline)) gpi_get_msb_32(uint32_t x)
{
	return (x >= 0x10000 ? 16 : 0) + gpi_get_msb_16(x >= 0x10000 ? x >> 16 : x);
//	return (x >= 0x10000) ? 16 + gpi_get_msb_16(x >> 16) : gpi_get_msb_16(x);
}

_GPI_SIZE_DISPATCHER_FUNCTION_1_32(gpi_get_msb, int_fast8_t)

#define gpi_get_msb(a)	 _GPI_SIZE_DISPATCHER_1_32(gpi_get_msb, a)

//**************************************************************************************************

static inline int_fast8_t __attribute__((always_inline)) gpi_get_lsb_8(uint8_t x)
{
	return gpi_get_msb_8(x & -x);
}

static inline int_fast8_t __attribute__((always_inline)) gpi_get_lsb_16(uint16_t x)
{
	return gpi_get_msb_16(x & -x);
}

static inline int_fast8_t __attribute__((always_inline)) gpi_get_lsb_32(uint32_t x)
{
	register uint16_t		y = x;
	register int_fast8_t	a = 0;
	
	if (!y)
	{
		y = x >> 16;
		
		if (!y)
			return -1;
			
		a = 16;
	}

	return a + gpi_get_lsb_16(y);
}

_GPI_SIZE_DISPATCHER_FUNCTION_1_32(gpi_get_lsb, int_fast8_t)

#define gpi_get_lsb(a)	 _GPI_SIZE_DISPATCHER_1_32(gpi_get_lsb, a)

//**************************************************************************************************

static inline uint_fast8_t __attribute__((always_inline)) gpi_popcnt_8(uint8_t i)
{
	return gpi_popcnt_lut[i];
}

static inline uint_fast8_t __attribute__((always_inline)) gpi_popcnt_16(uint16_t i)
{
	return gpi_popcnt_lut[i & 0xFF] + gpi_popcnt_lut[i >> 8];
}

_GPI_SIZE_DISPATCHER_FUNCTION_1_16(gpi_popcnt, uint_fast8_t)

#define gpi_popcnt(a)	 _GPI_SIZE_DISPATCHER_1_16(gpi_popcnt, a)

//**************************************************************************************************

static inline uint8_t __attribute__((always_inline)) gpi_bitswap_8(uint8_t i)
{
	return gpi_bitswap_lut[i];
}

static inline uint16_t __attribute__((always_inline)) gpi_bitswap_16(uint16_t i)
{
	return ((uint16_t)(gpi_bitswap_lut[i & 0xFF]) << 8) | gpi_bitswap_lut[i >> 8];
}

_GPI_SIZE_DISPATCHER_FUNCTION_1_16(gpi_bitswap, uint_fast8_t)

#define gpi_bitswap(a)	 _GPI_SIZE_DISPATCHER_1_16(gpi_bitswap, a)

//**************************************************************************************************

static inline uint16_t __attribute__((always_inline)) gpi_divu_16x8(uint16_t x, uint8_t d, int accurate)
{
	if (d < 2)		// ATTENTION: incl. division by zero
		return x;

	uint16_t y = gpi_mulu_16x16(x, gpi_div_lut[d]) >> 16;

	if (accurate)
		for (x -= gpi_mulu_16x16(y, d); x >= d; x -= d, y++);

	return y;
}

//**************************************************************************************************
//**************************************************************************************************

#endif // __GPI_MSP430_OLF_H__
