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
 *	@file					mixer_internal.h
 *
 *	@brief					internal mixer declarations
 *
 *	@version				$Id$
 *	@date					TODO
 *
 *	@author					Carsten Herrmann
 							Fabian Mager
 *
 ***************************************************************************************************

 	@details

	TODO

 **************************************************************************************************/

#ifndef __MIXER_INTERNAL_H__
#define __MIXER_INTERNAL_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "mixer.h"

#include "gpi/protothreads.h"
#include "gpi/tools.h"

#if MX_VERBOSE_PROFILE
	#include "gpi/profile.h"
#endif

#include <stdint.h>

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************

// check if required settings are defined

#ifndef MX_NUM_NODES
	#error MX_NUM_NODES is undefined
#endif

#ifndef MX_GENERATION_SIZE
	#error MX_GENERATION_SIZE is undefined
#endif

#ifndef MX_INITIATOR_ID
	#error MX_INITIATOR_ID is undefined
#endif

#if !MX_PAYLOAD_SIZE
	#error MX_PAYLOAD_SIZE is invalid
#endif

#ifndef MX_SLOT_LENGTH
	#error MX_SLOT_LENGTH is undefined
#endif

#if !MX_ROUND_LENGTH
	#error MX_ROUND_LENGTH is invalid
#endif

// default values for optional settings

#ifndef MX_COORDINATED_TX
	#define MX_COORDINATED_TX			1
#endif

#ifndef MX_REQUEST
	#define MX_REQUEST					1
	#ifndef MX_REQUEST_HEURISTIC
		#define MX_REQUEST_HEURISTIC	2
	#endif
#endif

#ifndef MX_SMART_SHUTDOWN
	#define MX_SMART_SHUTDOWN			1
	#ifndef MX_SMART_SHUTDOWN_MODE
		#define MX_SMART_SHUTDOWN_MODE	3
	#endif
#endif

#ifndef MX_WEAK_ZEROS
	#define MX_WEAK_ZEROS				1
#endif

#ifndef MX_BENCHMARK_NO_SIDELOAD
	#define MX_BENCHMARK_NO_SIDELOAD				0
#endif

#ifndef MX_BENCHMARK_NO_SYSTEMATIC_STARTUP
	#define MX_BENCHMARK_NO_SYSTEMATIC_STARTUP		0
#endif

#ifndef MX_BENCHMARK_NO_COORDINATED_STARTUP
	#define MX_BENCHMARK_NO_COORDINATED_STARTUP		0
#endif

#ifndef MX_BENCHMARK_FULL_RANDOM_TX
	#define MX_BENCHMARK_FULL_RANDOM_TX				0
#endif

#ifndef MX_BENCHMARK_PSEUDO_PAYLOAD
	#define MX_BENCHMARK_PSEUDO_PAYLOAD				0
#endif

// check settings

#if !MX_COORDINATED_TX
	#if MX_REQUEST && (MX_REQUEST_HEURISTIC > 0)
		#error MX_REQUEST_HEURISTIC > 0 needs MX_COORDINATED_TX turned on
	#endif
	#if MX_SMART_SHUTDOWN
		#warning MX_SMART_SHUTDOWN turned off because it needs MX_COORDINATED_TX
		#undef MX_SMART_SHUTDOWN
	#endif
#else
	#if MX_BENCHMARK_FULL_RANDOM_TX
		#error MX_BENCHMARK_FULL_RANDOM_TX contradicts MX_COORDINATED_TX
	#endif
#endif

#if MX_REQUEST
	#ifndef MX_REQUEST_HEURISTIC
		#error MX_REQUEST_HEURISTIC is undefined
	#endif
	#if MX_COORDINATED_TX && (MX_REQUEST_HEURISTIC == 0)
		#warning MX_REQUEST_HEURISTIC == 0 is not reasonable with MX_COORDINATED_TX on, changed it to 1
		#undef MX_REQUEST_HEURISTIC
		#define MX_REQUEST_HEURISTIC	1
	#endif
#endif

#if MX_SMART_SHUTDOWN
	#ifndef MX_SMART_SHUTDOWN_MODE
		#error MX_SMART_SHUTDOWN_MODE is undefined
	#elif (MX_SMART_SHUTDOWN_MODE > 5)
		#error MX_SMART_SHUTDOWN_MODE is invalid
	#endif
#else
	#undef MX_SMART_SHUTDOWN_MODE
#endif

// internal settings

#define MX_SLOT_LENGTH_RESYNC				((MX_SLOT_LENGTH * 5) / 2)

#define MX_HISTORY_WINDOW					(MX_ROUND_LENGTH * 3 / 4) // (3 * MX_NUM_NODES)
#if (MX_SMART_SHUTDOWN_MODE >= 4)
	#define MX_HISTORY_WINDOW_FINISHED		MX_HISTORY_WINDOW
	#define MX_HISTORY_WINDOW_ACKED			(1 * MX_NUM_NODES)
#else
	#define MX_HISTORY_WINDOW_FINISHED		(1 * MX_NUM_NODES)
#endif

// MX_HISTORY_DISCOVERY_BEHAVIOR flags:
// 1	do not prune during discovery phase
// 2	reload counters when leaving discovery phase
#ifndef MX_HISTORY_DISCOVERY_BEHAVIOR
	#define MX_HISTORY_DISCOVERY_BEHAVIOR	(1 + 2)
#endif

#ifndef MX_AGE_TO_INCLUDE_PROBABILITY
	// uint16_t
	#define MX_AGE_TO_INCLUDE_PROBABILITY	{ 32768 }
#endif

#ifndef MX_AGE_TO_TX_PROBABILITY
	// uint8_t, last entry = f(age up to infinity) (should be 0 if MX_COORDINATED_TX is on)
	#if MX_COORDINATED_TX
		#define MX_AGE_TO_TX_PROBABILITY	{ /*0xff, 0x7f,*/ 0 }
	#else
		#define MX_AGE_TO_TX_PROBABILITY	{ 128 }		// full random
	#endif
#endif

// MX_SMART_SHUTDOWN_MODE:
// 0	no smart shutdown
// 1	no unfinished neighbor, without full-rank map(s)
// 2	no unfinished neighbor
// 3	all nodes full rank
// 4	all nodes full rank, all neighbors ACKed knowledge of this fact
// 5	all nodes full rank, all nodes ACKed knowledge of this fact
#if (MX_SMART_SHUTDOWN_MODE >= 2)
	#define MX_SMART_SHUTDOWN_MAP		1
#else
	#define MX_SMART_SHUTDOWN_MAP		0
#endif

#if (MX_REQUEST || MX_SMART_SHUTDOWN_MAP || MX_WEAK_ZEROS)
	#define MX_INCLUDE_INFO_VECTOR		1
#else
	#define MX_INCLUDE_INFO_VECTOR		0
#endif

//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************

#define FAST_HYBRID_RATIO		(GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE)
#define HYBRID_SLOW_RATIO		(GPI_HYBRID_CLOCK_RATE / GPI_SLOW_CLOCK_RATE)

#define PHY_PAYLOAD_SIZE		(offsetof(Packet, phy_payload_end) - offsetof(Packet, phy_payload_begin))

#define FAST_T_WIDTH			(sizeof(uint_fast_t) * 8)

// NOTE: PADDING_SIZE() uses sizeof(uint_fast_t) instead of ALIGNMENT (from gpi/tools.h)
// because ALIGNMENT maybe greater than sizeof(uint_fast_t), which isn't necessary here
#define PADDING_SIZE(x)			((sizeof(uint_fast_t) - ((x) % sizeof(uint_fast_t))) % sizeof(uint_fast_t))
#define PADDING_MAX(a,b)		((int)(a) >= (int)(b) ? (a) : (b))

#if MX_VERBOSE_PROFILE
	#define PROFILE(...)				GPI_PROFILE(100, ## __VA_ARGS__)
	#define PROFILE_P(priority, ...)	GPI_PROFILE(priority, ## __VA_ARGS__)
#else
	#define PROFILE(...)		while (0)
	#define PROFILE_P(...)		while (0)
#endif

#if (GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE)
	#define TRACE_DUMP(group, fmt, src, size)					\
		do {													\
			if ((group) & gpi_trace_module_desc.msg_config)		\
				mx_trace_dump(fmt, src, size);					\
		} while (0)
#else
	#define TRACE_DUMP(group, fmt, src, size)	while (0)
#endif

//**************************************************************************************************

#if GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB)

	#define LED_GRID_TIMER_ISR	GPI_LED_1
	#define LED_SFD_ISR			GPI_LED_2
	#define LED_DMA_ISR			GPI_LED_2
	#define LED_TIMEOUT_ISR		GPI_LED_3
	#define LED_RX				GPI_LED_4
	#define LED_TX				GPI_LED_5
	#define LED_UPDATE_TASK		GPI_LED_5

#elif GPI_ARCH_IS_BOARD(TMOTE_PURE)

	#define LED_GRID_TIMER_ISR	GPI_LED_1
	#define LED_SFD_ISR			GPI_LED_2
	#define LED_DMA_ISR			GPI_LED_2
	#define LED_TIMEOUT_ISR		GPI_LED_3
	#define LED_RX				GPI_LED_NONE
	#define LED_TX				GPI_LED_NONE
	#define LED_UPDATE_TASK		GPI_LED_NONE

#elif GPI_ARCH_IS_BOARD(nRF_PCA10056)

	#define LED_GRID_TIMER_ISR	GPI_LED_1
	#define LED_RADIO_ISR		GPI_LED_3
	#define LED_TIMEOUT_ISR		GPI_LED_2
	#define LED_RX				GPI_LED_4
	#define LED_TX				GPI_LED_4
	#define LED_UPDATE_TASK		GPI_LED_NONE

#elif GPI_ARCH_IS_BOARD(nRF_PCA10059)

	#define LED_GRID_TIMER_ISR	GPI_LED_4
	#define LED_RADIO_ISR		GPI_LED_NONE
	#define LED_TIMEOUT_ISR		GPI_LED_NONE
	#define LED_RX				GPI_LED_3
	#define LED_TX				GPI_LED_2
	#define LED_UPDATE_TASK		GPI_LED_NONE

#elif GPI_ARCH_IS_BOARD(TUDNES_DPP2COM)

	#define LED_GRID_TIMER_ISR	GPI_LED_NONE
	#define LED_RADIO_ISR		GPI_LED_NONE
	#define LED_TIMEOUT_ISR		GPI_LED_NONE
	#define LED_RX				GPI_LED_NONE //GPI_LED_1
	#define LED_TX				GPI_LED_NONE
	#define LED_UPDATE_TASK		GPI_LED_NONE

#else
	#pragma message "mixer diagnostic LEDs are deactivated because GPI_ARCH_BOARD is unknown"
#endif

//**************************************************************************************************
//***** Forward Class and Struct Declarations ******************************************************



//**************************************************************************************************
//***** Global Typedefs and Class Declarations *****************************************************

#if GPI_ARCH_IS_CORE(MSP430)

	typedef uint16_t	uint_fast_t;
	typedef int16_t		int_fast_t;

	#define PRIdFAST	PRId16
	#define PRIuFAST	PRIu16
	#define PRIxFAST	"04" PRIx16

#elif GPI_ARCH_IS_CORE(ARMv7M)

	typedef uint32_t	uint_fast_t;
	typedef int32_t		int_fast_t;

	#define PRIdFAST	PRId32
	#define PRIuFAST	PRIu32
	#define PRIxFAST	"08" PRIx32

#else
	#error unsupported architecture
#endif

#if GPI_ARCH_IS_BOARD(TMOTE)

	#define PACKET_INTERNAL_TAIL						\
		struct __attribute__((packed)) {				\
			int8_t				rssi;					\
			union __attribute__((packed)) {				\
				int8_t			crc_corr;				\
				struct __attribute__((packed)) {		\
					uint8_t		correlation	: 7;		\
					uint8_t		crc_ok		: 1;		\
				};										\
			};											\
		}

#elif GPI_ARCH_IS_DEVICE(nRF52840)

	#define PACKET_INTERNAL_HEADER											\
		struct __attribute__((packed)) {									\
			uint8_t			_padding_1[2];	/* for alignment */				\
			uint8_t			ble_header[1];	/* unused in non-BLE mode */	\
			uint8_t			len;											\
		}

#endif

//**************************************************************************************************

typedef enum Packet_Info_Type_
{
	// if is_full_rank == 0
	IT_ROW_MAP						= 0,
//	RESERVED						= 1,
	IT_ROW_REQUEST					= 2,
	IT_COLUMN_REQUEST				= 3,

	// if is_full_rank == 1
	IT_FULL_RANK_MAP				= 0,
	IT_FULL_RANK_MAP_RADIO_OFF		= 1,
	IT_FULL_RANK_ACK_MAP			= 2,
	IT_FULL_RANK_ACK_MAP_RADIO_OFF	= 3,

	// irrespective of is_full_rank
	IT_WEAK_ZERO_MAP				= 4,

	// reserved / undefined -> info_vector should be ignored
//	RESERVED						= 5...13
//	RESERVED__FLAGS_2				= 14,
	IT_UNDEFINED					= 15,

	// following masks incorporate is_full_rank flag to simplify evaluation of dependent types
	// usage: (flags.all & EIT_mask) == EIT_...
	EIT_mask						= 0x8f,
	EIT_ROW_MAP						= 0x00,
	EIT_ROW_REQUEST					= 0x02,
	EIT_COLUMN_REQUEST				= 0x03,
	EIT_FULL_RANK_MAP				= 0x80,
	EIT_FULL_RANK_MAP_RADIO_OFF		= 0x81,
	EIT_FULL_RANK_ACK_MAP			= 0x82,
	EIT_FULL_RANK_ACK_MAP_RADIO_OFF	= 0x83,

	// following masks reduce number of cases by combining types
	// usage: (flags.all & EIT_..._mask) == EIT_..._pattern
	EIT_REQUEST_mask				= 0x8e,
	EIT_REQUEST_pattern				= 0x02,
	EIT_ROW_MAP_mask				= 0x8d,
	EIT_ROW_MAP_pattern				= 0x00,
	EIT_FULL_RANK_MAP_mask			= 0x8e,
	EIT_FULL_RANK_MAP_pattern		= 0x80,
	EIT_FULL_RANK_ACK_MAP_mask		= 0x8e,
	EIT_FULL_RANK_ACK_MAP_pattern	= 0x82,
	EIT_FULL_RANK_X_mask			= 0x8c,		// FULL_RANK_MAP or FULL_RANK_ACK_MAP
	EIT_FULL_RANK_X_pattern			= 0x80,
	EIT_RADIO_OFF_mask				= 0x8d,
	EIT_RADIO_OFF_pattern			= 0x81,

} Packet_Info_Type;

//**************************************************************************************************

typedef union __attribute__((packed)) Packet_Flags_tag
{
	uint8_t			all;

	struct __attribute__((packed))
	{
		uint8_t		info_type			: 4;
		uint8_t							: 1;	// reserved
		uint8_t		owner_forecast_1	: 1;	// during startup phase (can be reused thereafter)
		uint8_t		owner_forecast_2	: 1;	// during startup phase (can be reused thereafter)
		uint8_t		is_full_rank		: 1;
	};

} Packet_Flags;

//**************************************************************************************************

// internal (i.e., in memory) packet format
// ATTENTION: packet format is well designed with respect to alignment
typedef struct Packet_tag
{
	// platform-specific header
	// NOTE: can be used to map additional fields (if any) of a platform-internal packet format.
	// A typical use case is the insertion of some LENGTH field into the packet.
	// ATTENTION: must be done in a way that ensures alignment
#ifdef PACKET_INTERNAL_HEADER
	PACKET_INTERNAL_HEADER ;
#endif

	// PDU format as defined by Mixer
	struct __attribute__((packed))
	{
		union __attribute__((packed))
		{
			uint8_t		phy_payload_begin;	// just a marker (e.g. for offsetof(Packet, phy_payload_begin))
			uint16_t	slot_number;
		};

		uint8_t			sender_id;
		Packet_Flags	flags;

		// NOTE: coding_vector should be machine word aligned (because of wrapping/unwrapping)
		uint8_t			coding_vector[(MX_GENERATION_SIZE + 7) / 8];
		uint8_t			payload[MX_PAYLOAD_SIZE];

#if MX_INCLUDE_INFO_VECTOR
		uint8_t			info_vector[(MX_GENERATION_SIZE + 7) / 8];
#endif
	};

	// packet tail (not transmitted)
	union __attribute__((packed))
	{
		int8_t			phy_payload_end;	// just a marker (e.g. for offsetof(Packet, phy_payload_end))

		// standard fields (used on transport and processing layer)
		struct __attribute__((packed))
		{
			// padding that absorbs unwrapping of coding_vector and payload
			// ATTENTION: members near to phy_payload_end may be overwritten by word based
			// operations (e.g. memxor()), so don't overlay sensitive information there
			int8_t		_padding_2[PADDING_MAX(0,
							PADDING_SIZE((MX_GENERATION_SIZE + 7) / 8)	// coding_vector
							+ PADDING_SIZE(MX_PAYLOAD_SIZE)				// payload
#if MX_INCLUDE_INFO_VECTOR
							- ((MX_GENERATION_SIZE + 7) / 8)			// info_vector (not needed in unwrapped state)
#endif
							)];

			union __attribute__((packed))
			{
				uint8_t			tx_packet_status;

				struct __attribute__((packed))
				{
					uint8_t		rand			: 6;
					uint8_t		is_valid		: 1;	// meaning while is_ready == 0
					uint8_t		is_ready		: 1;
				};

				//struct __attribute__((packed))
				//{
				//	uint8_t						: 6;
				//	uint8_t		use_sideload	: 1;	// meaning while is_ready == 1
				//	uint8_t						: 1;
				//};
			};
		};

		// platform-specific tail, temporarily used on transport layer
		// NOTE: can be used to map additional fields (if any) of a platform-internal packet format
		// (e.g. CRC, RSSI, ...)
#ifdef PACKET_INTERNAL_TAIL
		PACKET_INTERNAL_TAIL ;
#endif
	};

	// pad such that sizeof(Packet) is aligned
	// ATTENTION: ensures that aligned fields are also aligned in memory in every Rx queue slot.
	// This is an important prerequisite to enable fast word based operations.
	uint_fast_t		_padding_3[0];

} Packet;

ASSERT_CT_STATIC(!(offsetof(Packet, phy_payload_begin) % sizeof(uint_fast_t)), alignment_issue);
ASSERT_CT_STATIC(!(sizeof(Packet) % sizeof(uint_fast_t)), alignment_issue);
// TODO: comment
ASSERT_CT_WARN_STATIC((MX_SMART_SHUTDOWN_MODE < 3) || (FIELD_SIZEOF(Packet, info_vector) * 8 >= MX_NUM_NODES), IV_to_small_for_reliable_smart_shutdown);

//**************************************************************************************************

typedef struct Matrix_Row_tag
{
	union
	{
		uint16_t		birth_slot;
   		uint_fast_t		_alignment_dummy;
	};

	union
	{
		struct
		{
			uint8_t				coding_vector_8[(MX_GENERATION_SIZE + 7) / 8];

			#if MX_BENCHMARK_PSEUDO_PAYLOAD
				uint8_t			payload_8[0];
			#else
				uint8_t			payload_8[MX_PAYLOAD_SIZE];
			#endif
		};

		struct
		{
			uint_fast_t			coding_vector[
									(MX_GENERATION_SIZE + (sizeof(uint_fast_t) * 8) - 1) /
									(sizeof(uint_fast_t) * 8)	];	// -> potentially longer

			#if MX_BENCHMARK_PSEUDO_PAYLOAD
				uint_fast_t		payload[0];
			#else
				uint_fast_t		payload[
									(MX_PAYLOAD_SIZE + sizeof(uint_fast_t) - 1) /
									sizeof(uint_fast_t)	];	// -> potentially shifted and longer
			#endif
		};
	};

} Matrix_Row;

//**************************************************************************************************

typedef struct Node_tag
{
	// linked list pointers
	uint8_t				prev;
	uint8_t				next;

	union
	{
		uint16_t		value;					// for raw access
		uint16_t		num_nodes;				// for sentinel nodes
		struct									// for real nodes
		{
		#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

			uint16_t	list_id				: 2;
			uint16_t	last_slot_number	: 14;

		#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

			uint16_t	last_slot_number	: 14;
			uint16_t	list_id				: 2;

		#else
			#error byte order is unknown
		#endif
		};
	};

#if (MX_REQUEST_HEURISTIC > 1)
	uint_fast_t			row_map[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];
#endif

	// value is either the slot in which the node was heard last or the number of nodes in the list
	// in case of the sentinel node. Slot numbers are shifted left by 2 and the 2 LSBs indicate a
	// node's current list/state (present, absent, finished). The shift allows to do computations
	// without masking the state bits.

} Node;

//**************************************************************************************************
#if MX_SMART_SHUTDOWN_MAP

typedef struct Full_Rank_Map_tag
{
//	#define HASH_FACTOR		((MX_NUM_NODES + MX_GENERATION_SIZE - 1) / MX_GENERATION_SIZE)
	#define HASH_FACTOR		(((MX_NUM_NODES + 7) / 8 + sizeof_member(Packet, info_vector) - 1) / sizeof_member(Packet, info_vector))

	uint8_t			map[HASH_FACTOR * sizeof_member(Packet, info_vector)];
	uint8_t			hash[sizeof_member(Packet, info_vector)];

} Full_Rank_Map;

#endif
//**************************************************************************************************

typedef struct Request_Marker_tag
{
	uint_fast_t		all_mask[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];
	uint_fast_t		any_mask[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];
	uint_fast16_t	any_pending;

} Request_Marker;

typedef struct Request_Data_tag
{
	Request_Marker	row;
	Request_Marker	column;
	uint16_t		last_update_slot;
	int16_t			help_index;
	uint_fast_t		help_bitmask;

	uint_fast_t		my_row_mask[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];
	uint_fast_t		my_column_mask[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];
	uint16_t		my_column_pending;

	uint_fast_t		padding_mask;

} Request_Data;

//**************************************************************************************************

typedef struct Weak_Zero_Map_tag
{
	uint_fast_t		weak_mask[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];
	uint_fast_t		strong_mask[sizeof_member(Matrix_Row, coding_vector) / sizeof(uint_fast_t)];

} Weak_Zero_Map;

//**************************************************************************************************

typedef enum Slot_Activity_tag
{
	RX			= 0,
	TX			= 1,
	STOP		= 2

} Slot_Activity;

//**************************************************************************************************

typedef enum Event_tag
{
	// order = priority: the lower the value, the higher its priority
	STOPPED = 0,
	SLOT_UPDATE,
	TX_READY,
	TRIGGER_TICK,
	RX_READY,
	DEADLINE_REACHED,

} Event;

//**************************************************************************************************

typedef struct pt 	Pt_Context;

//**************************************************************************************************
//***** Global Variables ***************************************************************************

// static internal data
extern struct mx
{
	Packet						rx_queue[4];
	volatile uint_fast_t		rx_queue_num_writing;
	volatile uint_fast_t		rx_queue_num_written;
	volatile uint_fast_t		rx_queue_num_read;

	Packet						tx_packet;
	const uint8_t* volatile		tx_sideload;
	const Matrix_Row*			tx_reserve;

	volatile uint16_t			slot_number;
	volatile unsigned int		events;				// type must be compatible to gpi_atomic_...()
	Gpi_Hybrid_Tick				round_deadline;
	uint16_t					round_deadline_update_slot;

	Matrix_Row					matrix[MX_GENERATION_SIZE];
	uint16_t					rank;
	Matrix_Row*					empty_row;
	Matrix_Row*					next_own_row;
	uint16_t					recent_innovative_slot;

#if MX_COORDINATED_TX
	#if (MX_SMART_SHUTDOWN_MODE >= 4)
		Node					history[MX_NUM_NODES + 4];		// incl. sentinel nodes
	#else
		Node					history[MX_NUM_NODES + 3];		// incl. sentinel nodes
	#endif
	#if !MX_BENCHMARK_NO_COORDINATED_STARTUP
		uint16_t				wake_up_slot;
		uint16_t				discovery_exit_slot;
	#endif
#endif

#if MX_SMART_SHUTDOWN
	volatile uint8_t			is_shutdown_approved;
	uint8_t						have_full_rank_partner;			// current or former neighbor
	#if MX_SMART_SHUTDOWN_MAP
		Full_Rank_Map			full_rank_map;
		volatile int8_t			full_rank_state;
	#endif
#endif

#if MX_REQUEST
	Request_Data				request;
#endif

#if MX_WEAK_ZEROS
	Weak_Zero_Map				weak_zero_map;
	uint16_t					weak_rank;
	uint16_t					weak_zero_release_slot;
	void*						weak_zero_return_msg;
#endif

	Mixer_Stat_Counter			stat_counter;

#if MX_VERBOSE_STATISTICS
	Gpi_Fast_Tick_Native		wake_up_timestamp;
#endif

	volatile Gpi_Hybrid_Tick	ref_time;
	volatile uint16_t			ref_slot;

} mx;

#if MX_COORDINATED_TX
	ASSERT_CT_STATIC(NUM_ELEMENTS(mx.history) > MX_NUM_NODES + 2);
	static Node* const			mx_absent_head = &mx.history[MX_NUM_NODES + 0];
	static Node* const			mx_present_head = &mx.history[MX_NUM_NODES + 1];
	static Node* const			mx_finished_head = &mx.history[MX_NUM_NODES + 2];
	#if (MX_SMART_SHUTDOWN_MODE >= 4)
		ASSERT_CT_STATIC(NUM_ELEMENTS(mx.history) > MX_NUM_NODES + 3);
		static Node* const		mx_acked_head = &mx.history[MX_NUM_NODES + 3];
	#endif
#endif

// the following is significant for efficient queue handling
ASSERT_CT_STATIC(IS_POWER_OF_2(NUM_ELEMENTS(((struct mx*)0)->rx_queue)), rx_queue_size_must_be_power_of_2);
ASSERT_CT_STATIC(!((uintptr_t)&mx.rx_queue & (sizeof(uint_fast_t) - 1)), rx_queue_is_not_aligned);
ASSERT_CT_STATIC(!((uintptr_t)&mx.tx_packet & (sizeof(uint_fast_t) - 1)), rx_queue_is_not_aligned);

//**************************************************************************************************

extern Pt_Context				pt_data[3];

//**************************************************************************************************
//***** Prototypes of Global Functions *************************************************************

#ifdef __cplusplus
	extern "C" {
#endif

void 			mx_trace_dump(const char *header, const void *p, uint_fast16_t size);

// internal helper functions
uint16_t 		mixer_rand();
int_fast16_t	mx_get_leading_index(const uint8_t *coding_vector);
void			mx_init_history();
void			mx_update_history(uint16_t node_id, Packet_Flags flags, uint16_t slot_number);
void			mx_purge_history();
//static uint8_t	mx_history_num(const Node *head);
uint16_t		mx_request_clear(uint_fast_t *dest, const void *src, unsigned int size);
void			mx_update_request(const Packet *p);
void			mx_clear_full_rank_map();

void			mixer_transport_init();
void			mixer_transport_arm_initiator();
void			mixer_transport_start();
int 			mixer_transport_set_next_slot_task(Slot_Activity task);
Slot_Activity	mixer_transport_get_next_slot_task();
void			mixer_transport_print_config();

PT_THREAD(		mixer_update_slot());
PT_THREAD(		mixer_maintenance());
PT_THREAD(		mixer_process_rx_data());
PT_THREAD(		mixer_decode(Pt_Context *pt));

#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

static inline __attribute__((always_inline)) void unwrap_chunk(uint8_t *p)
{
	// double-check alignment of packet fields
	ASSERT_CT(
		!(offsetof(Packet, coding_vector) % sizeof(uint_fast_t)),
		inconsistent_alignment);
	ASSERT_CT(
		offsetof(Packet, payload) ==
			offsetof(Packet, coding_vector) + sizeof_member(Packet, coding_vector),
		inconsistent_alignment);

	// double-check alignment of matrix row fields
	ASSERT_CT(
		!(offsetof(Matrix_Row, coding_vector) % sizeof(uint_fast_t)),
		inconsistent_alignment);
	ASSERT_CT(
		!(offsetof(Matrix_Row, payload) % sizeof(uint_fast_t)),
		inconsistent_alignment);
	ASSERT_CT(
		offsetof(Matrix_Row, coding_vector_8) == offsetof(Matrix_Row, coding_vector),
		inconsisten_alignment);
	ASSERT_CT(
		offsetof(Matrix_Row, payload_8) ==
			offsetof(Matrix_Row, coding_vector_8) + sizeof_member(Matrix_Row, coding_vector_8),
		inconsisten_alignment);

	// NOTE: condition gets resolved at compile time
	if (offsetof(Matrix_Row, payload_8) != offsetof(Matrix_Row, payload))
	{
//		#pragma GCC diagnostic push
//		#pragma GCC diagnostic ignored "-Warray-bounds"

		uint8_t			*s = p + sizeof_member(Matrix_Row, coding_vector_8);
		uint8_t			*d = s + sizeof_member(Matrix_Row, payload_8);
		unsigned int	i;

		for (i = offsetof(Matrix_Row, payload) - offsetof(Matrix_Row, payload_8); i-- > 0;)
			*d++ = *s++;

//		#pragma GCC diagnostic pop
	}
}

//**************************************************************************************************

static inline __attribute__((always_inline)) void unwrap_row(unsigned int i)
{
	unwrap_chunk(&(mx.matrix[i].coding_vector_8[0]));
}

//**************************************************************************************************

static inline __attribute__((always_inline)) void wrap_chunk(uint8_t *p)
{
	// NOTE: condition gets resolved at compile time
	if (offsetof(Matrix_Row, payload_8) != offsetof(Matrix_Row, payload))
	{
//		#pragma GCC diagnostic push
//		#pragma GCC diagnostic ignored "-Warray-bounds"

		uint8_t			*d = p + sizeof_member(Matrix_Row, coding_vector_8);
		uint8_t			*s = d + sizeof_member(Matrix_Row, payload_8);
		unsigned int	i;

		for (i = offsetof(Matrix_Row, payload) - offsetof(Matrix_Row, payload_8); i-- > 0;)
			*d++ = *s++;

//		#pragma GCC diagnostic pop
	}
}

//**************************************************************************************************
//#if MX_COORDINATED_TX
//
//static inline __attribute__((always_inline)) uint8_t mx_history_num(const Node *head)
//{
//	return head->num_nodes;
//}
//
//#endif	// MX_COORDINATED_TX
//**************************************************************************************************
//**************************************************************************************************

#endif // __MIXER_INTERNAL_H__
