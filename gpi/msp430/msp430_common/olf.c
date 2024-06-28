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
 *	@file					gpi/msp430/olf.c
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
//***** Trace Settings *****************************************************************************



//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/olf.h"

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

#ifdef LT
	#error adapt macro name
#endif

#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n

const int8_t gpi_get_msb_lut[256] =
{
    -1,    0,     1,     1,     2,     2,     2,     2,
	3,     3,     3,     3,     3,     3,     3,     3,
	LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
	LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

#undef LT

//**************************************************************************************************

#define LT2(n)	n,      n+1,      n+1,      n+2
#define LT4(n)	LT2(n), LT2(n+1), LT2(n+1), LT2(n+2)
#define LT6(n)	LT4(n), LT4(n+1), LT4(n+1), LT4(n+2)

const uint8_t gpi_popcnt_lut[256] =
{
    LT6(0), LT6(1), LT6(1), LT6(2)
};

#undef LT2
#undef LT4
#undef LT6

//**************************************************************************************************

#define LT1(n) 	    n+0x00,      n+0x80,      n+0x40,      n+0xC0
#define LT2(n) 	LT1(n+0x00), LT1(n+0x20), LT1(n+0x10), LT1(n+0x30)
#define LT3(n) 	LT2(n+0x00), LT2(n+0x08), LT2(n+0x04), LT2(n+0x0C)
#define LT		LT3(  0x00), LT3(  0x02), LT3(  0x01), LT3(  0x03)

const uint8_t gpi_bitswap_lut[256] = { LT };

#undef LT1
#undef LT2
#undef LT3
#undef LT

//**************************************************************************************************

#define LT0(n)	(uint16_t)(((n) == 0) ? 0 : 0x10000 / (n))
#define LT1(n)	LT0(n), LT0(n+0x01), LT0(n+0x02), LT0(n+0x03)
#define LT2(n)  LT1(n), LT1(n+0x04), LT1(n+0x08), LT1(n+0x0C)
#define LT3(n)	LT2(n), LT2(n+0x10), LT2(n+0x20), LT2(n+0x30)

// contains 1 / d in 0.16 format
const uint16_t gpi_div_lut[256] =
{
	LT3(0), LT3(0x40), LT3(0x80), LT3(0xC0)
};

#undef LT0
#undef LT1
#undef LT2
#undef LT3

//**************************************************************************************************
//***** Local Functions ****************************************************************************



//**************************************************************************************************
//***** Global Functions ***************************************************************************



//**************************************************************************************************
//**************************************************************************************************
