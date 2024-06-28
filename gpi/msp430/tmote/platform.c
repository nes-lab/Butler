/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2018, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					gpi/msp430/tmote/platform.c
 *
 *	@brief					platform interface functions
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
/*
#include <gpi/trace.h>

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
// define groups appropriate for your needs, assign one bit per group
// values > GPI_TRACE_LOG_USER (i.e. upper 8 bits) are reserved
#define TRACE_GROUP1		0x00000001
#define TRACE_GROUP2		0x00000002

// select active message groups, i.e., the messages to be printed (others will be dropped)
GPI_TRACE_CONFIG(<TODO: module name>, TRACE_BASE_SELECTION |  GPI_TRACE_LOG_USER);
*/
//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/platform.h"
#include "gpi/interrupts.h"
#include "gpi/clocks.h"
#include "gpi/trace.h"

#include <msp430.h>

#include <stdio.h>		// putchar(), getchar()

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

// init (reset) CPU core to defined state
// this function can be moved to generic MSP430 code if helpful
static void core_init()
{
	gpi_int_disable();

	// disable watchdog
	WDTCTL = WDTPW | WDTHOLD;

	// disable all peripheral modules in global enable registers
	ME1 = 0;
	ME2 = 0;
	IE1 = 0;
	IE2 = 0;
}

//**************************************************************************************************

// init UART
// note: if function is inlined and baudrate is constant, it gets well optimized
static inline void uart1_init(uint32_t baudrate)
{
	// fractional part:			   .000  .125  .250  .375  .500  .625  .750  .875
	const uint8_t	MCTL_LUT[8] = {0x00, 0x08, 0x22, 0x4A, 0x55, 0xB5, 0xBB, 0x10};

	uint32_t		ticks_per_bit;

//	ASSERT_CT(baudrate <= GPI_FAST_CLOCK_RATE / 4, baudrate_to_high);
//	ASSERT_CT(baudrate >= GPI_SLOW_CLOCK_RATE / 0xFFFF, baudrate_to_low);

	// init UART1: 8 data bits, 1 stop bit, no parity
	U1CTL |= SWRST;
	U1CTL  = CHAR | SWRST;
	U1RCTL = 0;

	// ticks_per_bit = round(clock / baudrate) with 3 fractional digits
	// generate baudrate from ACLK if possible (-> precise XO, independent from DCO)
	// ATTENTION: driving baudrate generator from DCO requires appropriate DCO accuracy
	if (GPI_SLOW_CLOCK_RATE / baudrate < 3)
	{
		U1TCTL = SSEL1;
		ticks_per_bit = (((GPI_FAST_CLOCK_RATE << 4) / baudrate) + 1) >> 1;
    }
	else
	{
		U1TCTL = SSEL0;
		ticks_per_bit = (((GPI_SLOW_CLOCK_RATE << 4) / baudrate) + 1) >> 1;
    }

	// translate fractional part to modulation control value
	U1MCTL = MCTL_LUT[ticks_per_bit & 7];

	// divider = integer part
	ticks_per_bit >>= 3;
	U1BR0  = ticks_per_bit & 0xFF;
	U1BR1  = ticks_per_bit >> 8;

	// start UART
	ME2   |= UTXE1 | URXE1;
	U1CTL &= ~SWRST;
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

void gpi_platform_init()
{
	// GPI_TRACE_FUNCTION -> don't TRACE because HW not ready

	// we assume bare-metal programming
	// If there is an underlying OS, the init code needs to be adapted.
	ASSERT_CT(GPI_ARCH_IS_OS(NONE));

	// init (reset) CPU core to defined state
	core_init();


	// init I/O ports
	// notice: we do this before clock init because the latter takes some time. Our goal is to
	// configure all pins into a compatible state as fast as possible to avoid (or at least shorten)
	// any high-current situations.

	// P1:  HUM_PWR	| HUM_SCL | HUM_SDA | CCA 	| FIFO 	| P_DVCC | -/BSLTX	| FIFOP
	P1SEL = 0       | 0		  |	0       | 0		| 0		| 0      | 0	    | 0		;
	P1OUT = 0		| 0       | 0       | 0   	| 0    	| 0      | 0	    | 0		;
	P1DIR = BV(7)	| BV(6)   | 0       | 0   	| 0    	| 0      | 0	    | 0		;
	P1IE  = 0;
		// attention: P1.1 (BSLTX) is connected to UART1_TX -> avoid short-circuit with UTXD1

	// P2:	UserINT | GIO3	| - 	| 1Wire | GIO2	| -/BSLRX	| GIO1 	| GIO0
	P2SEL = 0       | 0		| 0		| 0		| 0		| 0		   	| 0		| 0		;
	P2OUT = 0	    | 0		| 0		| 0		| 0		| 0		   	| 0		| 0		;
	P2DIR = 0	    | 0		| BV(5)	| 0		| 0		| 0		   	| 0		| 0		;
	P2IE  = 0;
		// notice: GIOx is/might be connected to expansion header, therefore we configure them
		// as inputs for safety reasons. If power consumption is more important, they should be
		// driven to avoid floating inputs.

	// P4:	FLASH_HOLD |  RADIO_RESETn | RADIO_VREG_EN | FLASH_CSn | -		| RADIO_CSn	| RADIO_SFD | -
	P4SEL = 0		   | 0			   | 0			   | 0		   | 0		| 0 	   	| 0			| 0		;
	P4OUT = BV(7)	   | BV(6)		   | 0			   | BV(4)	   | 0		| BV(2)	   	| 0			| 0		;
	P4DIR = BV(7)	   | BV(6)		   | BV(5)		   | BV(4)	   | BV(3)	| BV(2)	   	| 0			| 0		;
		// attention: P4 is initialized before P3 to make sure that all SPI select signals are deactivated
		// attention: P4.0 might be short circuited with P4.1 due to debug wiring -> don't drive P4.0

	// P3:	UART1_RX | UART1_TX	| UART0_RX	| UART0_TX	| RADIO_CLK	| RAD_MISO	| RAD_MOSI	| -
	P3SEL = BV(7)	 | BV(6)	| 0 		| 0 		| BV(3)		| BV(2)	   	| BV(1)		| 0		;
	P3OUT = 0		 | BV(6)	| 0 		| BV(4)		| 0			| 0 	   	| 0			| 0		;
	P3DIR = 0		 | BV(6)	| 0 		| BV(4)		| BV(3)		| 0 	   	| BV(1)		| BV(0)	;
		// notice: UART0_RX/TX is connected to expansion header and therefore maybe open
		// notice: floating SPI signals (until SPI controller is initialized) are uncritical
		// as long as all slave select signals are inactive

	// P5:	SVSout 	| LED3 	| LED2 	| LED1 	| - 	| - 	| - 	| -
	P5SEL = 0		| 0		| 0		| 0		| 0		| 0		| 0		| 0		;
	P5OUT = 0	    | BV(6)	| BV(5)	| BV(4)	| 0		| 0		| 0		| 0		;
	P5DIR = BV(7)   | BV(6)	| BV(5)	| BV(4)	| BV(3)	| BV(2)	| BV(1)	| BV(0)	;

	// P6:	SVSin/ADC7 | DAC0/ADC6 | ADC5	| ADC4 	| ADC3 	| ADC2 	| ADC1 	| ADC0
	P6SEL = 0	       | 0		   | 0		| 0		| 0		| 0		| 0		| 0		;
	P6OUT = 0	       | 0		   | 0		| 0		| 0		| 0		| 0		| 0		;
	P6DIR = 0	       | 0		   | 0		| 0		| 0		| 0		| 0		| 0		;
		// notice: most pins are connected to expansion header, therefore we configure them
		// as inputs for safety reasons. If power consumption is more important, they should be
		// driven to avoid floating inputs.

	if (GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB))
	{
		// configure pins used for GPIO tracing
		// FlockLab signal name:	LED1  | LED2  | LED3  | INT2  | INT1
		P6DIR |= 					BV(7) | BV(6) | BV(2) | BV(1) | BV(0);
		// configure pins used for GPIO actuation
		// FlockLab signal name:	  SIG1  | SIG2
		// P2DIR &= 					~(BV(7) | BV(3));
	}


	// init clock system
	// notice: this should be one of the first steps in the init phase

	// basic clock module:
	// LFXT1: 32768 Hz from XO
	// XT2: off
	// DCO: RSEL = 4, DCO = 3, MOD = 0, Rosc off => rate < 1.2 MHz (max. incl. temperature and VCC)
	// ACLK = LFXT1CLK / 1, MCLK = DCOCLK / 1, SMCLK = DCOCLK / 1
	// ATTENTION: the CPU is running, so some valid configuration is already there. When we change
	// the settings, we have to take care that the CPU keeps running for sure after every single step.
	// Hence, the order of changes must be well designed.
	// note: status register bits SCG0, SCG1, OSCOFF and CPUOFF are also relevant. However, since
	// the CPU is running (i.e., we are in active mode) we assume that they are already initialized.
	BCSCTL2 &= ~DCOR;				// Rosc off
	BCSCTL1 &= ~(RSEL1 | RSEL0);	// set RSEL; important:
	BCSCTL1 |= RSEL2;				// first clear, then set -> don't speed up in between
	DCOCTL  = DCO1 | DCO0;			// set DCO and MOD -> DCO init done
	BCSCTL2 = 0;					// switch (S)MCLK to DCO -> now doesn't need XT1/2 for sure
	BCSCTL1 = XT2OFF | RSEL2;		// XT2 off, init LFXT1 and ACLK

	// Timer_A: ACLK / 1 (slow clock from XO), continuous mode
	TACTL  &= ~(MC1 | MC0);					// stop counting (slau049f p. 11-4 note)
	TACTL   = TASSEL0 | TACLR;				// configure
	TACCTL0 = 0;
	TACCTL1 = 0;
	TACCTL2 = 0;
	TACTL  |= MC1;							// start counting

	// Timer_B: SMCLK / 1 (DCOCLK), continuous mode
	TBCTL  &= ~(MC1 | MC0);					// stop counting (slau049f p. 12-4 note)
	TBCTL   = TBSSEL1 | TBCLR;				// configure
	TBCCTL0 = 0;
	TBCCTL1 = 0;
	TBCCTL2 = 0;
	TBCCTL3 = 0;
	TBCCTL4 = 0;
	TBCCTL5 = 0;
	TBCCTL6 = CM0 | CCIS0 | SCS | CAP;		// capture rising edge of ACLK
	TBCTL  |= MC1;							// start counting

	// give XO some time to stabilize (slau049f p. 4-5 note)
	// ATTENTION: ACLK and Timer_A are not reliable before
	// notice: fast clock runs with 400...1200 kHz at this point
	// notice: we could interleave other init task (UART, radio, ...) as long as they do not
	// rely on clock accuracy
	{
		ASSERT_CT(sizeof(Gpi_Fast_Tick_Extended) >= sizeof(uint32_t));
		Gpi_Fast_Tick_Extended t = gpi_tick_fast_extended();
		while (gpi_tick_fast_extended() - t < 250000ul);
    }

//DEBUGOUT("clock init done\n");


	// init UART1
	// note: GPI_FAST_CLOCK_RATE / 4 (= 1.048.576 bps) can be used for debugging on Tmote Sky
	// ATTENTION: if baudrate is derived from DCO (SMCLK), then DCO should be stable before using the UART
//	if (GPI_ARCH_IS_BOARD(TMOTE_PURE))
//		uart1_init(GPI_FAST_CLOCK_RATE / 4);
//	else
		uart1_init(115200);


	// do initial DCO synchronization
	// ATTENTION: use gpi_msp430_dco_resync() if next call to gpi_msp430_dco_update() takes time
	gpi_msp430_dco_init(GPI_SLOW_CLOCK_RATE);
//DEBUGOUT("DCO sync done\n");


	// init TRACE DSR
	#if ((GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE) && GPI_TRACE_USE_DSR)
		TACCTL1 = CM0 | CCIS1 | CAP | CCIE;
		TACCTL1 &= ~CCIFG;
	#endif

	// TRACE_RETURN_VOID
}

//**************************************************************************************************

// provide putchar() if necessary
#if GPI_ARCH_IS_OS(NONE)

#if GPI_ARCH_IS_BOARD(TMOTE_INDRIYA)
	int uart1_writeb(unsigned char c)
#else
	int putchar(int c)
#endif
{
#if XON_XOFF
	// ATTENTION: don't use this code without interrupt driven RX because we could lose XON otherwise
	if (IFG2 & URXIFG1)
	{
		// if XOFF received
		if (0x13 == getchar())
		{
			// wait for XON
			while (0x11 != getchar());
        }
    }
#endif

	// convert "\n" to "\r\n"
	if (c == '\n')
	{
		while (!(IFG2 & UTXIFG1));
		U1TXBUF = '\r';
	}

	while (!(IFG2 & UTXIFG1));
	U1TXBUF = c;

	return c;
}

#endif	// GPI_ARCH_IS_OS(NONE)

//**************************************************************************************************

// provide getchar() if necessary
#if GPI_ARCH_IS_OS(NONE)

int getchar()
{
	while (!(IFG2 & URXIFG1))
		; // gpi_msp430_dco_update();
		
	return U1RXBUF;
}

#endif	// GPI_ARCH_IS_OS(NONE)

//**************************************************************************************************
//**************************************************************************************************
