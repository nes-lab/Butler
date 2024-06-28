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
 *	@file					gpi/msp430f16x/clocks.c
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
//***** Trace Settings *****************************************************************************
/*
#include <gpi/trace.h>

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
// define groups appropriate for your needs, assign one bit per group
// values > GPI_TRACE_LOG_USER (i.e. upper 8 bits) are reserved
#define TRACE_GROUP1		0x00000001
#define TRACE_GROUP2		0x00000002

// select active message groups, i.e., the messages to be printed (others will be dropped)
GPI_TRACE_CONFIG(<TODO: module name>, TRACE_BASE_SELECTION |  GPI_TRACE_LOG_USER);
*/
//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/platform_spec.h"
#include "gpi/tools.h"
#include "gpi/clocks.h"
#include "gpi/interrupts.h"
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



//**************************************************************************************************
//***** Local Functions ****************************************************************************

// updating BCSCTL1 needs special handling according to errata issue BCL5 (slaz146f p. 3)
static void BCSCTL1_set(uint8_t value)
{
	if (BCSCTL2 & DIVM1)
		BCSCTL1 = value;

	else
	{
		int ie = gpi_int_lock();

		BCSCTL2 |= DIVM1;
		BCSCTL1 = value;
		BCSCTL2 &= ~DIVM1;

		gpi_int_unlock(ie);
	}
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

Gpi_Slow_Tick_Extended gpi_tick_slow_extended()
{
	ASSERT_CT(sizeof(Gpi_Slow_Tick_Native) == sizeof(uint16_t));
	ASSERT_CT(sizeof(Gpi_Slow_Tick_Extended) == sizeof(uint32_t));

	static struct
	{
		uint16_t		high;
		uint16_t		last;
	} s	=
	{0, 0};

	register Generic32	o;

	// TODO: check how the function is used and decide if we can remove the int-lock or
	// provide and unlocked version (using the same static variables)
	// ATTENTION: int-lock makes the function reentrant; it is not without the int-lock.
	// Maybe it is possible to use a marker to avoid nested updates, e.g. the LSB of s.last.

	int ie = gpi_int_lock();

	o.u16_l = gpi_tick_slow_native();

	// extend format
	// ATTENTION: function has to be called periodically at least once per 0xFFFF ticks,
	// otherwise it will loose ticks in high part
	if (o.u16_l < s.last)
		s.high++;
	s.last = o.u16_l;

	o.u16_h = s.high;

	gpi_int_unlock(ie);

	return o.u32;
}

//**************************************************************************************************

Gpi_Fast_Tick_Extended gpi_tick_fast_extended()
{
	ASSERT_CT(sizeof(Gpi_Fast_Tick_Native) == sizeof(uint16_t));
	ASSERT_CT(sizeof(Gpi_Fast_Tick_Extended) == sizeof(uint32_t));

	static struct
	{
		uint16_t		high;
		uint16_t		last;
	} s	=
	{0, 0};

	register Generic32	o;

	// TODO: check how the function is used and decide if we can remove the int-lock or
	// provide and unlocked version (using the same static variables)
	// ATTENTION: int-lock makes the function reentrant; it is not without the int-lock.
	// Maybe it is possible to use a marker to avoid nested updates, e.g. the LSB of s.last.

	int ie = gpi_int_lock();

	o.u16_l = gpi_tick_fast_native();

	// extend format
	// ATTENTION: function has to be called periodically at least once per 0xF...F ticks,
	// otherwise it will loose ticks in high part
	// To catch such situations, we could additionally compare the slow ticks. This would
	// decrease the probability of missed overruns (significantly).
	if (o.u16_l < s.last)
		s.high++;
	s.last = o.u16_l;

	o.u16_h = s.high;

	gpi_int_unlock(ie);

	return o.u32;
}

//**************************************************************************************************

// convert a fast tick value from the near past (e.g. timer capture) to hybrid ticks
// ATTENTION: the call has to follow within <= 0xF...F fast ticks (max. Gpi_Fast_Tick_Native),
// otherwise the result will be wrong
Gpi_Hybrid_Tick gpi_tick_fast_to_hybrid(Gpi_Fast_Tick_Native fast_tick)
{
	register Gpi_Hybrid_Reference	r;
	register Gpi_Fast_Tick_Native	delta;

	// get last edge ticks
	// ATTENTION: this must happen <= 0xF...F fast ticks after the interesting point in time
	r = gpi_tick_hybrid_reference();

	ASSERT_CT(GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE < 0x10000, fast_slow_ratio_to_high);
	ASSERT_CT(IS_POWER_OF_2(GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE), fast_hybrid_ratio_must_be_power_of_2);
	ASSERT_CT(IS_POWER_OF_2(GPI_HYBRID_CLOCK_RATE / GPI_SLOW_CLOCK_RATE), hybrid_slow_ratio_must_be_power_of_2);

	// compute delta between edge capture and past value
	// attention: we expect that fast_tick is before edge tick in the typical case. But the edge is
	// from the (near) past, so it is also possible that fast_tick stems from a period < 1 slow
	// clock cycles after the edge. In this case, -delta < GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE
	// (+ some tolerance if fast clock is asynchronous to slow clock, e.g. DCO vs. XO). Hence we
	// split the interpretation of delta at this value.
	delta = r.fast_capture - fast_tick;

	// compute hybrid tick:
	// if fast_tick is behind edge for sure: sub delta with respect to datatypes
	// ATTENTION: we add a safety margin that compensates for clock drift (i.e. delta may be
	// > GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE even if fast_tick is between edge and next edge)
	if (-delta < ((GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE) * 103) / 100)
		r.hybrid_tick += -delta / (GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE);

	// else sub delta in standard way
	else
		r.hybrid_tick -= delta / (GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE);

	return r.hybrid_tick;
}

//**************************************************************************************************

// convert timestamp from hybrid ticks to microseconds
// note: providing an efficient implementation of this functionality is advantageous
// for debug messages and profiling purposes
uint32_t gpi_tick_hybrid_to_us(Gpi_Hybrid_Tick ticks)
{
	ASSERT_CT(sizeof(Gpi_Hybrid_Tick) == sizeof(uint32_t));
	ASSERT_CT(IS_POWER_OF_2(GPI_HYBRID_CLOCK_RATE), GPI_HYBRID_CLOCK_RATE_must_be_power_of_2);
	ASSERT_CT(GPI_HYBRID_CLOCK_RATE <= (1ul << 22), program_needs_adapted_shifts);
	ASSERT_CT(GPI_HYBRID_CLOCK_RATE >= (1ul << 20), program_needs_mul32x32_or_adapted_shifts);

	// note: 1000000 = 15625 * 64

	// choose MUL such that result must be right shifted by 16
	// examples:
	// if GPI_HYBRID_CLOCK_RATE == 2^22: mul by 15625, then div by 2^22 / 64 = 65536
	// if GPI_HYBRID_CLOCK_RATE == 2^20: mul by 15625 * 4, then div by (2^20 / 64) * 4 = 65536
	// if GPI_HYBRID_CLOCK_RATE == 2^16: mul by 15625 * 64 = 1000000, then div by (2^16 / 64) * 64 = 65536
	const uint16_t MUL = 15625 * ((1ul << 22) / GPI_HYBRID_CLOCK_RATE);

	// check if hardware mul is available in case porting to another device family member is considered
	ASSERT_CT(GPI_ARCH_IS_DEVICE(MSP430F16x));

	register int	ie;

	ie = gpi_int_lock();

	MPY = MUL;

	__asm__ volatile
	(
//		"mov	%[b], &0x0130		\n"
		"mov	%A[a], &0x0138		\n"
		"mov	&0x013c, %A[a]		\n"
		"mov	%B[a], &0x0138		\n"
		"add	&0x013a, %A[a]		\n"
		"mov	&0x013c, %B[a]		\n"
		"adc	%B[a]				\n"
	    : [a] "+r"(ticks)
	    : //[b] "r"(MUL)
	    : "cc"
	);

	gpi_int_unlock(ie);

	return ticks;
}

//**************************************************************************************************

// convert timestamp from slow ticks to microseconds
uint32_t gpi_tick_slow_to_us(Gpi_Slow_Tick_Extended ticks)
{
	ASSERT_CT(sizeof(Gpi_Slow_Tick_Extended) == sizeof(uint32_t));
	ASSERT_CT(GPI_SLOW_CLOCK_RATE >= 64, GPI_SLOW_CLOCK_RATE_too_small);
	ASSERT_CT(IS_POWER_OF_2(GPI_SLOW_CLOCK_RATE), GPI_SLOW_CLOCK_RATE_must_be_power_of_2);
		// to make sure that div gets replaced by shift

	// result = ticks * 1000000 / GPI_SLOW_CLOCK_RATE
	// note: 1000000 = 15625 * 64 ->
	// result = ticks * 15625 * 64 / GPI_SLOW_CLOCK_RATE = ticks * 15625 / (GPI_SLOW_CLOCK_RATE / 64)

	assert(ticks <= -1ul / 15625);

	ticks = gpi_mulu_32x16(ticks, 15625);
	return ticks / (GPI_SLOW_CLOCK_RATE / 64);
}

//*************************************************************************************************

void gpi_milli_sleep(uint16_t ms)
{
	ASSERT_CT(sizeof(Gpi_Slow_Tick_Extended) == sizeof(uint32_t));
	ASSERT_CT(GPI_SLOW_CLOCK_RATE < 0x10000, clock_to_fast);
	ASSERT_CT(GPI_SLOW_CLOCK_RATE >= 1000, clock_to_slow);

	uint32_t	ticks, start;

	start = gpi_tick_slow_extended();

	// note: the if condition is removed at compile time, i.e. it is like #if ... #else ... #endif
	if (IS_POWER_OF_2(GPI_SLOW_CLOCK_RATE))
		ticks = gpi_slu_32(ms, MSB(GPI_SLOW_CLOCK_RATE));
    else
		ticks = gpi_mulu_16x16(ms, GPI_SLOW_CLOCK_RATE);

	ticks /= 1000u;

	// round off to make sure that function sleeps *at least* ms (important if called with
	// ms = some max. datasheet value)
	// ATTENTION: we have to compensate not only rounding but also that start may be captured
	// just before an edge
	ticks += 2;

	while (gpi_tick_slow_extended() - start < ticks);
}

//*************************************************************************************************

// ATTENTION: gpi_micro_sleep() has to be fast
void gpi_micro_sleep(uint16_t us)
{
	ASSERT_CT(sizeof(Gpi_Fast_Tick_Native) == sizeof(uint16_t));

	register uint16_t	ticks, start;

	start = gpi_tick_fast_native();

	if (us > 10000)
	{
		gpi_milli_sleep((us + 999) / 1000);
		return;
    }

	// note: 1000000 = 15625 * 64
	// -> ticks = us * rate / 1000000 = us * rate / (15625 * 64) = us * (rate / 15625) / 64
	// choose MUL such that result must be right shifted by 16:
	// ticks = us * (rate * 1024 / 15625) / (64 * 1024) = us * (rate * 1024 / 15625) / 65536
	// by rounding up, we compensate for the (typically negligible) fractional part
	const uint32_t MUL = (GPI_FAST_CLOCK_RATE * 1024ull + 15624) / 15625;

	// gpi_micro_sleep() supports up to min(10000, 65535 / (GPI_FAST_CLOCK_RATE / 1000000)) us
	// (use gpi_milli_sleep for longer intervals)
	ASSERT_CT(10000 * (GPI_FAST_CLOCK_RATE / 15625 + 1) / 64 < 0x10000, ticks_must_be_changed_to_32_bit);

	// check if hardware mul is available in case porting to another device family member is considered
	ASSERT_CT(GPI_ARCH_IS_DEVICE(MSP430F16x));

	register int ie = gpi_int_lock();
	__asm__ volatile
	(
		"mov	%1, &__MPY			\n"
		"mov	%A2, &__OP2			\n"
		"mov	&__RESHI, %A0		\n"
		"mov	%B2, &__OP2			\n"
		"add	&__RESLO, %A0		\n"
//		"mov	&__RESHI, %B0		\n"
//		"adc	%B0					\n"
	    : "=r"(ticks)
	    : "r"(us), "i"(MUL)
	    : "cc"
	);
	gpi_int_unlock(ie);

	// round up
	ticks++;

	// subtract minimum overhead: 5 (mov arg + call) + 3 (start=get) + 7 (get ... ret) = 15
	if (ticks <= 15 / (GPI_MSP430_MCLK_RATE / GPI_FAST_CLOCK_RATE))
		return;
	ticks -= 15 / (GPI_MSP430_MCLK_RATE / GPI_FAST_CLOCK_RATE);

	while (gpi_tick_fast_native() - start < ticks);
}

//**************************************************************************************************

// main DCO control function
int16_t gpi_msp430_dco_sync(int reinit)
{
	const unsigned int	RSELmask = RSEL2 | RSEL1 | RSEL0;
	const unsigned int	DCOmask  = DCO2 | DCO1 | DCO0;
	//const unsigned int	MODmask  = MOD4 | MOD3 | MOD2 | MOD1 | MOD0;

	ASSERT_CT(sizeof(Gpi_Fast_Tick_Extended) == sizeof(uint32_t));

	static uint32_t		s_fast_tick_last = 0;
	static uint16_t		s_slow_tick_last = 0;
	static int16_t		s_deviation 	 = -0x7FFF;
	// notice: we don't use -0x8000 as return value since it would cause trouble if the caller
	// does something like this: if (x < 0) x = -x;

	Generic32			fast_tick;
	uint16_t			slow_tick, temp;

	// if full reinit: set DCO to a safe but reasonable fast speed to ensure that the get-tick-loop
	// is able to work (see comment below). see device-specific datasheet for effective values
	// (slas368g p. 37)
	if (reinit > 1)
	{
		BCSCTL1_set((BCSCTL1 & RSELmask) | RSEL2 | RSEL1);
		DCOCTL   = DCO2;
    }

	// get last edge ticks
	{
		// int lock is needed to make sure that gpi_tick_fast_extended() is close to the capture value,
		// more precisely that the tick difference is less than 0x10000. If application ensures
		// that this routine can't get interrupted for longer than this value, the int lock could
		// be removed.
		int ie = gpi_int_lock();

		// ATTENTION: the loop is time critical because MCLK (= DCOCLK) may run slow (until the
		// regulation has locked). If the number of cycles needed by the loop body is greater than
		// DCO / ACLK rate ratio, it will end up in an endless loop. Therefore we don't use
		// gpi_tick_slow_native() to get the optimal performance. If there are still problems
		// because of a low DCO / ACLK rate ratio, it might be an option to increase the clock
		// divider for ACLK (e.g. through the init phase).
		// ATTENTION: Compared to gpi_tick_slow_native(), we read TAR only once which is possible
		// because we detect a potential edge implicitly with CCIFG. For this way to be safe, it
		// is essential to read TAR at least 2 clock ticks before CCIFG because the CCIFG signal
		// may have a slight delay of 1 or 2 clock cycles caused by synchronization stages (see
		// SCS setting for instance).
		do
		{
			TBCCTL6 &= ~CCIFG;
			slow_tick = TAR;
			temp = TBCCR6;
		}
		while (TBCCTL6 & CCIFG);

		fast_tick.u32 = gpi_tick_fast_extended();

		gpi_int_unlock(ie);

		// assemble full fast tick value
		if (fast_tick.u16_l < temp)
			fast_tick.u16_h--;
		fast_tick.u16_l = temp;
	}

	// compute distance to last sync call
	fast_tick.u32 -= s_fast_tick_last;
	slow_tick	  -= s_slow_tick_last;

	// don't do anything if it doesn't have a minimum distance (in this case,
	// the granularity of the deviation value would be not fine enough for good regulation)
	if ((slow_tick < 16) && !reinit)
		return s_deviation;

	// store tick values for next call
	s_fast_tick_last += fast_tick.u32;
	s_slow_tick_last += slow_tick;

	// in case of reinit: store the tick values only
	if (reinit)
	{
		s_deviation = -0x7FFF;
		return s_deviation;
    }

	// compute deviation
	// note: the if condition is removed at compile time, i.e. it is like #if ... #else ... #endif
	ASSERT_CT(GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE < 0x10000, clock_ratio_to_high);
	if (IS_POWER_OF_2(GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE))
		fast_tick.u32 -= gpi_slu_32(slow_tick, MSB(GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE));
	else
		fast_tick.u32 -= gpi_mulu_16x16(slow_tick, GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE);

	// translate to signed16 with saturation
	s_deviation = fast_tick.s16_l;
	if (fast_tick.s32 < -0x0800)
		s_deviation = -0x0800;
	else if (fast_tick.s32 > 0x07FF)
		s_deviation = 0x07FF;

	// compute deviation per slow tick
	// notice: cast of slow_tick to signed is needed to generate the right division implementation
	s_deviation = (s_deviation << 4) / (int16_t)slow_tick;

	// choose control action dependent on deviation
	// ATTENTION: the time when this happens depends on how long this routine can be interrupted by
	// higher priority tasks (e.g. ISRs). Since the DCO is assumed to be quite stable (if we expect
	// VCC stable excl. transients, then the main influence is temperature which changes relatively
	// slowly), we don't expect problems if the control action is delayed a bit. Nevertheless it is
	// important to understand that such delays influence the control loop behaviour. This would
	// become more critical if the loop would use phase locking (i.e. cumulative deviation). So if
	// this function is called very errative or if it could be interrupted for long times, then it
	// might be worth to change this.
	{
		ASSERT_CT(GPI_FAST_CLOCK_RATE / GPI_SLOW_CLOCK_RATE < (10ul * 0x1000ul), clock_ratio_too_high);
		ASSERT_CT(GPI_FAST_CLOCK_RATE < 0x08000000ul, GPI_FAST_CLOCK_RATE_too_high);

		// important DCO specification (slas368g p. 37): S_DCO <= 1.16
		// -> if deviation > 0.16 / 2 = 8%, then switch to next DCO will decrease deviation
		// -> if deviation > 0.16 / 32 / 2 = 0.25%, then switch to next MOD will decrease deviation
		// if the first limit is exceeded, we directly update DCO. This gives faster tuning after
		// large changes and especially on initialization. A little margin on the comparison value
		// avoids unnecessary DCO changes (i.e. we implement a little hysteresis).
		// the second limit controls fine tuning whith updates to MOD (and DCO at the corner cases).
		// Here, we set the comparison value a bit below the limit since it is derived from an
		// unprobable extreme case. We expect less deviation also by switching at a little lower
		// value with high probability. Hence, the mean accuracy becomes a little bit better and
		// the error is expected to be < 0.25%.

		const int16_t	ppm85000 = (((16ul * GPI_FAST_CLOCK_RATE) / GPI_SLOW_CLOCK_RATE) * 85ul) / 1000ul;
		const int16_t	ppm2300  = (((16ul * GPI_FAST_CLOCK_RATE) / GPI_SLOW_CLOCK_RATE) * 23ul) / 10000ul;

		temp = DCOCTL;

		if (s_deviation > ppm85000)
		{
			if (temp & DCOmask)
				temp -= DCO0;
			else if (temp)
				temp = 0;
			else if (BCSCTL1 & RSELmask)
			{
				BCSCTL1_set(BCSCTL1 - RSEL0);
				temp = DCO2;
			}
		}

		else if (s_deviation > ppm2300)
		{
			if (temp)
				temp--;
			else if (BCSCTL1 & RSELmask)
			{
				BCSCTL1_set(BCSCTL1 - RSEL0);
				temp = DCO2;
			}
		}

		else if (s_deviation < -ppm2300)
		{
			if (temp < DCOmask)
				temp++;
			else if ((BCSCTL1 + RSEL0) & RSELmask)
			{
				BCSCTL1_set(BCSCTL1 + RSEL0);
				temp = DCO2;
			}
		}

		else if (s_deviation < -ppm85000)
		{
			if (temp < DCOmask)
				temp += DCO0;
			else if ((BCSCTL1 + RSEL0) & RSELmask)
			{
				BCSCTL1_set(BCSCTL1 + RSEL0);
				temp = DCO2;
			}
		}

		DCOCTL = temp;
	}

	return s_deviation;
}

//**************************************************************************************************

// start or realign DCO synchronization
int16_t gpi_msp430_dco_resync(Gpi_Slow_Tick_Native timeout)
{
	const int16_t			ppm2500 = ((16ul * GPI_FAST_CLOCK_RATE) / GPI_SLOW_CLOCK_RATE) / 400ul;
	Gpi_Slow_Tick_Native	now;

	gpi_msp430_dco_realign();

	for (now = gpi_tick_slow_native(); gpi_tick_slow_native() - now <= timeout;)
	{
		int16_t d = gpi_msp430_dco_update();
		if (d < 0)
			d = -d;
		if (d <= ppm2500)
			break;
    }

	return gpi_msp430_dco_update();
}

//**************************************************************************************************
//**************************************************************************************************
