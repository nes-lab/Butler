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
 *	@file					gpi/msp430/tmote/platform.h
 *
 *	@brief					platform interface functions, specific for tmote
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

#ifndef __GPI_MSP430_TMOTE_PLATFORM_H__
#define __GPI_MSP430_TMOTE_PLATFORM_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "gpi/platform_spec.h"
#include GPI_PLATFORM_PATH(../msp430f16x/platform.h)

#include "gpi/tools.h"

#include <msp430.h>

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************

#define GPI_LED_NONE	0
#define GPI_LED_1		(gpi_msp430_tmote_led_mask[1])
#define GPI_LED_2		(gpi_msp430_tmote_led_mask[2])
#define GPI_LED_3		(gpi_msp430_tmote_led_mask[3])
#define GPI_LED_4		(gpi_msp430_tmote_led_mask[4])
#define GPI_LED_5		(gpi_msp430_tmote_led_mask[5])

//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************

#if GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB)

	#define GPI_MSP430_TMOTE_LED_PORT		P6OUT
	//static volatile uint8_t __attribute__((weakref,alias("__P6OUT")))	LEDport;

	static const uint8_t gpi_msp430_tmote_led_mask[] = {0, BV(7), BV(6), BV(2), BV(0), BV(1), 0, 0};

#else

	#define GPI_MSP430_TMOTE_LED_PORT		P5OUT
	//static volatile uint8_t __attribute__((weakref,alias("__P5OUT")))	LEDport;

	static const uint8_t gpi_msp430_tmote_led_mask[] = {0, BV(4), BV(5), BV(6), 0, 0, 0, 0};

#endif

//**************************************************************************************************
//***** Forward Class and Struct Declarations ******************************************************



//**************************************************************************************************
//***** Global Typedefs and Class Declarations *****************************************************



//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Prototypes of Global Functions *************************************************************

#ifdef __cplusplus
	extern "C" {
#endif



#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

static inline void __attribute__((always_inline)) gpi_led_on(int mask)
{
	if (mask)
#if GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB)
		GPI_MSP430_TMOTE_LED_PORT |= mask;
#else
		GPI_MSP430_TMOTE_LED_PORT &= ~mask;
#endif
}

static inline void __attribute__((always_inline)) gpi_led_off(int mask)
{
	if (mask)
#if GPI_ARCH_IS_BOARD(TMOTE_FLOCKLAB)
		GPI_MSP430_TMOTE_LED_PORT &= ~mask;
#else
		GPI_MSP430_TMOTE_LED_PORT |= mask;
#endif
}

static inline void __attribute__((always_inline)) gpi_led_toggle(int mask)
{
	if (mask)
		GPI_MSP430_TMOTE_LED_PORT ^= mask;
}

//**************************************************************************************************

static inline void gpi_sleep()
{
	// enter LPM0
	// note: reenables interrupts (used as wake-up events)
	// note: entering deeper sleep modes would need more effort (e.g. wait
	// until UART Tx has finished, resync DCO and/or hybrid clock, ...)
	__bis_status_register(GIE | CPUOFF);
}

//**************************************************************************************************

#if GPI_ARCH_IS_OS(NONE)
	static inline void gpi_stdin_flush()	{ U1RXBUF;	}
#endif

//**************************************************************************************************
//**************************************************************************************************

#endif // __GPI_MSP430_TMOTE_PLATFORM_H__
