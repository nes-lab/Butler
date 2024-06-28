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
 *	@file					gpi/msp430/tmote/radio.h
 *
 *	@brief					CC2420 radio interface
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

#ifndef __GPI_MSP430_TMOTE_RADIO_H__
#define __GPI_MSP430_TMOTE_RADIO_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "gpi/platform_spec.h"
#include GPI_PLATFORM_PATH(cc2420_const.h)

#include <msp430.h>

//**************************************************************************************************
//***** Global (Public) Defines and Consts *********************************************************



//**************************************************************************************************
//***** Local (Private) Defines and Consts *********************************************************



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

// low-level hardware interface functions
static inline void 		gpi_radio_cs_on();
static inline void		gpi_radio_cs_off();

// low-level strobe and register access functions
static inline uint8_t	gpi_radio_strobe(uint8_t s);
uint16_t 				gpi_radio_reg_get(uint8_t a);
void 					gpi_radio_reg_set(uint8_t a, uint16_t d);

// standard init functions
void 					gpi_radio_init();
unsigned int			gpi_radio_dbm_to_power_level(int dbm);
void 					gpi_radio_set_tx_power(unsigned int pa_level);
void 					gpi_radio_set_channel(unsigned int channel);

#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

static inline void __attribute__((always_inline)) gpi_radio_cs_on()		{ P4OUT &= ~BV(2);	}
static inline void __attribute__((always_inline)) gpi_radio_cs_off()	{ P4OUT |=  BV(2);	}

//**************************************************************************************************

static inline uint8_t gpi_radio_strobe(uint8_t s)
{
	gpi_radio_cs_on();

	U0TXBUF = s;
	while (!(U0TCTL & TXEPT));

	gpi_radio_cs_off();

	return U0RXBUF;

	// note: if return value is not needed very often, think about returning void
	// (and adding an extra get_status function) because U0RXBUF is volatile, i.e.,
	// the compiler cannot optimize (remove) unnecessary read accesses by itself
}

//**************************************************************************************************
//**************************************************************************************************

#endif // __GPI_MSP430_TMOTE_RADIO_H__
