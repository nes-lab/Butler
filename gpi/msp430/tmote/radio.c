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
 *	@file					gpi/msp430/tmote/radio.c
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

#include "gpi/platform_spec.h"
#include GPI_PLATFORM_PATH(radio.h)

#include <assert.h>

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



//**************************************************************************************************
//***** Global Functions ***************************************************************************

uint16_t gpi_radio_reg_get(uint8_t a)
{
	uint16_t	r;

	U0RXBUF;

	gpi_radio_cs_on();

	U0TXBUF = 0x40 | a;
	while (!(IFG1 & UTXIFG0));
	U0TXBUF = 0;
	while (!(U0RCTL & OE));

	r = U0RXBUF;
	U0TXBUF = 0;
	r <<= 8;
	while (!(IFG1 & URXIFG0));

	gpi_radio_cs_off();

	r |= U0RXBUF;

	return r;
}

//*************************************************************************************************

void gpi_radio_reg_set(uint8_t a, uint16_t d)
{
	gpi_radio_cs_on();

	U0TXBUF = a;
	while (!(IFG1 & UTXIFG0));

	U0TXBUF = d >> 8;
	while (!(IFG1 & UTXIFG0));

	U0TXBUF = d & 0xFF;
	while (!(U0TCTL & TXEPT));

	gpi_radio_cs_off();
}

//*************************************************************************************************

unsigned int gpi_radio_dbm_to_power_level(int dbm)
{
	// see CC2420 datasheet (SWRS041c) p.51 table 9 (output power programming)
	// The table states 8 values explicitly (31, 27, 23, 19, 15, 11, 7, 3). It is unclear if these
	// values are just examples, or if the other values do not work correctly. To be safe, we use
	// only the explicitly stated values.
	const uint8_t PA_LEVEL_LUT[] =
	{
		//	-0  -1  -2  -3  -4  -5  -6  -7  -8  -9 -10 -11 -12 -13 -14 -15 -16 -17 -18 -19 -20 -21 -22 -23 -24 -25
			31, 27, 23, 23, 19, 19, 15, 15, 15, 11, 11, 11, 11,  7,  7,  7,  7,  7,  7,  7,  3,  3,  3,  3,  3,  3
		//	31, 27, 25, 23, 21, 19, 17, 15, 13, 12, 11, 10,  9,  8,  7,  7,  6,  6,  5,  5,  4,  4,  4,  3,  3,  3
	};

	dbm = -dbm;

	if (dbm < 0)
		dbm = 0;
	else if (dbm >= NUM_ELEMENTS(PA_LEVEL_LUT))
		dbm = NUM_ELEMENTS(PA_LEVEL_LUT) - 1;

	return PA_LEVEL_LUT[dbm];
}

//*************************************************************************************************

void gpi_radio_set_tx_power(unsigned int pa_level)
{
	uint16_t reg;

	if (pa_level > 31)
		pa_level = 31;

	reg = gpi_radio_reg_get(CC2420_TXCTRL);
	reg = (reg & ~0x1F) | pa_level;
	gpi_radio_reg_set(CC2420_TXCTRL, reg);
}

//*************************************************************************************************

void gpi_radio_set_channel(unsigned int channel)
{
	assert(channel >= 11);
	assert(channel <= 26);

	uint16_t reg;

	// wait for running transmissions to end
	while (gpi_radio_strobe(CC2420_SNOP) & BV(CC2420_TX_ACTIVE));

	// update FREQ
	reg = gpi_radio_reg_get(CC2420_FSCTRL);
	reg &= ~0x02FF;
	reg |= (2405 - 2048 - 55) + 5 * channel;
	gpi_radio_reg_set(CC2420_FSCTRL, reg);
}

//**************************************************************************************************

void gpi_radio_init()
{
	// init hardware interface between MCU and CC2420

	// init UART0 in SPI mode
	U0CTL |= SWRST;
	U0CTL  = CHAR | SYNC | MM | SWRST;
	U0TCTL = CKPH | SSEL1 | STC;
	U0RCTL = 0;
	U0BR0  = 2;
	U0BR1  = 0;
	U0MCTL = 0;
	ME1   |= USPIE0;
	U0CTL &= ~SWRST;

	// use TB1 to capture SFD edges
	TBCCTL1 = CM0 | SCS | CAP;
	P4SEL |= BV(1);


	// init CC2420

	// turn voltage regulator on
	P4OUT |= BV(5);
	gpi_milli_sleep(1);		// SWRS041c p. 13

	// generate reset pulse
	// -> all registers have reset value hereafter
	P4OUT &= ~BV(6);
	gpi_milli_sleep(1);
	P4OUT |= BV(6);

	// start crystal oscillator
	gpi_radio_strobe(CC2420_SXOSCON);

	// turn off address decoding
	gpi_radio_reg_set(CC2420_MDMCTRL0, 0x01E2);

	// change default values as recomended in the datasheet:
  	// correlation threshold = 20
	gpi_radio_reg_set(CC2420_MDMCTRL1, 20 << 6);
  	// RX bandpass filter ref. bias current = 3uA
	gpi_radio_reg_set(CC2420_RXCTRL1, gpi_radio_reg_get(CC2420_RXCTRL1) | 0x2000);

	// disable security features
	gpi_radio_reg_set(CC2420_SECCTRL0, gpi_radio_reg_get(CC2420_SECCTRL0) & ~0x0203);

	// set FIFOP threshold to maximum
	gpi_radio_reg_set(CC2420_IOCFG0, 0x007F);

	// wait until crystal oscillator has stabilized
	while (!(gpi_radio_strobe(CC2420_SNOP) & BV(CC2420_XOSC16M_STABLE)));
}

//**************************************************************************************************
//**************************************************************************************************
