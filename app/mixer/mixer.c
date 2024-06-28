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
 *	@file					mixer.c
 *
 *	@brief					mixer API functions
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
#define TRACE_VERBOSE		GPI_TRACE_MSG_TYPE_VERBOSE

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION	GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(mixer, GPI_TRACE_BASE_SELECTION | GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "mixer_internal.h"

#include "gpi/olf.h"
#include "gpi/platform.h"
#include "gpi/interrupts.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************

#define assert_return(c, ...)						\
	do {											\
		assert(c);									\
		if (!(c))	/* in case NDEBUG is set */		\
			GPI_TRACE_RETURN(__VA_ARGS__);			\
	} while (0)

//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************



//**************************************************************************************************
//***** Forward Declarations ***********************************************************************



//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************



//**************************************************************************************************
//***** Global Variables ***************************************************************************

// mixer internal data
struct mx		mx;

//**************************************************************************************************
//***** Local Functions ****************************************************************************

#if (GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE)

void mx_trace_dump(const char *header, const void *p, uint_fast16_t size)
{
	char 			msg[3 * 16 + 1];
	char	 		*m = &(msg[0]);
	const uint8_t	*pc = p;

	while (size-- > 0)
	{
		if (&msg[sizeof(msg)] - m <= 3)
		{
			GPI_TRACE_MSG(GPI_TRACE_MSG_TYPE_VERBOSE, "%s%s ...", header, msg);
			m = &(msg[0]);
		}

		m += sprintf(m, " %02" PRIx8, *pc++);
	}

	GPI_TRACE_MSG(GPI_TRACE_MSG_TYPE_VERBOSE, "%s%s", header, msg);
}

#endif	// GPI_TRACE_MODE

//**************************************************************************************************
//***** Global Functions ***************************************************************************

void mixer_init(uint8_t node_id)
{
	GPI_TRACE_FUNCTION();

	assert_return(node_id < MX_NUM_NODES);

	unsigned int i;

	mixer_transport_init();

	mx.rx_queue_num_writing = 0;
	mx.rx_queue_num_written = 0;
	mx.rx_queue_num_read = 0;

	mx.tx_packet.sender_id = node_id;
	mx.tx_packet.flags.all = 0;
	mx.tx_packet.is_ready = 0;
	mx.tx_sideload = NULL;
	mx.tx_reserve = NULL;

	for (i = 0; i < MX_GENERATION_SIZE; i++)
		mx.matrix[i].birth_slot = UINT16_MAX;

	mx.rank = 0;
	mx.next_own_row = &mx.matrix[NUM_ELEMENTS(mx.matrix)];
	mx.recent_innovative_slot = 0;

	mx.slot_number = 0;
	mx.events = 0;

	memset(&mx.stat_counter, 0, sizeof(mx.stat_counter));

	for (i = 0; i < NUM_ELEMENTS(pt_data); ++i)
		PT_INIT(&pt_data[i]);

	#if MX_COORDINATED_TX
		mx_init_history();
		#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
			mx.wake_up_slot = 0;
			mx.discovery_exit_slot = -1;	// will be updated on next slot update
		#endif
	#endif

	#if MX_REQUEST
		memset(&mx.request, 0, sizeof(mx.request));
		memset(&mx.request.my_row_mask, -1, sizeof(mx.request.my_row_mask));
		memset(&mx.request.my_column_mask, -1, sizeof(mx.request.my_column_mask));

		// ATTENTION: signed is important (arithmetic shift fills vacant positions with 1's)
		int_fast_t mask = 1 << (sizeof(uint_fast_t) * 8 - 1);
		for (i = sizeof(mx.request.my_row_mask) * 8; i-- > MX_GENERATION_SIZE;)
			mask >>= 1;
		mx.request.padding_mask = ~(mask << 1);
		GPI_TRACE_MSG(TRACE_VERBOSE, "request padding mask: %0*x",
			sizeof(mx.request.padding_mask) * 2, mx.request.padding_mask);

		i = NUM_ELEMENTS(mx.request.my_row_mask) - 1;
		mx.request.my_row_mask[i] &= mx.request.padding_mask;
		mx.request.my_column_mask[i] &= mx.request.padding_mask;
	#endif

	#if MX_SMART_SHUTDOWN
		mx.is_shutdown_approved = 0;
		mx.have_full_rank_partner = 0;
		#if MX_SMART_SHUTDOWN_MAP
			mx_clear_full_rank_map();
			mx.full_rank_state = 0;
		#endif
	#endif

	#if MX_WEAK_ZEROS
		memset(&mx.weak_zero_map, 0, sizeof(mx.weak_zero_map));
		// set bits of non-existing entries
		if (MX_GENERATION_SIZE < (sizeof(mx.weak_zero_map.strong_mask) * 8))
			mx.weak_zero_map.strong_mask[MX_GENERATION_SIZE / FAST_T_WIDTH] = -1 << (MX_GENERATION_SIZE % FAST_T_WIDTH);
		mx.weak_rank = 0;
		mx.weak_zero_release_slot = 0;
		mx.weak_zero_return_msg = NULL;
	#endif

	mx.ref_slot = 0;
	mx.ref_time = 0;

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void mixer_print_config(void)
{
	#define PRINT(s) printf("%-25s = %" PRId32 "\n", #s, (int32_t)s)

	PRINT(offsetof(Packet, coding_vector));
	PRINT(offsetof(Packet, payload));
	#if (MX_REQUEST || MX_SMART_SHUTDOWN_MAP)
		PRINT(offsetof(Packet, info_vector));
	#endif

	PRINT(MX_NUM_NODES);
	PRINT(MX_GENERATION_SIZE);
	PRINT(MX_PAYLOAD_SIZE);
	PRINT(MX_SLOT_LENGTH);
	PRINT(GPI_FAST_CLOCK_RATE);
	PRINT(MX_ROUND_LENGTH);
	PRINT(MX_INITIATOR_ID);

	PRINT(MX_AGE_TO_INCLUDE_PROBABILITY);
	PRINT(MX_AGE_TO_TX_PROBABILITY);

	PRINT(GPI_TICK_MS_TO_HYBRID2(1));
	PRINT(GPI_TICK_US_TO_HYBRID2(1));

	#if MX_COORDINATED_TX
		PRINT(MX_COORDINATED_TX);
		PRINT(MX_HISTORY_DISCOVERY_BEHAVIOR);
	#endif
	#if MX_REQUEST
		PRINT(MX_REQUEST);
		PRINT(MX_REQUEST_HEURISTIC);
	#endif
	#if MX_SMART_SHUTDOWN
		PRINT(MX_SMART_SHUTDOWN);
		PRINT(MX_SMART_SHUTDOWN_MODE);
	#endif
	#if MX_WEAK_ZEROS
		PRINT(MX_WEAK_ZEROS);
	#endif

	#if MX_VERBOSE_STATISTICS
		PRINT(MX_VERBOSE_STATISTICS);
	#endif
	#if MX_VERBOSE_PROFILE
		PRINT(MX_VERBOSE_PROFILE);
	#endif
	#if MX_VERBOSE_PACKETS
		PRINT(MX_VERBOSE_PACKETS);
	#endif

	#ifdef MX_PHY_MODE
		PRINT(MX_PHY_MODE);
	#endif

	#undef PRINT

	mixer_transport_print_config();
}

//**************************************************************************************************

void mixer_set_weak_release_slot(uint16_t slot)
{
	GPI_TRACE_FUNCTION();

	#if MX_WEAK_ZEROS
		mx.weak_zero_release_slot = slot;
		GPI_TRACE_MSG(TRACE_VERBOSE, "weak_zero release slot: %" PRIu16, mx.weak_zero_release_slot);
	#else
		printf("!!! PANIC: MX_WEAK_ZEROS not set !!!\n");
		assert(0);
	#endif

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void mixer_set_weak_return_msg(void *msg)
{
	GPI_TRACE_FUNCTION();

	#if MX_WEAK_ZEROS
		mx.weak_zero_return_msg = msg;
	#else
		printf("!!! PANIC: MX_WEAK_ZEROS not set !!!\n");
		assert(0);
	#endif

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

size_t mixer_write(unsigned int i, const void *msg, size_t size)
{
	GPI_TRACE_FUNCTION();

	assert_return(i < MX_GENERATION_SIZE, 0);

	size = MIN(size, sizeof(mx.matrix[0].payload_8));

	#if !MX_WEAK_ZEROS
		assert_return(NULL != msg, 0);
	#else
		if (NULL == msg)
		{
			// it is not allowed to write weak zero to an already non-weak message
			// (because we would have to rollback several things, see below)
			assert_return(UINT16_MAX == mx.matrix[i].birth_slot, 0);

			mx.weak_zero_map.weak_mask[i / FAST_T_WIDTH] |= gpi_slu(1, i % FAST_T_WIDTH);
//			mx.matrix[i].birth_slot = UINT16_MAX - 1;
			mx.weak_rank++;

			GPI_TRACE_RETURN(size);

			// NOTE: we do not clear the corresponding bits in mx.request.my_row_mask and
			// mx.request.my_column_mask because the row may get strong during the round.
			// This is not an appreciable problem because weak zeros are exchanged quite fast,
			// i.e., they are up-to-date with high probability. Hence, if the only pending
			// request bits are those of weak zeros (this is the situation that could degrade
			// request performance), there is a high probability that node(s) can jump to full
			// rank (this resolves the problem).
			// Doing it the other way around, i.e. excluding weak zeros from request bits,
			// would require some effort to re-include them in case they become strong.
		}
		else mx.weak_zero_map.strong_mask[i / FAST_T_WIDTH] |= gpi_slu(1, i % FAST_T_WIDTH);
	#endif

	gpi_memcpy_dma(mx.matrix[i].payload_8, msg, size);

	unwrap_row(i);

	memset(mx.matrix[i].coding_vector, 0, sizeof(mx.matrix[0].coding_vector));
	mx.matrix[i].coding_vector_8[i / 8] |= 1 << (i % 8);

	mx.matrix[i].birth_slot = 0;
	mx.rank++;

	if (NULL == mx.tx_reserve)
		mx.tx_reserve = &mx.matrix[i];

	if (mx.next_own_row > &mx.matrix[i])
		mx.next_own_row = &mx.matrix[i];

	#if MX_REQUEST
		mx.request.my_row_mask[i / FAST_T_WIDTH] &= ~(1 << (i % FAST_T_WIDTH));
		mx.request.my_column_mask[i / FAST_T_WIDTH] &= ~(1 << (i % FAST_T_WIDTH));
	#endif

	GPI_TRACE_RETURN(size);
}

//**************************************************************************************************

void mixer_arm(Mixer_Start_Mode mode)
{
	GPI_TRACE_FUNCTION();

	// mark an empty row (used by rx processing)
	mx.empty_row = NULL;
	if (mx.rank < MX_GENERATION_SIZE)
	{
		Matrix_Row *p = &(mx.matrix[NUM_ELEMENTS(mx.matrix)]);
		while (p-- > 0)
		{
			if (UINT16_MAX == p->birth_slot)
			{
				mx.empty_row = p;
				break;
			}
		}
	}

	// if initiator: arm TX (instead of RESYNC)
	if (mode & MX_ARM_INITIATOR)
	{
		assert(NULL != mx.tx_reserve);

		mx.tx_sideload = &(mx.next_own_row->coding_vector_8[0]);

		while (++mx.next_own_row < &(mx.matrix[NUM_ELEMENTS(mx.matrix)]))
		{
			if (0 == mx.next_own_row->birth_slot)
				break;
		}

		mixer_transport_arm_initiator();
	}

	// launch threads
	// NOTE: this gives all threads the opportunity to init thread-local data
	(void) PT_SCHEDULE(mixer_maintenance());
	(void) PT_SCHEDULE(mixer_update_slot());
	(void) PT_SCHEDULE(mixer_process_rx_data());

	// if sync round: don't update deadline before first packet reception
	// ATTENTION: the way of doing that here is a bit crude. It is associated to the
	// maintenance thread; look there for details.
	if (mode & MX_ARM_INFINITE_SCAN)
		mx.round_deadline_update_slot = 0;

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

Gpi_Hybrid_Tick mixer_start()
{
	GPI_TRACE_FUNCTION_FAST();

	mixer_transport_start();

	unsigned int event_mask = BV(SLOT_UPDATE) | BV(TRIGGER_TICK);
	while (event_mask)
	{
		// isolate highest priority pending event
		unsigned int event = mx.events & event_mask;
		event &= -event;	// mask LSB (extracts least significant 1 bit)

		switch (event)
		{
			// if a thread exits, it triggered the stop procedure of Mixer -> mask all events
			// except STOPPED. This ensures that the exited thread gets no longer scheduled.
			// ATTENTION: The thread may rely on that behavior.

			case BV(SLOT_UPDATE):
				if (!PT_SCHEDULE(mixer_update_slot()))
					event_mask = BV(STOPPED);
				break;

			case BV(TRIGGER_TICK):
				if (!PT_SCHEDULE(mixer_maintenance()))
					event_mask = BV(STOPPED);
				break;

			default:
			{
				GPI_TRACE_FLUSH();

				if (PT_WAITING == PT_SCHEDULE_STATE(mixer_process_rx_data()))
				{
					// after graceful stop has been performed
					if (BV(STOPPED) == event)
					{
						GPI_TRACE_MSG(TRACE_INFO, "interrupts stopped");

						// if deadline reached: set mx.slot_number to last slot
						// NOTE: Since the deadline has been reached, we know that we are there.
						// If we would not update mx.slot_number then it would be possible that
						// some of the stat_counters (slot_off...) get wrong values if node was
						// in RESYNC when stopping.
						if (mx.events & BV(DEADLINE_REACHED))
							mx.slot_number = MX_ROUND_LENGTH;

						// exit loop
						event_mask = 0;
						break;
					}

					// enter low-power mode
					gpi_int_disable();
					if (!(mx.events & event_mask))
					{
						#if MX_VERBOSE_STATISTICS

							ASSERT_CT(!(GPI_FAST_CLOCK_RATE % GPI_HYBRID_CLOCK_RATE), FAST_HYBRID_ratio_must_be_integer);
							ASSERT_CT(IS_POWER_OF_2(FAST_HYBRID_RATIO), FAST_HYBRID_ratio_must_be_power_of_2);

							#if (GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE)
								const int USE_NATIVE = 0;
							#else
								const int USE_NATIVE =
									((MX_SLOT_LENGTH_RESYNC * FAST_HYBRID_RATIO + 0x1000) <
									(Gpi_Fast_Tick_Native)GPI_TICK_FAST_MAX);
							#endif

							Gpi_Hybrid_Tick	time;

							if (USE_NATIVE)
								time = gpi_tick_fast_native();
							else time = gpi_tick_hybrid();

						#endif

						// enter sleep mode
						// NOTE: reenables interrupts (they serve as wake-up events)
						gpi_sleep();

						// ...
						// awake again

						#if MX_VERBOSE_STATISTICS
							// ATTENTION: time up to here includes execution time of one or more ISRs.
							// To support low-power time measurements, every (relevant) ISR stores
							// the wake-up timestamp on ISR entry (in case it is a wake-up event).
							// This is what we use here.
							if (USE_NATIVE)
								time = (Gpi_Fast_Tick_Native)(mx.wake_up_timestamp - time) / FAST_HYBRID_RATIO;
							else time = gpi_tick_fast_to_hybrid(mx.wake_up_timestamp) - time;

							mx.stat_counter.low_power_time += time;
						#endif
					}
					else gpi_int_enable();
				}
			}
		}
	}

	// try to solve (if not done already)
	if (mx.rank < MX_GENERATION_SIZE)
	{
		Pt_Context *pt = &pt_data[0];
		PT_INIT(pt);
		while (PT_SCHEDULE(mixer_decode(pt)));
	}

	GPI_TRACE_MSG(TRACE_INFO, "mixer stopped");

	GPI_TRACE_RETURN(mx.round_deadline);
}

//**************************************************************************************************

void mixer_print_statistics(void)
{
	#if MX_VERBOSE_STATISTICS
		mx.stat_counter.radio_on_time /= FAST_HYBRID_RATIO;

		#ifdef PRINT
			#error change macro name
		#endif

		printf("statistics:\n");

		#define PRINT(n)	\
			ASSERT_CT(sizeof(mx.stat_counter.n) == sizeof(uint16_t), n);	\
			printf(#n ": %" PRIu16 "\n", mx.stat_counter.n)

		PRINT(num_sent);
		PRINT(num_received);
		PRINT(num_resync);
		PRINT(num_grid_drift_overflow);
		PRINT(num_rx_window_overflow);
		PRINT(num_rx_success);
		PRINT(num_rx_broken);
		PRINT(num_rx_timeout);
		PRINT(num_rx_dma_timeout);
		PRINT(num_rx_dma_late);
		PRINT(num_rx_late);
		PRINT(num_tx_late);
		PRINT(num_tx_zero_packet);
		PRINT(num_tx_fifo_late);
		PRINT(num_grid_late);
		PRINT(num_rx_slot_mismatch);
		PRINT(num_rx_queue_overflow);
		PRINT(num_rx_queue_overflow_full_rank);
		PRINT(num_rx_queue_processed);
		PRINT(slot_full_rank);
		PRINT(slot_decoded);
		PRINT(slot_off);
		PRINT(discovery_exit_slot);
		PRINT(discovery_density);
		PRINT(wake_up_slot);

		#undef PRINT
		#define PRINT(n)	\
			ASSERT_CT(sizeof(Gpi_Hybrid_Tick) <= sizeof(long), n);		\
			printf(#n ": %luus\n", (unsigned long)gpi_tick_hybrid_to_us(mx.stat_counter.n))

		PRINT(radio_on_time);
		PRINT(low_power_time);

		#undef PRINT
	#endif
}

//**************************************************************************************************

void* mixer_read(unsigned int i)
{
	GPI_TRACE_FUNCTION();

	assert_return(i < MX_GENERATION_SIZE, (void*)NULL);

	if (UINT16_MAX == mx.matrix[i].birth_slot)
	{
		#if MX_WEAK_ZEROS
			if (mx.weak_zero_map.weak_mask[i / FAST_T_WIDTH] & gpi_slu(1, i % FAST_T_WIDTH))
				GPI_TRACE_RETURN(mx.weak_zero_return_msg);
		#endif

		GPI_TRACE_RETURN((void*)NULL);
	}

	uint8_t m = 1 << (i % 8);
	mx.matrix[i].coding_vector_8[i / 8] ^= m;
	int_fast16_t k = mx_get_leading_index(mx.matrix[i].coding_vector_8);
	mx.matrix[i].coding_vector_8[i / 8] ^= m;

	if (k >= 0)
		GPI_TRACE_RETURN((void*)NULL);

	unwrap_row(i);

	GPI_TRACE_RETURN(&mx.matrix[i].payload_8);
}

//**************************************************************************************************

int16_t mixer_stat_slot(unsigned int i)
{
//	GPI_TRACE_FUNCTION();

	assert_return(i < MX_GENERATION_SIZE, -1);

	#if MX_WEAK_ZEROS
		if (UINT16_MAX == mx.matrix[i].birth_slot)
		{
			if (mx.weak_zero_map.weak_mask[i / FAST_T_WIDTH] & gpi_slu(1, i % FAST_T_WIDTH))
			{
			#if MX_VERBOSE_STATISTICS
				if (mx.rank >= MX_GENERATION_SIZE)
					return mx.stat_counter.slot_full_rank;
//					GPI_TRACE_RETURN(mx.stat_counter.slot_full_rank);
				else
					return mx.stat_counter.slot_off;
//					GPI_TRACE_RETURN(mx.stat_counter.slot_off);
			#else
				return mx.slot_number;
//				GPI_TRACE_RETURN(mx.slot_number);
			#endif
			}
		}
	#endif

	return mx.matrix[i].birth_slot;
//	GPI_TRACE_RETURN(mx.matrix[i].birth_slot);
}

//**************************************************************************************************

Mixer_Stat_Counter* mixer_statistics(void)
{
	return &mx.stat_counter;
}

//**************************************************************************************************
//**************************************************************************************************
