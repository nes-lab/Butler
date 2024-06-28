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
 *	@file					tmote/mixer_transport.c
 *
 *	@brief					Mixer transport layer for Tmote Sky (MSP430 + CC2420)
 *
 *	@version				$Id$
 *	@date					TODO
 *
 *	@author					Carsten Herrmann
 *							Fabian Mager
 *
 ***************************************************************************************************

 	@details

	TODO

 **************************************************************************************************/
//***** Trace Settings *****************************************************************************

#include "gpi/trace.h"

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
#define TRACE_INFO			GPI_TRACE_MSG_TYPE_INFO
#define TRACE_WARNING		GPI_TRACE_MSG_TYPE_WARNING
#define TRACE_VERBOSE		GPI_TRACE_MSG_TYPE_VERBOSE

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION	GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(mixer_transport, GPI_TRACE_BASE_SELECTION | GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "../mixer_internal.h"

#include "gpi/tools.h"
#include "gpi/platform.h"
#include "gpi/clocks.h"
#include GPI_PLATFORM_PATH(radio.h)

#include <msp430.h>

#include <stdio.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************

#if MX_VERBOSE_PROFILE
	GPI_PROFILE_SETUP("mixer_transport.c", 1600, 4);
#endif

#define PROFILE_ISR(...)	PROFILE_P(0, ## __VA_ARGS__)

//**************************************************************************************************

// CPU clock cycles to hybrid, used only in timing parameters below
#define CTH(x)					((x) / (GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE))

ASSERT_CT_STATIC(IS_POWER_OF_2(FAST_HYBRID_RATIO), unefficient_FAST_HYBRID_RATIO);
ASSERT_CT_STATIC(IS_POWER_OF_2(HYBRID_SLOW_RATIO), unefficient_HYBRID_SLOW_RATIO);
ASSERT_CT_STATIC(GPI_MSP430_MCLK_RATE == GPI_FAST_CLOCK_RATE);

// timing parameters

// NOTE: a drift tolerance of 300 ppm (150 ppm on each side) should be a comfortable choice
// (typical clock crystals have < 20...50 ppm at 25�C and temperature coefficient < 0.04 ppm/K)
// NOTE: MIN(2500, ...) is effective in case of very long slots (up to seconds). It reduces the
// tolerance to avoid that RX_WINDOW overflows too fast (or even immediately).
#define DRIFT_TOLERANCE			MIN(2500, MAX((MX_SLOT_LENGTH + 999) / 1000, 1))	// +/- 1000 ppm

#define MAX_PROPAGATION_DELAY	GPI_TICK_US_TO_HYBRID(2)

#define PACKET_AIR_TIME			GPI_TICK_US_TO_HYBRID2((6 + PHY_PAYLOAD_SIZE + 2) * 32)
#define GRID_TO_SFD_OFFSET		GPI_TICK_US_TO_HYBRID(5 * 32 + 3)				// preamble + SFD + SFD signaling latency (due to bandwidth limitations, see CC2420 datasheet)
#define RX_TO_GRID_OFFSET		(CTH(20 + 16) + GPI_TICK_US_TO_HYBRID(6 * 32))	// software latency + SPI transmission time (SRXON) + RX_CALIBRATE period
#define TX_TO_GRID_OFFSET		(CTH(20 + 16) + GPI_TICK_US_TO_HYBRID(6 * 32))	// software latency + SPI transmission time (STXON) + TX_CALIBRATE period

#define RX_WINDOW_INCREMENT		(2 * DRIFT_TOLERANCE)			// times 2 is important to widen the window in next slot (times 1 would follow only)
#define RX_WINDOW_MAX			MIN(0x7FFF, MIN(15 * RX_WINDOW_INCREMENT, (MX_SLOT_LENGTH - PACKET_AIR_TIME - RX_TO_GRID_OFFSET) / 2))
#define RX_WINDOW_MIN			MIN(RX_WINDOW_MAX / 2, MAX(2 * RX_WINDOW_INCREMENT, GPI_TICK_US_TO_HYBRID(20)))		// minimum must cover variations in execution time from polling to RX on

#define GRID_DRIFT_FILTER_DIV	4
#define GRID_TICK_UPDATE_DIV	2
#define GRID_DRIFT_MAX			MIN(3 * DRIFT_TOLERANCE * GRID_TICK_UPDATE_DIV * GRID_DRIFT_FILTER_DIV, 0x7FFF)
	// P control loop has a permanent deviation since control_value = drift / GRID_TICK_UPDATE_DIV
	// -> to be able to compensate for drift up to DRIFT_TOLERANCE, GRID_DRIFT_MAX must be > DRIFT_TOLERANCE * GRID_TICK_UPDATE_DIV
	// Additional to this, the control loop doesn't get an new measurement in every slot, so the single-value
	// measurement may exceed the theoretical limit. Therefore we add a margin factor.
#define TX_OFFSET_FILTER_DIV	2
#define TX_OFFSET_MAX			(2 * MAX_PROPAGATION_DELAY + GPI_TICK_US_TO_HYBRID(2))

#define ISR_LATENCY_BUFFER		40		// in microseconds
	// influencing factors:
	// - interrupts locked on thread level: < 80 clock cycles (mul: ~40, memcpy_dma: ~40 + 2 * MEMCPY_DMA_BLOCKSIZE)
	// - interrupt latency for grid timer ISR: ~90 clock cycles (incl. ~30 caused by LED_ISR)
	// - rare number of exceedings is acceptable (can be measured with MX_VERBOSE_PROFILE and stat_counter.num_rx_late/num_tx_late)

//ASSERT_CT_STATIC((RX_WINDOW_MAX / RX_WINDOW_INCREMENT) >= 4, critical_RX_WINDOW_settings);

//**************************************************************************************************

// ATTENTION: wake-up pattern doesn't work with nested interrupts (because it would be overwritten)
// NOTE: order of instructions is chosen such that delay between LED on/off and function call/return
// is near to interrupt latency, so LED signaling length is near to real ISR length incl. interrupt latency
#if MX_VERBOSE_STATISTICS
	#define LED_ISR(name, led, vector...)	\
		__attribute__((interrupt(vector)/*, naked*/)) name() {		\
			__asm__ volatile (				\
				"bit	#16, 0(r1)	\n"		/* test if we woke up from LPM */\
				"jz		2f			\n"		/* if so: */\
				"mov	&__TBR, %2	\n"		/* store wake-up timestamp */\
				"2:					\n"		\
				"push	#1f			\n"		/* extend stack such that reti redirects here */\
				"clr	%1			\n"		/* clear wake-up pattern */\
				"xor.b	%3, %0		\n"		/* toggle LED */\
				"push	r2			\n"		/* push intermediate SR (mode = Active, GIE = 0) */\
				"jmp 	" #name "_	\n"		\
				"1:					\n"		\
				"xor.b	%3, %0		\n"		/* toggle LED */\
				"bic	%1, 0(r1)	\n"		/* arm selected wake-up pattern */\
				/*"reti				\n"*/	\
				: "=m"(GPI_MSP430_TMOTE_LED_PORT), "+m"(s_wake_up_pattern), "=m"(mx.wake_up_timestamp) : "i"(led) ); }	\
		void __attribute__((interrupt)) name ## _ ()
#else
	#define LED_ISR(name, led, vector...)	\
		__attribute__((interrupt(vector)/*, naked*/)) name() {		\
			__asm__ volatile (				\
				"push	#1f			\n"		/* extend stack such that reti redirects here */\
				"clr	%1			\n"		/* clear wake-up pattern */\
				"xor.b	%2, %0		\n"		/* toggle LED */\
				"push	r2			\n"		/* push intermediate SR (mode = Active, GIE = 0) */\
				"jmp 	" #name "_	\n"		\
				"1:					\n"		\
				"xor.b	%2, %0		\n"		/* toggle LED */\
				"bic	%1, 0(r1)	\n"		/* arm selected wake-up pattern */\
				/*"reti				\n"*/		\
				: "=m"(GPI_MSP430_TMOTE_LED_PORT), "+m"(s_wake_up_pattern) : "i"(led) ); }	\
		void __attribute__((interrupt)) name ## _ ()
#endif

//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************

typedef enum Slot_State_tag
{
	RESYNC		= 0,
	RX_RUNNING	= 6,
	TX_RUNNING	= 10,
	IDLE		= 10,	// ATTENTION: TX and IDLE are not distinguished here

} Slot_State;

//**************************************************************************************************
//***** Forward Declarations ***********************************************************************



//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

// don't include wake_up_pattern into struct for performance reasons
// (for details see LED_ISR())
static uint16_t				s_wake_up_pattern;

static struct
{
	Gpi_Fast_Tick_Native	sfd_capture;
	Gpi_Hybrid_Tick			sfd_nominal;
	Gpi_Hybrid_Tick			next_grid_tick;
	Gpi_Hybrid_Tick			next_trigger_tick;
	int16_t					grid_drift;
	int16_t					grid_drift_cumulative;
	uint16_t				rx_trigger_offset;
	uint16_t				tx_trigger_offset;

	Slot_State				slot_state;
	Slot_Activity			next_slot_task;

#if MX_VERBOSE_STATISTICS
	Gpi_Fast_Tick_Native	radio_start_timestamp;
#endif

} s;

//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Local Functions ****************************************************************************

// pin test functions
static inline unsigned int is_FIFO()		{ return P1IN & BV(3);	}
static inline unsigned int is_FIFOP()		{ return P1IN & BV(0);	}
static inline unsigned int is_SFD()			{ return P4IN & BV(1);	}

//**************************************************************************************************

// ATTENTION: to be called from ISRs only
static inline void __attribute__((always_inline)) set_event(Event event)
{
	mx.events |= BV(event);

	// for LED_ISRs
	s_wake_up_pattern = SCG1 | SCG0 | OSCOFF | CPUOFF;

	// for non-LED_ISRs or in case LED_ISR() is flat
	__bic_status_register_on_exit(SCG1 | SCG0 | OSCOFF | CPUOFF);
}

//**************************************************************************************************

static inline unsigned int __attribute__((always_inline)) write_tx_fifo(void *p, unsigned int size, const void *p2)
{
	static const uint8_t	zero = 0;
	register unsigned int	or_data = 0;
	register unsigned int	scratch, p2_inc;

	// if p2 == NULL: xor with 0 in the loop
	p2_inc = (NULL == p2) ? 0 : 1;
	p2 = (NULL == p2) ? &zero : p2;

//#define BENCHMARK 1
#if BENCHMARK
	register unsigned int	t1, t2;
	while (!(U0TCTL & TXEPT));
#endif

	// transfer loop
	// NOTE: measurements suggest that one SPI transfer (transport into shift register, transfer,
	// setting TXIFG) takes 20 clock cycles. For example without write back and or_data we see the
	// following values:
	// #bytes:		 1   2   3   4   5   6   7   8   9
	// #runtime:	19  35  51  71  91 111 131 151 171 ...
	// The first two iterations are always fast because U0TXBUF is immediately available. With no
	// additional nops, every subsequent transfer uses exactly two iterations in the polling loop,
	// such that writing to U0TXBUF ends after:
	// transfer:	 1   2   3   4   5  ... end
	// #cycle: 		13  29  45  65  85  ... +6
	// Adding an additional nop introduces a shift: the 4th byte comes fast such that the
	// following two have to wait longer. Thereafter the pattern repeats every 4 bytes:
	// transfer:	 1   2   3   4   5   6   7   8   9  10  11  12
	// #cycle: 		13  30  47  64  85 106 123 144 165 186 203 224
	// #runtime:	20  37  54  71  92 113 130 151 172 193 210 231
	// With > 3-4 nops, the code is slower than the SPI interface. The same holds for adding
	// writeback and or_data (together). Hence without the features, we would be able to process
	// two more xor pointers (3 cycles each) without penalty: one equivalent to 3 nops plus one
	// if we replace the polling loop. The last approach should be considered only if absolutely
	// necessary because it introduces strong timing dependencies (e.g. to the SPI BITCLK).
	__asm__ volatile
	(
#if BENCHMARK
		"mov	&__TBR, %[t1]			\n"
#endif
		"1:								\n"		// cycles:

		// data = *p++ ^ *p2; p2 += p2_inc;
		"mov.b	@%[p]+, %[d]			\n"		// 2
#if 1
		"xor.b	@%[p2], %[d]			\n"		// 2
		"add	%[p2_inc], %[p2]		\n"		// 1
#else
		// this variant can be used if time is critical. It safes 1 cycle but must be implemented
		// as self modifying code. The idea is to exchange the xor instruction with a nop of same
		// size and cycles if p2 == NULL.
		"xor.b	@%[p2]+, %[d]			\n"		// 2
//		"jmp	$+2						\n"		// = 2 cycle nop
#endif
		// while (!(IFG1 & TXIFG));
		"2:								\n"		// cycles:
		"bit.b	@%[rIFG1], %[rTXIFG]	\n"		// 2
		"jz		2b						\n"		// 2

		// U0TXBUF = data, implicitly trigger transfer
		"mov.b	%[d], &__U0TXBUF		\n"		// 4
#if 1
		// write back: *(p-1) = data
		"mov.b	%[d], -1(%[p])			\n"		// 4

		// or_data |= data
		"bis.b	%[d], %[od]				\n"		// 1
#endif
		// do {...} while (p != end);
		"cmp	%[end], %[p]			\n"		// 1
		"jne	1b						\n"		// 2

#if BENCHMARK
		"mov	&__TBR, %[t2]			\n"		// 3
#endif
		: [p] "+r"(p), [p2] "+&r"(p2), [d] "=&r"(scratch), [od] "+&r"(or_data)
#if BENCHMARK
			, [t1] "=&r"(t1), [t2] "=r" (t2)
#endif
		: [rIFG1] "r"(&IFG1), [rTXIFG] "r"(UTXIFG0), [p2_inc] "r"(p2_inc),
			[end] "r"((const uint8_t*)p + size)
		: "cc", "memory"
	);

#if BENCHMARK
	GPI_TRACE_MSG_FAST(1, "clock cycles for %u bytes: %u", size, t2 - t1);
#endif

	// return ored tx data
	// NOTE: can be interpreted as marker for non-zero tx data
	return or_data;

//	// return updated p
//	// NOTE: it may help to write more efficient code in the caller
//	return p;
}

//**************************************************************************************************

static void start_grid_timer()
{
//	GPI_TRACE_FUNCTION_FAST();

	// NOTE: MAX_TB_INTERVAL could be set to the largest possible value which can be covered by
	// timer B. However, the time is based on hybrid clock, which means that the accuracy
	// decreases with longer timer B intervals. So we choose a compromise.

	const Gpi_Hybrid_Tick	MAX_TB_INTERVAL = GPI_TICK_US_TO_HYBRID(500);
	Gpi_Hybrid_Reference	r;
	Gpi_Hybrid_Tick			d;

	ASSERT_CT(sizeof(Gpi_Hybrid_Tick) == sizeof(uint32_t));

	r = gpi_tick_hybrid_reference();
	d = s.next_trigger_tick - r.hybrid_tick;

//	GPI_TRACE_MSG_FAST(1, "d: %lu", d);

	// if we are late
	// NOTE: signed comparison is important
	// NOTE: first term covers execution time from getting r to writing TBCCR0
	if ((int32_t)d < GPI_TICK_US_TO_HYBRID(1000000 / GPI_SLOW_CLOCK_RATE + 50) + GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER))
	{
		// trigger grid timer immediately
		// NOTE: TBCCR0 is updated implicitly
		TBCCTL0 = CAP | CM1 | CM0 | SCS | CCIS1 | CCIE;
		TBCCTL0 |= CCIS0;
		TACCTL0 = 0;

		#if MX_VERBOSE_STATISTICS
			mx.stat_counter.num_grid_late++;
		#endif
	}

	else if (d <= MAX_TB_INTERVAL)
	{
		TBCCR0 = r.fast_capture + d * FAST_HYBRID_RATIO - GPI_TICK_US_TO_FAST(ISR_LATENCY_BUFFER);
		TBCCTL0 = CCIE;
		TACCTL0 = 0;
	}

	else
	{
		ASSERT_CT(HYBRID_SLOW_RATIO <= 0x10000);

		if (d > 0xF000ul * HYBRID_SLOW_RATIO)
			d = r.hybrid_tick + 0xE000ul * HYBRID_SLOW_RATIO;
		else d = s.next_trigger_tick - MAX_TB_INTERVAL / 2;

		TACCR0 = d / HYBRID_SLOW_RATIO;
		TACCTL0 = CCIE;

		// stop timer 0
		// ATTENTION: since TB0 is jointly used as grid and timeout timer, a caller to start_grid_timer()
		// may expect that TB0 is reinitialized in every case. Hence we do it here for sure.
		TBCCTL0 = 0;
	}

	s.slot_state = (RESYNC == s.slot_state) ? RESYNC : IDLE;
}

//**************************************************************************************************

// helper ISR for grid timer, see start_grid_timer() for details
void __attribute__((interrupt(TIMERA0_VECTOR))) timera0_isr()
{
	TACCTL0 = 0;

	start_grid_timer();
}

//**************************************************************************************************

// mode: 0 = RESYNC quick, 1 = RESYNC normal, 2 = start Tx (for initiator)
static inline void __attribute__((always_inline)) enter_resync(int mode)
{
	if (2 == mode)
	{
		// start Tx
		s.slot_state = IDLE;
		s.next_slot_task = TX;
	}
	else if (1 == mode)
	{
		// start Rx by activating RESYNC
		s.slot_state = RESYNC;
		s.next_slot_task = RX;
	}
	else if (STOP == s.next_slot_task)
	{
		// call grid timer ISR (and not timeout timer ISR)
		s.slot_state = IDLE;
	}
	else
	{
		// enter RESYNC
		s.slot_state = RESYNC;
		s.next_slot_task = RX;
	}

	// trigger grid timer immediately (with interrupt masked)
	// NOTE: TBCCR0 is updated implicitly
	TBCCTL0 = CAP | CM1 | CM0 | SCS | CCIS1;
	TBCCTL0 |= CCIS0;

	// if quick (and dirty): don't update s.next_grid_tick to save time
	// ATTENTION: this expects s.next_grid_tick to be initialized and causes the first RESYNC timeout
	// interval to be up to +/- MX_SLOT_LENGTH longer. Furthermore it saves a REORDER_BARRIER(), so
	// don't use quick with interrupts unlocked.
	if (0 != mode)
	{
		// init s.next_grid_tick to precise value
		// ATTENTION: this is important to align the initiator exactly at the grid
		// NOTE: TBCCR0 + buffer (see below) is the trigger tick. Grid timer ISR will assume
		// + ISR_LATENCY_BUFFER as polling interval. Hence,
		// grid tick = TBCCR0 + buffer + ISR_LATENCY_BUFFER + TX_TO_GRID_OFFSET.
		// NOTE: if mode == 1, the timing is not critical since we enter RESYNC mode
		s.next_grid_tick =
			gpi_tick_fast_to_hybrid(TBCCR0) +
			GPI_TICK_US_TO_HYBRID(50) +
			GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER) +
			TX_TO_GRID_OFFSET;

		// add some time buffer covering the remaining execution time from here until interrupt
		// gets unlocked
		// ATTENTION: don't do that before calling gpi_tick_fast_to_hybrid() because then
		// gpi_tick_fast_to_hybrid() would work on a future value.
		TBCCR0 += GPI_TICK_US_TO_FAST(50);

		REORDER_BARRIER();
	}

	// unmask grid timer interrupt
	TBCCTL0 |= CCIE;
}

//**************************************************************************************************

// timer B1 IRQ dispatcher
// could be removed if only one vector is used
void __attribute__((interrupt(TIMERB1_VECTOR), naked)) timerb_isr()
{
	// vectoring code
	// NOTE: additional indirection with br is used because jmp has a limited target address range
	// (relative to PC)
	__asm__ volatile
	(
		"add	&__TBIV, r0		\n"		// jump into vector table
		"reti					\n"		// no interrupt pending (must not happen)
		"jmp	1f				\n"		// capture/compare 1
		"reti					\n"		// ...
		"reti					\n"
		"reti					\n"
		"reti					\n"
		"reti					\n"		// capture/compare 6
		"reti					\n"		// timer overflow
		"1:	br	#SFD_isr		\n"
	);
}

//**************************************************************************************************

// SFD ISR
void LED_ISR(SFD_isr, LED_SFD_ISR)
{
	// don't TRACE, it is too time critical
	// GPI_TRACE_FUNCTION_FAST();

	// if Rx
	// NOTE: s.slot_state = RX_RUNNING or RESYNC
	if (TX_RUNNING != s.slot_state)
	{
		PROFILE_ISR();

		uint8_t	len;

		// test SFD -> suppress glitches and false alarms caused by dirty edges
		if (!is_SFD())
			return;

		gpi_led_on(LED_RX);

		// mask SFD IRQ and store SFD capture value for later use
		s.sfd_capture = TBCCR1;
		TBCCTL1 &= ~CCIE;

		// prepare SPI read cycle: activate CS, transmit RXFIFO address, clear URXIFG0
		gpi_radio_cs_on();
		U0TXBUF = 0x40 | CC2420_RXFIFO;
		while (!(U0TCTL & TXEPT));
		U0RXBUF;

		// wait until LEN field has been received (marked by FIFO signal)
		while (!is_FIFO());

		// read LEN field
		U0TXBUF = 0;
		while (!(IFG1 & URXIFG0));
		len = U0RXBUF;

		// mark invalid LEN
		if (len != PHY_PAYLOAD_SIZE + 2)
		{
			len = 0;

			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_rx_broken++;
			#endif
		}

		// if LEN is valid
		else
		{
			// activate DMA transfer, unmask DMA IRQ
			// each: clear interrupt flag, init transfer size, enable DMA channel
			// NOTE: channel 1 is a bit less time critical than channel 0 because
			// it triggers after the SPI transfer
			TACCTL2 &= ~CCIFG;
			DMA0SZ = len;
			DMA0CTL |= DMAEN;
			IFG1 &= ~URXIFG0;
			DMA1SZ = len;
			DMA1CTL |= DMAEN | DMAIE;
			DMA1CTL &= ~DMAIFG;

			// safety check: test interrupt latency, abort if critical
			// ATTENTION: for the DMA to work right, it has to be ready before the rising edge on
			// FIFO appears. If we are too late, the DMA interrupt will not fire and our processing
			// gets out of sync. Hence it is a good idea to handle this situation, although it
			// never appears if the interrupt latency is reasonably bounded.
			if (gpi_tick_fast_native() - s.sfd_capture > GPI_TICK_US_TO_FAST(64 - 4))
			{
				DMA0CTL &= ~DMAEN;
				DMA1CTL &= ~(DMAEN | DMAIE);

				len = 0;

				#if MX_VERBOSE_STATISTICS
					mx.stat_counter.num_rx_dma_late++;
				#endif
				GPI_TRACE_MSG_FAST(TRACE_WARNING, "!!! SFD latency limit exceeded -> check program, must not happen !!!");
			}

			else
			{
				// update timeout timer to monitor data transfer
				// NOTE: times 33 incl. tolerance for clock drift, + 100 DMA interrupt latency
				TBCCR0 = s.sfd_capture + GPI_TICK_US_TO_FAST2((1 + PHY_PAYLOAD_SIZE + 2) * 33 + 100);
				TBCCTL0 = CCIE;
			}
		}

		// in case of errors
		if (!len)
		{
			// trigger timeout timer (immediately) -> do error handling in its ISR
			TBCCTL0 = CAP | CM1 | CM0 | SCS | CCIS1 | CCIE;
			TBCCTL0 |= CCIS0;
		}

		PROFILE_ISR();
	}

	// if Tx
	else
	{
		PROFILE_ISR();

		// test SFD -> suppress glitches and false alarms caused by dirty edges
		if (is_SFD())
			return;

		// mask SFD IRQ
		TBCCTL1 &= ~CCIE;

		// state at this point: transmission completed, radio went to RX_CALIBRATE state

		// turn radio off
		// ATTENTION: this must happen within RX_CALIBRATE phase (i.e. near after SFD edge) to
		// ensure that RX_SFD_SEARCH will not be reached. This guarantees that RXFIFO is empty
		// for sure, so we do not need to check and flush it.
		gpi_radio_strobe(CC2420_SRFOFF);
		gpi_led_off(LED_TX);

		#if MX_VERBOSE_STATISTICS
		// if (s.radio_start_timestamp & 1)
		{
			mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
			s.radio_start_timestamp = 0;
		}
		#endif

		PROFILE_ISR();

		GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "SFD capture: %lu", gpi_tick_hybrid_to_us(gpi_tick_fast_to_hybrid(TBCCR1)));
	}

	// TRACE on return is less time critical than on entry
	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// DMA ISR
// triggered after packet has been transfered to rx queue
void LED_ISR(DMA_isr, LED_DMA_ISR, DACDMA_VECTOR)
{
	GPI_TRACE_FUNCTION_FAST();
	PROFILE_ISR();

	int 		strobe_resync = 0;
	Packet		*packet;

	// deactivate DMA for sure
	// NOTE: not really needed because DMAEN = 0 after DMAxSZ reaches zero, but to be sure...
	DMA0CTL &= ~DMAEN;
	DMA1CTL &= ~(DMAEN | DMAIE);

	// finalize SPI transfer
	gpi_radio_cs_off();

	// turn radio off
	// ATTENTION: this is important to avoid disturbing receptions and to safe energy
	// NOTE: if slot length is very high we could send CC2420 to power down mode here
	gpi_radio_strobe(CC2420_SRFOFF);
	gpi_led_off(LED_RX);

	#if MX_VERBOSE_STATISTICS
//	if (s.radio_start_timestamp & 1)
	{
		mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
		s.radio_start_timestamp = 0;
	}
	#endif

	// stop timeout timer
	// -> not needed because this is done implicitely below
	// TBCCTL0 = 0;

	packet = &mx.rx_queue[mx.rx_queue_num_written % NUM_ELEMENTS(mx.rx_queue)];

//	GPI_TRACE_MSG_FAST(1, "num_written = %u, num_writing = %u, num_read = %u",
//		mx.rx_queue_num_written, mx.rx_queue_num_writing, mx.rx_queue_num_read);

//	GPI_TRACE_MSG_FAST(1, "DMACTL0: %04X, DMACTL1: %04X", DMACTL0, DMACTL1);
//	GPI_TRACE_MSG_FAST(1, "DMA0CTL: %04X, DMA0SA: %04X, DMA0DA: %04X, DMA0SZ: %04X", DMA0CTL, DMA0SA, DMA0DA, DMA0SZ);
//	GPI_TRACE_MSG_FAST(1, "DMA1CTL: %04X, DMA1SA: %04X, DMA1DA: %04X, DMA1SZ: %04X", DMA1CTL, DMA1SA, DMA1DA, DMA1SZ);
//	GPI_TRACE_MSG_FAST(1, "DMA1DA: %04X, &q: %04X", DMA1DA, (uint16_t)&mx.rx_queue[mx.rx_queue_num_written % NUM_ELEMENTS(mx.rx_queue)]);
//	GPI_TRACE_MSG_FAST(1, "CRC: %02X", mx.rx_queue[mx.rx_queue_num_written % NUM_ELEMENTS(mx.rx_queue)].crc_corr);

	// check CRC of received packet, if ok: process packet
	if (packet->crc_ok)
	{
		PROFILE_ISR();

		#if MX_VERBOSE_STATISTICS
			mx.stat_counter.num_rx_success++;
		#endif
		GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "CRC ok");

		// update slot timing control values
		{
			Gpi_Hybrid_Tick	sfd_cap = gpi_tick_fast_to_hybrid(s.sfd_capture);

			ASSERT_CT(sizeof(Gpi_Hybrid_Tick) == sizeof(uint32_t));

			GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "SFD: nominal: %lu, capture: %lu, deviation: %+ld",
				gpi_tick_hybrid_to_us(s.sfd_nominal), gpi_tick_hybrid_to_us(sfd_cap),
				(int32_t)(sfd_cap - s.sfd_nominal) < 0 ? -gpi_tick_hybrid_to_us(s.sfd_nominal - sfd_cap) : gpi_tick_hybrid_to_us(sfd_cap - s.sfd_nominal)
			);

			// if RESYNC requested: realign slot grid based on SFD capture
			if (RESYNC == s.slot_state)
			{
				s.next_grid_tick = sfd_cap - GRID_TO_SFD_OFFSET + MX_SLOT_LENGTH;

				s.grid_drift = 0;
				s.grid_drift_cumulative = 0;
				s.tx_trigger_offset = TX_TO_GRID_OFFSET;

					// don't set Rx window to tight after resync because we don't have
					// any information on grid drift yet
				s.rx_trigger_offset = RX_TO_GRID_OFFSET + RX_WINDOW_MAX / 2;

				s.slot_state = RX_RUNNING;

				mx.slot_number = packet->slot_number;

				GPI_TRACE_MSG_FAST(TRACE_INFO, "(re)synchronized to slot %u", mx.slot_number);
			}

			// else use phase-lock control loop to track grid
			else
			{
				int32_t	gd;

				if (mx.slot_number != packet->slot_number)
				{
					#if MX_VERBOSE_STATISTICS
						mx.stat_counter.num_rx_slot_mismatch++;
					#endif
					GPI_TRACE_MSG_FAST(TRACE_WARNING, "!!! slot_number mismatch: expected: %" PRIu16 ", received: %" PRIu16 " !!!",
						mx.slot_number, packet->slot_number);

					mx.slot_number = packet->slot_number;
					// TODO: should not happen -> start RESYNC?
				}

				// update grid drift and next grid tick
				// NOTE: s.grid_drift uses fix point format with ld(GRID_DRIFT_FILTER_DIV) fractional digits

				// compute SFD deviation
				// NOTE: result is bounded by Rx window size
				sfd_cap -= s.sfd_nominal;

				// keep Rx window in this range (with some margin)
				s.rx_trigger_offset = RX_WINDOW_MIN;
				gd = (uint16_t)MAX(ABS(s.grid_drift / GRID_DRIFT_FILTER_DIV), ABS((int16_t)sfd_cap));
				if (s.rx_trigger_offset < (uint16_t)gd + RX_WINDOW_INCREMENT)
					s.rx_trigger_offset = (uint16_t)gd + RX_WINDOW_INCREMENT;
				s.rx_trigger_offset += RX_TO_GRID_OFFSET;

				// restore nominal grid tick (i.e. remove previously added control value)
				s.next_grid_tick -= s.grid_drift / (GRID_DRIFT_FILTER_DIV * GRID_TICK_UPDATE_DIV);

				// update grid drift:
				// new = 1/c * measurement + (c-1)/c * old = old - 1/c * old + 1/c * measurement
				// + GRID_DRIFT_FILTER_DIV / 2 leads to rounding
				s.grid_drift -= (s.grid_drift + GRID_DRIFT_FILTER_DIV / 2) / GRID_DRIFT_FILTER_DIV;
				gd = s.grid_drift;
				gd += (int32_t)sfd_cap;
				s.grid_drift = gd;

				// if drift exceeds limit: start RESYNC
				// NOTE: saturation could also help since obviously we are still able to receive
				// something (at the moment). Nevertheless it seems that we are in a critical
				// situation, so resync appears adequate as well.
				if (ABS(gd) > GRID_DRIFT_MAX)
				{
					#if MX_VERBOSE_STATISTICS
						mx.stat_counter.num_grid_drift_overflow++;
						mx.stat_counter.num_resync++;
					#endif
					GPI_TRACE_MSG_FAST(TRACE_INFO, "grid drift overflow: %ld > %d -> enter RESYNC",
						ABS(gd), GRID_DRIFT_MAX);

					strobe_resync = 1;
				}

				else
				{
					// update grid tick
					// NOTE: this realizes the proportional term of a PID controller
					s.next_grid_tick += s.grid_drift / (GRID_DRIFT_FILTER_DIV * GRID_TICK_UPDATE_DIV);

					// keep cumulative grid drift
					// NOTE: this is the base for the integral component of a PID controller
					gd = s.grid_drift_cumulative;
					gd += s.grid_drift;
					if (gd > 0x7FFF)
						gd = 0x7FFF;
					else if (gd < -0x8000l)
						gd = -0x8000l;
					s.grid_drift_cumulative = gd;

					// update tx trigger offset
					// NOTE: this realizes the integral term of a PID controller in an indirect way
					// (through a loopback with potentially high uncertainty on its reaction)
					s.tx_trigger_offset = s.grid_drift_cumulative / (GRID_DRIFT_FILTER_DIV * TX_OFFSET_FILTER_DIV);
					if ((int16_t)s.tx_trigger_offset < 0)
						s.tx_trigger_offset = 0;
					else if (s.tx_trigger_offset > TX_OFFSET_MAX)
						s.tx_trigger_offset = TX_OFFSET_MAX;
					s.tx_trigger_offset += TX_TO_GRID_OFFSET;

					GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "grid_drift_cum: %d, tx_offset: %u",
						s.grid_drift_cumulative, s.tx_trigger_offset - TX_TO_GRID_OFFSET);
				}
			}

			// special handling during start-up phase, see tx decision for details
			#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
				if (mx.slot_number < mx.discovery_exit_slot)
				{
					// ATTENTION: do not rely on mx.tx_sideload or mx.tx_reserve at this point
					// (mx.tx_sideload may change between here and next trigger tick, mx.tx_reserve
					// may point to an incosistent row since it is not guarded w.r.t. ISR level).
					// Instead, there is a very high probability that mx.tx_packet is ready since
					// we did not TX in current slot (otherwise we wouldn't be here).
					if (!strobe_resync && (STOP != s.next_slot_task) &&
						packet->flags.owner_forecast_1 && mx.tx_packet.is_ready)
					{
						GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "tx decision: no startup owner (owner_forecast_1 set)");
						s.next_slot_task = TX;
					}
				}
			#endif

			s.next_trigger_tick = s.next_grid_tick -
				((s.next_slot_task == TX) ? s.tx_trigger_offset : s.rx_trigger_offset);

			GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "next_grid: %lu, grid_drift: %+d (%+dus)",
				gpi_tick_hybrid_to_us(s.next_grid_tick), s.grid_drift,
				(int16_t)gpi_tick_hybrid_to_us(s.grid_drift / GRID_DRIFT_FILTER_DIV));
		}

		// check potential queue overflow, if ok: keep packet
		if (mx.rx_queue_num_writing - mx.rx_queue_num_read < NUM_ELEMENTS(mx.rx_queue))
		{
			mx.rx_queue_num_written++;

			// use packet as next Tx sideload (-> fast tx update)
			if (MX_GENERATION_SIZE != mx.rank)
				mx.tx_sideload = &(packet->coding_vector[0]);

			set_event(RX_READY);

			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_received++;
			#endif
		}
		#if MX_VERBOSE_STATISTICS
		else
		{
			GPI_TRACE_MSG_FAST(TRACE_INFO, "Rx queue overflow, NW: %u, NR: %u", mx.rx_queue_num_writing, mx.rx_queue_num_read);

			if (mx.rank < MX_GENERATION_SIZE)
				mx.stat_counter.num_rx_queue_overflow++;
			else mx.stat_counter.num_rx_queue_overflow_full_rank++;
		}
		#endif
		ASSERT_CT(NUM_ELEMENTS(mx.rx_queue) >= 2, single_entry_rx_queue_will_not_work);

		// start RESYNC if requested
		if (strobe_resync)
		{
			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_resync++;
			#endif

			enter_resync(0);
		}

		// handover to grid timer (if not already done by enter_resync())
		else start_grid_timer();

		PROFILE_ISR();
	}

	// if CRC not ok: regard packet as invisible
	else
	{
		#if MX_VERBOSE_STATISTICS
			mx.stat_counter.num_rx_broken++;
		#endif

		// trigger timeout timer (immediately) -> do error handling in its ISR
		TBCCTL0 = CAP | CM1 | CM0 | SCS | CCIS1 | CCIE;
		TBCCTL0 |= CCIS0;
	}

	PROFILE_ISR();
	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// timer B0 IRQ dispatcher
void __attribute__((interrupt(TIMERB0_VECTOR), naked)) timerb0_isr()
{
	__asm__ volatile
	(
		"add	%0, r0				\n"		// jump into vector table
		"push	#grid_timer_isr		\n"		//  0: resync: extend stack such that first reti redirects to
		"push	r2					\n"		//     second ISR (with interrupts locked) -> call both ISRs
		"br		#timeout_isr		\n"		//  6: timeout
		"br		#grid_timer_isr		\n"		// 10: grid timer
		: : "m"(s.slot_state)
	);

	// ATTENTION: changes to SR made in timeout_isr() get lost in case of resync (i.e., if called
	// in the chain). This is no problem as long as timeout_isr() doesn't call set_event() or
	// grid_timer_isr() calls set_event() for sure afterwards.

	ASSERT_CT(RESYNC     ==  0, definition_of_RESYNC_does_not_match_assembly_code);
	ASSERT_CT(RX_RUNNING ==  6, definition_of_RX_RUNNING_does_not_match_assembly_code);
	ASSERT_CT(TX_RUNNING == 10, definition_of_TX_RUNNING_does_not_match_assembly_code);
	ASSERT_CT(IDLE       == 10, definition_of_IDLE_does_not_match_assembly_code);
}

//**************************************************************************************************

// timeout ISR
// triggered if there was no successful packet transfer in a specific time interval
void LED_ISR(timeout_isr, LED_TIMEOUT_ISR)
{
	GPI_TRACE_FUNCTION_FAST();
	PROFILE_ISR();

	// deactivate DMA channels
	#if MX_VERBOSE_STATISTICS
		uint16_t dma_enabled = DMA0CTL & DMAEN;
	#endif
	DMA0CTL &= ~DMAEN;
	DMA1CTL &= ~(DMAEN | DMAIE);

	// mask IRQs
	// NOTE: stopping timeout timer is not needed since this is done implicitely by start_grid_timer()
	TBCCTL1 &= ~CCIE;

	// finalize open SPI transfer
	while (!(U0TCTL & TXEPT));
	gpi_radio_cs_off();

	// turn radio off
	gpi_radio_strobe(CC2420_SRFOFF);
	gpi_led_off(LED_RX | LED_TX);

	#if MX_VERBOSE_STATISTICS
	if (s.radio_start_timestamp & 1)
	{
		mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
		s.radio_start_timestamp = 0;
	}
	#endif

	// flush RXFIFO if needed
	if (is_FIFO())
	{
		// ATTENTION: SWRS041c p. 61 states that we must read at least one byte before SFLUSHRX.
		// Although there is a high probability that we already did this (by reading the LEN field),
		// there are cases where that may not be true (e.g. if transmission started very short
		// before the timeout). Hence we do it to be absolutely safe.
		gpi_radio_reg_get(CC2420_RXFIFO);

		gpi_radio_strobe(CC2420_SFLUSHRX);
		gpi_radio_strobe(CC2420_SFLUSHRX);	// SWRS041c p. 33
	}

	if (s.slot_state != RESYNC)
	{
		// update stat counters only on real timeouts
		#if MX_VERBOSE_STATISTICS
		if ((TBCCTL0 & (CAP | CCIS1)) != (CAP | CCIS1))
		{
			if (dma_enabled)
				mx.stat_counter.num_rx_dma_timeout++;
			else mx.stat_counter.num_rx_timeout++;
		}
		#endif

		// widen Rx time window
		s.rx_trigger_offset += RX_WINDOW_INCREMENT;

		// if Rx window exceeds limit: start RESYNC
		// NOTE: we start RESYNC immediately (instead of saturating) because we probably lost
		// synchronization and hence do not expect to receive something anymore
		if (s.rx_trigger_offset > RX_TO_GRID_OFFSET + RX_WINDOW_MAX)
		{
			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_rx_window_overflow++;
				mx.stat_counter.num_resync++;
			#endif
			GPI_TRACE_MSG_FAST(TRACE_INFO, "Rx window overflow: %u > %u -> enter RESYNC",
				s.rx_trigger_offset, RX_TO_GRID_OFFSET + RX_WINDOW_MAX);

			enter_resync(0);
		}

		else
		{
			// update next trigger tick
			s.next_trigger_tick = s.next_grid_tick -
				((s.next_slot_task == TX) ? s.tx_trigger_offset : s.rx_trigger_offset);

			// handover to grid timer
			// NOTE: this is done automatically while RESYNC is running
			// (more precisely: if RESYNC was active before entering current function)
			start_grid_timer();
		}
	}

	PROFILE_ISR();
	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// grid timer ISR
// this is one of the central transport layer routines
void LED_ISR(grid_timer_isr, LED_GRID_TIMER_ISR)
{
//	GPI_TRACE_FUNCTION_FAST();
	PROFILE_ISR();

	TBCCTL0 = 0;

	// if STOP requested: stop
	if (STOP == s.next_slot_task)
	{
		#if MX_VERBOSE_STATISTICS
			mx.stat_counter.slot_off = mx.slot_number;	// the old one (viewpoint: turn off after last processing)
		#endif

		set_event(STOPPED);
		GPI_TRACE_MSG_FAST(TRACE_INFO, "transport layer stopped");

		GPI_TRACE_RETURN_FAST();
	}

	// if Rx
	if ((RESYNC == s.slot_state) || (RX == s.next_slot_task))
	{
		PROFILE_ISR();

		Gpi_Hybrid_Reference						r;
		register int16_t __attribute__((unused))	late = 0;

		// poll to precise trigger time
		if (RESYNC != s.slot_state)
		{
			late = 1;

			register uint16_t	t = TBCCR0 + GPI_TICK_US_TO_FAST(ISR_LATENCY_BUFFER);

			PROFILE_ISR();

			while ((int16_t)(gpi_tick_fast_native() - t) < 0)
				late = 0;

			PROFILE_ISR();
		}

		gpi_radio_strobe(CC2420_SRXON);

		#if MX_VERBOSE_STATISTICS
			s.radio_start_timestamp = gpi_tick_fast_native() | 1;
			if (late)
				mx.stat_counter.num_rx_late++;
		#endif

		// unmask rising edge SFD IRQ
		TBCCTL1 &= ~(CM1 | CM0 | CCIFG);
		TBCCTL1 |= CM0 | CCIE;

		// allocate rx queue destination slot
		DMA1DA = (uint16_t)&mx.rx_queue[mx.rx_queue_num_written % NUM_ELEMENTS(mx.rx_queue)];
		mx.rx_queue_num_writing = mx.rx_queue_num_written + 1;

//		GPI_TRACE_MSG_FAST(1, "DMACTL0: %04X, DMACTL1: %04X", DMACTL0, DMACTL1);
//		GPI_TRACE_MSG_FAST(1, "DMA0CTL: %04X, DMA0SA: %04X, DMA0DA: %04X, DMA0SZ: %04X", DMA0CTL, DMA0SA, DMA0DA, DMA0SZ);
//		GPI_TRACE_MSG_FAST(1, "DMA1CTL: %04X, DMA1SA: %04X, DMA1DA: %04X, DMA1SZ: %04X", DMA1CTL, DMA1SA, DMA1DA, DMA1SZ);

		s.sfd_nominal = s.next_grid_tick + GRID_TO_SFD_OFFSET;

		r = gpi_tick_hybrid_reference();

		// if RESYNC: restart grid timer (-> potentially long interval)
		// NOTE: timeout timer is called implicitly while RESYNC
		if (s.slot_state == RESYNC)
		{
			// ATTENTION: don't do s.next_grid_tick += MX_SLOT_LENGTH_RESYNC because grid timer is also
			// triggered by frames from interferers (Rx -> SFD -> ... (broken/invalid) -> timeout
			// -> grid timer) and hence current time might be far away from s.next_grid_tick. With
			// s.next_grid_tick += MX_SLOT_LENGTH_RESYNC, s.next_grid_tick could end up in the far future
			// if it gets incremented frequently.
			s.next_grid_tick = r.hybrid_tick + MX_SLOT_LENGTH_RESYNC;
			s.next_trigger_tick = s.next_grid_tick;
			start_grid_timer();
		}

		// else start timeout timer
		else
		{
			Gpi_Hybrid_Tick		t;

			// timeout point = grid tick + GRID_TO_SFD_OFFSET + window, window = trigger offset - RX_TO_GRID_OFFSET
			t = s.next_grid_tick + s.rx_trigger_offset - RX_TO_GRID_OFFSET + GRID_TO_SFD_OFFSET;

			TBCCR0 = r.fast_capture + (t - r.hybrid_tick) * FAST_HYBRID_RATIO;
			TBCCTL0 = CCIE;

			s.slot_state = RX_RUNNING;

			GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "timeout: %lu", gpi_tick_hybrid_to_us(t));
		}

		PROFILE_ISR();
		GPI_TRACE_MSG_FAST(TRACE_INFO, "Rx started");
	}

	// if Tx
	else
	{
		PROFILE_ISR();

		uint8_t		*p;

		ASSERT_CT(!((uintptr_t)&mx.tx_packet & 1), tx_packet_is_not_aligned);

//		GPI_TRACE_MSG_FAST(1, "CC2420 state: %02X %u", gpi_radio_strobe(CC2420_SNOP), gpi_radio_reg_get(CC2420_FSMSTATE));

		register int16_t __attribute__((unused))	late = 0;

		// poll to precise trigger time
		{
			late = 1;

			register uint16_t	t = TBCCR0 + GPI_TICK_US_TO_FAST(ISR_LATENCY_BUFFER);

			PROFILE_ISR();

			while ((int16_t)(gpi_tick_fast_native() - t) < 0)
				late = 0;

			PROFILE_ISR();
		}

		gpi_radio_strobe(CC2420_STXON);
		gpi_led_on(LED_TX);

		#if MX_VERBOSE_STATISTICS
			s.radio_start_timestamp = gpi_tick_fast_native() | 1;
			if (late)
				mx.stat_counter.num_tx_late++;
		#endif

		gpi_radio_cs_on();

		U0TXBUF = CC2420_TXFIFO;

		// LEN
		while (!(IFG1 & UTXIFG0));
		U0TXBUF = PHY_PAYLOAD_SIZE + 2;

		p = (uint8_t*)&mx.tx_packet;

		// finalize header
		{
			uint16_t slot_number = mx.slot_number + 1;

			mx.tx_packet.slot_number = slot_number;
			mx.tx_packet.flags.all = 0;
			#if MX_REQUEST
				ASSERT_CT(IT_ROW_MAP == 0);
			#else
				mx.tx_packet.flags.info_type = IT_UNDEFINED;
			#endif

			#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
				if (slot_number < MX_GENERATION_SIZE)
				{
					if (0 == mx.matrix[slot_number].birth_slot)
						mx.tx_packet.flags.owner_forecast_1 = 1;

					if (slot_number + 1 < MX_GENERATION_SIZE)
						if (0 == mx.matrix[slot_number + 1].birth_slot)
							mx.tx_packet.flags.owner_forecast_2 = 1;
				}
			#endif

			if (MX_GENERATION_SIZE == mx.rank)
			{
				mx.tx_packet.flags.is_full_rank = 1;
				mx.tx_packet.flags.info_type = IT_FULL_RANK_MAP;

				#if MX_SMART_SHUTDOWN
					#if (MX_SMART_SHUTDOWN_MODE >= 3)
						if (mx.full_rank_state >= 1)
						{
							if (mx.is_shutdown_approved)
								mx.tx_packet.flags.info_type = IT_FULL_RANK_ACK_MAP_RADIO_OFF;
							else mx.tx_packet.flags.info_type = IT_FULL_RANK_ACK_MAP;
						}
						else
					#endif
					if (mx.is_shutdown_approved)
						mx.tx_packet.flags.info_type = IT_FULL_RANK_MAP_RADIO_OFF;
				#endif
			}

			#if MX_REQUEST
			else /*if (slot_number >= MX_GENERATION_SIZE)*/
			{
				// f(x) = (1 - 2 ^ (-0.2 * x)) / (1 - 2 ^ (-0.2))
				static const uint8_t LUT1[] =
				{
					 0,  1,  2,  3,  3,  4,  4,  5,  5,  6,
					 6,  6,  6,  6,  7,  7,  7,  7,  7,  7,
					 7,  7,  7,  7,  7,  7,  8
				};

//				// f(x) = (1 - 2 ^ (-0.125 * x)) / (1 - 2 ^ (-0.125))
//				static const uint8_t LUT1[] =
//				{
//					 0,  1,  2,  3,  4,  4,  5,  5,  6,  7,  7,
//					 7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 10,
//					10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11,
//					11, 11, 11, 12
//				};

				// f(x) = 2 ^ (-x) (scaled)
				// -> could be computed directly, but the (small) LUT is the faster variant
				static const uint8_t LUT2[] = {0x3f, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

				uint8_t  	x;
				uint16_t 	age = slot_number - mx.recent_innovative_slot;
				uint8_t	 	rand = mx.tx_packet.rand;		// prepared on thread level

				#if MX_WEAK_ZEROS
					if (slot_number >= mx.weak_zero_release_slot)
						x = LUT1[MIN(MX_GENERATION_SIZE - mx.rank - mx.weak_rank, NUM_ELEMENTS(LUT1) - 1)];
					else
				#endif
				x = LUT1[MIN(MX_GENERATION_SIZE - mx.rank, NUM_ELEMENTS(LUT1) - 1)];

				if ((age >= x) ||
				#if MX_COORDINATED_TX
					#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
						((slot_number >= mx.discovery_exit_slot) && (0 == mx_present_head->num_nodes))
					#else
						(0 == mx_present_head->num_nodes)
					#endif
				#else
					0
				#endif
				)
				{
					if (rand < LUT2[MIN(mx.request.my_column_pending, NUM_ELEMENTS(LUT2) - 1)])
						mx.tx_packet.flags.info_type = IT_ROW_REQUEST;
					else mx.tx_packet.flags.info_type = IT_COLUMN_REQUEST;
				}
			}
			#endif

			#if MX_WEAK_ZEROS
			do
			{
				// don't send weak zero map if not yet released
				if (slot_number < mx.weak_zero_release_slot)
					break;

				// don't send weak zero map if no weak zeros are known
				if (0 == mx.weak_rank)
					break;

				#if MX_SMART_SHUTDOWN
					// don't overwrite radio-off signal
					if ((mx.tx_packet.flags.all & EIT_RADIO_OFF_mask) == EIT_RADIO_OFF_pattern)
						break;

					// no need to send weak zero map if all nodes are finished
					if ((mx.tx_packet.flags.all & EIT_FULL_RANK_ACK_MAP_mask) == EIT_FULL_RANK_ACK_MAP_pattern)
						break;
				#endif

				// send weak zero map with first transmission
				// NOTE: this behavior is particularly beneficial if the majority of weak zeros
				// is injected at the initiator and if mx.weak_zero_release_slot <= 1
				if (0 == mx.stat_counter.num_sent)
				{
					mx.tx_packet.flags.info_type = IT_WEAK_ZERO_MAP;
					break;
				}

				#if MX_COORDINATED_TX
					// no need to send weak zero map if there are no unfinished neighbors
					if (0 == mx_present_head->num_nodes)
						break;
				#endif

				#if MX_REQUEST
					// send weak zero map if explicitly selected (as the way to help on request)
					if (INT16_MIN == mx.request.help_index)
					{
						mx.tx_packet.flags.info_type = IT_WEAK_ZERO_MAP;
						break;
					}
				#else
					// don't waste chances to send something meaningful
					if (mx.tx_packet.flags.info_type == IT_UNDEFINED)
					{
						mx.tx_packet.flags.info_type = IT_WEAK_ZERO_MAP;
						break;
					}
				#endif

				// send weak zero map with every second transmission
				if (mx.stat_counter.num_sent & 1)
					mx.tx_packet.flags.info_type = IT_WEAK_ZERO_MAP;
			}
			while (0);
			#endif

			// send header
			while (p != &(mx.tx_packet.coding_vector[0]))
			{
				while (!(IFG1 & UTXIFG0));
				U0TXBUF = *p++;
			}
		}

		// send coding vector and payload
		{
			ASSERT_CT(offsetof(Packet, payload) ==
				offsetof(Packet, coding_vector) + sizeof(mx.tx_packet.coding_vector),
				inconsistent_program);

			// NOTE: we cast const away which is a bit dirty. We need this only to restore
			// sideload's packed version which is such a negligible change that we prefer
			// mx.tx_sideload to appear as const.
			uint8_t	*ps = (uint8_t*)mx.tx_sideload;

			#if MX_REQUEST

				int16_t help_index = mx.request.help_index;

				// if we are column request helper
				if ((help_index < 0) && (help_index > INT16_MIN))
				{
					help_index = -help_index - 1;

					if (!mx.tx_packet.is_ready ||
						!(((uint_fast_t*)p)[help_index / FAST_T_WIDTH] & mx.request.help_bitmask))
						ps = &(mx.matrix[help_index].coding_vector_8[0]);

					mx.request.last_update_slot = mx.slot_number + 1;
				}

				// if we are row request helper
				else if (help_index > 0)
				{
					help_index--;

					// NOTE: we don't have to check the packet because if there is one ready, then it
					// has been specifically build in response to the pending request. if the packet
					// is not ready, it is right to do the sideload anyway.
					// if (!mx.tx_packet.is_ready || (help_index < mx_get_leading_index(p)))
					{
						ps = &(mx.matrix[help_index].coding_vector_8[0]);
						mx.request.last_update_slot = mx.slot_number + 1;
					}
				}

			#endif

			// if sideload points to a matrix row: restore its packed version (in place)
			// NOTE: if it points to rx queue, the format is still packed
			// NOTE: we could also do this when we set mx.tx_sideload (i.e. at a less time critical
			// point), but it is very easy to forget about that. Hence we do it here to avoid
			// programming mistakes.
			// NOTE: the outer condition is resolved at compile time
			if (offsetof(Matrix_Row, payload_8) != offsetof(Matrix_Row, payload))
			{
				if ((uintptr_t)ps - (uintptr_t)&mx.matrix < sizeof(mx.matrix))
					wrap_chunk(ps);
			}

			if (!mx.tx_packet.is_ready)
			{
				assert(NULL != ps);

				write_tx_fifo(ps, sizeof(mx.tx_packet.coding_vector) + sizeof(mx.tx_packet.payload), NULL);
				p += sizeof(mx.tx_packet.coding_vector) + sizeof(mx.tx_packet.payload);

				#if MX_VERBOSE_PACKETS || MX_REQUEST
					gpi_memcpy_dma_aligned(mx.tx_packet.coding_vector, ps,
						(sizeof(mx.tx_packet.coding_vector) + sizeof(mx.tx_packet.payload) + 1) & ~1);

					// mark the packet as broken since it could be possible that we interrupt
					// prepare_tx_packet() right now, hence writing data back may damage the packet
					mx.tx_packet.is_valid = 0;
				#endif
			}
			else
			{
				#if MX_BENCHMARK_NO_SIDELOAD
					ps = NULL;
				#endif

				// following flag could be used to prohibit sideload dynamically
				// if (!mx.tx_packet.use_sideload)
				//	ps = NULL;

				// send coding vector with sideload and test if result is a zero packet;
				// if so: abort (below)
				// NOTE: another option might be to write some other useful information into the
				// packet instead of dropping it
				if (!write_tx_fifo(p, sizeof(mx.tx_packet.coding_vector), ps))
					p = NULL;
				else
				{
					p += sizeof(mx.tx_packet.coding_vector);
					ps += (NULL == ps) ? 0 : sizeof(mx.tx_packet.coding_vector);
					write_tx_fifo(p, sizeof(mx.tx_packet.payload), ps);
					p += sizeof(mx.tx_packet.payload);
				}
			}

//			GPI_TRACE_MSG_FAST(TRACE_VERBOSE,
//				"is_ready: %u, mx.tx_sideload: %p, ps: %p, mx.req.help_index: %d",
//				(int)mx.tx_packet.is_ready, mx.tx_sideload, ps, (int)mx.request.help_index);
//			GPI_TRACE_MSG_FAST(TRACE_VERBOSE,
//				"matrix: %p...%p, rx_queue: %p...%p",
//				&mx.matrix, &mx.matrix[NUM_ELEMENTS(mx.matrix)],
//				&mx.rx_queue, &mx.rx_queue[NUM_ELEMENTS(mx.rx_queue)]);
		}

		// send info vector
		#if MX_INCLUDE_INFO_VECTOR
		if (NULL != p)
		{
			ASSERT_CT(offsetof(Packet, info_vector) ==
				offsetof(Packet, payload) + sizeof(mx.tx_packet.payload),
				inconsistent_program);

			void *ps;

			switch (mx.tx_packet.flags.all & EIT_mask)
			{
				#if MX_REQUEST
					case EIT_ROW_MAP:
					case EIT_ROW_REQUEST:
						ps = &mx.request.my_row_mask[0];
						break;
					case EIT_COLUMN_REQUEST:
						ps = &mx.request.my_column_mask[0];
						break;
				#endif

				#if MX_SMART_SHUTDOWN_MAP
					case EIT_FULL_RANK_MAP:
					case EIT_FULL_RANK_MAP_RADIO_OFF:
					case EIT_FULL_RANK_ACK_MAP:
					case EIT_FULL_RANK_ACK_MAP_RADIO_OFF:
						ps = &mx.full_rank_map.hash[0];
						break;
				#endif

				default: switch (mx.tx_packet.flags.info_type)
				{

				#if MX_WEAK_ZEROS
					case IT_WEAK_ZERO_MAP:
						ps = &mx.weak_zero_map.weak_mask[0];
						break;
				#endif

					case IT_UNDEFINED:
						// use arbitrary but aligned data
						ps = &mx.tx_packet.coding_vector;
						break;

					default:
						assert(0);
						ps = &mx.tx_packet.coding_vector;
						break;
				}
			}

			write_tx_fifo(ps, sizeof(mx.tx_packet.info_vector), NULL);
			p += sizeof(mx.tx_packet.info_vector);

			#if MX_VERBOSE_PACKETS
				gpi_memcpy_dma_inline(mx.tx_packet.info_vector, ps, (sizeof(mx.tx_packet.info_vector) + 1) & ~1);
			#endif
		}
		#endif

		if (NULL != p)
		{
			while (p != (uint8_t*)&(mx.tx_packet.phy_payload_end))
			{
				while (!(IFG1 & UTXIFG0));
				U0TXBUF = *p++;
			}
		}

		while (!(U0TCTL & TXEPT));
		gpi_radio_cs_off();

		// if transmission aborted
		if (NULL == p || (U0RXBUF & BV(CC2420_TX_UNDERFLOW)))
		{
			gpi_radio_strobe(CC2420_SRFOFF);
			gpi_led_off(LED_TX);
			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
				s.radio_start_timestamp = 0;
			#endif

			gpi_radio_strobe(CC2420_SFLUSHTX);

			if (NULL == p)
			{
				#if MX_VERBOSE_STATISTICS
					mx.stat_counter.num_tx_zero_packet++;
				#endif
				GPI_TRACE_MSG_FAST(TRACE_INFO, "sideload produced zero-packet -> Tx aborted");
			}
			else
			{
				#if MX_VERBOSE_STATISTICS
					mx.stat_counter.num_tx_fifo_late++;
				#endif
				GPI_TRACE_MSG_FAST(TRACE_WARNING, "!!! TXFIFO underflow occured, Tx aborted -> check program, must not happen !!!");
				// TODO: assert?
			}
		}

		else
		{
			// unmask falling edge SFD IRQ
			TBCCTL1 &= ~(CM1 | CM0 | CCIFG);
			TBCCTL1 |= CM1 | CCIE;

			set_event(TX_READY);
			mx.stat_counter.num_sent++;

			GPI_TRACE_MSG_FAST(TRACE_INFO, "Tx started");
		}

		s.slot_state = TX_RUNNING;
		mx.tx_packet.is_ready = 0;
		mx.tx_sideload = NULL;
		s.next_slot_task = RX;

		PROFILE_ISR();
	}

	if (RESYNC != s.slot_state)
	{
		mx.slot_number++;
		set_event(SLOT_UPDATE);

		s.next_grid_tick += MX_SLOT_LENGTH + s.grid_drift / (GRID_DRIFT_FILTER_DIV * GRID_TICK_UPDATE_DIV);

		s.next_trigger_tick = s.next_grid_tick -
			((s.next_slot_task == TX) ? s.tx_trigger_offset : s.rx_trigger_offset);

		if (TX_RUNNING == s.slot_state)
			start_grid_timer();
	}

	GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "slot_state: %d, next_grid: %lu, rx_offset: %u",
		s.slot_state, (unsigned long)gpi_tick_hybrid_to_us(s.next_grid_tick),
		(unsigned int)gpi_tick_hybrid_to_us(s.rx_trigger_offset));

	// set general purpose trigger event
	// compared to SLOT_UPDATE, TRIGGER_TICK is generated also during RESYNC periods (once
	// in a while). It can be used for maintenance tasks that are less time critical.
	set_event(TRIGGER_TICK);

	PROFILE_ISR();
//	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

void mixer_transport_init()
{
	GPI_TRACE_FUNCTION();

	TACCTL2 = CM0 | CAP; 	// Capture on falling edge
	P1SEL |= BV(3);			// FIFO
							// Select peripheral module function (P1.3/TA2)

	DMACTL0 &= ~0x00FF;
	DMACTL0 |=  0x0031;		// DMA2TSEL0: TACCR2 CCIFG ; DMA2TSEL1: URXIFG0
	DMA0CTL = DMADSTBYTE | DMASRCBYTE;		// Select dst and src as a byte
	DMA0SA = (uint16_t)mx.rx_queue;			// dummy
	DMA0DA = (uint16_t)&U0TXBUF;

	DMA1CTL = DMADSTINCR1 | DMADSTINCR0 | DMADSTBYTE | DMASRCBYTE;		// increment dst address by 1 after each byte transfer
	DMA1SA = (uint16_t)&U0RXBUF;

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void mixer_transport_print_config(void)
{
	#define PRINT(s) printf("%-25s = %ld\n", #s, (long)s)

	PRINT(MAX_PROPAGATION_DELAY);
	PRINT(RX_WINDOW_INCREMENT);
	PRINT(RX_WINDOW_MIN);
	PRINT(RX_WINDOW_MAX);
	PRINT(GRID_DRIFT_FILTER_DIV);
	PRINT(GRID_TICK_UPDATE_DIV);
	PRINT(GRID_DRIFT_MAX);
	PRINT(TX_OFFSET_FILTER_DIV);
	PRINT(TX_OFFSET_MAX);
	PRINT(DRIFT_TOLERANCE);
	PRINT(GRID_TO_SFD_OFFSET);
	PRINT(PACKET_AIR_TIME);
	printf("%-25s = %" PRId32 "\n", "air bytes (excl. pre + adr)", (int32_t)(offsetof(Packet, phy_payload_end) + 1)); // +1 because offsets begin at 0
	printf("%-25s = %" PRId32 "\n", "min. slot length",
		(int32_t)(PACKET_AIR_TIME + MAX(TX_OFFSET_MAX, RX_TO_GRID_OFFSET + RX_WINDOW_MAX) + GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER) + GPI_TICK_US_TO_HYBRID(1000000 / GPI_SLOW_CLOCK_RATE + 30)));

	#undef PRINT
}

//**************************************************************************************************

void mixer_transport_arm_initiator()
{
	GPI_TRACE_FUNCTION();

	s.grid_drift			= 0;
	s.grid_drift_cumulative = 0;
	s.tx_trigger_offset 	= TX_TO_GRID_OFFSET;
	s.rx_trigger_offset 	= RX_TO_GRID_OFFSET + RX_WINDOW_MAX / 2;

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void mixer_transport_start()
{
	GPI_TRACE_FUNCTION_FAST();

	GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "start grid timer");

	if (mx.tx_sideload)		// if initiator
		enter_resync(2);
	else enter_resync(1);

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

int mixer_transport_set_next_slot_task(Slot_Activity next_task)
{
	GPI_TRACE_FUNCTION_FAST();

	// if next_task == RX: done
	// NOTE: it is the automatic standard selection at the beginning of each slot (with the
	// exception of RESYNC), so we can save the effort. Besides that, DMA ISR is allowed to
	// select TX during start-up phase -> don't overwrite that.
	if (RX != next_task)
	{
		Gpi_Hybrid_Reference	r;

		// compute a deadline such that we can check if we are too close to the grid trigger tick
		// NOTE: we have to cover three terms:
		// - the activity-to-grid offset, which is dependent on the selected task
		// - the ISR_LATENCY_BUFFER
		// - the execution time needed for the code block within the gpi_int_lock range below
		// Additionally, hybrid_tick returned by gpi_tick_hybrid_reference() may lie up to one
		// slow tick -- i.e. 1000000 / GPI_SLOW_CLOCK_RATE microseconds -- in the past.
		r = gpi_tick_hybrid_reference();
		r.hybrid_tick = s.next_trigger_tick - r.hybrid_tick;
		r.hybrid_tick -=
			MAX(TX_OFFSET_MAX, RX_TO_GRID_OFFSET + RX_WINDOW_MAX) +
			GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER) +
			GPI_TICK_US_TO_HYBRID(1000000 / GPI_SLOW_CLOCK_RATE + 100);
		r.hybrid_tick *= FAST_HYBRID_RATIO;
		if (r.hybrid_tick >= 0x8000)
			r.fast_capture += 0x7FFF;
		else r.fast_capture += r.hybrid_tick;

		int ie = gpi_int_lock();
		REORDER_BARRIER();

		if ((RESYNC != s.slot_state) || (STOP == next_task))
		{
			if ((int16_t)(r.fast_capture - gpi_tick_fast_native()) < 0)
				next_task = -1;
			else
			{
				// NOTE: next_task == TX or STOP at this point

				gpi_led_toggle(LED_UPDATE_TASK);

				s.next_slot_task = next_task;
				s.next_trigger_tick = s.next_grid_tick - s.tx_trigger_offset;	// also ok for STOP

				// if IDLE or RESYNC (RESYNC only if next_task == STOP)
				if (RX_RUNNING != s.slot_state)
					start_grid_timer();

				gpi_led_toggle(LED_UPDATE_TASK);
			}
		}

		REORDER_BARRIER();
		gpi_int_unlock(ie);
	}

	if (-1 == next_task)
	{
		GPI_TRACE_MSG(TRACE_WARNING, "!!! WARNING: rx/tx decision was late -> check program, should not happen !!!");
		GPI_TRACE_RETURN(0);
	}
	else
	{
		GPI_TRACE_MSG(TRACE_INFO, "next slot task: %s", (next_task == TX) ? "TX" : ((next_task == RX) ? "RX" : "STOP"));
		GPI_TRACE_RETURN(1);
	}
}

//**************************************************************************************************

Slot_Activity mixer_transport_get_next_slot_task()
{
	// ATTENTION: function might be called with interrupts locked, so keep it fast

	return s.next_slot_task;
}

//**************************************************************************************************
//**************************************************************************************************
