//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/clocks.h"

//**************************************************************************************************
//**** Testbed specific settings *******************************************************************

#define TESTBED_LOCAL    1
#define TESTBED_FLOCKLAB 2

#ifndef TESTBED // local testbed is default setting
	#define TESTBED TESTBED_LOCAL
#endif

// INITIAL_TOLERANCE_MS can be used to specify the expected maximum clock drift between nodes at
// the beginning of a Butler execution. Further information regarding this can be found in the
// paper in Section 6.3 in the paragraph on "Duration and overhead". Alternatively, as done here,
// INITIAL_TOLERANCE_MS can be set to 0 and BUTLER_DURATION to a conservative value.
#define INITIAL_TOLERANCE_MS    0
#define INITIAL_TOLERANCE_TICKS GPI_TICK_MS_TO_FAST(INITIAL_TOLERANCE_MS)

// FlockLab 2 testbed: https://gitlab.ethz.ch/tec/public/flocklab/wiki
#if TESTBED == TESTBED_FLOCKLAB
	#define NUM_NODES       23
	#define BUTLER_DURATION (100 + (INITIAL_TOLERANCE_TICKS / SYNC_SLOT_LEN)) // [slots]
	#define P_TX            (100 / NUM_NODES) // transmit probabilit in [%]
// local testbed (desk setup)
#else
	#define NUM_NODES       3
	#define BUTLER_DURATION (20 + (INITIAL_TOLERANCE_TICKS / SYNC_SLOT_LEN)) // [slots]
	#define P_TX            (100 / NUM_NODES) // transmit probabilit in [%]
#endif
