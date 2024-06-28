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
 *	@file					mixer_request.c
 *
 *	@brief					Mixer request management
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

#include "gpi/trace.h"

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
//#define TRACE_GROUP1		0x00000001
//#define TRACE_GROUP2		0x00000002

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION	GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(mixer_request, GPI_TRACE_BASE_SELECTION | GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "mixer_internal.h"

#include "gpi/olf.h"

#if MX_REQUEST

#if MX_VERBOSE_PROFILE
	GPI_PROFILE_SETUP("mixer_request.c", 300, 4);
#endif

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

static inline void request_or(uint_fast_t *dest, const uint8_t *src, unsigned int size)
{
	// NOTE: process 8-bit-wise because src may be unaligned

	uint8_t	*pd = (uint8_t*)dest;

	while (pd < (uint8_t*)&dest[size / sizeof(*dest)])
		*pd++ |= *src++;
}

//**************************************************************************************************

static inline void request_and(uint_fast_t *dest, const void *src, unsigned int size)
{
	// NOTE: process 8-bit-wise because src may be unaligned

	uint8_t			*pd = (uint8_t*)dest;
	const uint8_t	*ps = (const uint8_t*)src;

	while (pd < (uint8_t*)&dest[size / sizeof(*dest)])
		*pd++ &= *ps++;
}

//**************************************************************************************************

static inline __attribute__((always_inline)) uint16_t request_clear(uint_fast_t *dest, const void *src, unsigned int size)
{
	// NOTE: process 8-bit-wise because src may be unaligned

	uint8_t			*pd = (uint8_t*)dest;
	const uint8_t	*ps = (const uint8_t*)src;
	uint16_t		 c = 0;

	while (pd < (uint8_t*)&dest[size / sizeof(*dest)])
	{
		*pd &= ~(*ps++);
		c += gpi_popcnt_8(*pd++);
	}

	return c;
}

//**************************************************************************************************

static void update_request_marker(Request_Marker *m, const Packet *p)
{
	PROFILE("update_request_marker() begin");

	// No pending requests, thus, we can directly copy the packet request information.
	if (!(m->any_pending))
	{
		gpi_memcpy_dma_inline(m->any_mask, p->info_vector, sizeof(p->info_vector));
		gpi_memcpy_dma_inline(m->all_mask, p->info_vector, sizeof(p->info_vector));

		m->any_pending = 1;	// temporary, will be updated together with following coding vector snoop
	}
	else
	{
		request_or (m->any_mask, p->info_vector, sizeof(p->info_vector));
		request_and(m->all_mask, p->info_vector, sizeof(p->info_vector));
	}

	// Set unused bits (beyond MX_GENERATION_SIZE) to 0.
	m->any_mask[NUM_ELEMENTS(m->any_mask) - 1] &= mx.request.padding_mask;
	m->all_mask[NUM_ELEMENTS(m->all_mask) - 1] &= mx.request.padding_mask;

	mx.request.last_update_slot = p->slot_number;

	PROFILE("update_request_marker() end");
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

uint16_t mx_request_clear(uint_fast_t *dest, const void *src, unsigned int size)
{
	return request_clear(dest, src, size);
}

//**************************************************************************************************

void mx_update_request(const Packet *p)
{
	GPI_TRACE_FUNCTION();
	PROFILE("mx_update_request() begin");

	if ((p != &mx.tx_packet) && !(p->flags.is_full_rank))
	{
		// ATTENTION: in extreme overload situations (which can not happen if system is configured
		// reasonable), the content of p can change while we are working on it. It would hurt the
		// request information, which is a significant, but not fatal, error. In contrast, we have
		// to make absolutely sure that there are no side effects to memory outside of the request
		// information. This could happen at the following memcpy() calls if we wouldn't handle
		// sender_id in a save way.

		uint8_t sender_id = p->sender_id;
		if (sender_id >= MX_NUM_NODES)
			GPI_TRACE_RETURN();

		if (p->flags.info_type == IT_COLUMN_REQUEST)
		{
			update_request_marker(&mx.request.column, p);

			#if MX_REQUEST_HEURISTIC > 1
			// NOTE: Column requests ask for messages (rows) that are not present at the node (not
			// even as part of a linear combination). A bit set in the column request implies that
			// the corresponding row is missing too, that's why we OR the IV with the row_map. A
			// bit not set in the column request either means the message is already present or it
			// is not present but part in some linear combination.
				request_or(mx.history[sender_id].row_map, p->info_vector, sizeof(p->info_vector));
			#endif
		}

		else if (p->flags.info_type == IT_ROW_REQUEST)
			update_request_marker(&mx.request.row, p);

		// NOTE: ROW_MAP and ROW_REQUEST share the same IV. Here we basically check if the IV
		// contains this information.
		#if MX_REQUEST_HEURISTIC > 1
			if ((p->flags.all & EIT_ROW_MAP_mask) == EIT_ROW_MAP_pattern)
				gpi_memcpy_dma_inline(mx.history[sender_id].row_map, p->info_vector, sizeof(p->info_vector));
		#endif
	}

	// snoop coding vector and update any_pending data

	if (mx.request.column.any_pending)
	{
		uint_fast16_t last = mx.request.column.any_pending;

		mx.request.column.any_pending =
			request_clear(mx.request.column.any_mask, p->coding_vector, sizeof(p->coding_vector));
		request_clear(mx.request.column.all_mask, p->coding_vector, sizeof(p->coding_vector));

		if (mx.request.column.any_pending != last)
			mx.request.last_update_slot = p->slot_number;
	}

	if (mx.request.row.any_pending)
	{
		int_fast16_t i = mx_get_leading_index(p->coding_vector);
		if (i >= 0)
		{
			uint_fast_t m = gpi_slu(1, i);

			if (mx.request.row.any_mask[i / (sizeof(m) * 8)] & m)
			{
				mx.request.row.any_mask[i / (sizeof(m) * 8)] &= ~m;
				mx.request.row.all_mask[i / (sizeof(m) * 8)] &= ~m;

				mx.request.last_update_slot = p->slot_number;
			}
		}

		mx.request.row.any_pending = 0;
		for (i = NUM_ELEMENTS(mx.request.row.any_mask); i-- > 0;)
		{
			if (mx.request.row.any_mask[i])
			{
				mx.request.row.any_pending = 1;
				break;
			}
		}
	}

	PROFILE("mx_update_request() end");

	TRACE_DUMP(1, "any_row_mask:", mx.request.row.any_mask, sizeof(mx.request.row.any_mask));
	TRACE_DUMP(1, "any_column_mask:", mx.request.column.any_mask, sizeof(mx.request.column.any_mask));

	GPI_TRACE_RETURN();
}

//**************************************************************************************************
//**************************************************************************************************

#endif	// MX_REQUEST
