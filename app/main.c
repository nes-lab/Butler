/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2021, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					main.c
 *
 *	@brief					Example application using Butler to safely enable multi-initiator Mixer.
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
#include "mixer/mixer.h"

#include "gpi/clocks.h"
#include "gpi/interrupts.h"
#include "gpi/olf.h"
#include "gpi/platform.h"
#include "gpi/tools.h"
#include GPI_PLATFORM_PATH(radio.h)

#include <nrf.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************



//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************



//**************************************************************************************************
//***** Forward Declarations ***********************************************************************

uint16_t mixer_rand(void);

//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

static uint8_t  log_node_id;
static uint32_t round;
static uint32_t msgs_decoded;
static uint32_t msgs_not_decoded;
static uint32_t msgs_weak;
static uint32_t msgs_wrong;

//**************************************************************************************************
//***** Global Variables ***************************************************************************

// UNIQUE_ID is a variable with very special handling. Its init value gets overridden with the id of
// the node in the testbed during device programming. Thus, it is well suited as a node id variable.
#if TESTBED == TESTBED_FLOCKLAB
uint16_t UNIQUE_ID = 0; // any value
#elif TESTBED == TESTBED_LOCAL
	#if GPI_ARCH_IS_BOARD(nRF_PCA10059)
// Without enabling serial communication, we have to set the right node ID at compile time in
// the local test setup.
uint16_t __attribute__((section(".data"))) UNIQUE_ID = DONGLE_NODE_ID;
	#elif GPI_ARCH_IS_BOARD(nRF_PCA10056)
// We can provide the node ID at startup for the nRF52840-DK via UART.
uint16_t __attribute__((section(".data"))) UNIQUE_ID = 0;
	#else
		#error "Unsupported local board"
	#endif
#else
	#error "Unknown testbed"
#endif

// Assign UNIQUE_ID to volatile variable somewhere to prevent some optimizations.
volatile uint16_t phy_node_id;

//**************************************************************************************************
//***** Local Functions ****************************************************************************

#if GPI_ARCH_IS_BOARD(nRF5_FLOCKLAB)
// Read node ID on FlockLab based on factory information (device addr.).
static uint16_t flocklab_node_id(void)
{
	const uint32_t dev_addr[] = {
	    0x9866f68a, 0xfe694776, 0x4e14e2f8, 0x8045ddde,
	    0xea673b1f, 0x546931a7, 0x4db62047, 0x38057982, /* observers  1 -  8 */
	    0x322c95bb, 0x05840339, 0x6251e878, 0xe29d4310,
	    0x3dbb14a0, 0,          0,          0xa9bf0f2b, /* observers  9 - 16 */
	    0,          0,          0x73d0188a, 0xae33933c,
	    0x183d13fe, 0xd3e8a7ab, 0x0b59d912, 0x054fead2, /* observers 17 - 24 */
	    0,          0x7f15a6a9, 0x069fcd53, 0,
	    0xa271b29d, 0,          0xb86f91c3, 0 /* observers 25 - 32 */
	};

	uint32_t i;
	for (i = 0; i < NUM_ELEMENTS(dev_addr); i++)
	{
		if (dev_addr[i] == NRF_FICR->DEVICEADDR[0]) { return i + 1; }
	}

	return 0;
}
#endif

//**************************************************************************************************

// Print statistics from Mixer and Butler for the log parser.
static void print_results(int node_id)
{
	unsigned int slot, slot_min, rank, i;

	rank = 0;
	for (i = 0; i < MX_GENERATION_SIZE; ++i)
	{
		if (mixer_stat_slot(i) >= 0) rank++;
	}

	// stats
	// mixer_print_statistics();
	butler_print_statistics();

	PRINT_HEADER();
	printf("round=%" PRIu32 " rank=%" PRIu32 " dec=%" PRIu32 " notDec=%" PRIu32 " weak=%" PRIu32
	       " wrong=%" PRIu32 "\n",
	       round, rank, msgs_decoded, msgs_not_decoded, msgs_weak, msgs_wrong);
}

//**************************************************************************************************

// Initializes the hardware and sets up the node for the execution of Mixer and Butler.
static void initialization(void)
{
	// init platform
	gpi_platform_init();
	gpi_int_enable();

	// Start random number generator (RNG) now so that we definitely have some random value as a
	// seed later in the initialization.
	NRF_RNG->INTENCLR = BV_BY_NAME(RNG_INTENCLR_VALRDY, Clear);
	NRF_RNG->CONFIG   = BV_BY_NAME(RNG_CONFIG_DERCEN, Enabled); // bias correction
	// NRF_RNG->CONFIG = BV_BY_NAME(RNG_CONFIG_DERCEN, Disabled); // bias correction
	NRF_RNG->TASKS_START = 1;

	// enable SysTick timer if needed
	SysTick->LOAD = -1u;
	SysTick->VAL  = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

	// init RF transceiver
	gpi_radio_init(MX_PHY_MODE);
	gpi_radio_set_tx_power(gpi_radio_dbm_to_power_level(MX_TX_PWR_DBM));
	switch (MX_PHY_MODE)
	{
	case BLE_1M:
	case BLE_2M:
	case BLE_125k:
	case BLE_500k:
		gpi_radio_set_channel(39);
		gpi_radio_ble_set_access_address(~0x8E89BED6);
		break;

	case IEEE_802_15_4: gpi_radio_set_channel(26); break;

	default: printf("ERROR: MX_PHY_MODE is invalid!\n"); assert(0);
	}

// TODO: This could be moved to the GPI.
#if GPI_ARCH_IS_BOARD(nRF_PCA10059)
	// If nRF52 USB Dongle is powered from USB (high voltage mode),
	// GPIO output voltage is set to 1.8 V by default, which is not
	// enough to turn on green and blue LEDs. Therefore, GPIO voltage
	// needs to be increased to 3.0 V by configuring the UICR register.
	// if (BV_TEST_BY_NAME(NRF_POWER->MAINREGSTATUS, POWER_MAINREGSTATUS_MAINREGSTATUS, High))
	if (NRF_POWER->MAINREGSTATUS &
	    (POWER_MAINREGSTATUS_MAINREGSTATUS_High << POWER_MAINREGSTATUS_MAINREGSTATUS_Pos))
	{
		// Configure UICR_REGOUT0 register only if it is set to default value.
		if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
		    (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
		{
			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

			NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
			                    (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

			// System reset is needed to update UICR registers.
			NVIC_SystemReset();
		}
	}
#endif

	printf("Hardware initialized. Compiled at __DATE__ __TIME__ = " __DATE__ " " __TIME__ "\n");

	phy_node_id = UNIQUE_ID;

	/*
	 * Pearson hashing (from Wikipedia)
	 *
	 * Pearson hashing is a hash function designed for fast execution on processors with 8-bit
	 * registers. Given an input consisting of any number of bytes, it produces as output a single
	 * byte that is strongly dependent on every byte of the input. Its implementation requires only
	 * a few instructions, plus a 256-byte lookup table containing a permutation of the values 0
	 * through 255.
	 */

	// T table for Pearson hashing from RFC 3074.
	uint8_t T[256] = {
	    251, 175, 119, 215, 81,  14,  79,  191, 103, 49,  181, 143, 186, 157, 0,   232, 31,  32,
	    55,  60,  152, 58,  17,  237, 174, 70,  160, 144, 220, 90,  57,  223, 59,  3,   18,  140,
	    111, 166, 203, 196, 134, 243, 124, 95,  222, 179, 197, 65,  180, 48,  36,  15,  107, 46,
	    233, 130, 165, 30,  123, 161, 209, 23,  97,  16,  40,  91,  219, 61,  100, 10,  210, 109,
	    250, 127, 22,  138, 29,  108, 244, 67,  207, 9,   178, 204, 74,  98,  126, 249, 167, 116,
	    34,  77,  193, 200, 121, 5,   20,  113, 71,  35,  128, 13,  182, 94,  25,  226, 227, 199,
	    75,  27,  41,  245, 230, 224, 43,  225, 177, 26,  155, 150, 212, 142, 218, 115, 241, 73,
	    88,  105, 39,  114, 62,  255, 192, 201, 145, 214, 168, 158, 221, 148, 154, 122, 12,  84,
	    82,  163, 44,  139, 228, 236, 205, 242, 217, 11,  187, 146, 159, 64,  86,  239, 195, 42,
	    106, 198, 118, 112, 184, 172, 87,  2,   173, 117, 176, 229, 247, 253, 137, 185, 99,  164,
	    102, 147, 45,  66,  231, 52,  141, 211, 194, 206, 246, 238, 56,  110, 78,  248, 63,  240,
	    189, 93,  92,  51,  53,  183, 19,  171, 72,  50,  33,  104, 101, 69,  8,   252, 83,  120,
	    76,  135, 85,  54,  202, 125, 188, 213, 96,  235, 136, 208, 162, 129, 190, 132, 156, 38,
	    47,  1,   7,   254, 24,  4,   216, 131, 89,  21,  28,  133, 37,  153, 149, 80,  170, 68,
	    6,   169, 234, 151};

	// Pearsong hashing algorithm as described in RFC 3074.
	// -> http://www.apps.ietf.org/rfc/rfc3074.html
	char   *key  = __TIME__;
	uint8_t hash = 8; // length of __TIME__ string
	for (uint8_t i = 8; i > 0;)
		hash = T[hash ^ key[--i]];

	printf("version hash = %" PRIu8 "\n", hash);

#if GPI_ARCH_IS_BOARD(nRF_PCA10056)
	// get phy_node_id
	// if not set by programming toolchain on testbed
	if (0 == phy_node_id)
	{
		uint16_t data[2];

		// read from nRF UICR area
		gpi_nrf_uicr_read(&data, 0, sizeof(data));

		// check signature
		if (0x55AA == data[0])
		{
			GPI_TRACE_MSG(TRACE_INFO, "non-volatile config is valid");
			phy_node_id = data[1];
		}
		else
			GPI_TRACE_MSG(TRACE_INFO, "non-volatile config is invalid");

		// if signature is invalid
		while (0 == phy_node_id)
		{
			printf("Node ID not set. enter value: ");

			// read from console
			// scanf("%u", &phy_node_id);
			char s[8];
			phy_node_id = atoi(getsn(s, sizeof(s)));

			printf("\nNode ID set to %u\n", phy_node_id);

			// until input value is valid
			if (0 == phy_node_id) continue;

			// store new value in UICR area

			data[0] = 0x55AA;
			data[1] = phy_node_id;

			gpi_nrf_uicr_erase();
			gpi_nrf_uicr_write(0, &data, sizeof(data));

			// ATTENTION: Writing to UICR requires NVMC->CONFIG.WEN to be set which in turn
			// invalidates the instruction cache (permanently). Besides that, UICR updates take
			// effect only after reset (spec. 4413_417 v1.0 4.3.3 page 24). Therefore we do a soft
			// reset after the write procedure.
			printf("Restarting system...\n");
			gpi_milli_sleep(100); // safety margin (e.g. to empty UART Tx FIFO)
			NVIC_SystemReset();

			break;
		}
	}
#elif GPI_ARCH_IS_BOARD(nRF5_FLOCKLAB)
	phy_node_id = flocklab_node_id();
#endif

	printf("starting node %u ...\n", phy_node_id);

	// Stop RNG because we only need one random number as seed.
	NRF_RNG->TASKS_STOP = 1;
	uint8_t  rng_value  = BV_BY_VALUE(RNG_VALUE_VALUE, NRF_RNG->VALUE);
	uint32_t rng_seed   = rng_value * gpi_mulu_16x16(phy_node_id + 1, gpi_tick_fast_native());
	printf("random seed for Mixer is %" PRIu32 "\n", rng_seed);
	// init RNG with randomized seed
	mixer_rand_seed(rng_seed);

	// translate phy_node_id to logical node id used with mixer
	for (log_node_id = 0; log_node_id < NUM_ELEMENTS(nodes); ++log_node_id)
	{
		if (nodes[log_node_id] == phy_node_id) break;
	}
	if (log_node_id >= NUM_ELEMENTS(nodes))
	{
		printf("!!! PANIC: node mapping not found for node %u !!!\n", phy_node_id);
		while (1)
			;
	}
	printf("mapped physical node %u to logical id %u\n", phy_node_id, log_node_id);

	mixer_print_config();
	butler_print_config();
}



//**************************************************************************************************
//***** Global Functions ***************************************************************************

int main()
{
	// don't TRACE before gpi_platform_init()
	// GPI_TRACE_FUNCTION();

	Gpi_Hybrid_Tick deadline;
	unsigned int    i;
	reference_t     ref;

	initialization();

	// deadline for first round is now (-> start as soon as possible)
	deadline = gpi_tick_hybrid();

	// infinite execution loop
	for (round = 1; 1; round++)
	{
		printf("preparing round %" PRIu32 " ...\n", round);

		while (gpi_tick_compare_hybrid(gpi_tick_hybrid(), deadline) < 0)
			;

		// Execute Butler before Mixer to synchronize all nodes.
		// NOTE: Butler init must not run after mixer_init because it changes several settings
		// (e.g., radio, PPI, ...) which would have to be correctly restored otherwise.
		ref = butler_start(log_node_id);

		uint8_t data[7];

		// init Mixer
		mixer_init(log_node_id);
		mixer_set_weak_release_slot(WEAK_RELEASE_SLOT);
		mixer_set_weak_return_msg((void *)-1);

		// provide some test data messages
		{
			data[1] = log_node_id;
			data[2] = phy_node_id;
			data[3] = round;
			data[4] = round >> 8;
			data[5] = round >> 16;
			data[6] = round >> 24;

			for (i = 0; i < MX_GENERATION_SIZE; i++)
			{
				data[0] = i;

				if (payload_distribution[i] == phy_node_id)
					mixer_write(i, data, MIN(sizeof(data), MX_PAYLOAD_SIZE));
			}
		}

		unsigned int is_initiator = 0;

		// We bootstrap the network by running a "normal" Mixer round at the beginning (fixed
		// initiator and all other nodes do an infinite scan).
		if (round == 1)
		{
			mixer_arm((MX_INITIATOR_ID == phy_node_id) ? MX_ARM_INITIATOR : MX_ARM_INFINITE_SCAN);

			if (MX_INITIATOR_ID == phy_node_id) deadline += 3 * MX_SLOT_LENGTH;
		}
		// In all other rounds we either select a fixed or random subset of nodes as initiator.
		else
		{
#if TESTBED == TESTBED_FLOCKLAB
			// if (1 == phy_node_id | 31 == phy_node_id | 10 == phy_node_id | 29 == phy_node_id)
			// if (11 == phy_node_id | mixer_rand() < 16384) // 10%=6553, 25%=16384
			if (11 == phy_node_id | 23 == phy_node_id)
#else
			if (MX_INITIATOR_ID == phy_node_id)
#endif
			{
				is_initiator = 1;
			}

			// arm mixer
			if (is_initiator) { mixer_arm(MX_ARM_INITIATOR); }
			else { mixer_arm(0); }
		}


#if START_ROUND_VIA_GPIO
		// Wait for GPIO pin to start the round. We use this on FlockLab to control the initial time
		// offset between nodes on FlockLab.
		while ((NRF_P0->IN & BV(29)) == 0)
			;

		// Delay initiator(s) a little bit.
		// if (is_initiator)
		// {
		// 	gpi_micro_sleep(20);
		// }
		if (1 == phy_node_id) { gpi_micro_sleep(20); }
		else if (31 == phy_node_id) { gpi_micro_sleep(120); }
#else
		// TODO: This introduces some inaccuracy of a few instructions. Perhaps solve with timer?
		// Could also be used to simultaneously trigger tracing GPIO.
		// start when sync point reached
		while (gpi_tick_compare_fast_native(gpi_tick_fast_native(), ref.time) < 0)
			;
#endif

#if TESTBED == TESTBED_FLOCKLAB
		// Using GPI_LED_1 and FlockLabs GPIO tracing capabilities to measure Butler's accuracy.
		gpi_led_on(GPI_LED_1);
#endif

		// Start the execution of Mixer.
		deadline = mixer_start();

		while (gpi_tick_compare_hybrid(gpi_tick_hybrid(), deadline) < 0)
			;

#if TESTBED == TESTBED_FLOCKLAB
		gpi_led_off(GPI_LED_1);
#endif

		// Evaluate the Mixer round.
		msgs_decoded     = 0;
		msgs_not_decoded = 0;
		msgs_weak        = 0;
		msgs_wrong       = 0;

		for (i = 0; i < MX_GENERATION_SIZE; i++)
		{
			void *p = mixer_read(i);
			if (NULL == p) { msgs_not_decoded++; }
			else if ((void *)-1 == p) { msgs_weak++; }
			else
			{
				memcpy(data, p, sizeof(data));
				if ((data[0] == i) && (data[2] == payload_distribution[i])) { msgs_decoded++; }
				else { msgs_wrong++; }

				// use message 0 to check/adapt round number
				if ((0 == i) && (MX_PAYLOAD_SIZE >= 7))
				{
					Generic32 r;

					r.u8_ll = data[3];
					r.u8_lh = data[4];
					r.u8_hl = data[5];
					r.u8_hh = data[6];

					if (1 == round)
					{
						round = r.u32;
						printf("synchronized to round %" PRIu32 "\n", r.u32);
					}
					else if (r.u32 != round)
					{
						printf("round mismatch: received %" PRIu32 " <> local %" PRIu32
						       "! trying resync ...\n",
						       r.u32, round);
						round = 0; // increments to 1 with next round loop iteration
					}
				}
			}
		}

		// print statistics for the log
		print_results(log_node_id);

#if !START_ROUND_VIA_GPIO
		// set start time for next round
		deadline += GPI_TICK_MS_TO_HYBRID2(PRINT_TIME_MS);
#endif
	}

	GPI_TRACE_RETURN(0);
}

//**************************************************************************************************
//**************************************************************************************************
