
#include "../msp430_common/platform.c"
#include "../msp430_common/olf.c"
#include "../msp430_common/trace.c"

#include "../msp430f16x/platform.c"
#include "../msp430f16x/olf.c"
#include "../msp430f16x/clocks.c"
#include "../msp430f16x/profile.c"

#include "platform.c"
#include "radio.c"

#if GPI_ARCH_IS_BOARD(TMOTE_INDRIYA)
	#include "indriya_putchar.c"
#endif
