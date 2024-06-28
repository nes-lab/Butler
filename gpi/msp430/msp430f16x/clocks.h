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
 *	@file					gpi/msp430f16x/clocks.h
 *
 *	@brief					general-purpose slow, fast, and hybrid clock
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

#ifndef __GPI_MSP430F16x_CLOCKS_H__
#define __GPI_MSP430F16x_CLOCKS_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "gpi/tools.h"			// ASSERT_CT
#include "gpi/interrupts.h"		// gpi_int_lock()...
#include "gpi/olf.h"

#include <msp430.h>

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************

#ifndef GPI_SLOW_CLOCK_RATE
	#define	GPI_SLOW_CLOCK_RATE		(1ul << 15)
		// TODO: 1 << 14 would accelerate gpi_tick_hybrid() (shift by 8 instead of 7). On the other
		// hand, gpi_tick_slow_...() would need corresponding adaptations. Think about it.
#endif

#ifndef GPI_FAST_CLOCK_RATE
	#define GPI_FAST_CLOCK_RATE		(1ul << 22)
#endif

#ifndef GPI_HYBRID_CLOCK_RATE
	#define GPI_HYBRID_CLOCK_RATE	(GPI_FAST_CLOCK_RATE)
#endif

//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************

// remove int locks in hybrid clock functions
// Setting this define avoids some int locks and improves performance a bit. On the other hand,
// it makes (some of) the hybrid clock functions non-reentrant. Since this is dangerous if
// enabled without care, the option is disabled by default.
// #define GPI_MSP430_CLOCKS_HYBRID_FRAGILE	1

#define GPI_MSP430_MCLK_RATE		GPI_FAST_CLOCK_RATE

//#define GPI_MSP430_CLOCKS_SLOW_TIMER	TAR
//#define GPI_MSP430_CLOCKS_FAST_TIMER	TBR

//**************************************************************************************************
//***** Forward Class and Struct Declarations ******************************************************



//**************************************************************************************************
//***** Global Typedefs and Class Declarations *****************************************************

typedef uint16_t	Gpi_Slow_Tick_Native;			// machine word
typedef uint32_t	Gpi_Slow_Tick_Extended;			// for long intervals
typedef uint16_t	Gpi_Fast_Tick_Native;
typedef uint32_t	Gpi_Fast_Tick_Extended;
typedef uint32_t	Gpi_Hybrid_Tick;

typedef struct Gpi_Hybrid_Reference_tag
{
	Gpi_Hybrid_Tick			hybrid_tick;
	Gpi_Fast_Tick_Native	fast_capture;

} Gpi_Hybrid_Reference;

//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Prototypes of Global Functions *************************************************************

#ifdef __cplusplus
	extern "C" {
#endif

// DCO control functions
// DCO is specific for MSP430, hence the functions do not belong to the generic header file
static inline int16_t		gpi_msp430_dco_init(Gpi_Slow_Tick_Native timeout);
int16_t						gpi_msp430_dco_resync(Gpi_Slow_Tick_Native timeout);
static inline void			gpi_msp430_dco_realign();
static inline int16_t		gpi_msp430_dco_update();

// forward declarations (needed below)
Gpi_Slow_Tick_Extended		gpi_tick_slow_extended();
Gpi_Hybrid_Tick				gpi_tick_fast_to_hybrid(Gpi_Fast_Tick_Native fast_tick);
uint32_t 					gpi_tick_hybrid_to_us(Gpi_Hybrid_Tick ticks);

#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

static inline __attribute__((always_inline)) Gpi_Slow_Tick_Native gpi_tick_slow_native()
{
	// attention: TAR is asynchronous to MCLK -> read multiple times to be safe also near edges
	// (sl049f p. 11-4 note)
	//
	// locking interrupts:
	// + pro: runtime is deterministic
	// - con: slower, especially if function doesn't get inlined (because of its size)
	// not locking interrupts:
	// + pro: compact and fast
	// - con: number of loop iterations could be high under rare conditions
	//
	// The loop reiterates if a long enough ISR interrupts the two reads. This is not critical
	// as long as it does not happen continuously. The last would mean that the system is
	// permanently under extremely high CPU load (able to process only very few instructions
	// between IRQs). We don't think that this is a typical scenario, hence we don't catch it.
	// Nevertheless it is possible to do so by activating the int-locked version.

#if 0
	register uint16_t	a, b, c;
	register int ie = gpi_int_lock();
	a = TAR;
	b = TAR;
	c = TAR;
	gpi_int_unlock(ie);
	if (a != b)		// if edge near [read a ... read b]
		a = c;		// use c -> far enough away from edge
#else
	register uint16_t	a, b;
	do
	{
		a = TAR;
		b = TAR;
    }
	while (a != b);
#endif

	return a;
}

//**************************************************************************************************

static inline __attribute__((always_inline)) Gpi_Fast_Tick_Native gpi_tick_fast_native()
{
	// notice: direct access to TBR is possible only because its source clock is synchronous to MCLK
	// (sl049f p. 12-4 note)
	return TBR;
}

//**************************************************************************************************

static inline Gpi_Hybrid_Tick gpi_tick_hybrid()
{
	return gpi_tick_fast_to_hybrid(gpi_tick_fast_native());
}

//*************************************************************************************************

// note: always_inline is used to avoid suboptimal stack usage
// (which otherwise happens sometimes (msp430-gcc 4.6.3) causing unnecessary performance degradation)
static inline __attribute__((always_inline)) Gpi_Hybrid_Reference gpi_tick_hybrid_reference()
{
	ASSERT_CT(sizeof(Gpi_Slow_Tick_Native) == sizeof(uint16_t));
	ASSERT_CT(sizeof(Gpi_Fast_Tick_Native) == sizeof(uint16_t));
	ASSERT_CT(sizeof(Gpi_Slow_Tick_Extended) == sizeof(uint32_t));
	ASSERT_CT(sizeof(Gpi_Hybrid_Tick) == sizeof(uint32_t));

	register Generic32		t;
	register uint16_t		slow, cap;
	Gpi_Hybrid_Reference	r;

	// lock interrupts if requested
	// note: due to the active use of CCIFG, the code is not reentrant without ints locked
	#if (!GPI_MSP430_CLOCKS_HYBRID_FRAGILE)
		register int ie = gpi_int_lock();
	#endif

	// get edge ticks
	// ATTENTION: Compared to gpi_tick_slow_native() we read TAR only once, which is possible
	// because we detect a potential edge implicitly with CCIFG. For this way to be safe, it is
	// essential to read TAR at least 2 clock ticks before CCIFG because the CCIFG signal may
	// have a slight delay of 1 or 2 clock cycles caused by synchronization stages (see SCS
	// setting for instance).
	do
	{
		TBCCTL6 &= ~CCIFG;
		slow = TAR;
		cap  = TBCCR6;
    }
	while (TBCCTL6 & CCIFG);

	// unlock interrupts
	#if (!GPI_MSP430_CLOCKS_HYBRID_FRAGILE)
		gpi_int_unlock(ie);
	#endif

	// extent slow value
	// note: we don't do gpi_tick_slow_extended() within the loop directly because it is
	// expensive in comparison to the loop
	// ATTENTION: It must be guaranteed that the delay between the loop and the
	// gpi_tick_slow_extended() call is less than 0x10000 slow ticks. Since this is a large
	// amount of time in a typical configuration (in the range of seconds) and critical for
	// gpi_tick_slow_extended() anyway, we don't implement any locks here.
	t.u32 = gpi_tick_slow_extended();
	if (t.u16_l < slow)
		t.u16_h--;
	t.u16_l = slow;

	// convert edge ticks to hybrid timebase
	ASSERT_CT(IS_POWER_OF_2(GPI_HYBRID_CLOCK_RATE / GPI_SLOW_CLOCK_RATE),
		hybrid_slow_ratio_must_be_power_of_2);
	t.u32 = gpi_slu_32(t.u32, MSB(GPI_HYBRID_CLOCK_RATE / GPI_SLOW_CLOCK_RATE));

	r.hybrid_tick = t.u32;
	r.fast_capture = cap;

	return r;
}

//**************************************************************************************************

static inline uint32_t gpi_tick_fast_to_us(Gpi_Fast_Tick_Extended ticks)
{
	ASSERT_CT(sizeof(Gpi_Hybrid_Tick) == sizeof(Gpi_Fast_Tick_Extended));
	ASSERT_CT(GPI_FAST_CLOCK_RATE == GPI_HYBRID_CLOCK_RATE);

	return gpi_tick_hybrid_to_us(ticks);

	// could be extended like this, but take care with all the value ranges
	// return gpi_tick_hybrid_to_us(ticks / (GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE));
}

//**************************************************************************************************

// gpi_msp430_dco_sync() is an internal function, don't call it explicitly
int16_t gpi_msp430_dco_sync(int reinit);

static inline int16_t gpi_msp430_dco_init(Gpi_Slow_Tick_Native timeout)
{
	gpi_msp430_dco_sync(2);
	return gpi_msp430_dco_resync(timeout);
}

static inline void 		gpi_msp430_dco_realign()	{ gpi_msp430_dco_sync(1);			}
static inline int16_t	gpi_msp430_dco_update()		{ return gpi_msp430_dco_sync(0);	}

//**************************************************************************************************
//**************************************************************************************************

#endif // __GPI_MSP430F16x_CLOCKS_H__
