/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2023, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					butler.c
 *
 *	@brief					Butler's core
 *
 *	@version				$Id$
 *	@date					TODO
 *
 *	@author					Fabian Mager
 *
 ***************************************************************************************************

 	@details

	TODO

 **************************************************************************************************/
//***** Trace Settings *****************************************************************************

#include "gpi/trace.h"

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
// define groups appropriate for your needs, assign one bit per group
// values > GPI_TRACE_LOG_USER (i.e. upper bits) are reserved
#define TRACE_INFO GPI_TRACE_MSG_TYPE_INFO

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(main, GPI_TRACE_BASE_SELECTION | GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "butler.h"
#include "butler_config.h"
#include "gpi/clocks.h"
#include "gpi/interrupts.h"
#include "gpi/olf.h"
#include "gpi/platform.h"
#include "gpi/platform_spec.h"
#include "gpi/tools.h"
#include GPI_PLATFORM_PATH(radio.h)
#include <inttypes.h>
#include <nrf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************

#define MAIN_TIMER             NRF_TIMER0
#define MAIN_TIMER_IRQ         TIMER0_IRQn
#define MAIN_TIMER_CC_INDEX    3
#define MAIN_TIMER_CC_REG      (MAIN_TIMER->CC[MAIN_TIMER_CC_INDEX])
#define EVENT_CAPTURE_CC_INDEX 2
#define EVENT_CAPTURE_CC_REG   (MAIN_TIMER->CC[EVENT_CAPTURE_CC_INDEX])
#define PPI_CH_1               17

#define PHY_PAYLOAD_SIZE                                                                           \
	(offsetof(butler_pkt_t, phy_payload_end) - offsetof(butler_pkt_t, phy_payload_begin))

// NOTE: VECTOR_TABLE_RADIO_IRQ points to the location in the vector table containing the pointer to
// the radio IRQ. This address can be different when some bootloader code starts at 0x20000000.
#define VECTOR_TABLE_RADIO_IRQ  UINT32_C(0x20000044)
#define VECTOR_TABLE_TIMER0_IRQ UINT32_C(0x20000060)

// Maximum TX-to-RX or RX-to-TX turnaround time in IEEE 802.15.4 mode (nRF52840_PS_v1.1).
#define RADIO_TURNAROUND_DELAY          GPI_TICK_US_TO_FAST(40)
#define RX_TX_RAMP_UP_DELAY             GPI_TICK_US_TO_FAST(40)
#define NRF_TRANSCEIVER_DELAY           GPI_TICK_US_TO_FAST(40) // ~ 10 bit times
#define TX_TIME_ONE_BYTE                GPI_TICK_US_TO_FAST(32) // 250 kbps = 32 us/B
#define FRAMESTART_EVENT_DELAY          (TX_TIME_ONE_BYTE * 6)
#define SPECIFIC_FRAMESTART_EVENT_DELAY GPI_TICK_US_TO_FAST(176)
#define ISR_EXECUTION_DELAY             GPI_TICK_US_TO_FAST(15) // (overprovisioned)
#define SYNC_SLOT_LEN                   GPI_TICK_US_TO_FAST(335)
#define TEARDOWN_OFFSET                 GPI_TICK_MS_TO_FAST(50) // (overprovisioned)
#define PREPARE_TX_BUFFER               GPI_TICK_US_TO_FAST(5)
#define RESYNC_STAT_BUFFER_SIZE         100 // buffer size for statistics

// Ensure that the chosen P_TX value is the minimum transmit probability. Thus, TX decisions are
// not completely random but enforced after longer RX durations.
#define MAX_SLOTS_BEFORE_TX (BUTLER_DURATION / (BUTLER_DURATION * P_TX / 100))
// At the beginning of Butler we want to guarantee a tranmission within a certain time.
#define GUARANTEED_TX_AFTER_SLOT (2 * NUM_NODES)

//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************

typedef uint8_t countdown_t;

typedef struct
{
	// platform-specific header
	// 802.15.4
	struct __attribute__((packed))
	{
		// uint8_t	_padding_1[2];	/* for alignment */
		uint8_t preamble[4];
		uint8_t sfd;
		uint8_t len;
	};

	// PDU format
	struct __attribute__((packed))
	{
		union __attribute__((packed))
		{
			uint8_t phy_payload_begin; // just a marker (e.g., offsetof(Packet, phy_payload_begin))
			uint8_t sync_id;
		};

		countdown_t countdown;
	};

	// packet tail (not transmitted)
	union __attribute__((packed))
	{
		int8_t phy_payload_end; // just a marker (e.g., offsetof(Packet, phy_payload_end))

		struct __attribute__((packed))
		{
			int8_t rssi;

			union __attribute__((packed))
			{
				int8_t crc_corr;

				struct __attribute__((packed))
				{
					uint8_t correlation : 7;
					uint8_t crc_ok : 1;
				};
			};
		};
	};
} butler_pkt_t;


//**************************************************************************************************
//***** Forward Declarations ***********************************************************************

void butler_RadioIRQ(void);
void butler_TIMER0IRQ(void);

//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

static uint32_t rand_state = 1; // seed for random generator
static uint32_t rand_init  = 0; // random generator initialized

// State of the node.
static volatile struct
{
	Gpi_Fast_Tick_Native now;
	Gpi_Fast_Tick_Native sync_time;
	countdown_t          sync_countdown;
	uint8_t              sync_id;
	Gpi_Fast_Tick_Native deadline;
	unsigned int         synced;
	unsigned int         shutdown;
	unsigned int         last_action;
	uint8_t              node_id;
	butler_pkt_t         packet;
} s;

// statistics
static volatile struct
{
	unsigned int tx_cnt;
	unsigned int rx_timeout;
	unsigned int rx_invalid_len;
	unsigned int rx_crc_error;
	unsigned int rx_success;
	unsigned int rx_same;
	countdown_t  countdown_first_tx;
	unsigned int is_sync_leader;
	unsigned int proposed_signal;
	unsigned int resyncs;
	uint8_t      resync_idx;
	uint8_t      resync_id[RESYNC_STAT_BUFFER_SIZE];
	countdown_t  resync_countdown[RESYNC_STAT_BUFFER_SIZE];
	uint8_t      resync_reason[RESYNC_STAT_BUFFER_SIZE];
} stats;

static volatile uintptr_t prev_radioIRQ;
static volatile uintptr_t prev_TIMER0IRQ;
static uintptr_t         *ptr_radioIRQ  = (uintptr_t *)VECTOR_TABLE_RADIO_IRQ;
static uintptr_t         *ptr_TIMER0IRQ = (uintptr_t *)VECTOR_TABLE_TIMER0_IRQ;

static uint16_t p_tx_value = P_TX * 65535 / 100;

//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Local Functions ****************************************************************************

// RNG (XorShift)
static uint16_t butler_rand(void)
{
	rand_state ^= rand_state << 2;
	rand_state ^= rand_state >> 15;
	rand_state ^= rand_state << 17;

	return rand_state;
}

//**************************************************************************************************

// Redirecting and setting up ISR's (timer, radio) for Butler.
static void butler_setup(void)
{
	GPI_TRACE_FUNCTION_FAST();

	// Vector table is located at address 0x0 in flash per default. Compiling with VECTORS_IN_RAM=1
	// copies the vector table to RAM at address 0x20000000 and adjusts the VTOR. Alternatively,
	// this can also be done during execution, see:
	//
	// static uint32_t vector_table_ram[VECTOR_TABLE_SIZE] __attribute__ ((aligned (256)));
	// void vector_table_init()
	// {
	//		memcpy(vector_table_ram, (uint32_t *) (SCB->VTOR), VECTOR_TABLE_SIZE);
	//		__disable_irq();
	//		SCB->VTOR = (uint32_t)&vector_table_ram; // Set VTOR to point to new vector table
	//		__enable_irq();
	// }

	prev_radioIRQ  = *ptr_radioIRQ;
	prev_TIMER0IRQ = *ptr_TIMER0IRQ;

	// redirecting ISR's
	gpi_int_disable();
	*ptr_radioIRQ  = (uintptr_t)&butler_RadioIRQ;
	*ptr_TIMER0IRQ = (uintptr_t)&butler_TIMER0IRQ;
	gpi_int_enable();

	//*** radio setup
	NRF_RADIO->SHORTS = 0;
	NRF_RADIO->PCNF1  = BV_BY_VALUE(RADIO_PCNF1_MAXLEN, PHY_PAYLOAD_SIZE + 2);

	// arm main timer IRQ
	// NOTE: MAIN_TIMER could have an ISR outside Butler that needs to run in parallel. Use a
	// dispatcher to serve both.
	MAIN_TIMER->INTENCLR = -1u;
	NVIC_SetPriority(MAIN_TIMER_IRQ, 0);
	NVIC_ClearPendingIRQ(MAIN_TIMER_IRQ);
	NVIC_EnableIRQ(MAIN_TIMER_IRQ);

	// arm radio IRQ
	NRF_RADIO->INTENCLR = -1u;
	NVIC_SetPriority(RADIO_IRQn, 0);
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);

	// NRF_POWER->TASKS_CONSTLAT = 1;

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Restoring ISR state.
static void butler_teardown(void)
{
	GPI_TRACE_FUNCTION_FAST();

	// redirecting ISR's
	gpi_int_disable();
	*ptr_radioIRQ  = prev_radioIRQ;
	*ptr_TIMER0IRQ = prev_TIMER0IRQ;
	gpi_int_enable();

	// disable active PPI channels
	NRF_PPI->CHENCLR = BV(PPI_CH_1);

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Since transmissions must be aligned to the local slot grid, this function returns the next grid
// tick based on the given 'time' parameter.
static Gpi_Fast_Tick_Native next_tx_time(Gpi_Fast_Tick_Native time)
{
	GPI_TRACE_FUNCTION_FAST();

	// compute the start of the next sync slot after "time"
	Gpi_Fast_Tick_Native time_since_sync = time - s.sync_time;
	uint32_t             t               = (time_since_sync + SYNC_SLOT_LEN - 1) / SYNC_SLOT_LEN;

	GPI_TRACE_RETURN_FAST(s.sync_time + t * SYNC_SLOT_LEN);
}

//**************************************************************************************************

// Determine the remaining number of slots until the end of Butler (i.e., the reference time) based
// on the given timestamp 'now'.
static countdown_t remaining_slots(Gpi_Fast_Tick_Native now)
{
	GPI_TRACE_FUNCTION_FAST();

	// GPI_TRACE_MSG_FAST(TRACE_INFO, "time passed since sync=%u (%u slots)", (now - s.sync_time) /
	// GPI_TICK_US_TO_HYBRID2(1), (now - s.sync_time) / SYNC_SLOT_LEN);

	GPI_TRACE_RETURN_FAST(s.sync_countdown - (now - s.sync_time) / SYNC_SLOT_LEN);
}

//**************************************************************************************************

// Set timer to trigger MAIN_TIMER ISR.
static inline void set_timer(Gpi_Fast_Tick_Native value)
{
	GPI_TRACE_FUNCTION_FAST();

	MAIN_TIMER_CC_REG                               = value;
	MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX] = 0;
	NVIC_ClearPendingIRQ(MAIN_TIMER_IRQ);
	MAIN_TIMER->INTENSET = 0x10000 << MAIN_TIMER_CC_INDEX;

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Clear and disable timer interrupts.
static inline void disable_rx_timer(void)
{
	GPI_TRACE_FUNCTION_FAST();

	MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX] = 0;
	NVIC_ClearPendingIRQ(MAIN_TIMER_IRQ);
	MAIN_TIMER->INTENCLR = 0x10000 << MAIN_TIMER_CC_INDEX;

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Prepare the packet and start the transmission in the next slot. Returns the actual starting time.
static inline Gpi_Fast_Tick_Native prepare_tx(Gpi_Fast_Tick_Native earliest_start)
{
	GPI_TRACE_FUNCTION_FAST();

	// NOTE: We immediately issue the radio's TX mode and prepare the packet contents etc.
	// thereafter. This works because it takes some time until the radio state has changed. If it
	// would take longer to set things up, it would be better to turn on the radio as late as
	// possible because it will already start transmitting the carrier (interference) after enabling
	// it.
	NRF_RADIO->TASKS_TXEN = 1;

	// NOTE: Due to possible timer overflows we work with time intervals instead of comparing two
	// absolute times.
	Gpi_Fast_Tick_Native t_remaining = s.deadline - earliest_start;

	// Remaining time is smaller than slot length.
	if (t_remaining <= SYNC_SLOT_LEN)
	{
		NRF_RADIO->TASKS_DISABLE = 1;
		NRF_PPI->CHENCLR         = BV(PPI_CH_1);
		s.shutdown               = 1;

		GPI_TRACE_RETURN_FAST(0);
	}

	// Propose own sync signal if not already synced to another.
	if (!s.synced)
	{
		s.synced                                 = 1;
		stats.is_sync_leader                     = 1;
		stats.proposed_signal                    = 1;
		stats.resync_id[stats.resync_idx]        = s.sync_id;
		stats.resync_countdown[stats.resync_idx] = s.sync_countdown;
		stats.resync_reason[stats.resync_idx]    = 0;
		stats.resync_idx++;

		// GPI_TRACE_MSG_FAST(TRACE_INFO, "proposing sync signal");
	}

	// Align the transmission to the next grid tick (slot). Set timer and use shortcut (not
	// interrupt) to trigger transmission.
	Gpi_Fast_Tick_Native t                          = next_tx_time(earliest_start);
	MAIN_TIMER_CC_REG                               = t;
	MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX] = 0;
	MAIN_TIMER->INTENCLR                            = 0x10000 << MAIN_TIMER_CC_INDEX;

	NRF_PPI->CH[PPI_CH_1].EEP = (uintptr_t) & (MAIN_TIMER->EVENTS_COMPARE[MAIN_TIMER_CC_INDEX]);
	NRF_PPI->CH[PPI_CH_1].TEP = (uintptr_t) & (NRF_RADIO->TASKS_START);
	NRF_PPI->CHENSET          = BV(PPI_CH_1);

	// clear RX events
	NRF_RADIO->EVENTS_FRAMESTART = 0;
	NRF_RADIO->INTENCLR          = BV_BY_VALUE(RADIO_INTENSET_FRAMESTART, 1);

	// unmask IRQ
	NRF_RADIO->EVENTS_END = 0;
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NRF_RADIO->INTENSET = BV_BY_VALUE(RADIO_INTENSET_END, 1);

	// set packet contents
	NRF_RADIO->PACKETPTR = (uintptr_t)&s.packet.len;
	s.packet.len         = PHY_PAYLOAD_SIZE + 2; // + 2 because of CRC
	s.packet.sync_id     = s.sync_id;
	s.packet.countdown   = remaining_slots(t);

	if (stats.countdown_first_tx == 0) { stats.countdown_first_tx = s.packet.countdown; }

	stats.tx_cnt++;

	GPI_TRACE_RETURN_FAST(t);
}

//**************************************************************************************************

// Start listening for incoming packets for 'duration' time. Returns the actual duration, which
// could vary to mitigate idle times or prevent deadline overflows.
static inline Gpi_Fast_Tick_Native prepare_rx(Gpi_Fast_Tick_Native duration)
{
	GPI_TRACE_FUNCTION_FAST();

	NRF_RADIO->TASKS_RXEN = 1;
	NRF_RADIO->SHORTS     = BV_BY_NAME(RADIO_SHORTS_RXREADY_START, Enabled);
	NRF_RADIO->PACKETPTR  = (uintptr_t)&s.packet.len;
	s.packet.len          = 0;

	// NOTE: Due to possible timer overflows we work with time intervals instead of comparing two
	// absolute times.
	s.now                            = gpi_tick_fast_native();
	Gpi_Fast_Tick_Native t_remaining = s.deadline - s.now;

	if (t_remaining <= SYNC_SLOT_LEN)
	{
		NRF_RADIO->TASKS_DISABLE = 1;
		NRF_PPI->CHENCLR         = BV(PPI_CH_1);
		s.shutdown               = 1;

		GPI_TRACE_RETURN_FAST(0);
	}

	Gpi_Fast_Tick_Native t_rx;

	// Do not listen beyond the deadline.
	if (duration >= t_remaining)
	{
		t_rx          = s.deadline;
		s.last_action = 1;
	}
	else
	{
		// To avoid unnecessary idle times, we adjust the end of RX to close the gap to the
		// following TX slot.
		t_rx = next_tx_time(s.now + duration) - RADIO_TURNAROUND_DELAY - ISR_EXECUTION_DELAY;

		if (s.deadline - t_rx <= SYNC_SLOT_LEN) { s.last_action = 1; }
	}

	set_timer(t_rx);

	// clear TX events
	NRF_RADIO->EVENTS_END = 0;
	NRF_RADIO->INTENCLR   = BV_BY_VALUE(RADIO_INTENSET_END, 1);

	// connect RADIO->EVENTS_FRAMESTART to TIMER->TASKS_CAPTURE (via PPI)
	NRF_PPI->CH[PPI_CH_1].EEP = (uintptr_t) & (NRF_RADIO->EVENTS_FRAMESTART);
	NRF_PPI->CH[PPI_CH_1].TEP = (uintptr_t) & (MAIN_TIMER->TASKS_CAPTURE[EVENT_CAPTURE_CC_INDEX]);
	NRF_PPI->CHENSET          = BV(PPI_CH_1);

	NRF_RADIO->EVENTS_FRAMESTART = 0;
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NRF_RADIO->INTENSET = BV_BY_VALUE(RADIO_INTENSET_FRAMESTART, 1);

	GPI_TRACE_RETURN_FAST(t_rx);
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

// MAIN_TIMER ISR
void butler_TIMER0IRQ(void)
{
	GPI_TRACE_FUNCTION_FAST();

	disable_rx_timer();

	stats.rx_timeout++;

	// If last_action is set and we run into a timeout, we can safely shutdown.
	if (s.last_action)
	{
		s.shutdown               = 1;
		NRF_RADIO->TASKS_DISABLE = 1;
		NRF_PPI->CHENCLR         = BV(PPI_CH_1);

		// GPI_TRACE_MSG_FAST(TRACE_INFO, "RX timeout after last action -> shutdown.");
		GPI_TRACE_RETURN_FAST();
	}

	// Corner case: We received a packet which could be ignored and continued RX. When this happens
	// just before the timer triggers, we could be in RxRu state.
	while (BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, RxRu))
		;

	if (BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, Rx)) NRF_RADIO->TASKS_STOP = 1;

	// The radio was issued to stop and we wait for its RxIdle state.
	while (!BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, RxIdle))
		;

	s.now                     = gpi_tick_fast_native();
	Gpi_Fast_Tick_Native t_tx = prepare_tx(s.now + RADIO_TURNAROUND_DELAY + PREPARE_TX_BUFFER);

	// GPI_TRACE_MSG_FAST(TRACE_INFO, "RX timeout, start TX at %u", t_tx /
	// GPI_TICK_US_TO_HYBRID2(1));

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Radio ISR
void butler_RadioIRQ(void)
{
	GPI_TRACE_FUNCTION_FAST();

	// After successful TX.
	if (NRF_RADIO->EVENTS_END && (NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk))
	{
		NRF_RADIO->EVENTS_END = 0;
		NRF_PPI->CHENCLR      = BV(PPI_CH_1);
		NVIC_ClearPendingIRQ(RADIO_IRQn);
		NRF_RADIO->INTENCLR = BV_BY_VALUE(RADIO_INTENCLR_END, 1);
		disable_rx_timer();

		while (!BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, TxIdle))
			;

		// Compute when (slot) to transmit the next time.
		// NOTE: The time to execute this loop is negligible and minimally delays the time until the
		// node starts listening. If time would be a critical factor, there are other options to
		// compute this, for example, incrementally slot after slot.
		unsigned int slots_until_tx = 1;
		uint16_t     ptx            = p_tx_value;
		while ((butler_rand() > ptx) && (slots_until_tx < MAX_SLOTS_BEFORE_TX))
		{
			slots_until_tx++;
		}

		Gpi_Fast_Tick_Native rx_duration = slots_until_tx * SYNC_SLOT_LEN;
		Gpi_Fast_Tick_Native t_rx        = prepare_rx(rx_duration + RADIO_TURNAROUND_DELAY);

		// GPI_TRACE_MSG_FAST(TRACE_INFO, "TX done, start RX until %u", t_rx /
		// GPI_TICK_US_TO_HYBRID2(1));
	}

	// Beginning of packet reception.
	else if (NRF_RADIO->EVENTS_FRAMESTART && (NRF_RADIO->INTENSET & RADIO_INTENSET_FRAMESTART_Msk))
	{
		Gpi_Fast_Tick_Native t_framestart = EVENT_CAPTURE_CC_REG;
		Gpi_Fast_Tick_Native slot_start =
		    t_framestart - NRF_TRANSCEIVER_DELAY - SPECIFIC_FRAMESTART_EVENT_DELAY;
		NRF_RADIO->EVENTS_FRAMESTART = 0;
		NRF_RADIO->INTENCLR          = BV_BY_VALUE(RADIO_INTENCLR_FRAMESTART, 1);

		// We are here because of the FRAMESTART event, so we can already evaluate the PHR (len)
		// field. When len field is invalid we continue RX as planned.
		if (s.packet.len != (PHY_PAYLOAD_SIZE + 2))
		{
			if (BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, Rx)) NRF_RADIO->TASKS_STOP = 1;

			NRF_RADIO->PACKETPTR = (uintptr_t)&s.packet.len;
			s.packet.len         = 0;
			NVIC_ClearPendingIRQ(RADIO_IRQn);
			NRF_RADIO->INTENSET = BV_BY_VALUE(RADIO_INTENSET_FRAMESTART, 1);

			while (!BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, RxIdle))
				;

			NRF_RADIO->TASKS_START = 1;

			stats.rx_invalid_len++;

			// GPI_TRACE_MSG_FAST(TRACE_INFO, "Wrong packet length (%u)! Continue RX.",
			// s.packet.len);
		}
		else
		{
			// NOTE: Determining the remaining number of slots simply based on the reception
			// timestamp is unreliable due to clock drift. For example, a slightly earlier reception
			// may lead to the reference time being one slot off. Since Butler's execution is short
			// and the slots are rather long compared to the clock drift, we can safely determine
			// the remaining number of slots in the middle of the slot.
			countdown_t local_countdown = remaining_slots(slot_start + SYNC_SLOT_LEN / 2);

			// Wait for the end of the packet.
			while (!BV_TEST_BY_NAME(NRF_RADIO->STATE, RADIO_STATE_STATE, RxIdle))
				;

			// Check CRC and continue RX when packet is broken.
			if (BV_TEST_BY_NAME(NRF_RADIO->CRCSTATUS, RADIO_CRCSTATUS_CRCSTATUS, CRCError))
			{
				NRF_RADIO->PACKETPTR = (uintptr_t)&s.packet.len;
				s.packet.len         = 0;
				NVIC_ClearPendingIRQ(RADIO_IRQn);
				NRF_RADIO->INTENSET = BV_BY_VALUE(RADIO_INTENSET_FRAMESTART, 1);

				// Since we are already in RxIdle state, we can directly start listening again.
				NRF_RADIO->TASKS_START = 1;

				stats.rx_crc_error++;

				GPI_TRACE_MSG_FAST(TRACE_INFO, "CRC error! Continue RX.");
			}
			// successful packet reception
			else
			{
				unsigned int resync = 0;

				if (s.synced)
				{
					// Ignore packet if we are synced to the same sync signal.
					if (s.packet.sync_id == s.sync_id) { stats.rx_same++; }
					// Resync to received packet if its countdown is lower.
					else if (s.packet.countdown < local_countdown) { resync = 2; }
					else if (s.packet.countdown == local_countdown)
					{
						// NOTE: Due to possible timer overflows we work with time intervals instead
						// of comparing two absolute times.
						s.now = gpi_tick_fast_native();
						// Determine the duration between now and the start of the local sync slot.
						Gpi_Fast_Tick_Native dur_local_slot_start =
						    s.now -
						    ((s.sync_countdown - local_countdown) * SYNC_SLOT_LEN + s.sync_time);
						// Determine the duration between now and the start of the packet.
						Gpi_Fast_Tick_Native dur_rx_slot_start = s.now - slot_start;

						// Compute difference between local grid and the grid of the received
						// packet.
						unsigned int dist = 0;
						if (dur_rx_slot_start > dur_local_slot_start)
							dist = dur_rx_slot_start - dur_local_slot_start;
						else
							dist = dur_local_slot_start - dur_rx_slot_start;

						// A few uncertainties and slightly varying clock drifts can lead to
						// resynchronization between the same two signals back and forth. However,
						// this does not affect Butler's correctness and can be ignored (see paper).
						if (dist <= 0)
						{
							// Sync to lower sync_id when both signals are within the threshold
							// (currently fixed to 0).
							if (s.packet.sync_id < s.sync_id) { resync = 4; }
						}
						else
						{
							// The packet was received before the local sync slot started (longer
							// duration).
							if (dur_rx_slot_start > dur_local_slot_start) { resync = 3; }
						}
					}
				}
				else { resync = 1; }

				// resync reasons:
				//		1: unsynced
				//		2: lower cd
				//		3: equal cd, earlier start
				//		4: equal cd, equal start (within threshold), lower sync_id

				if (resync)
				{
					disable_rx_timer();

					s.sync_time      = slot_start;
					s.sync_countdown = s.packet.countdown;
					s.sync_id        = s.packet.sync_id;
					s.deadline       = s.sync_countdown * SYNC_SLOT_LEN + s.sync_time;
					s.synced         = 1;

					stats.resyncs++;
					stats.is_sync_leader = 0;

					stats.resync_id[stats.resync_idx]        = s.sync_id;
					stats.resync_countdown[stats.resync_idx] = s.sync_countdown;
					stats.resync_reason[stats.resync_idx]    = resync;
					stats.resync_idx++;

					// At this point the radio is already in RXIdle state.
					s.now                     = gpi_tick_fast_native();
					Gpi_Fast_Tick_Native t_tx = 0;
					// Transmit after resync.
					t_tx = prepare_tx(s.now + RADIO_TURNAROUND_DELAY + PREPARE_TX_BUFFER);
					// GPI_TRACE_MSG_FAST(TRACE_INFO, "RX successful (RESYNC), start TX at %u", t_tx
					// / GPI_TICK_US_TO_HYBRID2(1));
				}
				// If the node did not resync, continue listening.
				else
				{
					NRF_RADIO->PACKETPTR = (uintptr_t)&s.packet.len;
					s.packet.len         = 0;
					NVIC_ClearPendingIRQ(RADIO_IRQn);
					NRF_RADIO->INTENSET = BV_BY_VALUE(RADIO_INTENSET_FRAMESTART, 1);

					NRF_RADIO->TASKS_START = 1;

					GPI_TRACE_MSG_FAST(TRACE_INFO, "RX successful, continue RX.");
				}

				stats.rx_success++;

				// GPI_TRACE_MSG_FAST(TRACE_INFO, "RX successful, t_framestart=%u", t_framestart /
				// GPI_TICK_US_TO_HYBRID2(1));
			}
		}
	}
	else { GPI_TRACE_MSG_FAST(TRACE_INFO, "Unknown state!"); }

	// GPI_TRACE_MSG_FAST(TRACE_INFO, "sync_time=%u sync_countdown=%" PRIu8 " sync_id=%" PRIu8 "
	// deadline=%u", s.sync_time / GPI_TICK_US_TO_HYBRID2(1), s.sync_countdown, s.sync_id,
	// s.deadline / GPI_TICK_US_TO_HYBRID2(1));

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Prints statistics about the last Butler execution.
void butler_print_statistics(void)
{
	GPI_TRACE_FUNCTION_FAST();

#ifdef PRINT
	#error change macro name
#endif

	printf("sync statistics:\n");

#define PRINT(n)                                                                                   \
	ASSERT_CT(sizeof(stats.n) == sizeof(unsigned int), n);                                         \
	printf(#n ": %u\n", stats.n)

	PRINT(resyncs);
	PRINT(tx_cnt);
	PRINT(rx_timeout);
	PRINT(rx_invalid_len);
	PRINT(rx_crc_error);
	PRINT(rx_success);
	PRINT(rx_same);
	PRINT(is_sync_leader);
	PRINT(proposed_signal);

#undef PRINT
	// #define PRINT(n)												\
	// 	ASSERT_CT(sizeof(Gpi_Hybrid_Tick) <= sizeof(long), n);		\
	// 	printf(#n ": %luus\n", (unsigned long)gpi_tick_hybrid_to_us(stats.n))

	// PRINT(rx_duration);

	// #undef PRINT

	printf("countdown_first_tx: %" PRIu8 "\n", stats.countdown_first_tx);
	printf("sync_id: %" PRIu8 "\n", s.sync_id);
	printf("sync_countdown: %" PRIu8 "\n", s.sync_countdown);

	unsigned int i;

	printf("resync_id=[");
	for (i = 0; i < stats.resync_idx; ++i)
	{
		printf("%" PRIu8 ";", stats.resync_id[i]);
	}
	printf("]\n");

	printf("resync_countdown=[");
	for (i = 0; i < stats.resync_idx; ++i)
	{
		printf("%" PRIu8 ";", stats.resync_countdown[i]);
	}
	printf("]\n");

	printf("resync_reason=[");
	for (i = 0; i < stats.resync_idx; ++i)
	{
		printf("%" PRIu8 ";", stats.resync_reason[i]);
	}
	printf("]\n");

	GPI_TRACE_RETURN_FAST();
}

//**************************************************************************************************

// Print details of Butler's configuration.
void butler_print_config(void)
{
#define PRINT(s) printf("%-25s = %" PRId32 "\n", #s, (int32_t)s)

	PRINT(P_TX);
	PRINT(BUTLER_DURATION);
	PRINT(MAX_SLOTS_BEFORE_TX);
	PRINT(GUARANTEED_TX_AFTER_SLOT);
	PRINT(SYNC_SLOT_LEN);
	PRINT(TEARDOWN_OFFSET);
	PRINT(INITIAL_TOLERANCE_MS);

#undef PRINT
}

//**************************************************************************************************

// Main API function. Starts the execution of Butler and returns the reference time and the sync
// origin after execution.
reference_t butler_start(uint8_t node_id)
{
	GPI_TRACE_FUNCTION_FAST();

	// one time random generator initializion
	if (!rand_init)
	{
		NRF_RNG->INTENCLR = BV_BY_NAME(RNG_INTENCLR_VALRDY, Clear);
		NRF_RNG->CONFIG   = BV_BY_NAME(RNG_CONFIG_DERCEN, Disabled); // bias correction
		NRF_RNG->SHORTS   = BV_BY_NAME(RNG_SHORTS_VALRDY_STOP, Enabled); // stop after VALRDY event
		NRF_RNG->TASKS_START = 1;

		while (NRF_RNG->EVENTS_VALRDY == 0)
			;

		NRF_RNG->TASKS_STOP = 1;
		rand_state          = BV_BY_VALUE(RNG_VALUE_VALUE, NRF_RNG->VALUE);
		rand_init           = 1;
	}

	// setup ISR's
	butler_setup();

	// init internal state and stats
	s.sync_time      = gpi_tick_fast_native();
	s.sync_countdown = BUTLER_DURATION;
	s.sync_id        = node_id;
	s.deadline       = s.sync_countdown * SYNC_SLOT_LEN + s.sync_time;
	s.synced         = 0;
	s.shutdown       = 0;
	s.last_action    = 0;
	s.node_id        = node_id;

	stats.resyncs            = 0;
	stats.tx_cnt             = 0;
	stats.rx_timeout         = 0;
	stats.rx_invalid_len     = 0;
	stats.rx_crc_error       = 0;
	stats.rx_success         = 0;
	stats.countdown_first_tx = 0;
	stats.is_sync_leader     = 0;
	stats.proposed_signal    = 0;
	stats.resync_idx         = 0;
	stats.rx_same            = 0;

	for (unsigned int i = 0; i < RESYNC_STAT_BUFFER_SIZE; i++)
	{
		stats.resync_id[i]        = 0;
		stats.resync_countdown[i] = 0;
		stats.resync_reason[i]    = 0;
	}

	// decide whether to start with TX or RX
	uint16_t r = butler_rand();

	// Start with TX.
	if (r < p_tx_value)
	{
		s.now                     = gpi_tick_fast_native();
		Gpi_Fast_Tick_Native t_tx = prepare_tx(s.now + RX_TX_RAMP_UP_DELAY + PREPARE_TX_BUFFER);

		// GPI_TRACE_MSG_FAST(TRACE_INFO, "starting with TX");
	}
	// Start with RX.
	else
	{
		unsigned int         initial_tx_slot = (butler_rand() % GUARANTEED_TX_AFTER_SLOT) + 1;
		Gpi_Fast_Tick_Native rx_duration     = initial_tx_slot * SYNC_SLOT_LEN;
		Gpi_Fast_Tick_Native t_rx            = prepare_rx(rx_duration + RX_TX_RAMP_UP_DELAY);

		// GPI_TRACE_MSG_FAST(TRACE_INFO, "starting with RX, TX at %u (slot %u)", t_rx /
		// GPI_TICK_US_TO_HYBRID2(1), initial_tx_slot);
	}

	// execution loop until finished
	while (!s.shutdown)
	{
		GPI_TRACE_FLUSH();
	}

	// teardown, restore state
	butler_teardown();

	reference_t ret = {.origin = s.sync_id, .time = s.deadline + TEARDOWN_OFFSET};

	// At the moment it is not possible to return a struct via trace functionality.
	// GPI_TRACE_RETURN_FAST(ret);
	return ret;
}

//**************************************************************************************************
//**************************************************************************************************