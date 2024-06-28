// mixer configuration file
// Adapt the settings to the needs of your application.

#include "gpi/platform_spec.h" // GPI_ARCH_IS_...
#include "gpi/tools.h" // NUM_ELEMENTS()

// testbed-dependent settings
#define TESTBED_LOCAL    1
#define TESTBED_FLOCKLAB 2

#ifndef TESTBED
	#define TESTBED TESTBED_LOCAL
#endif

// testbed-dependent settings
#if TESTBED == TESTBED_FLOCKLAB

	#define UNIQUE_ID FLOCKLAB_NODE_ID
	#define PRINT_HEADER()
	#define GPIO_P1_1_ON()
	#define GPIO_P1_1_OFF()

	#define PRINT_TIME_MS   300 // 100
	#define MX_ROUND_LENGTH 500 // in #slots
	#define MX_PAYLOAD_SIZE 16
    // #define MX_SLOT_LENGTH		GPI_TICK_US_TO_HYBRID2(1300) // 23 msgs a 16 B
	#define MX_SLOT_LENGTH GPI_TICK_US_TO_HYBRID2(2000) // 23 msgs a 16 B

static const uint8_t nodes[]                = {1,  2,  3,  4,  6,  7,  8,  9,  10, 11, 12, 13,
                                               16, 19, 20, 21, 22, 23, 24, 26, 27, 29, 31};
static const uint8_t payload_distribution[] = {1,  2,  3,  4,  6,  7,  8,  9,  10, 11, 12, 13,
                                               16, 19, 20, 21, 22, 23, 24, 26, 27, 29, 31};

#elif TESTBED == TESTBED_LOCAL

    // tiny test on developer's desk
	#define UNIQUE_ID       TOS_NODE_ID
	#define PRINT_HEADER()  printf("# ID:%u ", phy_node_id)
	#define GPIO_P1_1_ON()  NRF_P1->OUTSET = BV(1)
	#define GPIO_P1_1_OFF() NRF_P1->OUTCLR = BV(1)

	#define DONGLE_NODE_ID  2 // only used by nRF52840-Dongle
	#define PRINT_TIME_MS   100
	#define MX_ROUND_LENGTH 300 // in #slots
	#define MX_PAYLOAD_SIZE 16
	#define MX_SLOT_LENGTH  GPI_TICK_US_TO_HYBRID2(1300) // 23 msgs a 16 B

static const uint8_t nodes[]                = {1, 2};
static const uint8_t payload_distribution[] = {1, 2, 1, 2, 1, 2};

#else

	#error "Unknown testbed"

#endif

// basic settings
#define MX_NUM_NODES       NUM_ELEMENTS(nodes)
#define MX_GENERATION_SIZE NUM_ELEMENTS(payload_distribution)
#define MX_INITIATOR_ID    payload_distribution[0]
#define WEAK_RELEASE_SLOT  1

// Possible values (Gpi_Radio_Mode):
//		IEEE_802_15_4	= 1
//		BLE_1M			= 2
//		BLE_2M			= 3
//		BLE_125k		= 4
//		BLE_500k		= 5
#define MX_PHY_MODE 1
// Values mentioned in the manual (nRF52840_PS_v1.1):
// +8dBm,  +7dBm,  +6dBm,  +5dBm,  +4dBm,  +3dBm, + 2dBm,
//  0dBm,  -4dBm,  -8dBm, -12dBm, -16dBm, -20dBm, -40dBm
#define MX_TX_PWR_DBM 8

#define MX_SMART_SHUTDOWN 1
// 0	no smart shutdown
// 1	no unfinished neighbor, without full-rank map(s)
// 2	no unfinished neighbor
// 3	all nodes full rank
// 4	all nodes full rank, all neighbors ACKed knowledge of this fact
// 5	all nodes full rank, all nodes ACKed knowledge of this fact
#define MX_SMART_SHUTDOWN_MODE 2

// turn verbose log messages on or off
#define MX_VERBOSE_STATISTICS 1
#define MX_VERBOSE_PACKETS    0
#define MX_VERBOSE_PROFILE    0

// special test configurations
#define START_ROUND_VIA_GPIO 0
