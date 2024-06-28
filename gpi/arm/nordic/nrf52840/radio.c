/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2019, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					gpi/arm/nordic/nrf52840/radio.c
 *
 *	@brief					nRF52840 radio interface
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

#include <gpi/trace.h>

// message groups for TRACE messages (used in GPI_TRACE_MSG() calls)
#define TRACE_ERROR			GPI_TRACE_MSG_TYPE_ERROR

// select active message groups, i.e., the messages to be printed (others will be dropped)
#ifndef GPI_TRACE_BASE_SELECTION
	#define GPI_TRACE_BASE_SELECTION	GPI_TRACE_LOG_STANDARD | GPI_TRACE_LOG_PROGRAM_FLOW
#endif
GPI_TRACE_CONFIG(gpi_radio, GPI_TRACE_BASE_SELECTION |  GPI_TRACE_LOG_USER);

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "radio.h"
#include "gpi/platform.h"	// BV_BY_NAME() ...
#include "gpi/olf.h"

#include <nrf.h>

#include <assert.h>
#include <stdint.h>

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

Gpi_Radio_Mode gpi_radio_get_mode()
{
	GPI_TRACE_FUNCTION();

	Gpi_Radio_Mode	mode;
	
	switch (NRF_RADIO->MODE & RADIO_MODE_MODE_Msk)
	{
		case BV_BY_NAME(RADIO_MODE_MODE, Ble_1Mbit):			mode = BLE_1M;			break;
		case BV_BY_NAME(RADIO_MODE_MODE, Ble_2Mbit):			mode = BLE_2M;			break;
		case BV_BY_NAME(RADIO_MODE_MODE, Ble_LR125Kbit):		mode = BLE_125k;		break;
		case BV_BY_NAME(RADIO_MODE_MODE, Ble_LR500Kbit):		mode = BLE_500k;		break;
		case BV_BY_NAME(RADIO_MODE_MODE, Ieee802154_250Kbit):	mode = IEEE_802_15_4;	break;
		default:												mode = INVALID;			break;
    }	
	
	GPI_TRACE_RETURN(mode);
}

//**************************************************************************************************

unsigned int gpi_radio_dbm_to_power_level(int dbm)
{
	GPI_TRACE_FUNCTION();

	// mark values that are explicitly mentioned in the datasheet (4413_417 v1.0 page 315)
	// background: The table states some values explicitly. It is unclear if these values
	// are just examples, or if the other values do not work correctly. To be safe, we use
	// only the explicitly mentioned values.
	const uint64_t VALID_LUT = UINT64_C(0x01FD111110000100);	// +15 ... -48

	int i;

	// limit range
	// NOTE: use values with VALID_LUT set to 1 -> get_lsb() below works for sure
	dbm = MIN(dbm, 8);
	dbm = MAX(dbm, -40);

	i = dbm + 48;

	// if requested value is invalid: use next smaller value 	
	if (!(VALID_LUT & (UINT64_C(1) << i)))
		i = gpi_get_msb_64(VALID_LUT & ~(UINT64_C(-1) << i));
	
	GPI_TRACE_RETURN(i - 48);
}

//*************************************************************************************************

void gpi_radio_set_tx_power(unsigned int pa_level)
{
	GPI_TRACE_FUNCTION();
	
	pa_level &= 0xff;

	NRF_RADIO->TXPOWER = pa_level;
	
	GPI_TRACE_RETURN();
}

//*************************************************************************************************

void gpi_radio_set_channel(unsigned int channel)
{
	GPI_TRACE_FUNCTION();

	switch (gpi_radio_get_mode())
	{
		case IEEE_802_15_4:
		{
			assert(channel >= 11);
			assert(channel <= 26);

			NRF_RADIO->FREQUENCY =
				BV_BY_VALUE(RADIO_FREQUENCY_FREQUENCY, (channel - 10) * 5)	|
				BV_BY_NAME(RADIO_FREQUENCY_MAP, Default);
			
			break;
        }

		case BLE_1M:
		case BLE_2M:
		case BLE_125k:
		case BLE_500k:
		{
			assert(channel <= 39);
			
			unsigned int freq;

			// mapping of channel: see Bluetooth Core Spec. v5.1 Vol. 6 Part B section 1.4.1
			switch (channel)
			{
				case 0 ... 10:	freq = 4 + ((channel - 0) * 2);		break;
				case 11 ... 36:	freq = 28 + ((channel - 11) * 2);	break;
				case 37:		freq = 2;							break;
				case 38:		freq = 26;							break;
				default:		freq = 80;							break;
            }

			NRF_RADIO->FREQUENCY =
				BV_BY_VALUE(RADIO_FREQUENCY_FREQUENCY, freq) |
				BV_BY_NAME(RADIO_FREQUENCY_MAP, Default);

			// whitening LFSR init value is derived from channel index
			// (see Bluetooth Core Spec. v5.1 Vol. 6 Part B section 3.2 for details)			
			NRF_RADIO->DATAWHITEIV = channel;
			
			break;
        }

		default:
			assert(0);
    }
	
	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void gpi_radio_ble_set_access_address(unsigned int address)
{
	GPI_TRACE_FUNCTION();

	NRF_RADIO->BASE0 = address << 8;
	NRF_RADIO->PREFIX0 = address >> 24;
			
	GPI_TRACE_RETURN();
}

//**************************************************************************************************

void gpi_radio_init(Gpi_Radio_Mode mode)
{
	GPI_TRACE_FUNCTION();

	// reset peripheral
	// -> all registers have reset value hereafter
	NRF_RADIO->POWER = BV_BY_NAME(RADIO_POWER_POWER, Disabled);
	gpi_milli_sleep(1);
	NRF_RADIO->POWER = BV_BY_NAME(RADIO_POWER_POWER, Enabled);
	gpi_milli_sleep(1);

	NRF_RADIO->SHORTS = 0;
	NRF_RADIO->INTENCLR = -1u;

	// init radio wrt. operating mode
	switch (mode)
	{
		case IEEE_802_15_4:
		{
			NRF_RADIO->MODE = BV_BY_NAME(RADIO_MODE_MODE, Ieee802154_250Kbit);
			
			// The nRF52840 Product Specification (4413_417 v1.0) is vague regarding the required
			// settings of PCNF0 and PCNF1 in IEEE 802.15.4 mode. The correct settings can be
			// validated by looking into nRF IEEE 802.15.4 radio driver, which can be found at
			// https://github.com/NordicSemiconductor/nRF-IEEE-802.15.4-radio-driver
			// Look into their nrf_radio_init() function and compare the settings.
			
			NRF_RADIO->PCNF0 =
				BV_BY_VALUE(RADIO_PCNF0_LFLEN, 8) |
				BV_BY_NAME(RADIO_PCNF0_PLEN, 32bitZero) |
				BV_BY_NAME(RADIO_PCNF0_CRCINC, Include);
				
			NRF_RADIO->PCNF1 =
				BV_BY_VALUE(RADIO_PCNF1_MAXLEN, 127);
			
			NRF_RADIO->CRCCNF =
				BV_BY_NAME(RADIO_CRCCNF_LEN, Two) |
				BV_BY_NAME(RADIO_CRCCNF_SKIPADDR, Ieee802154);
			NRF_RADIO->CRCPOLY = 0x011021;
			NRF_RADIO->CRCINIT = 0;
	
			NRF_RADIO->MODECNF0 =
				BV_BY_NAME(RADIO_MODECNF0_RU, Fast)	|
					// default = 130us, fast = 40us (4413_417 v1.0 "6.20.15.8 Radio timing" page 329)
				BV_BY_NAME(RADIO_MODECNF0_DTX, Center);

			// following settings are established implicitly as default values after reset
			// NRF_RADIO->SFD = 0xA7;
			// NRF_RADIO->EDCNT = 0;
			// NRF_RADIO->CCACTRL = 0;
			// NRF_RADIO->TIFS = 0;		// don't care as long as shortcuts are not used (4413_417 v1.0 page 294)
			// NRF_RADIO->DACNF = 0;	// not used in 802.15.4 mode
			
			// we assume that the following settings have no impact in 802.15.4 mode:
			// NRF_RADIO->BASE0/1
			// NRF_RADIO->PREFIX0/1
			// NRF_RADIO->TXADDRESS
			// NRF_RADIO->RXADDRESS
			// NRF_RADIO->DATAWHITEIV
			// NRF_RADIO->MHRMATCH...
			
			gpi_radio_set_channel(11);		// just as reset (default) value
			gpi_radio_set_tx_power(0);		// just as reset (default) value
			
			break;
        }
		
		case BLE_1M:
		case BLE_2M:
		case BLE_125k:
		case BLE_500k:
		{
			switch (mode)
			{
				case BLE_1M:
					NRF_RADIO->MODE = BV_BY_NAME(RADIO_MODE_MODE, Ble_1Mbit);
					NRF_RADIO->PCNF0 = BV_BY_NAME(RADIO_PCNF0_PLEN, 8bit);
					// ATTENTION: CILEN and TERMLEN are not don't care, they must be set to 0
					// (we observed troubles with other values, at least on chip rev. 1)
					break;
				
				case BLE_2M:
					NRF_RADIO->MODE = BV_BY_NAME(RADIO_MODE_MODE, Ble_2Mbit);
					NRF_RADIO->PCNF0 = BV_BY_NAME(RADIO_PCNF0_PLEN, 16bit);
					// ATTENTION: CILEN and TERMLEN are not don't care, they must be set to 0
					// (we observed troubles with other values, at least on chip rev. 1)
					break;
				
				case BLE_125k:
					NRF_RADIO->MODE = BV_BY_NAME(RADIO_MODE_MODE, Ble_LR125Kbit);
					NRF_RADIO->PCNF0 =
						BV_BY_NAME(RADIO_PCNF0_PLEN, LongRange) |
						BV_BY_VALUE(RADIO_PCNF0_CILEN, 2) |
						BV_BY_VALUE(RADIO_PCNF0_TERMLEN, 3);
					break;
				
				case BLE_500k:
					NRF_RADIO->MODE = BV_BY_NAME(RADIO_MODE_MODE, Ble_LR500Kbit);
					NRF_RADIO->PCNF0 =
						BV_BY_NAME(RADIO_PCNF0_PLEN, LongRange) |
						BV_BY_VALUE(RADIO_PCNF0_CILEN, 2) |
						BV_BY_VALUE(RADIO_PCNF0_TERMLEN, 3);
					break;

				default:
					assert(0);				
            }		
			
			NRF_RADIO->PCNF0 |=
				BV_BY_VALUE(RADIO_PCNF0_LFLEN, 8) |
				BV_BY_VALUE(RADIO_PCNF0_S0LEN, 1) |
				BV_BY_VALUE(RADIO_PCNF0_S1LEN, 0) |
				BV_BY_NAME(RADIO_PCNF0_S1INCL, Automatic) |
				BV_BY_NAME(RADIO_PCNF0_CRCINC, Exclude);
				
			NRF_RADIO->PCNF1 =
				BV_BY_VALUE(RADIO_PCNF1_MAXLEN, 255) |
				BV_BY_VALUE(RADIO_PCNF1_STATLEN, 0) |
				BV_BY_VALUE(RADIO_PCNF1_BALEN, 3) |
				BV_BY_NAME(RADIO_PCNF1_ENDIAN, Little) |
				BV_BY_NAME(RADIO_PCNF1_WHITEEN, Enabled);
			
			NRF_RADIO->CRCCNF =
				BV_BY_NAME(RADIO_CRCCNF_LEN, Three) |
				BV_BY_NAME(RADIO_CRCCNF_SKIPADDR, Skip);
			NRF_RADIO->CRCPOLY = 0x00065B;
			NRF_RADIO->CRCINIT = 0x555555;

			NRF_RADIO->MODECNF0 =
				BV_BY_NAME(RADIO_MODECNF0_RU, Fast)	|
				BV_BY_NAME(RADIO_MODECNF0_DTX, Center);

			// by default use logical address 0 for Rx and Tx
			NRF_RADIO->TXADDRESS = 0;
			NRF_RADIO->RXADDRESSES = 1;

			// following settings are established implicitly as default values after reset
			// NRF_RADIO->TIFS = 0;		// don't care as long as shortcuts are not used (4413_417 v1.0 page 294)
			// NRF_RADIO->DACNF = 0;
			// NRF_RADIO->DAB[n]/DAP[n] = 0;	// don't care while DACNF = 0
			// NRF_RADIO->MHRMATCH...			// precise meaning of these settings is unclear

			gpi_radio_set_channel(37);						// just as reset (default) value
			gpi_radio_set_tx_power(0);						// just as reset (default) value
			gpi_radio_ble_set_access_address(0x8E89BED6);	// just as reset (default) value

			break;
        }
		
		default:
			GPI_TRACE_MSG(TRACE_ERROR, "ERROR: invalid radio mode %u", (int)mode);
			assert(0);
    }
	
	GPI_TRACE_RETURN();
}

//**************************************************************************************************
//**************************************************************************************************
