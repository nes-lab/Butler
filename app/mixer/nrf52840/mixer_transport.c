/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2019, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					nrf52840/mixer_transport.c
 *
 *	@brief					Mixer transport layer for Nordic nRF52840
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

	ATTENTION: Product Specification (4413_417 v1.0) is inconsistent: In Fig. 123 FRAMESTART
	is triggered after reception of PHR while the text says that it is generated after SFD.
	We assume that it gets triggered after PHR, not SFD, for the following reasons:
	- On page 296 it says "Frames with zero length will be discarded, and the FRAMESTART
	  event will not be generated in this case." This behavior would be impossible with
	  FRAMESTART triggered before PHR.
	- Bitcounter connected to FRAMESTART (with shortcut) and BCC set to
	  (1 (PHR) + payload size + 2 (CRC) ) * 8 does not trigger BCCMATCH at the end of a valid
	  frame, without much doubt because Bitcounter starts to count after PHR, not before.

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
#include "../armv7-m/memxor.h"

#include "gpi/tools.h"
#include "gpi/platform.h"
#include "gpi/clocks.h"
#include "gpi/interrupts.h"
#include "gpi/olf.h"
#include GPI_PLATFORM_PATH(radio.h)

#include <nrf.h>

#include <stdio.h>
#include <inttypes.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************

#if MX_VERBOSE_PROFILE
	GPI_PROFILE_SETUP("mixer_transport.c", 1700, 4);
#endif

#define PROFILE_ISR(...)	PROFILE_P(0, ## __VA_ARGS__)

//**************************************************************************************************

ASSERT_CT_STATIC(MX_PHY_MODE == IEEE_802_15_4, MX_PHY_MODE_invalid);

ASSERT_CT_STATIC(IS_POWER_OF_2(FAST_HYBRID_RATIO), unefficient_FAST_HYBRID_RATIO);
//ASSERT_CT_STATIC(IS_POWER_OF_2(HYBRID_SLOW_RATIO), unefficient_HYBRID_SLOW_RATIO);

//**************************************************************************************************
// timing parameters

// NOTE: a drift tolerance of 300 ppm (150 ppm on each side) should be a comfortable choice
// (typical clock crystals have < 20...50 ppm at 25°C and temperature coefficient < 0.04 ppm/K)
// note: MIN(2500, ...) is effective in case of very long slots (up to seconds). It reduces the
// tolerance to avoid that RX_WINDOW overflows too fast (or even immediately).
#define DRIFT_TOLERANCE			MIN(2500, MAX((MX_SLOT_LENGTH + 999) / 1000, 1))	// +/- 1000 ppm

#define MAX_PROPAGATION_DELAY	GPI_TICK_US_TO_HYBRID(2)

#define PACKET_AIR_TIME			GPI_TICK_US_TO_HYBRID2((6 + PHY_PAYLOAD_SIZE + 2) * 32)
// NOTE: Comparing FRAMESTART events of transmitter and receiver shows ~40us time delta. It seems
// like this time delta splits evenly among the transmitter and receiver path. The delta is ~10us for
// BLE 1M and ~5us for BLE 2M, which corresponds to about 10 bit times in each mode.
#define GRID_TO_EVENT_OFFSET	GPI_TICK_US_TO_HYBRID(6 * 32 + 20)		// preamble + SFD + PHR + event signaling latency
// NOTE: 40us ramp up time in fast mode, 130us in default mode (RADIO_MODECNF0_RU).
#define RX_TO_GRID_OFFSET		(0 + GPI_TICK_US_TO_HYBRID(40))		// software latency + RX ramp up time
// TODO: check 16
#define TX_TO_GRID_OFFSET		(16 + GPI_TICK_US_TO_HYBRID(40))		// software latency + TX ramp up time

// #define RX_WINDOW_INCREMENT		(2 * DRIFT_TOLERANCE)			// times 2 is important to widen the window in next slot (times 1 would follow only)
// #define RX_WINDOW_INCREMENT		GPI_TICK_US_TO_HYBRID(10)
#define RX_WINDOW_INCREMENT		GPI_TICK_US_TO_HYBRID(10)
// #define RX_WINDOW_MAX			MIN(0x7FFFFFFF, MIN(15 * RX_WINDOW_INCREMENT, (MX_SLOT_LENGTH - PACKET_AIR_TIME - RX_TO_GRID_OFFSET) / 2))
// #define RX_WINDOW_MIN			MIN(RX_WINDOW_MAX / 2, MAX(2 * RX_WINDOW_INCREMENT, GPI_TICK_US_TO_HYBRID(1)))		// minimum must cover variations in execution time from timer polling to RX on
// #define RX_WINDOW_MAX			GPI_TICK_US_TO_HYBRID(160)
// #define RX_WINDOW_MIN			GPI_TICK_US_TO_HYBRID(100)
#define RX_WINDOW_MAX			GPI_TICK_US_TO_HYBRID(160)
#define RX_WINDOW_MIN			GPI_TICK_US_TO_HYBRID(60)
// #define RX_WINDOW_MAX			GPI_TICK_US_TO_HYBRID(120)
// #define RX_WINDOW_MIN			GPI_TICK_US_TO_HYBRID(20)

#define GRID_DRIFT_FILTER_DIV	4
#define GRID_TICK_UPDATE_DIV	2
#define GRID_DRIFT_MAX			MIN(3 * DRIFT_TOLERANCE * GRID_TICK_UPDATE_DIV * GRID_DRIFT_FILTER_DIV, 0x7FFFFF)
	// P control loop has a permanent deviation since control_value = drift / GRID_TICK_UPDATE_DIV
	// -> to be able to compensate for drift up to DRIFT_TOLERANCE, GRID_DRIFT_MAX must be > DRIFT_TOLERANCE * GRID_TICK_UPDATE_DIV
	// Additional to this, the control loop doesn't get an new measurement in every slot, so the single-value
	// measurement may exceed the theoretical limit. Therefore we add a margin factor.
#define TX_OFFSET_FILTER_DIV	2
#define TX_OFFSET_MAX			(2 * MAX_PROPAGATION_DELAY + GPI_TICK_US_TO_HYBRID(2))

#define ISR_LATENCY_BUFFER		20		// in microseconds
	// influencing factors:
	// - interrupts locked on thread level
	// - interrupt latency for grid timer ISR
	// - rare number of exceedings is acceptable (can be measured with MX_VERBOSE_PROFILE and stat_counter.num_rx_late/num_tx_late)

//ASSERT_CT_STATIC((RX_WINDOW_MAX / RX_WINDOW_INCREMENT) >= 4, critical_RX_WINDOW_settings);

//**************************************************************************************************

#if   (0 == GPI_FAST_CLOCK_NRF_TIMER)
	#define MAIN_TIMER				NRF_TIMER0
	#define MAIN_TIMER_IRQ			TIMER0_IRQn
	#define MAIN_TIMER_ISR_NAME		TIMER0_IRQHandler
#elif (1 == GPI_FAST_CLOCK_NRF_TIMER)
	#define MAIN_TIMER				NRF_TIMER1
	#define MAIN_TIMER_IRQ			TIMER1_IRQn
	#define MAIN_TIMER_ISR_NAME		TIMER1_IRQHandler
#elif (2 == GPI_FAST_CLOCK_NRF_TIMER)
	#define MAIN_TIMER				NRF_TIMER2
	#define MAIN_TIMER_IRQ			TIMER2_IRQn
	#define MAIN_TIMER_ISR_NAME		TIMER2_IRQHandler
#elif (3 == GPI_FAST_CLOCK_NRF_TIMER)
	#define MAIN_TIMER				NRF_TIMER3
	#define MAIN_TIMER_IRQ			TIMER3_IRQn
	#define MAIN_TIMER_ISR_NAME		TIMER3_IRQHandler
#elif (4 == GPI_FAST_CLOCK_NRF_TIMER)
	#define MAIN_TIMER				NRF_TIMER4
	#define MAIN_TIMER_IRQ			TIMER4_IRQn
	#define MAIN_TIMER_ISR_NAME		TIMER4_IRQHandler
#else
	#error GPI_FAST_CLOCK_NRF_TIMER is invalid
#endif

#define MAIN_TIMER_CC_INDEX			3
#define MAIN_TIMER_CC_REG			(MAIN_TIMER->CC[MAIN_TIMER_CC_INDEX])
#define EVENT_CAPTURE_CC_INDEX		2
#define EVENT_CAPTURE_CC_REG		(MAIN_TIMER->CC[EVENT_CAPTURE_CC_INDEX])

#define NRF_PPI_CHANNEL				19	// timer <-> radio (start, SFD capture, ...)
#define NRF_PPI_CHANNEL_2			18	// FRAMESTART -> Bitcounter

//**************************************************************************************************

#if MX_VERBOSE_STATISTICS
	#define LED_ISR(name, led)									\
		name ## _ ();											\
		void name() {											\
			if (gpi_wakeup_event) {								\
				mx.wake_up_timestamp = gpi_tick_fast_native();	\
				gpi_wakeup_event = 0;							\
			}													\
			gpi_led_toggle(led);								\
			name ## _ ();										\
			gpi_led_toggle(led);								\
		}														\
		void name ## _ ()
#else
	#define LED_ISR(name, led)									\
		name ## _ ();											\
		void name() {											\
			gpi_led_toggle(led);								\
			name ## _ ();										\
			gpi_led_toggle(led);								\
		}														\
		void name ## _ ()
#endif

//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************

typedef enum Slot_State_tag
{
	RESYNC		= 0,
	RX_RUNNING	= 16,
	TX_RUNNING	= 12,
	IDLE		= 12,	// ATTENTION: TX and IDLE are not distinguished here

} Slot_State;

//**************************************************************************************************
//***** Forward Declarations ***********************************************************************

void	timeout_isr();
void	grid_timer_isr();

//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

static struct
{
	Gpi_Hybrid_Tick			event_tick_nominal;
	Gpi_Hybrid_Tick			next_grid_tick;
	Gpi_Hybrid_Tick			next_trigger_tick;
	int32_t					grid_drift;
	int32_t					grid_drift_cumulative;
	uint32_t				rx_trigger_offset;
	uint32_t				tx_trigger_offset;

	Packet					tx_fifo;

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

// trigger grid/timeout timer (immediately)
static inline void trigger_main_timer(int use_int_lock)
{
	register int	tmp, ie;

	if (use_int_lock)
		ie = gpi_int_lock();

	// use asm to safeguard time-critical code
	// 		MAIN_TIMER->TASKS_CAPTURE[MAIN_TIMER_CC_INDEX] = 1;
	// 		MAIN_TIMER->CC[MAIN_TIMER_CC_INDEX] += 3;
	// ATTENTION: some callees rely on the fact that CC_REG gets updated implicitly
	// ATTENTION: Choosing the increment value is difficult due to missing details in the
	// documentation. We found that values < 8 can cause problems (missed events), while
	// values >= 8 seem to work reliable. We use 10 to have some safety margin.
	// NOTE: An alternative could be to trigger the IRQ via NVIC->STIR. Though, one has to
	// check if the masking/unmasking is consistent when implementing it this way.
	__asm__ volatile
	(
		"str	%0, [%1, %2]	\n"
		"ldr	%0, [%1, %3]	\n"
		"add	%0, #10			\n"
		"str	%0, [%1, %3]	\n"
		: "=&r"(tmp)
		: "r"(MAIN_TIMER),
			"i"(offsetof(typeof(*MAIN_TIMER), TASKS_CAPTURE[MAIN_TIMER_CC_INDEX])),
			"i"(offsetof(typeof(*MAIN_TIMER), CC[MAIN_TIMER_CC_INDEX])),
			"0"(1)
		: "memory"
	);

	if (use_int_lock)
		gpi_int_unlock(ie);

// TODO: if helpful: wait until compare time has been reached
// TODO: remove intlock if not really needed (calls from thread-level are allowed only at defined points)
}

//**************************************************************************************************

static inline void unmask_main_timer(int clear_pending)
{
	if (clear_pending)
	{
		MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX] = 0;
		NVIC_ClearPendingIRQ(MAIN_TIMER_IRQ);
	}

	MAIN_TIMER->INTENSET = 0x10000 << MAIN_TIMER_CC_INDEX;
}

static inline void mask_main_timer()
{
	MAIN_TIMER->INTENCLR = 0x10000 << MAIN_TIMER_CC_INDEX;
}

//**************************************************************************************************

// ATTENTION: to be called from ISRs only
static inline __attribute__((always_inline)) void set_event(Event event)
{
	// ATTENTION: use API function to ensure that load/store exclusive works right (if used)
	gpi_atomic_set(&(mx.events), BV(event));
//	mx.events |= BV(event);
}

//**************************************************************************************************

static inline unsigned int write_tx_fifo(unsigned int pos, const void *p1, const void *p2, int size)
{
	uint32_t		*dest = (uint32_t*)((uint8_t*)&s.tx_fifo + pos);
	uint32_t		or_data = 0;
	register int	tmp1, tmp2;

	// NOTE: We do not assume that dest, p1, p2, or size is aligned because we want to support
	// unaligned values. Fortunately Cortex-M4 supports unaligned accesses (with corresponding
	// performance penalty), so we do not need to handle them explicitly.

	__asm__ volatile
	(
		".align 2				\n"
		"1:						\n"
		"orr.w	%3, %5			\n"
		"ldr.w	%5, [%1], #4	\n"
		"cmp.n	%2, #0			\n"
		"itt	ne				\n"
			// NOTE: cmp + itt seems more wasteful than it is because itt gets folded on cmp,
			// i.e. it executes in 0 cycles. Hence, cmp + itt is same/less expensive than cbz
			// (if branch not taken / taken).
		"ldrne	%6, [%2], #4	\n"
		"eorne	%5, %6			\n"
		"str	%5, [%0], #4	\n"
		"subs	%4, #4			\n"
		"bgt	1b				\n"

		// following instructions ensure that or_data is not influenced by padding bytes
		// in case size is unaligned
		"adds	%4, #4			\n"
		"lsl	%4, 3			\n"
		"mov	%6, #-1			\n"
		"lsl	%6, %4			\n"
		"bic	%5, %6			\n"
		"orr	%3, %5			\n"

		: "+&r"(dest), "+&r"(p1), "+&r"(p2), "+&r"(or_data), "+&r"(size), "=&r"(tmp1), "=&r"(tmp2)
		: "5"(0)
		: "cc", "memory"
	);

	// return value can be used to check if the data written to the FIFO was all-zeros
	return or_data;
}

//**************************************************************************************************

static void start_grid_timer()
{
//	GPI_TRACE_FUNCTION_FAST();

#if GPI_HYBRID_CLOCK_USE_VHT
	// NOTE: MAX_TB_INTERVAL could be set to the largest possible value which can be covered by
	// fast clock. However, the time is based on hybrid clock, which means that the accuracy
	// decreases with longer (purely) fast clock intervals. So we choose a compromise.
	const Gpi_Hybrid_Tick	MAX_TB_INTERVAL = GPI_TICK_US_TO_HYBRID(500);
#endif

	Gpi_Hybrid_Reference	r;
	Gpi_Hybrid_Tick			d, t;

	r = gpi_tick_hybrid_reference();
	d = s.next_trigger_tick - r.hybrid_tick;

	t = s.next_trigger_tick - GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER);
#if GPI_HYBRID_CLOCK_USE_VHT
	// cover potential execution time from getting r to writing capture reg
	t -= GPI_TICK_US_TO_HYBRID(1000000 / GPI_SLOW_CLOCK_RATE + 50);
	#error check value (GPI_HYBRID_CLOCK_USE_VHT is not fully implemented / tested)
#endif

//	GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "d: %lu", (long)d);

	// if we are late
	if (gpi_tick_compare_hybrid(r.hybrid_tick, t) >= 0)
	{
		// trigger grid timer immediately
		// NOTE: capture register is updated implicitly
		trigger_main_timer(0);
		unmask_main_timer(0);
#if GPI_HYBRID_CLOCK_USE_VHT
		mask_slow_timer();
#endif

		#if MX_VERBOSE_STATISTICS
			mx.stat_counter.num_grid_late++;
		#endif
	}

	// else if trigger tick is in reach for fast timer
#if GPI_HYBRID_CLOCK_USE_VHT
	else if (d <= MAX_TB_INTERVAL)
#else
	else
#endif
	{
		MAIN_TIMER_CC_REG = r.fast_capture + (d * FAST_HYBRID_RATIO) - GPI_TICK_US_TO_FAST(ISR_LATENCY_BUFFER);
		unmask_main_timer(1);
#if GPI_HYBRID_CLOCK_USE_VHT
		mask_slow_timer();
#endif
	}

	// else if trigger tick is far away
#if GPI_HYBRID_CLOCK_USE_VHT
	else
	{
		#error GPI_HYBRID_CLOCK_USE_VHT is not implemented

/*		ASSERT_CT(HYBRID_SLOW_RATIO <= 0x10000);

		if (d > 0xF000ul * HYBRID_SLOW_RATIO)
			d = r.hybrid_tick + 0xE000ul * HYBRID_SLOW_RATIO;
		else d = s.next_trigger_tick - MAX_TB_INTERVAL / 2;

		SLOW_TIMER_CC_REG = d / HYBRID_SLOW_RATIO;
		unmask_slow_timer();

		// stop main timer
		// ATTENTION: since main timer is jointly used as grid and timeout timer, a caller to
		// start_grid_timer() may expect that main timer is reinitialized in every case. Hence,
		// we do it here for sure.
		mask_main_timer();
*/	}
#endif

	s.slot_state = (RESYNC == s.slot_state) ? RESYNC : IDLE;

//	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************
#if GPI_HYBRID_CLOCK_USE_VHT

#error GPI_HYBRID_CLOCK_USE_VHT is not implemented
/*
// helper ISR for grid timer, see start_grid_timer() for details
void ..._isr()
{
	mask_slow_timer();

	start_grid_timer();
}
*/
#endif
//**************************************************************************************************

// mode: 0 = RESYNC quick, 1 = RESYNC normal, 2 = start Tx (for initiator)
static inline __attribute__((always_inline)) void enter_resync(int mode)
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
	// NOTE: MAIN_TIMER_CC_REG is updated implicitly
	trigger_main_timer(1);

	// if quick (and dirty): don't update s.next_grid_tick to save time
	// ATTENTION: this expects s.next_grid_tick to be initialized and causes the first RESYNC timeout
	// interval to be up to +/- MX_SLOT_LENGTH longer. Furthermore it saves a REORDER_BARRIER(), so
	// don't use quick with interrupts unlocked.
	if (0 != mode)
	{
		// init s.next_grid_tick to precise value
		// ATTENTION: this is important to align the initiator exactly at the grid
		// NOTE: MAIN_TIMER_CC_REG + buffer (see below) is the trigger tick. Grid timer ISR will
		// assume + ISR_LATENCY_BUFFER as polling interval. Hence,
		// grid tick = MAIN_TIMER_CC_REG + buffer + ISR_LATENCY_BUFFER + TX_TO_GRID_OFFSET.
		// NOTE: if mode == 1, the timing is not critical since we enter RESYNC mode
		s.next_grid_tick =
			gpi_tick_fast_to_hybrid(MAIN_TIMER_CC_REG) +
			GPI_TICK_US_TO_HYBRID(20) +
			GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER) +
			TX_TO_GRID_OFFSET;

		// add some time buffer covering the remaining execution time from here until interrupt
		// gets unlocked
		// ATTENTION: don't do that before calling gpi_tick_fast_to_hybrid() because then
		// gpi_tick_fast_to_hybrid() would work on a future value.
		MAIN_TIMER_CC_REG += GPI_TICK_US_TO_FAST(20);

		REORDER_BARRIER();
		__DMB();
	}

	// unmask grid timer interrupt
	unmask_main_timer(0);
}

//**************************************************************************************************

void LED_ISR(RADIO_IRQHandler, LED_RADIO_ISR)
{
	GPI_TRACE_FUNCTION_FAST();
	PROFILE_ISR("radio ISR entry");

	// if Rx
	// NOTE: s.slot_state = RX_RUNNING or RESYNC
	if (TX_RUNNING != s.slot_state)
	{
		// if frame detected
		if (NRF_RADIO->EVENTS_FRAMESTART && BV_TEST_BY_VALUE(NRF_RADIO->INTENSET, RADIO_INTENSET_FRAMESTART, 1))
		{
			gpi_led_on(LED_RX);
			GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "header detected");

			NRF_RADIO->INTENCLR = BV_BY_VALUE(RADIO_INTENCLR_FRAMESTART, 1);

			// NOTE: We leave EVENTS_FRAMESTART set. Together with INTEN_FRAMESTART,
			// this can be used to evaluate the Rx state:
			// EVENT  INTEN  state
			//   0      0    - (radio off)
			//   0      1    idle, scanning for sync header
			//   1      1    FRAMESTART received, IRQ pending
			//   1      0    FRAMESTART received and processed, DMA running

			volatile uint8_t *len = (uint8_t*)(NRF_RADIO->PACKETPTR);

			// wait for PHR (LEN field)
			// NOTE: Theoretically it is available for sure since FRAMESTART already appeared (see
			// our general comment on FRAMESTART). We test it to be absolutely sure, e.g. in case
			// there is a delay in the DMA transport.
			// NOTE: len has been cleared when starting Rx (in grid timer ISR)
			while (!(*len) && BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, Rx));

			// if LEN is valid
			if (*len == (PHY_PAYLOAD_SIZE + 2))
			{
				// update timeout timer to monitor data transfer
				// NOTE: times 33 incl. tolerance for clock drift, + 10 for DMA latency
				MAIN_TIMER_CC_REG =
					EVENT_CAPTURE_CC_REG +
					GPI_TICK_US_TO_FAST2((PHY_PAYLOAD_SIZE + 2) * 33 + 10);
				unmask_main_timer(1);
			}

			// if LEN is invalid
			else
			{
				GPI_TRACE_MSG_FAST(TRACE_INFO, "broken packet, LEN: %u", (int)*len);

				#if MX_VERBOSE_STATISTICS
					mx.stat_counter.num_rx_broken++;
				#endif

				// trigger timeout timer (immediately) -> do error handling in its ISR
				trigger_main_timer(0);
			}

			goto _return_;
		}

		// situation at this point: Rx done, radio entering DISABLED state
		// ATTENTION: turning radio off (here: due to shortcut / PPI) is important
		// to avoid disturbing receptions and to safe energy
		// NOTE: We could be here because Bitcounter triggered (i.e., LEN exceeds limit).
		// Since LEN is valid also in this case and BCMATCH triggers RADIO->TASKS_DISABLE
		// (via PPI link), there is no reason to distinguish this case here.

		gpi_led_off(LED_RX);

		// mask radio interrupts and events
		NRF_RADIO->INTENCLR = -1u;
		NRF_PPI->CHENCLR = BV(NRF_PPI_CHANNEL) | BV(NRF_PPI_CHANNEL_2);

		// stop timeout timer
		// -> not needed because this is done implicitely below
		// mask_main_timer();

		#if MX_VERBOSE_STATISTICS
//		if (s.radio_start_timestamp & 1)
		{
			mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
			s.radio_start_timestamp = 0;
		}
		#endif

		Packet	*packet = (Packet*)(NRF_RADIO->PACKETPTR - offsetof(Packet, len));
//		packet = &mx.rx_queue[mx.rx_queue_num_written % NUM_ELEMENTS(mx.rx_queue)];

		// if LEN or CRC not ok: regard packet as invisible
		if ((packet->len != (PHY_PAYLOAD_SIZE + 2)) ||
			BV_TEST_BY_NAME(NRF_RADIO->CRCSTATUS, RADIO_CRCSTATUS_CRCSTATUS, CRCError))
		{
			GPI_TRACE_MSG_FAST(TRACE_INFO, "broken packet received, LEN: %u, CRC: %u",
				(int)(packet->len), NRF_RADIO->CRCSTATUS);

			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_rx_broken++;
			#endif

			// trigger timeout timer (immediately) -> do error handling there
			// NOTE: don't need to unmask timer here because it already is
			trigger_main_timer(0);

			// invalidate len (in case of CRC errors) to avoid misleading interpretation
			// of timeout reason in timeout ISR
			packet->len = 0;
		}

		// if packet ok: process packet
		else
		{
			PROFILE_ISR("radio ISR process Rx packet begin");

			int	strobe_resync = 0;

			GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "CRC ok");

			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_rx_success++;
			#endif

			// update slot timing control values
			{
				Gpi_Hybrid_Tick	event_tick = gpi_tick_fast_to_hybrid(EVENT_CAPTURE_CC_REG);

				ASSERT_CT(sizeof(Gpi_Hybrid_Tick) >= sizeof(uint32_t));

				GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "SFD event: nominal: %lu, capture: %lu, deviation: %+ld",
					(long)gpi_tick_hybrid_to_us(s.event_tick_nominal),
					(long)gpi_tick_hybrid_to_us(event_tick),
					gpi_tick_compare_hybrid(event_tick, s.event_tick_nominal) < 0 ?
						-(long)gpi_tick_hybrid_to_us(s.event_tick_nominal - event_tick) :
						(long)gpi_tick_hybrid_to_us(event_tick - s.event_tick_nominal)
				);

				mx.ref_time = event_tick;
				mx.ref_slot = packet->slot_number;

				// if RESYNC requested: realign slot grid based on capture value
				if (RESYNC == s.slot_state)
				{
					s.next_grid_tick = event_tick - GRID_TO_EVENT_OFFSET + MX_SLOT_LENGTH;

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

					// compute SFD event deviation
					// NOTE: result is bounded by Rx window size
					event_tick -= s.event_tick_nominal;

					// keep Rx window in this range (with some margin)
					s.rx_trigger_offset = RX_WINDOW_MIN;
					gd = (uint32_t)MAX(ABS(s.grid_drift / GRID_DRIFT_FILTER_DIV), ABS((int32_t)event_tick));
					if (s.rx_trigger_offset < (uint32_t)gd + RX_WINDOW_INCREMENT)
						s.rx_trigger_offset = (uint32_t)gd + RX_WINDOW_INCREMENT;
					s.rx_trigger_offset += RX_TO_GRID_OFFSET;

					// restore nominal grid tick (i.e. remove previously added control value)
					s.next_grid_tick -= s.grid_drift / (GRID_DRIFT_FILTER_DIV * GRID_TICK_UPDATE_DIV);

					// update grid drift:
					// new = 1/c * measurement + (c-1)/c * old = old - 1/c * old + 1/c * measurement
					// + GRID_DRIFT_FILTER_DIV / 2 leads to rounding
					s.grid_drift -= (s.grid_drift + GRID_DRIFT_FILTER_DIV / 2) / GRID_DRIFT_FILTER_DIV;
					gd = s.grid_drift;
					gd += (int32_t)event_tick;
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
						GPI_TRACE_MSG_FAST(TRACE_INFO, "grid drift overflow: %d > %d -> enter RESYNC",
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
						if ((int32_t)s.tx_trigger_offset < 0)
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
					(long)gpi_tick_hybrid_to_us(s.next_grid_tick), s.grid_drift,
					(s.grid_drift >= 0) ?
						(int)gpi_tick_hybrid_to_us(s.grid_drift / GRID_DRIFT_FILTER_DIV) :
						-(int)gpi_tick_hybrid_to_us(-s.grid_drift / GRID_DRIFT_FILTER_DIV)
				);
			}

			// check potential queue overflow, if ok: keep packet
			if (mx.rx_queue_num_writing - mx.rx_queue_num_read < NUM_ELEMENTS(mx.rx_queue))
			{
				mx.rx_queue_num_written++;

				// use packet as next Tx sideload (-> fast tx update)
				if (MX_GENERATION_SIZE != mx.rank)
					mx.tx_sideload = &(packet->coding_vector[0]);

				#if (!MX_COORDINATED_TX && !MX_REQUEST)
					// NOTE: We have to set the RX_READY event for packet processing here in case
					// it will not be done on the processing layer.
					set_event(RX_READY);
				#endif

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

			PROFILE_ISR("radio ISR process Rx packet end");
		}
	}

	// if Tx
	else
	{
		// mask radio interrupts and events
		NRF_RADIO->INTENCLR = -1u;

		// situation at this point: transmission completed, radio entering DISABLED state
		// NOTE: radio is turned off automatically due to shortcut

		gpi_led_off(LED_TX);

		#if MX_VERBOSE_STATISTICS
		// if (s.radio_start_timestamp & 1)
		{
			mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
			s.radio_start_timestamp = 0;
		}
		#endif

		GPI_TRACE_MSG_FAST(TRACE_INFO, "Tx done");
	}

	_return_:
	PROFILE_ISR("radio ISR return");
	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// grid/timeout timer IRQ dispatcher
// ATTENTION: must be subordinated to gpi fast clock ISR if same timer is used and gpi ISR present
void __attribute__((naked)) MAIN_TIMER_ISR_NAME()
{
	// asm block implements optimized version of the following behavior:
	// switch (s.slot_state)
	// {
	//	case RESYNC:			timeout_isr(); grid_timer_isr();	break;
	//	case RX_RUNNING:		timeout_isr();						break;
	//	case TX_RUNNING, IDLE:	grid_timer_isr();					break;
	//	default:				undefined behavior
	// }
	__asm__ volatile
	(
		"ldr	r0, 1f				\n"		// r0 = s.slot_state
		"ldrb	r0, [r0]			\n"
		"add	pc, r0				\n"		// jump into vector table (see ARM DUI 0553A for details)
		".align 2					\n"		// ensure alignment and correct offset
		"push.w	{lr}				\n"		//  0: resync: call both ISRs (-> return to here)
		"bl.w	timeout_isr			\n"
		"pop.w	{lr}				\n"
		"b.w	grid_timer_isr		\n"		// 12: grid timer (don't return to here)
		"b.w	timeout_isr			\n"		// 16: timeout (don't return to here)
		"1:							\n"
		".word	%c0					\n"
		: : "i"(&s.slot_state)
	);
}

// don't include ASSERTS into naked function to avoid problems
// (in case non-optimized compilation generates code for them)
ASSERT_CT_STATIC(RESYNC     ==  0, definition_of_RESYNC_does_not_match_assembly_code);
ASSERT_CT_STATIC(RX_RUNNING == 16, definition_of_RX_RUNNING_does_not_match_assembly_code);
ASSERT_CT_STATIC(TX_RUNNING == 12, definition_of_TX_RUNNING_does_not_match_assembly_code);
ASSERT_CT_STATIC(IDLE       == 12, definition_of_IDLE_does_not_match_assembly_code);

//**************************************************************************************************

// timeout ISR
// triggered if there was no successful packet transfer in a specific time interval
void LED_ISR(timeout_isr, LED_TIMEOUT_ISR)
{
	GPI_TRACE_FUNCTION_FAST();
	PROFILE_ISR("timeout ISR entry");

	// NOTE: being here implies that Rx is active (state = RESYNC or RX_RUNNING)

	#if MX_VERBOSE_STATISTICS
		int frame_running =
				NRF_RADIO->EVENTS_FRAMESTART &&
				BV_TEST_BY_VALUE(NRF_RADIO->INTENSET, RADIO_INTENSET_FRAMESTART, 0);
	#endif

	// mask interrupts and events
	// NOTE: stopping timeout timer is not needed since this is done implicitely below
	NRF_RADIO->INTENCLR = -1u;
	NRF_PPI->CHENCLR = BV(NRF_PPI_CHANNEL) | BV(NRF_PPI_CHANNEL_2);

	// turn radio off
	if (!BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, RxDisable))
		NRF_RADIO->TASKS_DISABLE = 1;

	// ATTENTION: it can take up to four clock cycles before INTENCLR from above becomes effective
	// (see ARM Cortex-M4 Generic User Guide). To avoid the need for an explicit delay before
	// clearing the core-internal pending state we just insert some code inbetween.
	NVIC_ClearPendingIRQ(RADIO_IRQn);

	gpi_led_off(LED_RX | LED_TX);

	#if MX_VERBOSE_STATISTICS
	if (s.radio_start_timestamp & 1)
	{
		mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
		s.radio_start_timestamp = 0;
	}
	#endif

	if (s.slot_state != RESYNC)
	{
		// update stat counters only on real timeouts
		#if MX_VERBOSE_STATISTICS
			if (!frame_running)
				mx.stat_counter.num_rx_timeout++;
			else if (*(uint8_t*)(NRF_RADIO->PACKETPTR) == (PHY_PAYLOAD_SIZE + 2))
				mx.stat_counter.num_rx_dma_timeout++;
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

	PROFILE_ISR("timeout ISR return");
	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// grid timer ISR
// this is one of the central transport layer routines
void LED_ISR(grid_timer_isr, LED_GRID_TIMER_ISR)
{
	GPI_TRACE_FUNCTION_FAST();
	PROFILE_ISR("grid timer ISR entry");

	mask_main_timer();

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
		PROFILE_ISR("grid timer ISR start Rx begin");

		Gpi_Hybrid_Reference	r;
		Gpi_Fast_Tick_Native 	trigger_tick;
		int_fast8_t				late = 1;

		// compute exact trigger time
		trigger_tick = MAIN_TIMER_CC_REG + GPI_TICK_US_TO_FAST(ISR_LATENCY_BUFFER);

		if (RESYNC != s.slot_state)
		{
			// arm timer for exact trigger time
			MAIN_TIMER_CC_REG = trigger_tick;
			MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX] = 0;

			// connect TIMER->TASKS_COMPARE to RADIO->RXEN (via PPI)
			NRF_PPI->CH[NRF_PPI_CHANNEL].EEP =
				(uintptr_t)&(MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX]);
			NRF_PPI->CH[NRF_PPI_CHANNEL].TEP =
				(uintptr_t)&(NRF_RADIO->TASKS_RXEN);
			NRF_PPI->CHENSET = BV(NRF_PPI_CHANNEL);

			// wait until trigger time has been reached
			PROFILE_ISR();
			while (gpi_tick_compare_fast_native(gpi_tick_fast_native(), trigger_tick) < 0)
				late = 0;
			PROFILE_ISR();

			// test if start was successful
			// NOTE: avoids race condition if trigger tick is reached short before the polling loop
			if (BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, RxRu))
				late = 0;

			// disable PPI channel
			NRF_PPI->CHENCLR = BV(NRF_PPI_CHANNEL);

			#if MX_VERBOSE_STATISTICS
				if (late)
					mx.stat_counter.num_rx_late++;
			#endif
		}

		// during RESYNC or if we are late: start manually (immediately)
		if (late)
		{
			NRF_RADIO->TASKS_RXEN = 1;

			#if MX_VERBOSE_STATISTICS
				trigger_tick = gpi_tick_fast_native();
			#endif
		}

		#if MX_VERBOSE_STATISTICS
			s.radio_start_timestamp = trigger_tick | 1;
		#endif

		// connect RADIO->EVENTS_FRAMESTART to TIMER->TASKS_CAPTURE (via PPI)
		NRF_PPI->CH[NRF_PPI_CHANNEL].EEP =
			(uintptr_t)&(NRF_RADIO->EVENTS_FRAMESTART);
		NRF_PPI->CH[NRF_PPI_CHANNEL].TEP =
			(uintptr_t)&(MAIN_TIMER->TASKS_CAPTURE[EVENT_CAPTURE_CC_INDEX]);
		NRF_PPI->CHENSET = BV(NRF_PPI_CHANNEL);

		// allocate rx queue destination slot
		NRF_RADIO->PACKETPTR =
			(uintptr_t)&mx.rx_queue[mx.rx_queue_num_written % NUM_ELEMENTS(mx.rx_queue)] +
			offsetof(Packet, len);
		mx.rx_queue_num_writing = mx.rx_queue_num_written + 1;

		// enable Bitcounter to monitor frame length
		NRF_PPI->CHENSET = BV(NRF_PPI_CHANNEL_2);

		// set LEN = 0 (see timeout ISR for details)
		*(uint8_t*)(NRF_RADIO->PACKETPTR) = 0;

		NRF_RADIO->EVENTS_FRAMESTART = 0;
		NRF_RADIO->EVENTS_END = 0;
		NRF_RADIO->EVENTS_BCMATCH = 0;
//		NVIC_ClearPendingIRQ(RADIO_IRQn);

		// unmask IRQs
		NRF_RADIO->INTENSET =
			BV_BY_VALUE(RADIO_INTENSET_FRAMESTART, 1) |
			BV_BY_VALUE(RADIO_INTENSET_END, 1) |
			BV_BY_VALUE(RADIO_INTENSET_BCMATCH, 1);

		s.event_tick_nominal = s.next_grid_tick + GRID_TO_EVENT_OFFSET;

		r = gpi_tick_hybrid_reference();

		// if RESYNC: restart grid timer (-> potentially long interval)
		// NOTE: timeout timer is called implicitly while RESYNC
		if (s.slot_state == RESYNC)
		{
			// ATTENTION: don't do s.next_grid_tick += MX_SLOT_LENGTH_RESYNC because grid timer is also
			// triggered by frames from interferers (Rx -> SFD -> ... (broken/invalid) -> timeout
			// -> grid timer) and hence current time might be far away from s.next_grid_tick. With
			// s.next_grid_tick += MX_SLOT_LENGTH_RESYNC, s.next_grid_tick could end up in the far
			// future if it gets incremented frequently.
			s.next_grid_tick = r.hybrid_tick + MX_SLOT_LENGTH_RESYNC;
			s.next_trigger_tick = s.next_grid_tick;
			start_grid_timer();
		}

		// else start timeout timer
		else
		{
			Gpi_Hybrid_Tick		t;

			// timeout point = grid tick + GRID_TO_EVENT_OFFSET + window, window = trigger offset - RX_TO_GRID_OFFSET
			t = s.next_grid_tick + s.rx_trigger_offset - RX_TO_GRID_OFFSET + GRID_TO_EVENT_OFFSET;

			MAIN_TIMER_CC_REG = r.fast_capture + (t - r.hybrid_tick) * FAST_HYBRID_RATIO;
			unmask_main_timer(1);

			s.slot_state = RX_RUNNING;

			GPI_TRACE_MSG_FAST(TRACE_VERBOSE, "timeout: %lu", (long)gpi_tick_hybrid_to_us(t));
		}

		PROFILE_ISR("grid timer ISR start Rx end");
		GPI_TRACE_MSG_FAST(TRACE_INFO, "Rx started");
	}

	// if Tx
	else
	{
		PROFILE_ISR("grid timer ISR start Tx begin");

		ASSERT_CT(!((uintptr_t)&mx.tx_packet % sizeof(uint_fast_t)), alignment_issue);
		ASSERT_CT(!((uintptr_t)&s.tx_fifo % sizeof(uint_fast_t)), alignment_issue);

		Gpi_Fast_Tick_Native 	trigger_tick;
		int_fast8_t				late = 1;
		uint8_t					*p;

		// compute exact trigger time
		trigger_tick = MAIN_TIMER_CC_REG + GPI_TICK_US_TO_FAST(ISR_LATENCY_BUFFER);

		// arm timer for exact trigger time
		MAIN_TIMER_CC_REG = trigger_tick;
		MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX] = 0;

		// connect TIMER->TASKS_COMPARE to RADIO->TXEN (via PPI)
		NRF_PPI->CH[NRF_PPI_CHANNEL].EEP =
			(uintptr_t)&(MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX]);
		NRF_PPI->CH[NRF_PPI_CHANNEL].TEP =
			(uintptr_t)&(NRF_RADIO->TASKS_TXEN);
		NRF_PPI->CHENSET = BV(NRF_PPI_CHANNEL);

		// wait until trigger time has been reached
		PROFILE_ISR();
		while (gpi_tick_compare_fast_native(gpi_tick_fast_native(), trigger_tick) < 0)
			late = 0;
		PROFILE_ISR();

		// test if start was successful
		// NOTE: avoids race condition if trigger tick is reached short before the polling loop
		if (BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, TxRu))
			late = 0;

		// disable PPI channel
		NRF_PPI->CHENCLR = BV(NRF_PPI_CHANNEL);

		// if we are late: start manually (immediately)
		if (late)
		{
			NRF_RADIO->TASKS_TXEN = 1;

			#if MX_VERBOSE_STATISTICS
				trigger_tick = gpi_tick_fast_native();
			#endif
		}

		// init DMA
		// ATTENTION: must be done before START task gets triggered (via shortcut)
		// NOTE: while a delay in the following steps "only" damages the transmitted data,
		// a wrong PACKETPTR can crash the system (via HardFault exception)
		NRF_RADIO->PACKETPTR = (uintptr_t)&s.tx_fifo.len;
		s.tx_fifo.len = PHY_PAYLOAD_SIZE + 2;
		REORDER_BARRIER();

		gpi_led_on(LED_TX);

		#if MX_VERBOSE_STATISTICS
			s.radio_start_timestamp = trigger_tick | 1;
			if (late)
				mx.stat_counter.num_tx_late++;
		#endif

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

			write_tx_fifo(
				offsetof(Packet, phy_payload_begin), &mx.tx_packet.phy_payload_begin, NULL,
				offsetof(Packet, coding_vector) - offsetof(Packet, phy_payload_begin));
		}

		// write coding vector and payload
		{
			ASSERT_CT(offsetof(Packet, payload) ==
				offsetof(Packet, coding_vector) + sizeof(mx.tx_packet.coding_vector),
				inconsistent_program);

			const unsigned int	CHUNK_SIZE = sizeof(mx.tx_packet.coding_vector) + sizeof(mx.tx_packet.payload);

			p = &(mx.tx_packet.coding_vector[0]);

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

				write_tx_fifo(offsetof(Packet, coding_vector), ps, NULL, CHUNK_SIZE);

				#if MX_VERBOSE_PACKETS || MX_REQUEST
					// mark the packet as broken since it could be possible that we interrupt
					// prepare_tx_packet() right now, hence writing data may damage the packet
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

				// write coding vector with sideload and test if result is a zero packet;
				// if so: abort (below)
				// NOTE: another option might be to write some other useful information into the
				// packet instead of dropping it
				// NOTE: It would be a bit more precise to (1) write (only) the coding vector,
				// (2) test if zero, (3) write payload only if test is non-zero. However, we
				// write the full chunk at once to gain better performance in the typical case
				// (that is non-zero coding vector). Processing the full chunk at once not only
				// saves the second call, it also keeps the alignment(!). We expect that this way
				// is more efficient with relatively moderate packet sizes as in IEEE 802.15.4.
				if (!write_tx_fifo(offsetof(Packet, coding_vector), p, ps, CHUNK_SIZE))
					p = NULL;
			}

//			GPI_TRACE_MSG_FAST(TRACE_VERBOSE,
//				"is_ready: %u, mx.tx_sideload: %p, ps: %p, mx.req.help_index: %d",
//				(int)mx.tx_packet.is_ready, mx.tx_sideload, ps, (int)mx.request.help_index);
//			GPI_TRACE_MSG_FAST(TRACE_VERBOSE,
//				"matrix: %p...%p, rx_queue: %p...%p",
//				&mx.matrix, &mx.matrix[NUM_ELEMENTS(mx.matrix)],
//				&mx.rx_queue, &mx.rx_queue[NUM_ELEMENTS(mx.rx_queue)]);
		}

		// write info vector
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

			write_tx_fifo(offsetof(Packet, info_vector), ps, NULL, sizeof(mx.tx_packet.info_vector));
		}
		#endif

		// if zero packet: abort transmission
		if (NULL == p)
		{
			NRF_RADIO->TASKS_DISABLE = 1;
			gpi_led_off(LED_TX);

			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.radio_on_time += gpi_tick_fast_native() - s.radio_start_timestamp;
				s.radio_start_timestamp = 0;
				mx.stat_counter.num_tx_zero_packet++;
			#endif

			GPI_TRACE_MSG_FAST(TRACE_INFO, "sideload produced zero-packet -> Tx aborted");
		}

		else
		{
			// unmask IRQ
			NRF_RADIO->EVENTS_END = 0;
//			NVIC_ClearPendingIRQ(RADIO_IRQn);
			NRF_RADIO->INTENSET = BV_BY_VALUE(RADIO_INTENSET_END, 1);

			// update mx.tx_packet to enable evaluation on processing layer
			// NOTE: not all fields are needed for MX_REQUEST,
			// particularly payload could be dropped (e.g. if time is critical)
			#if (MX_VERBOSE_PACKETS || MX_REQUEST)
				gpi_memcpy_dma_aligned(&mx.tx_packet, &s.tx_fifo, sizeof(Packet));
			#endif

			set_event(TX_READY);
			mx.stat_counter.num_sent++;

			GPI_TRACE_MSG_FAST(TRACE_INFO, "Tx started");
		}

		s.slot_state = TX_RUNNING;
		mx.tx_packet.is_ready = 0;
		mx.tx_sideload = NULL;
		s.next_slot_task = RX;

		PROFILE_ISR("grid timer ISR start Tx end");
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

	PROFILE_ISR("grid timer ISR return");
	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

void mixer_transport_init()
{
	GPI_TRACE_FUNCTION();

	assert(MX_PHY_MODE == gpi_radio_get_mode());

	// connect shortcuts
	// NOTE: alternatively, Tx START could be triggered from timer via PPI
	// -> replace READY_START by RXREADY_START in this case
	NRF_RADIO->SHORTS =
		BV_BY_NAME(RADIO_SHORTS_READY_START, Enabled) |
		BV_BY_NAME(RADIO_SHORTS_END_DISABLE, Enabled) |
		BV_BY_NAME(RADIO_SHORTS_FRAMESTART_BCSTART, Enabled);

	// set MAXLEN to avoid Rx buffer overflows
	NRF_RADIO->PCNF1 = BV_BY_VALUE(RADIO_PCNF1_MAXLEN, PHY_PAYLOAD_SIZE + 2);

	// init Bitcounter to monitor frame length and
	// connect RADIO->EVENTS_BCMATCH to RADIO->TASKS_DISABLE (via PPI)
	// NOTE: We use the Bitcounter in addition to MAXLEN because MAXLEN stops just the DMA transfer,
	// not the whole radio (4413_417 v1.0 page 304). We want to stop the whole radio to safe energy.
	// NOTE: init Bitcounter such that it triggers when LEN > (not >=) nominal length
	NRF_RADIO->BCC = (PHY_PAYLOAD_SIZE + 2) * 8 + 1;
	NRF_PPI->CH[NRF_PPI_CHANNEL_2].EEP = (uintptr_t)&(NRF_RADIO->EVENTS_BCMATCH);
	NRF_PPI->CH[NRF_PPI_CHANNEL_2].TEP = (uintptr_t)&(NRF_RADIO->TASKS_DISABLE);

	// arm main timer IRQ
	// TODO: main timer could have an ISR outside Mixer -> connect to that's dispatcher if present
	MAIN_TIMER->INTENCLR = -1u;
	NVIC_SetPriority(MAIN_TIMER_IRQ, 0);
	NVIC_ClearPendingIRQ(MAIN_TIMER_IRQ);
	NVIC_EnableIRQ(MAIN_TIMER_IRQ);

	// arm radio IRQ
	NRF_RADIO->INTENCLR = -1u;
	NVIC_SetPriority(RADIO_IRQn, 0);
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);

	// TODO: move the following two settings to main function or platform

	// disable deep sleep (not implementated on nRF52840) and sleep-on-exit behavior
	SCB->SCR = 0;

	// set nRF52840 low power submode = low power
	// NOTE: can be changed to constant latency mode if there seem to be problems
	NRF_POWER->TASKS_LOWPWR = 1;
	// NRF_POWER->TASKS_CONSTLAT = 1;

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void mixer_transport_print_config(void)
{
	#define PRINT(s) printf("%-25s = %" PRId32 "\n", #s, (int32_t)s)

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
	PRINT(GRID_TO_EVENT_OFFSET);
	PRINT(PACKET_AIR_TIME);

	printf("%-25s = %" PRIu32 "\n", "RX_WINDOW / INCREMENT", (uint32_t)((RX_WINDOW_MAX - RX_WINDOW_MIN) / RX_WINDOW_INCREMENT));
	printf("%-25s = %" PRIu32 "\n", "air bytes (excl. pre + adr)", (uint32_t)(offsetof(Packet, phy_payload_end) + 1)); // +1 because offsets begin at 0
	printf("%-25s = %" PRIu32 "\n", "min. slot length",
		(uint32_t)((PACKET_AIR_TIME + RX_TO_GRID_OFFSET + 2 * RX_WINDOW_MAX + GPI_TICK_US_TO_HYBRID(ISR_LATENCY_BUFFER)
		+ GPI_TICK_US_TO_HYBRID(25)
		// + duration of MAX(radio_isr, timeout_isr) = ca. 20us -> take 25us
		// everything times 300ppm (1.0003)
		#if GPI_HYBRID_CLOCK_USE_VHT
			+ GPI_TICK_US_TO_HYBRID(1000000 / GPI_SLOW_CLOCK_RATE + 30)));
		#else
			) * 1.0003));
		#endif

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
			GPI_TICK_US_TO_HYBRID(1000000 / GPI_SLOW_CLOCK_RATE + 30);
		r.hybrid_tick *= FAST_HYBRID_RATIO;
		if (r.hybrid_tick > GPI_TICK_FAST_MAX / 2)
			r.fast_capture += GPI_TICK_FAST_MAX / 2;
		else r.fast_capture += r.hybrid_tick;

		int ie = gpi_int_lock();
		REORDER_BARRIER();

		if ((RESYNC != s.slot_state) || (STOP == next_task))
		{
			if (gpi_tick_compare_fast_native(gpi_tick_fast_native(), r.fast_capture) >= 0)
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
