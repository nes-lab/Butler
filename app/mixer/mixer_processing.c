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
 *	@file					mixer_processing.c
 *
 *	@brief					Mixer processing layer
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
#define TRACE_INFO				GPI_TRACE_MSG_TYPE_INFO
#define TRACE_WARNING			GPI_TRACE_MSG_TYPE_WARNING
#define TRACE_ERROR				GPI_TRACE_MSG_TYPE_ERROR
#define TRACE_VERBOSE			GPI_TRACE_MSG_TYPE_VERBOSE
#define TRACE_VERBOSE_MATRIX	GPI_TRACE_MSG_TYPE_VERBOSE

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION	GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(mixer_processing, GPI_TRACE_BASE_SELECTION | GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "mixer_internal.h"
#include "mixer_discovery.h"

#include "gpi/tools.h"
#include "gpi/platform.h"
#include "gpi/platform_spec.h"
#include "gpi/interrupts.h"
#include "gpi/olf.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#if GPI_ARCH_IS_CORE(MSP430)
	#include "tmote/memxor.h"
#elif GPI_ARCH_IS_CORE(ARMv7M)
	#include "armv7-m/memxor.h"
#else
	#error unsupported architecture
#endif

#if MX_VERBOSE_PROFILE
	GPI_PROFILE_SETUP("mixer_processing.c", 1800, 4);
#endif

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************



//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************



//**************************************************************************************************
//***** Forward Declarations ***********************************************************************



//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

Pt_Context						pt_data[3];

static Pt_Context* const		pt_update_slot 		= &pt_data[0];
static Pt_Context* const		pt_process_rx_data 	= &pt_data[1];
static Pt_Context* const		pt_maintenance 		= &pt_data[2];

#if (MX_COORDINATED_TX || MX_REQUEST)
	static unsigned int			rx_queue_num_read_update;
#endif

//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Local Functions ****************************************************************************

#if MX_VERBOSE_PACKETS
	#define TRACE_PACKET(p)		trace_packet(p)

static void trace_packet(const Packet *p)
{
	char msg[40 + (sizeof(p->coding_vector) * 2 * 2) + (sizeof(p->payload) * 2)];
	char *ps = msg;
	int  i;

	ASSERT_CT(2 == sizeof(p->slot_number), check_PRI_formats);
	ASSERT_CT(1 == sizeof(p->sender_id), check_PRI_formats);
	ASSERT_CT(1 == sizeof(p->flags), check_PRI_formats);

	#if !(GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB) || GPI_ARCH_IS_BOARD(TMOTE_INDRIYA))
		ps += sprintf(ps, "# ID:%u ", (int)mx.tx_packet.sender_id + 1);
	#endif

	// node id MSB marks vector bit order (for log parser):
	// 0: LSB first, big-endian
	// 1: LSB first, little-endian
	ps += sprintf(ps, "%04" PRIx16 " - %04" PRIx16 " - %02" PRIx8 " - ",
		p->slot_number, (uint16_t)(p->sender_id |
		#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
			0x8000
		#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
			0
		#else
			#error unsupported architecture
		#endif
		), p->flags.all);

	for (i = 0; i < sizeof(p->coding_vector); i++)
#if MX_INCLUDE_INFO_VECTOR
		ps += sprintf(ps, "%02" PRIx8, p->info_vector[i]);
//		ps += sprintf(ps, "%02" PRIx8, (uint8_t)~(p->info_vector[i]));
#else
		ps += sprintf(ps, "00");
#endif

	ps += sprintf(ps, " - ");

	for (i = 0; i < sizeof(p->coding_vector); i++)
		ps += sprintf(ps, "%02" PRIx8, p->coding_vector[i]);

	ps += sprintf(ps, " - ");

	for (i = 0; i < sizeof(p->payload); i++)
		ps += sprintf(ps, "%02" PRIx8, p->payload[i]);

	assert(ps < &msg[sizeof(msg)]);

	printf("%s\n", msg);
}

#else
	#define TRACE_PACKET(p)		while(0)
#endif	// MX_VERBOSE_PACKETS

//**************************************************************************************************

#if (GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE)
	#if 0
		#define TRACE_MATRIX()		trace_matrix()
	#else
		#pragma message "TRACE_MATRIX macro is deactivated despite GPI_TRACE_MODE_TRACE"
		#define TRACE_MATRIX()
	#endif

static void trace_matrix()
{
	char 		msg[128];
	char		*m = &(msg[0]);
	uint16_t	r, i;
	uint8_t		v;

	GPI_TRACE_MSG(TRACE_VERBOSE_MATRIX, "matrix:");

	for (r = 0; r < NUM_ELEMENTS(mx.matrix); r++)
	{
		m = &(msg[0]);
		m += sprintf(m, "%3" PRId16 ":", r);

		for (i = 0; i < NUM_ELEMENTS(mx.matrix[0].coding_vector_8); i++)
		{
			v = mx.matrix[r].coding_vector_8[i];

			if (&msg[sizeof(msg)] - m <= 3)
			{
				GPI_TRACE_MSG(TRACE_VERBOSE_MATRIX, "%s ...", msg);
				m = &(msg[0]);
			}

			m += sprintf(m, " %02" PRIx8, v);
		}

		if (&msg[sizeof(msg)] - m <= 3)
		{
			GPI_TRACE_MSG(TRACE_VERBOSE_MATRIX, "%s ...", msg);
			m = &(msg[0]);
		}

		*m++ = ' ';
		*m = '\0';

		for (i = 0; i < NUM_ELEMENTS(mx.matrix[0].payload_8); i++)
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Warray-bounds"

			const uint8_t	offset = offsetof(Matrix_Row, payload) - offsetof(Matrix_Row, payload_8);

			if (i < offset)
				v = mx.matrix[r].payload_8[i + NUM_ELEMENTS(mx.matrix[0].payload_8)];
			else v = mx.matrix[r].payload_8[i];

			#pragma GCC diagnostic pop

			if (&msg[sizeof(msg)] - m <= 3)
			{
				GPI_TRACE_MSG(TRACE_VERBOSE_MATRIX, "%s ...", msg);
				m = &(msg[0]);
			}

			m += sprintf(m, " %02" PRIx8, v);
		}

		GPI_TRACE_MSG(TRACE_VERBOSE_MATRIX, "%s", msg);
	}
}

#else
	#define TRACE_MATRIX()		while (0)
#endif	// GPI_TRACE_MODE

//**************************************************************************************************
//**************************************************************************************************

static inline void clear_event(Event event)
{
	gpi_atomic_clear(&mx.events, BV(event));
}

// TODO: copied from mixer_transport_ble.c
static inline __attribute__((always_inline)) void set_event(Event event)
{
	// ATTENTION: use API function to ensure that load/store exclusive works right (if used)
	gpi_atomic_set(&(mx.events), BV(event));
}

//**************************************************************************************************
#if MX_SMART_SHUTDOWN_MAP

void mx_clear_full_rank_map()
{
	GPI_TRACE_FUNCTION();

	memset(&mx.full_rank_map, 0, sizeof(mx.full_rank_map));

	// set bits of non-existing nodes
	if (MX_NUM_NODES < (sizeof(mx.full_rank_map.map) * 8))
	{
		uint_fast8_t i;

		mx.full_rank_map.map[MX_NUM_NODES / 8] = -1 << (MX_NUM_NODES % 8);

		for (i = (MX_NUM_NODES + 7) / 8; i < sizeof(mx.full_rank_map.map); ++i)
			mx.full_rank_map.map[i] = -1;
	}

	TRACE_DUMP(TRACE_VERBOSE, "full-rank map:", &mx.full_rank_map.map, sizeof(mx.full_rank_map.map));

	GPI_TRACE_RETURN();
}

#endif	// MX_SMART_SHUTDOWN_MAP
//**************************************************************************************************
#if (MX_SMART_SHUTDOWN_MODE >= 2)

static void update_full_rank_map(const Packet *p)
{
	GPI_TRACE_FUNCTION();
	PROFILE("update_full_rank_map() entry");

	const uint8_t 	*ps;
	uint8_t			*pd;
	unsigned int 	i;

	// if we reached full rank (p == NULL marks this special call)
	if (NULL == p)
		mx.full_rank_map.map[mx.tx_packet.sender_id / 8] |= gpi_slu(1, mx.tx_packet.sender_id % 8);

	// update map and history
	else if (p->flags.is_full_rank)
	{
		// if FULL_RANK_ACK received
		if ((p->flags.all & EIT_FULL_RANK_ACK_MAP_mask) == EIT_FULL_RANK_ACK_MAP_pattern)
		{
			// FULL_RANK_ACK implies that all nodes reached full rank -> switch to ACK mode
			if (mx.full_rank_state < 1)
			{
				memset(&mx.full_rank_map.map, -1, sizeof(mx.full_rank_map.map));
				update_full_rank_map(NULL);
			}
		}

		// if in ACK mode: drop non-ACK data (which is obsolete now)
		else if (mx.full_rank_state != 0)
			GPI_TRACE_RETURN();

		mx.full_rank_map.map[p->sender_id / 8] |= gpi_slu(1, p->sender_id % 8);

		if ((p->flags.all & EIT_FULL_RANK_X_mask) == EIT_FULL_RANK_X_pattern)
		{
			// NOTE: using nested loops unfolds info_vector against full_rank_map, e.g. like this:
			// pd: 0 1 2 3 4 5 6 7 8 9 ...
			// ps: 0 1 2 3 0 1 2 3 0 1 ...
			for (pd = &mx.full_rank_map.map[0]; pd < &mx.full_rank_map.map[NUM_ELEMENTS(mx.full_rank_map.map)]; )
				for (ps = &(p->info_vector[0]); ps < &(p->info_vector[sizeof(p->info_vector)]);)
					*pd++ |= *ps++;
		}

		Node *pn;

		for (pn = &mx.history[mx_present_head->next]; pn != mx_present_head;)
		{
			i = ARRAY_INDEX(pn, mx.history);
			// ATTENTION: depending on sizeof(mx.history[0]), the compiler may generate an
			// expensive division operation. This is not such critical at this place because
			// every neighbor is moved to the finished list only once.

			// ATTENTION: update pn before changing the node, else pn->next may point to a finished
			// node and the for loop becomes an endless loop (because pn != mx_present_head will
			// never be true)
			pn = &mx.history[pn->next];

			if ((mx.full_rank_state > 0) || (mx.full_rank_map.map[i / 8] & gpi_slu(1, i % 8)))
			{
				Packet_Flags flags = {0};
				flags.is_full_rank = 1;

				mx_update_history(i, flags, mx.slot_number);

				// NOTE: using mx.slot_number causes a history update for node i. This is not critical
				// since node i is present (we took it from the present-list). If we would take the
				// slot number from node i's history entry, we could destroy the order in the
				// finished-list. For the same reason we don't take slot number from p because the
				// processing here can fall a bit behind the regular history updates (under high
				// CPU load).
			}
		}

#if (MX_SMART_SHUTDOWN_MODE >= 4)
		if (mx.full_rank_state > 0)
		{
			for (pn = &mx.history[mx_finished_head->next]; pn != mx_finished_head;)
			{
				// ATTENTION: comments see above

				i = ARRAY_INDEX(pn, mx.history);
				pn = &mx.history[pn->next];

				if ((mx.full_rank_state > 1) || (mx.full_rank_map.map[i / 8] & gpi_slu(1, i % 8)))
				{
					// NOTE: setting is_full_rank is enough, distinguishing between finished
					// and ACKed state is done in mx_update_history()
					Packet_Flags flags = {0};
					flags.is_full_rank = 1;

					mx_update_history(i, flags, mx.slot_number);
				}
			}
		}
#endif
	}

	// update hash
	if (sizeof(mx.full_rank_map.map) <= sizeof(mx.full_rank_map.hash))
		gpi_memcpy_dma_inline(mx.full_rank_map.hash, mx.full_rank_map.map, sizeof(mx.full_rank_map.map));
	else
	{
		for (i = 0; i < NUM_ELEMENTS(mx.full_rank_map.hash); ++i)
		{
			// ATTENTION: mx.full_rank_map.hash is also read on ISR level. Therefore it is important
			// to work on a temporary variable such that mx.full_rank_map.hash never marks unfinished
			// nodes.

			uint8_t	hash = -1;

			for (ps = &mx.full_rank_map.map[i];
				 ps < &mx.full_rank_map.map[NUM_ELEMENTS(mx.full_rank_map.map)];
				 ps += NUM_ELEMENTS(mx.full_rank_map.hash))
			{
				hash &= *ps;
			}

			gpi_atomic_write(&(mx.full_rank_map.hash[i]), hash);
		}
	}

	// update full_rank state
	if ((mx.rank >= MX_GENERATION_SIZE) && (mx.full_rank_state >= 0) && (mx.full_rank_state < 2))
	{
		int_fast8_t state = 1;

		for (ps = &mx.full_rank_map.hash[0];
			 ps < &mx.full_rank_map.hash[NUM_ELEMENTS(mx.full_rank_map.hash)];
			 ++ps)
		{
			if (-1 != (int8_t)*ps)
			{
				state = 0;
				break;
			}
		}

		if (state)
		{
			GPI_TRACE_MSG(TRACE_INFO, "full rank map complete");

			state = mx.full_rank_state;

			if (state == 0)
			{
				// mark temporary state
				mx.full_rank_state = -1;

				// ATTENTION: mx.full_rank_map.hash is also read on ISR level. Therefore it is important
				// to ensure that mx.full_rank_map.hash never marks unfinished nodes. However, the
				// opposite case (not marking finished nodes) is not a critical problem. Hence, we do
				// not handle it here explicitly. (Besides that, this part of the program is reached
				// only once, so maybe once sending a suboptimal info_vector does not disturb much.)

				// clear map and hash
				mx_clear_full_rank_map();

				// recursive call to set own flag and update hash
				// ATTENTION: temporary state is important to avoid endless nesting loop
				update_full_rank_map(NULL);
			}

			// update state
			mx.full_rank_state = ++state;
		}
	}

	PROFILE("update_full_rank_map() return");

	TRACE_DUMP(1, "full-rank map: ", mx.full_rank_map.map, sizeof(mx.full_rank_map.map));
	TRACE_DUMP(1, "full-rank hash:", mx.full_rank_map.hash, sizeof(mx.full_rank_map.hash));
	GPI_TRACE_MSG(TRACE_VERBOSE, "full-rank state: %" PRId8, mx.full_rank_state);

	GPI_TRACE_RETURN();
}

#endif
//**************************************************************************************************
#if MX_WEAK_ZEROS

static void update_weak_zero_map(const Packet *p)
{
	GPI_TRACE_FUNCTION();
	PROFILE("update_weak_zero_map() entry");

	ASSERT_CT(!(offsetof(Packet, coding_vector) % sizeof(uint_fast_t)), alignment_issue);

	const uint_fast_t	*cv;
	uint_fast_t			*sm, *wm, x;

	// update strong map
	cv = (uint_fast_t*)&(p->coding_vector[0]);
	sm = &(mx.weak_zero_map.strong_mask[0]);
	while (sm < &(mx.weak_zero_map.strong_mask[NUM_ELEMENTS(mx.weak_zero_map.strong_mask)]))
		*sm++ |= *cv++;

	// update weak map from strong map
	sm = &(mx.weak_zero_map.strong_mask[0]);
	wm = &(mx.weak_zero_map.weak_mask[0]);
	while (wm < &(mx.weak_zero_map.weak_mask[NUM_ELEMENTS(mx.weak_zero_map.weak_mask)]))
	{
		x = *wm & *sm++;
		if (x)
		{
			// ATTENTION: mx.weak_zero_map.weak_mask is also read on ISR level. Therefore it is
			// important that *wm never marks entries that are not known to be weak zeros, i.e,
			// to avoid inconsistent intermediate results.
			*(volatile uint_fast_t*)wm = *wm ^ x;

			mx.weak_rank -= gpi_popcnt(x);
//			do
//			{
//				mx.matrix[TODO + get_lsb(x)].birth_slot = TODO;
//				x &= -x;
//			}
//			while (x);
		}
		++wm;
	}

	// update weak map from info vector
	if (p->flags.info_type == IT_WEAK_ZERO_MAP)
	{
		const uint8_t	*iv;
		uint8_t			*sm8, *wm8;

		iv = &(p->info_vector[0]);
		sm8 = (uint8_t*)&(mx.weak_zero_map.strong_mask[0]);
		wm8 = (uint8_t*)&(mx.weak_zero_map.weak_mask[0]);
		while (iv < &(p->info_vector[NUM_ELEMENTS(p->info_vector)]))
		{
			x = *iv++ & ~*sm8++;
			x &= ~*wm8;
			if (x)
			{
				// ATTENTION: mx.weak_zero_map.weak_mask is also read on ISR level. Therefore it is
				// important that *wm never marks entries that are not known to be weak zeros, i.e,
				// to avoid inconsistent intermediate results.
				*(volatile uint8_t*)wm8 = *wm8 ^ x;

				mx.weak_rank += gpi_popcnt_8(x);
//				do
//				{
//					mx.matrix[TODO + get_lsb(x)].birth_slot = TODO;
//					x &= -x;
//				}
//				while (x);
				mx.recent_innovative_slot = p->slot_number;
			}
			++wm8;
		}
	}

	PROFILE("update_weak_zero_map() return");

	TRACE_DUMP(1, "strong map:", mx.weak_zero_map.strong_mask, sizeof(mx.weak_zero_map.strong_mask));
	TRACE_DUMP(1, "weak map:  ", mx.weak_zero_map.weak_mask, sizeof(mx.weak_zero_map.weak_mask));
	GPI_TRACE_MSG(TRACE_VERBOSE, "weak rank: %" PRIu16, mx.weak_rank);

	GPI_TRACE_RETURN();
}

#endif // MX_WEAK_ZEROS
//**************************************************************************************************
//***** Global Functions ***************************************************************************

static void prepare_tx_packet()
{
	GPI_TRACE_FUNCTION();
	PROFILE("prepare_tx_packet() entry");

	const uint16_t	CHUNK_SIZE = sizeof(mx.tx_packet.coding_vector) + sizeof(mx.tx_packet.payload);
	Matrix_Row		*p;
	void			*pp[MEMXOR_BLOCKSIZE];
	int				pp_used = 0;
	int_fast16_t	used = 0;
	#if MX_REQUEST
		Matrix_Row	*help_row = 0;
	#endif

	assert_msg(NULL != mx.tx_reserve, "Tx without data -> must not happen");

	// clear mx.tx_packet by adding itself to the xor list
	pp[pp_used++] = &mx.tx_packet.coding_vector;

#if !MX_BENCHMARK_NO_SYSTEMATIC_STARTUP

	// prefer unsent own rows because they are innovative for sure
	// NOTE: unsent own rows are innovative always, also while (row) requests are pending.
	// However, we follow the standard behavior in this case to be consistent with the several
	// request mechanisms. (By the way, there is a good chance that unsent own rows belong to the
	// requested rows anyhow.)
	if (mx.next_own_row < &mx.matrix[NUM_ELEMENTS(mx.matrix)])
#if MX_REQUEST
		if (mx.request.help_index <= 0)
#endif
	{
		p = mx.next_own_row;

		// mark that we don't need the reserve
		used++;

		// restore packed version (in place)
		wrap_chunk(p->coding_vector_8);

		// add it to xor list
		pp[pp_used++] = &(p->coding_vector);

		// look for next own row
		while (++mx.next_own_row < &mx.matrix[NUM_ELEMENTS(mx.matrix)])
		{
			if (0 == mx.next_own_row->birth_slot)
				break;
		}
	}

#endif

	if (!used)
	{
		#if MX_REQUEST
			if (mx.request.help_index > 0)
				help_row = &mx.matrix[mx.request.help_index - 1];
		#endif

		// traverse matrix
		for (p = &mx.matrix[0]; p < &mx.matrix[NUM_ELEMENTS(mx.matrix)]; p++)
		{
			if (UINT16_MAX == p->birth_slot)
				continue;

			#if MX_REQUEST
				// if row request help index selected: skip all up to that row
				// NOTE: the help row itself will be automatically included by sideload
				if (p <= help_row)
					continue;
			#endif

			PROFILE("prepare_tx_packet() mixer_rand() begin");

			uint16_t r = mixer_rand();

			PROFILE("prepare_tx_packet() mixer_rand() end");

			// choose any available row as reserve, update from time to time
			// -> as reserve sideload and for the case that we select nothing by rolling the dice
			if (!(r & 7))
				mx.tx_reserve = p;

			// include current row?
			{
				static const uint16_t LUT[] = MX_AGE_TO_INCLUDE_PROBABILITY;
				ASSERT_CT(sizeof(LUT) > 0, MX_AGE_TO_INCLUDE_PROBABILITY_is_invalid);

				if (!(r < LUT[MIN(mx.slot_number - p->birth_slot, NUM_ELEMENTS(LUT) - 1)]))
					continue;
			}

			// mark that we don't need the reserve
			used++;

			// restore packed version (in place)
			wrap_chunk(p->coding_vector_8);

			// add it to xor list, work through if needed
			pp[pp_used++] = &(p->coding_vector);
			if(NUM_ELEMENTS(pp) == pp_used)
			{
				PROFILE("prepare_tx_packet() memxor_block(full) begin");

				// NOTE: calling with NUM_ELEMENTS(pp) instead of pp_used leads to a bit better
				// code because NUM_ELEMENTS(pp) is a constant (msp430-gcc 4.6.3)
				memxor_block(mx.tx_packet.coding_vector, pp, CHUNK_SIZE, NUM_ELEMENTS(pp));
				pp_used = 0;

				PROFILE("prepare_tx_packet() memxor_block(full) end");
			}

			ASSERT_CT(!(offsetof(Packet, coding_vector) % sizeof(uint_fast_t)), alignment_issue);
			ASSERT_CT(offsetof(Packet, payload) ==
				offsetof(Packet, coding_vector) + sizeof_member(Packet, coding_vector),
				inconsistent_program);
			ASSERT_CT(offsetof(Matrix_Row, payload_8) ==
				offsetof(Matrix_Row, coding_vector) + sizeof_member(Matrix_Row, coding_vector_8),
				inconsistent_program);
		}
	}

	// if we didn't select any row: use the reserve
	// NOTE: mx.tx_reserve != NULL checked by assertion above
	// ATTENTION: don't do that if request is active to make sure that helper sideload is successfull.
	// This may generate an empty packet which is no problem because of the sideload. The other way
	// around could lead to a critical corner case if the sideload points to the same row as
	// mx.tx_reserve: The result would be a zero packet, i.e. no successful transmission and - in case
	// of high tx probability - a subsequent try to transmit. Since the request situation does not
	// change in this time, there is a good chance that we rebuild the same packet. If this happens,
	// the whole procedure starts again and again and does not end before mx.tx_reserve gets updated.
	// But this never happens if the requested row is the last one in the matrix.
	#if MX_REQUEST
		if (!used && !help_row)
	#else
		if (!used)
	#endif
	{
		// NOTE: we cast const away which is a bit dirty. We need this only to restore packed
		// version which is such a negligible change that we prefer mx.tx_reserve to appear as const.
		p = (Matrix_Row*)mx.tx_reserve;

		// restore packed version (in place)
		wrap_chunk(p->coding_vector_8);

		// add it to xor list
		// NOTE: memcpy instead of memxor would also be possible here,
		// but the situation is not very time critical (xored nothing up to here)
		pp[pp_used++] = &(p->coding_vector);
	}

	// work through the xor list
	if (pp_used)
		memxor_block(mx.tx_packet.coding_vector, pp, CHUNK_SIZE, pp_used);

	PROFILE("prepare_tx_packet() return");

	TRACE_DUMP(1, "tx_packet:", &(mx.tx_packet.phy_payload_begin), PHY_PAYLOAD_SIZE);

	GPI_TRACE_RETURN();
}

//**************************************************************************************************

PT_THREAD(mixer_update_slot())
{
	Pt_Context* const	pt = pt_update_slot;

	PT_BEGIN(pt);

#if MX_COORDINATED_TX
	static uint16_t				owner, last_owner_update;
	#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
		static Packet_Flags		owner_forecast[2];
	#endif
#endif

	// init variables at thread startup
	// NOTE: approach is useful because thread gets reinitialized (PT_INIT) on each mixer round
#if (MX_COORDINATED_TX || MX_REQUEST)
	rx_queue_num_read_update	= 0;
#endif
#if MX_COORDINATED_TX
	owner 			  = 0;
	last_owner_update = 0;
	#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
		owner_forecast[0].all = 0;
		owner_forecast[1].all = 0;
	#endif
#endif

	while (1)
	{
		PT_WAIT_UNTIL(pt, mx.events & BV(SLOT_UPDATE));
		clear_event(SLOT_UPDATE);

		#if MX_VERBOSE_PACKETS
			if (mx.events & BV(TX_READY))
				TRACE_PACKET(&mx.tx_packet);
		#endif

//TRACE_DUMP(1, "my_row_mask:", mx.request.my_row_mask, sizeof(mx.request.my_row_mask));
//TRACE_DUMP(1, "my_column_mask:", mx.request.my_column_mask, sizeof(mx.request.my_column_mask));

		Slot_Activity		next_task;
		uint16_t			p = 0;

		#if MX_COORDINATED_TX
			uint16_t		density;		// local network density
			Packet_Flags	flags = {0};
		#endif

		// use local variable since mx.slot_number is volatile (and for performance)
		// NOTE: some pieces of code rely on the assumption that slot_number doesn't change
		// while the thread is active. Although this is true if system runs without overload,
		// we use a local variable to be absolutely safe.
		uint16_t	slot_number = mx.slot_number;

		GPI_TRACE_MSG_FAST(TRACE_INFO, "slot %" PRIu16, slot_number);
		PROFILE("mixer_update_slot() begin");

		// update owner forecast
		// NOTE: we do no overload handling (i.e., we assume that we arrive here once per slot)
		// because influence of occasional misinterpretation is quite limited
		#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
			owner_forecast[1] = owner_forecast[0];
			owner_forecast[0].all = 0;
		#endif

		// maintain request status
		#if MX_REQUEST
			if (slot_number - mx.request.last_update_slot > 3)
			{
				mx.request.row.any_pending = 0;
				mx.request.column.any_pending = 0;
			}
			else if (mx.events & BV(TX_READY))
				mx_update_request(&mx.tx_packet);

			PROFILE("mixer_update_slot() update request status done");
		#endif

		// if Rx packet(s) pending: update request status and history
		#if (MX_COORDINATED_TX || MX_REQUEST)
			while (rx_queue_num_read_update != mx.rx_queue_num_written)
			{
				PROFILE("mixer_update_slot() update history begin");

				Packet *p = &mx.rx_queue[rx_queue_num_read_update % NUM_ELEMENTS(mx.rx_queue)];

				uint8_t  		sender_id   = p->sender_id;
				#if MX_COORDINATED_TX
					uint16_t	rx_slot		= p->slot_number;
								flags		= p->flags;
				#endif

				if (sender_id >= MX_NUM_NODES)
				{
					// don't do much here, it is handled in Rx processing
					rx_queue_num_read_update++;
					continue;
				}

				#if MX_REQUEST
					mx_update_request(p);
				#endif

				REORDER_BARRIER();

				// NOTE: since the current thread has higher priority than Rx packet processing,
				// we should never see an overflow here. Nevertheless we test it for safety. If
				// it would happen we would lose some history updates which is not very critical.
				// In addition request data may get hurt which again is not such critical.
				if (mx.rx_queue_num_writing - rx_queue_num_read_update > NUM_ELEMENTS(mx.rx_queue))
				{
					GPI_TRACE_MSG(TRACE_WARNING, "WARNING: rx queue num_read_2 overflow -> check program, should not happen");
					rx_queue_num_read_update = mx.rx_queue_num_written;
					continue;
				}
				else
				{
					rx_queue_num_read_update++;

				    // TODO:
				    // Workaround: Because process_rx_data depends on progress of update_slot
				    // (because process_rx_data modifies the pkt in place, update_slot must run
				    // before that to correctly process request information), we trigger
				    // process_rx_data every time update_slot is done, to keep up with packet
				    // processing.
				    set_event(RX_READY);
				}

				#if MX_COORDINATED_TX
					mx_update_history(sender_id, flags, rx_slot);
					GPI_TRACE_MSG(TRACE_INFO, "node %u history update", sender_id);
				#endif

				// update owner_forecast
				#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
					if (rx_slot == slot_number)
						owner_forecast[0] = flags;
					else if (rx_slot == slot_number - 1)
						owner_forecast[1] = flags;
				#endif

				PROFILE("mixer_update_slot() update history end");
			}
		#endif

		// compute density after history update
		#if MX_COORDINATED_TX
			density = 1 + mx_present_head->num_nodes + mx_finished_head->num_nodes;
			#if (MX_SMART_SHUTDOWN_MODE >= 4)
				density += mx_acked_head->num_nodes;
			#endif
			assert(density < 256);
		#endif

		// coordinate startup/discovery phase
		#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
			if (slot_number <= mx.discovery_exit_slot)
			{
				if (0 == mx.wake_up_slot)
				{
					mx.wake_up_slot = slot_number;
					GPI_TRACE_MSG(TRACE_INFO, "wake up slot: %" PRIu16, mx.wake_up_slot);
				}

				// if exit slot not reached so far: keep exit slot up-to-date
				if ((slot_number < mx.discovery_exit_slot) && (density < MX_NUM_NODES))
				{
					uint16_t s;

					// choose exit slot depending on number of discovered neighbors
					s = (mx.wake_up_slot - 1) +
						MX_DISCOVERY_EXIT_SLOT_LUT[MIN(density, MX_DISCOVERY_EXIT_SLOT_LUT_SIZE) - 1];

					// ATTENTION: mx.discovery_exit_slot is also read on ISR level,
					// so writes should be atomic
					gpi_atomic_write(&mx.discovery_exit_slot, s);

					GPI_TRACE_MSG(TRACE_VERBOSE,
						"discovery exit slot: %" PRIu16 " (density: %" PRIu16 ", wake up: %" PRIu16 ")",
						s, density, mx.wake_up_slot);

					#if MX_VERBOSE_STATISTICS
						mx.stat_counter.discovery_exit_slot = s;
						mx.stat_counter.discovery_density 	= density;
						mx.stat_counter.wake_up_slot 		= mx.wake_up_slot;
					#endif
				}

				// if exit slot reached or all nodes known (as neighbors): reload history counters
				#if (MX_HISTORY_DISCOVERY_BEHAVIOR & 2)
				else
				{
					// NOTE: Since the current thread has higher priority than Rx packet processing
					// we can expect that program arrives here once per slot. However, under very
					// high load, combined with very aggressive timing configuration, it might be
					// possible that building a new Tx packet takes too long or that Rx processing
					// does not yield frequently enough such that we miss the discovery exit slot
					// here. Although we think that such configurations should be avoided in general:
					// note that a potential miss and therefore skipping the history update is not
					// extremely critical. Also note that tests of the form
					// (slot_number <, <=, > mx.discovery_exit_slot) still work.

					GPI_TRACE_MSG(TRACE_INFO, "leaving discovery phase -> history update");

					Node *node;

					// NOTE: When doing a history update we normally have to unlink and relink the
					// node to keep the lists sorted. This is not needed here because after the
					// update all nodes have the same value (implicitly establishing the order).
					//
					// NOTE: Instead of iterating through the lists we could also iterate through
					// the nodes. If almost all nodes are neighbors, this would be faster. However,
					// in a large network we expect that the number of neighbors is typically less
					// than the number of non-neighbors.

					for (node = &mx.history[mx_present_head->next]; node != mx_present_head; node = &mx.history[node->next])
						node->last_slot_number = slot_number;

					for (node = &mx.history[mx_finished_head->next]; node != mx_finished_head; node = &mx.history[node->next])
						node->last_slot_number = slot_number;

					#if (MX_SMART_SHUTDOWN_MODE >= 4)
						for (node = &mx.history[mx_acked_head->next]; node != mx_acked_head; node = &mx.history[node->next])
							node->last_slot_number = slot_number;
					#endif
				}
				#endif
			}
		#endif

		// coordinate smart shutdown
		#if MX_SMART_SHUTDOWN
			if (MX_GENERATION_SIZE == mx.rank && !mx.is_shutdown_approved)
			{
				#if (MX_SMART_SHUTDOWN_MODE <= 2)
					// ATTENTION: testing mx.have_full_rank_partner is important
					// to avoid that initiator turns off immediately in one-to-all scenarios
					if ((0 == mx_present_head->num_nodes) && (mx.have_full_rank_partner))
					#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
						if (slot_number > mx.discovery_exit_slot)
					#endif
				#elif (MX_SMART_SHUTDOWN_MODE == 3)
					if (1 == mx.full_rank_state)
				#elif (MX_SMART_SHUTDOWN_MODE == 4)
					if ((0 == mx_present_head->num_nodes) && (0 == mx_finished_head->num_nodes) &&
						(mx.have_full_rank_partner))
				#elif (MX_SMART_SHUTDOWN_MODE == 5)
					if (2 == mx.full_rank_state)
				#else
					#error MX_SMART_SHUTDOWN_MODE is invalid
				#endif
				{
					mx.is_shutdown_approved = 1;
					GPI_TRACE_MSG(TRACE_INFO, "smart shutdown approved");
				}
			}
		#endif

		// decide what to do in next slot
		PROFILE("mixer_update_slot() tx decision begin");
		do
		{
			// don't TX as long as we have no data (i.e. we are not initiated)
			if (mx.rank < 1)
				break;

			// stop if done
			#if MX_SMART_SHUTDOWN
				if ((mx.tx_packet.flags.all & EIT_RADIO_OFF_mask) == EIT_RADIO_OFF_pattern)
				{
					GPI_TRACE_MSG(TRACE_INFO, "smart shutdown initiated");
					while (!mixer_transport_set_next_slot_task(STOP));
					PT_EXIT(pt);
				}
			#endif

			// honor startup owner (if known)
			// NOTE: owner_forecast[1].owner_forecast_2 is the previous packet which points
			// to the owner of the next slot, which is checked here.
			// owner_forecast[0].owner_forecast_1 is the current packet which points to the
			// owner of the next slot, which was already checked after reception.
			#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
			if (slot_number < mx.discovery_exit_slot)
			{
				GPI_TRACE_MSG(TRACE_VERBOSE, "owner forecast: %02" PRIx8 " %02" PRIx8,
					owner_forecast[0].all, owner_forecast[1].all);

				if (owner_forecast[1].owner_forecast_2)
				{
					GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: foreign startup slot (owner_forecast_2 set)");
					break;
				}
			}
			#endif

			#if MX_REQUEST
				uint16_t __attribute__((unused)) relative_rank = 0;
			#endif
			int_fast8_t		is_helper = 0;

			ASSERT_CT(MX_NUM_NODES < 256, inconsistent_program);

			// determine request help index
			#if MX_REQUEST
			{
				PROFILE("mixer_update_slot() tx decision request help 1");

				uint_fast_t		help_bitmask = 0;
				uint_fast_t		*pr;

				gpi_atomic_write(&mx.request.help_index, 0);

				// scan column requests
				// start with all_mask
				pr = &mx.request.column.all_mask[0];
				while (/*(is_helper <= 0) &&*/ mx.request.column.any_pending)
				{
					is_helper = -1;

					uint_fast_t		*po = &mx.request.my_row_mask[0];
					uint_fast_t		x;

					for (x = *pr++; po < &mx.request.my_row_mask[NUM_ELEMENTS(mx.request.my_row_mask)];)
					{
						if (!x)
						{
							x = *pr++;		// ATTENTION: dirty in the sense of access violation
							po++;
							continue;
						}

						#ifndef __BYTE_ORDER__
							#error __BYTE_ORDER__ is undefined
						#elif !((__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
							#error __BYTE_ORDER__ is invalid
						#endif

						// isolate first set bit
						#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
							help_bitmask = x & -x;			// isolate LSB
						#else
							#error TODO						// isolate MSB
						#endif

						// if we can help: exit loop
						if (!(*po & help_bitmask))
							break;

						// else clear bit in x
						x &= ~help_bitmask;
					}

					// if we can help: continue below
					if (po < &mx.request.my_row_mask[NUM_ELEMENTS(mx.request.my_row_mask)])
					{
						// NOTE: help_bitmask has only one bit set,
						// so it doesn't matter if we use get_msb() or get_lsb()
						// + 1 so we can distinguish between column helper (idx < 0) and row helper
						// (idx > 0).
						int16_t help_index = -(1 + ARRAY_INDEX(po, mx.request.my_row_mask) * FAST_T_WIDTH + gpi_get_msb(help_bitmask));

						// ATTENTION: set mx.help_bitmask before mx.help_index to ensure that
						// mx.help_bitmask is valid when mx.help_index becomes active
						mx.request.help_bitmask = help_bitmask;
						REORDER_BARRIER();

						gpi_atomic_write(&mx.request.help_index, help_index);
						is_helper = 1;
						break;
					}

					// break after scanning any_mask
					// NOTE: -2 matches position of pr
					if (ARRAY_INDEX(pr, mx.request.column.any_mask) - 2 < NUM_ELEMENTS(mx.request.column.any_mask))
						break;

					// scan any_mask (after scanning all_mask)
					pr = &mx.request.column.any_mask[0];
				}

				// scan row requests
				// start with all_mask
				pr = &mx.request.row.all_mask[0];
				while ((is_helper <= 0) && mx.request.row.any_pending)
				{
					is_helper = -1;

					uint_fast_t		*po = &mx.request.my_row_mask[0];
					uint_fast_t		x;

					for (x = *pr++; po < &mx.request.my_row_mask[NUM_ELEMENTS(mx.request.my_row_mask)];)
					{
						if (!x)
						{
							x = *pr++;		// ATTENTION: dirty in the sense of access violation
							po++;
							continue;
						}

						#ifndef __BYTE_ORDER__
							#error __BYTE_ORDER__ is undefined
						#elif !((__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
							#error __BYTE_ORDER__ is invalid
						#endif

						// isolate first set bit
						#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
							help_bitmask = x & -x;			// isolate LSB
						#else
							#error TODO						// isolate MSB
						#endif

						// if we can help: exit loop
						if (!(*po & help_bitmask))
							break;

						// else clear bit in x
						x &= ~help_bitmask;
					}

					// if we can help
					if (po < &mx.request.my_row_mask[NUM_ELEMENTS(mx.request.my_row_mask)])
					{
						// NOTE: help_bitmask has only one bit set,
						// so it doesn't matter if we use get_msb() or get_lsb()
						int16_t help_index = ARRAY_INDEX(po, mx.request.my_row_mask) * FAST_T_WIDTH + gpi_get_msb(help_bitmask);

						// ATTENTION: set mx.help_bitmask before mx.help_index to ensure that
						// mx.help_bitmask is valid when mx.help_index becomes active
						mx.request.help_bitmask = help_bitmask;
						REORDER_BARRIER();

						// invalidate tx packet if it is not able to help
						// NOTE: it is rebuild in this case
						// NOTE: a side effect of this is that the grid timer ISR doesn't
						// need to check the packet before sideloading the helper row
						if (mx.tx_packet.is_ready)
						{
							if (mx_get_leading_index(mx.tx_packet.coding_vector) <= help_index)
							{
								// ATTENTION: during startup phase it is possible that Rx ISR makes
								// a TX decision for next slot (in response to flags.owner_forecast_1).
								// The ISR is allowed to select TX if mx.tx_packet.is_ready. Therefore
								// it is important that mx.tx_packet does not get invalidated on
								// thread level in this case.
								#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)

									int ie = gpi_int_lock();

									if (TX != mixer_transport_get_next_slot_task())
										mx.tx_packet.is_ready = 0;
									// TODO: check if setting to -1 is right here because we know which
									// index can help but act later on like we don't.
									else help_index = -1;

									gpi_int_unlock(ie);

								#else
									mx.tx_packet.is_ready = 0;
								#endif
							}
						}

						// activate help_index if packet is able to help
						// NOTE: the opposite case can occur only during startup phase (when Rx ISR
						// makes TX decision, see above). In this phase there is an increased chance
						// that we generate a zero packet if we enable help_index without checking
						// it, so don't do that. (Reason: the first packet(s) that we build contain
						// just our own messages (uncoded), and there is a good chance that we
						// selected one of them to help with).
						if (help_index >= 0)
						{
							gpi_atomic_write(&mx.request.help_index, 1 + help_index);
							is_helper = 1;
						}
						else GPI_TRACE_MSG(TRACE_VERBOSE, "don't help due to uncertainty (index %" PRId16 ")", help_index);

						// continue below
						// NOTE: if we did not enable help_index, there is a little chance that we
						// could do something more in case we currently scanned the all_mask and
						// there is a smaller index pending in the any_mask. However, we do not
						// handle that case and break anyhow because the likehood for that to happen
						// is neglibible (even the probability that we do not activate help_index
						// is quite low).
						break;
					}

					// break after scanning any_mask
					// NOTE: -2 matches position of pr
					if (ARRAY_INDEX(pr, mx.request.row.any_mask) - 2 < NUM_ELEMENTS(mx.request.row.any_mask))
						break;

					// scan any_mask (after scanning all_mask)
					pr = &mx.request.row.any_mask[0];
				}

				#if MX_WEAK_ZEROS
					if (mx.rank >= MX_GENERATION_SIZE && (is_helper < 0))
					{
						// Arriving here, i.e., (seemingly) being unable to help even though we
						// have full rank, means that the missing entries are weak zeros (more
						// precisely: at a specific point in the past we decided to believe that
						// they are weak zeros (we have froozen the state of the weak zero map
						// and used it to jump to full rank)). Hence, from our point of view,
						// we can help by transmitting the weak zero map.
						//
						// NOTE: at this point we do not need to check weak_zero_release_slot
						// because it already passed for sure (we have full rank, i.e., either
						// weak_zero_release_slot has passed or all entries are strong; but in
						// the latter case we wouldn't be here because is_helper >= 0 for sure.)
						//
						// NOTE: mx.request.help_bitmask is don't care in this case

						gpi_atomic_write(&mx.request.help_index, INT16_MIN);
						is_helper = 1;
					}
				#endif

				PROFILE("mixer_update_slot() tx decision request help 2");
				GPI_TRACE_MSG(TRACE_VERBOSE, "request help index: %" PRId16, mx.request.help_index);

				// requests must not influence TX decision during discovery phase
				// NOTE: they are allowed to influence the packet content
				#if (MX_COORDINATED_TX && !MX_BENCHMARK_NO_COORDINATED_STARTUP)
					if (slot_number < mx.discovery_exit_slot)
						is_helper = 0;
				#endif

				if (is_helper != 0)
				{
					ASSERT_CT(MX_NUM_NODES < 256, inconsistent_program);

					PROFILE("mixer_update_slot() tx decision request help 3");

					// relative rank = rank / MX_GENERATION_SIZE,
					// stored in 0.16 signed fixed point format
					relative_rank = gpi_mulu_16x16(mx.rank, 0xffff / MX_GENERATION_SIZE);

					// n = number of potential helpers
					uint_fast8_t n = 0;

					// all full rank neighbors can help
					#if MX_COORDINATED_TX
						n += mx_finished_head->num_nodes;
						#if (MX_SMART_SHUTDOWN_MODE >= 4)
							n += mx_acked_head->num_nodes;
						#endif
						GPI_TRACE_MSG(TRACE_VERBOSE, "n_finished: %" PRIuFAST8, n);
					#endif

					// if I can help
					if (is_helper >= 0)
					{
						// add me
						n++;
						GPI_TRACE_MSG(TRACE_VERBOSE, "+me: %" PRIuFAST8, n);

						// add non-full rank neighbors which are also able to help

						#if MX_WEAK_ZEROS
							// weak-zero help is performed only by full rank nodes
							if (mx.request.help_index != INT16_MIN)
						#endif
						{
						#if MX_REQUEST_HEURISTIC == 0
							n += 1;
						#elif MX_REQUEST_HEURISTIC == 1
							n += (UINT16_C(3) * mx_present_head->num_nodes + 2) / 4;
						#elif MX_REQUEST_HEURISTIC == 2

							uint_fast16_t i = (ABS(mx.request.help_index) - 1) / (8 * sizeof_member(Node, row_map[0]));
							GPI_TRACE_MSG(TRACE_VERBOSE, "i: %" PRIdFAST16", m: %" PRIxFAST, i, help_bitmask);

							Node *p;
							for (p = &mx.history[mx_present_head->next]; p != mx_present_head; p = &mx.history[p->next])
							{
								if (!(p->row_map[i] & help_bitmask))
								{
									n++;
									GPI_TRACE_MSG(TRACE_VERBOSE, "+node %u: %" PRIuFAST8, (int)ARRAY_INDEX(p, mx.history), n);
								}
							}

						#else
							#error MX_REQUEST_HEURISTIC is invalid
						#endif
						}
					}
					else
					{
						// don't know which index other helpers choose -> improvise:
						// guess number of helpers; heuristic: one quarter of the neighbors
						#if MX_REQUEST_HEURISTIC == 0
							n += 1;
						#else
							n += (mx_present_head->num_nodes + 2) / 4;
						#endif

						// NOTE: intuition behind heuristic 1: since we are neighbors, there is an
						// increased probability that the majority of us is in the same situation,
						// i.e., probably the majority considers the same index; and if I can(not)
						// help than probably they can(not). estimate the majority as 3/4 of us
						// (with rounding)
					}

					GPI_TRACE_MSG(TRACE_VERBOSE, "+heuristic: %" PRIuFAST8, n);

					if (is_helper > 0)
					{
						// p = 1 / n
						if (n < 2)
							p = UINT16_MAX;
						else p = gpi_divu_16x8(UINT16_MAX, n, 0);
					}
					else
					{
						// p = (1 / e) / (d - n)
						//#if !MX_COORDINATED_TX
						//	p = 24109 / 2;
						//#else
						//	if ((density - n) < 2)
						//		p = 24109;
						//	else p = gpi_divu_16x8(24109, density - n, 0);
						//#endif

						// TODO: add comment

						static const uint16_t LUT[] =
						{
							32768,     0, 16384, 19418, 20736, 21475, 21948, 22277,
							22519, 22704, 22851, 22970, 23068, 23151, 23222, 23283
						};

						p = LUT[MIN(n, NUM_ELEMENTS(LUT) - 1)];
						if ((density - n) >= 2)
								p = gpi_divu_16x8(p, density - n, 0);
					}

					// tx probability:
					// is_helper   my slot              foreign slot   concurrent slot
					// 0           1                    0              1 / (d + 1), possibly incl. aging
					// +           p+ * rr + (1 - rr)   p+ * rr        p+ = 1 / n
					// -           p- * rr + (1 - rr)   p- * rr        p- = (1 / e) / (d - n)
					// (rr = relative rank, d = density, n = number of helpers, e = Euler's number)

					PROFILE("mixer_update_slot() tx decision request help 4");

					GPI_TRACE_MSG(TRACE_VERBOSE, "request: is_helper = %" PRIdFAST8", p = %" PRIu16
						", rr = %" PRIu16 ", n = %" PRIuFAST8 ", index = %" PRId16,
						is_helper, p, relative_rank, n, mx.request.help_index);
				}
			}
			#endif

			PROFILE("mixer_update_slot() tx decision coord");

			#if MX_COORDINATED_TX

				// determine owner of next slot:
				// 		owner = (slot_number + 1) % MX_NUM_NODES;
				// NOTE: slot_number + 1 is the theoretical value. We do - 1 because the first
				// slot has number 1 while node IDs are 0-based. Hence we assign slot 1 to the
				// first node (with ID 0).
				// NOTE: modulo/division is expensive -> do it more efficient:
				// Instead of dividing, we simply increment owner from slot to slot with manual
				// wrap-around. Some checks ensure that it also works if slot_number jumps (e.g.
				// because of resynchronization).

				uint16_t diff = slot_number - last_owner_update;

				// limit number of potential loop iterations
				if (diff >= 8 * MX_NUM_NODES)
					owner = (slot_number + 1 - 1) % MX_NUM_NODES;
				else
				{
					// skip full wrap-around cycles
					while (diff >= MX_NUM_NODES)
						diff -= MX_NUM_NODES;

					// update owner
					owner += diff;
					if (owner >= MX_NUM_NODES)
						owner -= MX_NUM_NODES;
				}

				last_owner_update = slot_number;

				GPI_TRACE_MSG(TRACE_VERBOSE, "owner of slot %" PRIu16 ": %" PRIu16, slot_number + 1, owner);

				// if my slot: TX
				if (owner == mx.tx_packet.sender_id)
				{
					GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: my slot");

					#if MX_REQUEST
						// adapt tx probability if request pending
						// -> possibly place our slot at the disposal of other helpers
						// formula: p = p * rr + (1 - rr); p = 0.16, rr = 0.16
						if (is_helper < 0)
							p = (gpi_mulu_16x16(p, relative_rank) >> 16) - relative_rank;
						else
					#endif

					p = UINT16_MAX;
					break;
				}

			#endif

			// TX in last slot -> don't TX (except it is our slot)
			#if !MX_BENCHMARK_FULL_RANDOM_TX
			if (mx.events & BV(TX_READY))
			{
				// with one exception: if we did tx in slot 1 -- i.e. we are the initiator --
				// we also transmit in slot 2 because we know that no other node uses slot 2
				// (in best case they received the first packet in slot 1 and prepare their
				// first tx packet during slot 2)
				if (slot_number == 1)
				{
					p = UINT16_MAX;
					GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: initiator in slot 2");
					break;
				}

				p = 0;
				GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: tx in previous slot");
				break;
			}
			#endif

			#if MX_COORDINATED_TX

				// during start-up phase
				#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
				// if (slot_number < MX_GENERATION_SIZE)
				{
					// if transmitter of current packet has next message: TX
					// NOTE: originator of next message won't TX (except for the unprobable case
					// that it is the slot owner) since TX in consecutive slots is prohibited. All
					// receivers use the slots to push the message into its environment (more far
					// away from the transmitter). By the way they generate a wake-up wave for
					// their back-country.
					// ATTENTION: while the approach is right, it would be wrong to do that here
					// because it would be too late. Here, we are already within the next slot.
					// Therefore this case is handled in Rx ISR (as an exception), immediately
					// after the packet has been received.
					// if (flags.owner_forecast_1)
					// {
					//	p = UINT16_MAX;
					//	break;
					// }

					// if we are the originator of the next message: TX
					// NOTE: in start-up phase, the slots are assigned to owners by node IDs
					// *and initial messages* (i.e. slots can have two owners in this phase).
					// NOTE: matrix rows use 0-based indexes, hence we index with slot_number
					// instead of slot_number + 1.
					if ((slot_number + 1 < MX_GENERATION_SIZE) && (0 == mx.matrix[slot_number].birth_slot))
					{
						GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: startup owner");
						p = UINT16_MAX;
						break;
					}
				}
				#endif

				// foreign slot
				if (mx.history[owner].list_id != ARRAY_INDEX(mx_absent_head, mx.history) - MX_NUM_NODES)
				{
					GPI_TRACE_MSG(TRACE_VERBOSE, "foreign slot");

					#if MX_REQUEST
						// adapt tx probability if request pending
						// -> possibly jump in as helper
						// formula: p = p * rr; p = 0.16, rr = 0.16
						if (is_helper != 0)
							p = gpi_mulu_16x16(p, relative_rank) >> 16;
						else
					#endif

					p = 0;
					break;
				}

				// concurrent slot during discovery phase
				#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
				if (slot_number + 1 < mx.discovery_exit_slot)
				{
					GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: discovery phase");

					uint16_t s = slot_number - mx.wake_up_slot;

					// TX with probability max(1 - s / 4, MX_DISCOVERY_PTX)
					//
					// NOTE: instead of applying max() straight-forward, the operation can be
					// performed more efficiently by testing against the transition point since
					// the latter is constant. It can be derived as follows:
					//		1 - s / 4 > MX_DISCOVERY_PTX
					//	=>	1 - MX_DISCOVERY_PTX > s / 4
					//	=>	s < (1 - MX_DISCOVERY_PTX) * 4
					//	in 0.16 fixed-point format:
					//		s < (64K - MX_DISCOVERY_PTX) * 4 / 64K
					//	=>	s < (64K - MX_DISCOVERY_PTX) >> 14
					// The computation of the other term can be simplified as follows:
					//		1 - s / 4
					//	in 0.16 fixed-point format:
					//		= 0x10000 - 0x10000 / 4 * s = 0x10000 - 0x4000 * s
					//		= 0x10000 - (s << 14), approx. 0xffff - (s << 14)
					// If the case s == 0 is handled explicitly, one could exploit that 0x10000
					// overruns the 16-bit format as follows:
					//		= 0x10000 - 0x4000 * s
					//	=>	= 0 - 0x4000 * s = -0x4000 * s
					//		= 0xc000 * s
					//		= (s << 15) + (s << 14)

					// In 802.15.4 mode we use MX_DISCOVERY_PTX because of the favorable capture behavior.
					// TODO: Here we assume 802.15.4 mode to be MX_PHY_MODE 1. Associate with Gpi_Radio_Mode in radio.h
					#if MX_PHY_MODE == 1
						if (s < ((0x10000 - MX_DISCOVERY_PTX + 0x3fff) >> 14))
							p = UINT16_MAX - gpi_slu(s, 14);
						else p = MX_DISCOVERY_PTX;
					#else
						if (s < ((0x10000 - (MX_DISCOVERY_PTX / 2) + 0x3fff) >> 14))
							p = UINT16_MAX - gpi_slu(s, 14);
						else p = (MX_DISCOVERY_PTX / 2);
					#endif

					break;
				}
				#endif

			#endif

			// concurrent slot
			{
				GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision: concurrent slot");

				// if request pending: p has been computed already
				if (is_helper != 0)
					break;

				static const uint8_t age_to_tx_LUT[] = MX_AGE_TO_TX_PROBABILITY;
				ASSERT_CT(sizeof(age_to_tx_LUT) > 0, MX_AGE_TO_TX_PROBABILITY_is_invalid);

				uint16_t age = slot_number - mx.recent_innovative_slot;

				// formula to realize:
				// p = 1 / (d + 1) + d / (d + 1) * LUT[age]
				//   = A / B with A := 1 + d * LUT[age] and B := d + 1

				// compute A, store it in 8.8 fixed point format
				p = age_to_tx_LUT[MIN(age, NUM_ELEMENTS(age_to_tx_LUT) - 1)];
				#if !MX_COORDINATED_TX
					p <<= 8;
				#else
					p *= density;
					p += 0x100; // 2^16 >> 8

					// compute A / B, store it in 0.16 fixed point format
					p = gpi_divu_16x8(p, density + 1, 0) << 8;

					GPI_TRACE_MSG(TRACE_VERBOSE, "tx decision age: %" PRIu16 ", density: %" PRIu16, age, density);
				#endif
			}

		} while (0);

		PROFILE("mixer_update_slot() tx decision p done");
		GPI_TRACE_MSG(TRACE_INFO, "tx decision p: %" PRIu16, p);

		next_task = RX;

		if (p && (mixer_rand() <= p))
			next_task = TX;

		clear_event(TX_READY);

		PROFILE("mixer_update_slot() tx decision activate 1");

		if (TX == next_task)
		{
			// if TX and packet preparation pending: select short-term transmit data
			// in case there is not enough time to finish the full packet
			if (!mx.tx_packet.is_ready)
			{
				int ie = gpi_int_lock();

				if (NULL == mx.tx_sideload)
					mx.tx_sideload = &(mx.tx_reserve->coding_vector_8[0]);

				// NOTE: (next_task == TX) => (rank > 0) => (mx.tx_reserve != NULL) for sure

				gpi_int_unlock(ie);
			}
		}

		mixer_transport_set_next_slot_task(next_task);

		PROFILE("mixer_update_slot() tx decision activate 2");

		next_task = mixer_transport_get_next_slot_task();
		GPI_TRACE_MSG(TRACE_INFO, "next slot task: %s", (next_task == TX) ? "TX" : ((next_task == RX) ? "RX" : "STOP"));

		// if we need a new tx packet: build one
		// ATTENTION: we have to make sure that at least one of mx.tx_sideload or mx.tx_packet is valid
		// in case of TX. Since mx.tx_sideload may be reset by rx processing, we have to provide a
		// valid packet before leaving current thread if next task == TX. Therefore we don't rely
		// on TX_READY because that one is not signaled before our first transmission. Thereafter,
		// TX_READY and !is_ready are quiet equivalent - except for the fact that mx.tx_packet.is_ready
		// may also be reset during tx decision (to enforce assembly of a new packet in response to
		// request processing). Hence, checking is_ready is the right way here.
		if (!mx.tx_packet.is_ready && (mx.rank > 0))
		{
			PROFILE("mixer_update_slot() prepare tx packet begin");

			// is_valid is used to detect if the packet may have been hurt by the ISR while preparing it
			mx.tx_packet.is_valid = 1;
			REORDER_BARRIER();

			prepare_tx_packet();

			REORDER_BARRIER();

			if (!mx.tx_packet.is_valid)
			{
				// if mx.tx_packet gets hurt by ISR, then we can not use it. On the other hand we
				// know that next_task TX has been done already, using the sideload (while we
				// prepared the packet, that is why we are here). So the packet is broken, but
				// we don't need it anymore.

				GPI_TRACE_MSG(TRACE_VERBOSE, "tx packet hurt by ISR -> dropped it");
			}

			else
			{
				// prepare a random number for ISR
				// ATTENTION: don't write it to mx.tx_packet.rand immediately
				// because write access to mx.tx_packet.rand is non-atomic (typically)
				uint8_t r = mixer_rand();

				REORDER_BARRIER();
				int ie = gpi_int_lock();

				// NOTE: it is possible that ISR changed the packet (by applying the sideload)
				// between testing is_valid above and the gpi_int_lock() call. Nevertheless the packet
				// is valid in this case since applying the sideload doesn't break it. So we could
				// declare it as ready anyway. The reason why we don't do that is that we don't
				// want to send the same packet twice.
				if (mx.tx_packet.is_valid)
				{
					mx.tx_packet.rand = r;
					mx.tx_packet.is_ready = 1;

					// reset mx.tx_sideload if it points into the matrix
					// NOTE: prepare_tx_packet() already considered all matrix rows. So if we wouldn't
					// reset mx.tx_sideload, there would be a probability that we transmit a zero-
					// packet. In particular, this would happen for sure if rank is 1.
					// NOTE: don't change mx.tx_sideload if it points to rx queue. This may also result
					// in a zero-packet, but 1) this is unprobable and 2) it happens at most once per
					// rx packet.
					// NOTE: don't touch mx.tx_sideload if it points to mx.empty_row. It plays a special
					// role and is temporarily used by rx processing. mx.tx_sideload pointing to
					// mx.empty_row is the same as pointing to rx queue.
					if (((uintptr_t)mx.tx_sideload - (uintptr_t)&mx.matrix < sizeof(mx.matrix)) &&
						(Matrix_Row*)(mx.tx_sideload - offsetof(Matrix_Row, coding_vector)) != mx.empty_row)
					{
						mx.tx_sideload = NULL;
					}

					REORDER_BARRIER();

					// NOTE: mx.tx_packet.is_ready is reset on ISR level (after transmission)
				}

				gpi_int_unlock(ie);
			}

			PROFILE("mixer_update_slot() prepare tx packet end");
		}


		// maintain history
		#if MX_COORDINATED_TX
			#if (!MX_BENCHMARK_NO_COORDINATED_STARTUP && (MX_HISTORY_DISCOVERY_BEHAVIOR & 1))
				if (slot_number > mx.discovery_exit_slot)
			#endif
			mx_purge_history();
		#endif

		#if MX_COORDINATED_TX
			#if (GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE)
			{
				int i;

				for (i = 0; i < MX_NUM_NODES; ++i)
				{
					if (mx.history[i].list_id != (ARRAY_INDEX(mx_absent_head, mx.history) - MX_NUM_NODES))
						GPI_TRACE_MSG(TRACE_VERBOSE, "history %2u: %u %u", i,
							(int)(mx.history[i].list_id), (int)(mx.history[i].last_slot_number));
				}
			#if 0
				for (i = 0; i < MX_NUM_NODES; ++i)
				{
					if (mx.history[i].list_id == (ARRAY_INDEX(mx_present_head, mx.history) - MX_NUM_NODES))
					{
						char msg[16];
						sprintf(msg, "row map %2u:", i);
						TRACE_DUMP(1, msg, mx.history[i].row_map, sizeof(mx.history[0].row_map));
					}
				}
			#else
				#pragma message "printing of row map is deactivated despite GPI_TRACE_MODE_TRACE"
			#endif
			}
			#endif
		#endif

		PROFILE("mixer_update_slot() end");
	}

	PT_END(pt);
}

//**************************************************************************************************

PT_THREAD(mixer_process_rx_data())
{
	Pt_Context* const	pt = pt_process_rx_data;

	static const unsigned int PAYLOAD_SHIFT =
		offsetof(Matrix_Row, payload) - offsetof(Matrix_Row, payload_8);

	// ATTENTION: ensure that PAYLOAD_SIZE is aligned because memxor_block() may rely on that
	// ATTENTION: don't use sizeof(mx.matrix[0].payload) because it might be too small due to
	// MX_BENCHMARK_PSEUDO_PAYLOAD
	static const unsigned int PAYLOAD_SIZE =
		FIELD_SIZEOF(Packet, payload) + PADDING_SIZE(FIELD_SIZEOF(Packet, payload));

	PT_BEGIN(pt);

	while (1)
	{
		// NOTE: PT_WAIT_UNTIL returns PT_WAITING which is a different state than the return value
		// of yielding during execution of this thread. When yielded, the thread is scheduled soon
		// again without a specific condition whereas the PT_WAITING state here is only left in case
		// RX_READY is set.
		PT_WAIT_UNTIL(pt, mx.events & BV(RX_READY));
		clear_event(RX_READY);

		// Workaround: Because process_rx_data depends on progress of update_slot (because
		// process_rx_data modifies the pkt in place, update_slot must run before that to correctly
		// process request information), we trigger process_rx_data every time update_slot is done,
		// to keep up with packet processing. while (mx.rx_queue_num_read !=
		// mx.rx_queue_num_written)
		while (mx.rx_queue_num_read != rx_queue_num_read_update)
		{
			PROFILE("mixer_process_rx_data() begin");

			// if we yield within the loop, we must declare persistent variables as static
			static Packet	*p;
			void			*pp[MEMXOR_BLOCKSIZE];
			unsigned int	pp_used;
			int_fast16_t	i;

			p = &mx.rx_queue[mx.rx_queue_num_read % NUM_ELEMENTS(mx.rx_queue)];

			TRACE_DUMP(1, "Rx packet:", &(p->phy_payload_begin), PHY_PAYLOAD_SIZE);

			if (p->sender_id >= MX_NUM_NODES)
			{
				GPI_TRACE_MSG(TRACE_INFO, "Rx: invalid sender_id %u -> drop packet", p->sender_id);
				goto continue_;
			}

			TRACE_PACKET(p);

			// update full-rank map
			#if MX_SMART_SHUTDOWN_MAP
				if (((p->flags.all & EIT_FULL_RANK_X_mask) == EIT_FULL_RANK_X_pattern) ||
					(p->flags.is_full_rank && (mx.full_rank_state == 0)))
				{
					update_full_rank_map(p);

					// if we are not finished: yield
					// because update_full_rank_map() might have taken a bit of time
					if (mx.rank < MX_GENERATION_SIZE)
						PT_YIELD(pt);
				}
			#endif

			// if we already have full rank: done
			if (mx.rank >= MX_GENERATION_SIZE)
				goto continue_;

			#if MX_WEAK_ZEROS
				// ATTENTION: weak zero map does not get further updates after reaching full rank.
				// This is important because reaching full rank is an irrevocable state change.
				// It is not allowed to fall back from this state for several internal reasons
				// (e.g., it would disturb the full rank acknowledge procedure). Since weak zeros
				// might have contributed to the full rank state, they must be preserved afterwards.
				update_weak_zero_map(p);
			#endif

			PROFILE("mixer_process_rx_data() checkpoint 1");

			// if mx.tx_sideload points to current packet: move mx.tx_sideload away because data in
			// Rx queue slot will become invalid while processing
			// NOTE: there is no problem in cases where we simply free the slot by incrementing
			// mx.rx_queue_num_read (without touching the slot content) because the instance which
			// overrides the data -- Rx ISR processing -- updates mx.tx_sideload by itself (and is
			// never active in parallel to Tx).
			// ATTENTION: it is important to understand that the slot in progress (-> num_writing)
			// is vacant, i.e. it may be filled with data without updating mx.tx_sideload afterwards
			// (e.g. in response to a missed CRC check). This is no problem as long as we never set
			// mx.tx_sideload back to an older queue entry than it is (if it points into the queue).
			// If we don't do that, then the ISR ensures that mx.tx_sideload never points to the
			// vacant slot.
			if (mx.tx_sideload == &(p->coding_vector[0]))
			{
				uint8_t	*pr;

				// if there is an empty row available (which is always the case als long as not
				// full rank): copy the packet to this row and use it as sideload
				// NOTE: it is important that we don't simply invalidate mx.tx_sideload because
				// the case that it points to the current packet is standard (except for high load
				// situations). If we invalidate it, there is a significant probability that fast
				// tx update doesn't happen (only if rx processing finishes before next tx slot).
				if (NULL == mx.empty_row)
					pr = NULL;
				else
				{
					pr = &(mx.empty_row->coding_vector_8[0]);

					gpi_memcpy_dma_aligned(pr, p->coding_vector,
						sizeof(mx.matrix[0].coding_vector) + sizeof(mx.matrix[0].payload));

					#if MX_BENCHMARK_PSEUDO_PAYLOAD
						#if GPI_ARCH_IS_CORE(MSP430)
							__delay_cycles(MX_PAYLOAD_SIZE);
						#else
							#error MX_BENCHMARK_PSEUDO_PAYLOAD is unsupported on current architecture
						#endif
					#else

					unwrap_chunk(pr);

					#endif
				}

				int ie = gpi_int_lock();

				if (mx.tx_sideload == &(p->coding_vector[0]))
					mx.tx_sideload = pr;

				gpi_int_unlock(ie);
			}

			// unwrap unaligned packet elements
			unwrap_chunk(&(p->coding_vector[0]));

			PROFILE("mixer_process_rx_data() checkpoint 2");

			// traverse matrix / coding vector
			pp_used = 0;
			while (1)
			{
				PROFILE("mixer_process_rx_data() matrix iteration begin");

				// get leading coefficient
				i = mx_get_leading_index(p->coding_vector);

				if (i < 0)
				{
					// if this is the last received packed: invalidate mx.tx_sideload because the
					// packet was not innovative -> ensures that the prepared tx packet won't
					// get hurt
					{
						int ie = gpi_int_lock();

						if (mx.rx_queue_num_written - mx.rx_queue_num_read == 1)
							mx.tx_sideload = NULL;

						gpi_int_unlock(ie);
					}

					break;
				}

				// if corresponding row is empty (i.e. packet is innovative): fill it, rank increase
				if (UINT16_MAX == mx.matrix[i].birth_slot)
				{
					PROFILE("mixer_process_rx_data() new row begin");

					mx.matrix[i].birth_slot = p->slot_number;
					mx.recent_innovative_slot = p->slot_number;

					ASSERT_CT(offsetof(Packet, payload) ==
						offsetof(Packet, coding_vector) + sizeof_member(Packet, coding_vector),
						inconsistent_program);
					ASSERT_CT(offsetof(Matrix_Row, payload) ==
						offsetof(Matrix_Row, coding_vector) + sizeof_member(Matrix_Row, coding_vector),
						inconsistent_program);

					if (pp_used)
						memxor_block(&(p->payload[PAYLOAD_SHIFT]), pp, PAYLOAD_SIZE, pp_used);

					// if mx.tx_sideload doesn't point into rx queue: set mx.tx_sideload to current
					// rx queue packet
					// NOTE: First, if mx.tx_sideload points to current row, it must be changed
					// because row is inconsistent while copying. Second, we are here because the
					// rx packet is innovative. So the only reason not to change mx.tx_sideload is
					// that it points to a newer packet in the rx queue which we didn't process yet.
					// NOTE: at this point the rx queue packet is valid (again) since processing
					// has been done (actually, the rx queue packet will be copied into the row)
					// NOTE: there is no problem if mx.tx_reserve points to current row here
					// because it is not used on ISR level
					{
						int ie = gpi_int_lock();

						if ((uintptr_t)mx.tx_sideload - (uintptr_t)&mx.rx_queue >= sizeof(mx.rx_queue))
							mx.tx_sideload = &(p->coding_vector[0]);

						gpi_int_unlock(ie);
					}

					// if mx.request.help_index points to current row: invalidate mx.request.help_index
					// because row is inconsistent while copying
					// NOTE: if the program is correct, it is impossible that help_index points to
					// an empty row, so we use an assertion. Nevertheless we handle the situation
					// for the case that assertions are inactive (i.e. NDEBUG).
					// NOTE: assert() sits within the condition body to keep time with interrupts
					// locked as short as possible in the normal case
					#if MX_REQUEST
					{
						int ie = gpi_int_lock();

						if (ABS(mx.request.help_index) - 1 == i)
						{
							assert(ABS(mx.request.help_index) - 1 != i);
							GPI_TRACE_MSG_FAST(TRACE_ERROR, "!!! request help index points to empty row -> check program, must not happen !!!");
							mx.request.help_index = 0;
						}

						gpi_int_unlock(ie);
					}
					#endif

					gpi_memcpy_dma_aligned(mx.matrix[i].coding_vector, p->coding_vector,
						sizeof(mx.matrix[0].coding_vector) + sizeof(mx.matrix[0].payload));

					#if MX_BENCHMARK_PSEUDO_PAYLOAD
						#if GPI_ARCH_IS_CORE(MSP430)
							__delay_cycles(MX_PAYLOAD_SIZE);
						#else
							#error MX_BENCHMARK_PSEUDO_PAYLOAD is unsupported on current architecture
						#endif
					#endif

					mx.rank++;

					// update mx.tx_reserve
					// NOTE: there are two reasons to do so:
					// 1) init mx.tx_reserve if it is NULL
					// 2) it may be beneficial if it points to a quite new row
					#if 0	// activate only for special purposes like evaluating most stupid behavior
					if (NULL == mx.tx_reserve)
					#endif
					mx.tx_reserve = &mx.matrix[i];

					// update mx.empty_row if needed
					// NOTE: mx.empty_row is kept static to avoid expensive search runs everytime
					// an empty row is needed. starting from its last position is much cheaper.
					if (mx.empty_row == &mx.matrix[i])
					{
						if (MX_GENERATION_SIZE == mx.rank)
							mx.empty_row = NULL;
						else
						{
							while (mx.empty_row-- > &mx.matrix[0])
							{
								if (UINT16_MAX == mx.empty_row->birth_slot)
									break;
							}

							if (mx.empty_row < &mx.matrix[0])
							{
								mx.empty_row = &mx.matrix[NUM_ELEMENTS(mx.matrix)];
								while (mx.empty_row-- > &mx.matrix[0])
								{
									if (UINT16_MAX == mx.empty_row->birth_slot)
										break;
								}
							}

							assert(mx.empty_row >= &mx.matrix[0]);
						}
					}

					// update request mask
					#if MX_REQUEST
						mx.request.my_row_mask[i / FAST_T_WIDTH] &=	~gpi_slu(1, i % FAST_T_WIDTH);
						mx.request.my_column_pending =
							mx_request_clear(mx.request.my_column_mask, mx.matrix[i].coding_vector, sizeof(mx.request.my_column_mask));
					#endif

					PROFILE("mixer_process_rx_data() new row done");

					GPI_TRACE_MSG(TRACE_VERBOSE, "new row %u, rank: %u", i, mx.rank);
					TRACE_MATRIX();
					GPI_TRACE_MSG(TRACE_VERBOSE, "empty row: %d", (NULL == mx.empty_row) ? -1 :  ARRAY_INDEX(mx.empty_row, mx.matrix));

					// if we reached full rank with current packet: solve (decode)
					// -> see below (after the loop)

					break;
				}

				PROFILE("mixer_process_rx_data() matrix iteration checkpoint A");

				// else substitute
				memxor(p->coding_vector, mx.matrix[i].coding_vector, sizeof(mx.matrix[0].coding_vector));
				pp[pp_used++] = &(mx.matrix[i].payload);
				if (NUM_ELEMENTS(pp) == pp_used)
				{
					// NOTE: calling with NUM_ELEMENTS(pp) instead of pp_used leads to a bit
					// better code because NUM_ELEMENTS(pp) is a constant (msp430-gcc 4.6.3)
					memxor_block(&(p->payload[PAYLOAD_SHIFT]), pp, PAYLOAD_SIZE, NUM_ELEMENTS(pp));

					// yield after each block to keep thread-level response time small (enough)
					PT_YIELD(pt);

					pp_used = 0;
				}

				PROFILE("mixer_process_rx_data() matrix iteration end");
			}

			// if we reached full rank with current packet: solve (decode)
			// NOTE: this may take some time. Although it would not be very critical if we
			// lose some packets meanwhile, we still yield to transmit something from time
			// to time.
			#if MX_WEAK_ZEROS
				if (mx.rank + (p->slot_number >= mx.weak_zero_release_slot ? mx.weak_rank : 0) >= MX_GENERATION_SIZE)
			#else
				if (mx.rank >= MX_GENERATION_SIZE)
			#endif
			{
				static Pt_Context	pt_decode;

				mx.rank = MX_GENERATION_SIZE;

				#if MX_VERBOSE_STATISTICS
					mx.stat_counter.slot_full_rank = p->slot_number;
				#endif

				// make sure that mx.tx_sideload doesn't point into rx queue anymore
				// ATTENTION: this is important because Rx ISR doesn't update mx.tx_sideload
				// anymore after full rank has been reached. If we wouldn't change it here,
				// then it may point to an invalid entry after queue wrap-around.
				// NOTE: gpi_int_lock() is only needed if access to pointers is not atomic
				// (e.g. on 8 bit machines)
				REORDER_BARRIER();		// make sure that mx.rank is written back
				int ie = gpi_int_lock();
				mx.tx_sideload = NULL;
				gpi_int_unlock(ie);

				// yield because packet processing may already have taken some time
				PT_YIELD(pt);

				PROFILE("mixer_process_rx_data() decode begin");

				// start decode thread
				// ATTENTION: don't use PT_SPAWN() because it returns PT_WAITING if child
				// thread yields. Here, we have to make sure that we return PT_YIELDED in
				// this case.
				// PT_SPAWN(pt, &pt_decode, decode(&pt_decode));
				PT_INIT(&pt_decode);
				while (PT_SCHEDULE(mixer_decode(&pt_decode)))
					PT_YIELD(pt);

				#if MX_SMART_SHUTDOWN_MAP
					update_full_rank_map(NULL);
				#endif

				PROFILE("mixer_process_rx_data() decode end");
			}

			continue_:

			mx.rx_queue_num_read++;

			#if MX_VERBOSE_STATISTICS
				mx.stat_counter.num_rx_queue_processed++;
			#endif

			PROFILE("mixer_process_rx_data() end");
			PT_YIELD(pt);
		}
	}

	PT_END(pt);
}

//**************************************************************************************************

PT_THREAD(mixer_decode(Pt_Context *pt))
{
	PT_BEGIN(pt);

	static int_fast16_t		i;

	GPI_TRACE_MSG_FAST(TRACE_INFO, "start decoding...");
	PROFILE("mixer_decode() entry");

	for (i = NUM_ELEMENTS(mx.matrix); i-- > 0;)
	{
		const unsigned int	SZB = sizeof(uint_fast_t) * 8;
		void				*pp[MEMXOR_BLOCKSIZE];
		unsigned int		pp_used;
		uint_fast_t			k, m, *pcv;

		// check if row is empty
		// ATTENTION: this is needed if decode() called before reaching full rank
		// (e.g. at end of round)
		if (UINT16_MAX == mx.matrix[i].birth_slot)
			continue;

		PROFILE("mixer_decode() row begin");

		// if mx.tx_sideload points into the matrix: invalidate mx.tx_sideload because rows are
		// inconsistent while solving. The same holds for s_request.help_index.
		// ATTENTION: we have to redo this check after every yield
		// NOTE: there is no problem if s_tx_reserve points into the matrix
		// because it is not used on ISR level
		{
			uint8_t	*p = &(mx.matrix[i].coding_vector_8[0]);

			COMPUTE_BARRIER(p);

			int ie = gpi_int_lock();

			if (mx.tx_sideload == p)
				mx.tx_sideload = NULL;

			gpi_int_unlock(ie);

			#if MX_REQUEST
				ie = gpi_int_lock();

				if (ABS(mx.request.help_index) - 1 == i)
					mx.request.help_index = 0;

				gpi_int_unlock(ie);
			#endif
		}

		pp_used = 0;

		k = i + 1;
		pcv = &(mx.matrix[i].coding_vector[k / SZB]);
		m = *pcv++ & ((-1 << (SZB - 1)) >> ((SZB - 1) - (k % SZB)));
		k &= ~(SZB - 1);

		while (k < NUM_ELEMENTS(mx.matrix))
		{
			if (!m)
			{
				m = *pcv++;
				k += SZB;
				continue;

				// NOTE: dereferencing pcv is dirty at this point because it can
				// point behind the coding vector. This is not critical because
				// we don't use this value (hence we don't catch it in favor of
				// performance), but in the strict sense this is an access violation.
			}

			k += gpi_get_lsb(m);

			if (k >= NUM_ELEMENTS(mx.matrix))
				break;

			// check if row to substitute is empty
			// ATTENTION: this is needed if decode() called before reaching full rank
			// (e.g. at end of round)
			if (UINT16_MAX != mx.matrix[k].birth_slot)
			{
				pp[pp_used++] = &(mx.matrix[k].coding_vector[0]);
				if (NUM_ELEMENTS(pp) == pp_used)
				{
					ASSERT_CT(offsetof(Matrix_Row, payload) ==
						offsetof(Matrix_Row, coding_vector) + sizeof_member(Matrix_Row, coding_vector),
						inconsistent_code);

					PROFILE("mixer_decode() row memxor_block(full) begin");

					// NOTE: calling with NUM_ELEMENTS(pp) instead of pp_used leads to a bit better
					// code because NUM_ELEMENTS(pp) is a constant (msp430-gcc 4.6.3)
					memxor_block(&(mx.matrix[i].coding_vector[0]), pp,
						sizeof(mx.matrix[0].coding_vector) + sizeof(mx.matrix[0].payload), NUM_ELEMENTS(pp));

					#if MX_BENCHMARK_PSEUDO_PAYLOAD
						#if GPI_ARCH_IS_CORE(MSP430)
							__delay_cycles((3 + NUM_ELEMENTS(pp)) * MX_PAYLOAD_SIZE);
						#else
							#error MX_BENCHMARK_PSEUDO_PAYLOAD is unsupported on current architecture
						#endif
					#endif

					pp_used = 0;

					PROFILE("mixer_decode() row memxor_block(full) end");
				}
			}

			k &= ~(SZB - 1);
			m &= m - 1;
		}

		if (pp_used)
		{
			ASSERT_CT(offsetof(Matrix_Row, payload) ==
				offsetof(Matrix_Row, coding_vector) + sizeof_member(Matrix_Row, coding_vector),
				inconsistent_code);

			memxor_block(&(mx.matrix[i].coding_vector[0]), pp,
				sizeof(mx.matrix[0].coding_vector) + sizeof(mx.matrix[0].payload), pp_used);

			#if MX_BENCHMARK_PSEUDO_PAYLOAD
				#if GPI_ARCH_IS_CORE(MSP430)
					__delay_cycles(3 * MX_PAYLOAD_SIZE);
					while (pp_used--)
						__delay_cycles(MX_PAYLOAD_SIZE);
				#else
					#error MX_BENCHMARK_PSEUDO_PAYLOAD is unsupported on current architecture
				#endif
			#endif
		}

		PROFILE("mixer_decode() row end");

		// yield after each row to keep thread-level response time small (enough)
		// ATTENTION: matrix has to be in a consistent state at this point
		PT_YIELD(pt);
	}

	#if MX_VERBOSE_STATISTICS
		mx.stat_counter.slot_decoded = mx.slot_number;
	#endif

	GPI_TRACE_MSG(TRACE_INFO, "decoding done");
	TRACE_MATRIX();

	PT_END(pt);
}

//**************************************************************************************************

PT_THREAD(mixer_maintenance())
{
	Pt_Context* const	pt = pt_maintenance;

	PT_BEGIN(pt);

	// init variables at thread startup
	// NOTE: approach is useful because thread gets reinitialized (-> PT_INIT) on each mixer round
	mx.round_deadline = gpi_tick_hybrid() + (GPI_TICK_HYBRID_MAX / 2);
	mx.round_deadline_update_slot = UINT16_MAX;

	while (1)
	{
		PT_WAIT_UNTIL(pt, mx.events & BV(TRIGGER_TICK));
		clear_event(TRIGGER_TICK);

		Gpi_Hybrid_Tick	now = gpi_tick_hybrid();

		// monitor round length
		// NOTE: we test once per slot, and STOP executes gracefully at the next slot boundary
		// (or both a bit relaxed during RESYNC). Hence, the timing (e.g. when in the slot is
		// "now"?) is not very critical here.
		if ((mx.slot_number >= MX_ROUND_LENGTH) || (gpi_tick_compare_hybrid(now, mx.round_deadline) >= 0))
		{
			GPI_TRACE_MSG(TRACE_INFO, "max. round length reached -> STOP initiated");

			gpi_atomic_set(&mx.events, BV(DEADLINE_REACHED));

			while (!mixer_transport_set_next_slot_task(STOP));
			PT_EXIT(pt);
		}
		else if (mx.round_deadline_update_slot != mx.slot_number)
		{
			// ATTENTION: updating round deadline only on slot_number updates is important
			// for right behaviour during RESYNC phases

			ASSERT_CT((GPI_TICK_HYBRID_MAX / 2) / MX_SLOT_LENGTH >= MX_ROUND_LENGTH, round_period_overflow);

			mx.round_deadline_update_slot = mx.slot_number;

			if ((mx.ref_slot != 0) & (mx.ref_time != 0))
			{
				mx.round_deadline = mx.ref_time +
					gpi_mulu((Gpi_Hybrid_Tick)MX_SLOT_LENGTH, (typeof(mx.ref_slot))MX_ROUND_LENGTH - mx.ref_slot);
			}
			else
			{
				mx.round_deadline = now +
					gpi_mulu((Gpi_Hybrid_Tick)MX_SLOT_LENGTH, (typeof(mx.slot_number))MX_ROUND_LENGTH - mx.slot_number);
			}

			GPI_TRACE_MSG(TRACE_INFO, "round deadline: %lu (%luus from now)",
				(unsigned long)mx.round_deadline, (unsigned long)gpi_tick_hybrid_to_us(mx.round_deadline - now));
		}

		#if MX_VERBOSE_PROFILE

			static unsigned int s_snapshot_index = 0;

			Gpi_Profile_Ticket	ticket;
			const char			*module_name;
			uint16_t			line;
			uint32_t			timestamp;

			gpi_milli_sleep(10);

			s_snapshot_index++;

			memset(&ticket, 0, sizeof(ticket));

			while (gpi_profile_read(&ticket, &module_name, &line, &timestamp))
			{
				#if !(GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB) || GPI_ARCH_IS_BOARD(TMOTE_INDRIYA))
					printf("# ID:%u ", mx.tx_packet.sender_id + 1);
				#endif

				printf("profile %u %s %4" PRIu16 ": %" PRIu32 "\n", s_snapshot_index, module_name, line, timestamp);
			}

		#endif
	}

	PT_END(pt);
}

//**************************************************************************************************
//**************************************************************************************************
