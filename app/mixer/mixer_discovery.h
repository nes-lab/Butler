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
 *	@file					mixer_discovery.h
 *
 *	@brief					constant data used in Mixer's discovery phase
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

#ifndef __MIXER_DISCOVERY_H__
#define __MIXER_DISCOVERY_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************

#include "mixer_internal.h"

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

// NOTE: All constants are declared in a way that allows MX_NUM_NODES to include terms that are
// evaluated by the compiler, i.e., terms that are not interpretable by the preprocessor (typical
// examples are NUM_ELEMENTS(...), sizeof(...)). This prohibits constructs like
//	#if (MX_NUM_NODES == 1)
//		...
//	#elif (MX_NUM_NODES == 2)
//		...
// because they would not work.
// NOTE: The constants defined here look expensive, but all of them get resolved at compile time
// due to constant propagation and linker garbage collection, even at low optimization levels.

static const uint16_t MX_DISCOVERY_PTX = (
	(!!(MX_NUM_NODES ==   2) * 32768) +
	(!!(MX_NUM_NODES ==   3) * 23509) +
	(!!(MX_NUM_NODES ==   4) * 18396) +
	(!!(MX_NUM_NODES ==   5) * 15446) +
	(!!(MX_NUM_NODES ==   6) * 13587) +
	(!!(MX_NUM_NODES ==   7) * 12349) +
	(!!(MX_NUM_NODES ==   8) * 11495) +
	(!!(MX_NUM_NODES ==   9) * 10888) +
	(!!(MX_NUM_NODES ==  10) * 10444) +
	(!!(MX_NUM_NODES ==  11) * 10444) +
	(!!(MX_NUM_NODES ==  12) * 10444) +
	(!!(MX_NUM_NODES ==  13) * 10444) +
	(!!(MX_NUM_NODES ==  14) * 10444) +
	(!!(MX_NUM_NODES ==  15) * 10444) +
	(!!(MX_NUM_NODES ==  16) * 10444) +
	(!!(MX_NUM_NODES ==  17) * 10444) +
	(!!(MX_NUM_NODES ==  18) * 10444) +
	(!!(MX_NUM_NODES ==  19) * 10444) +
	(!!(MX_NUM_NODES ==  20) * 10444) +
	(!!(MX_NUM_NODES ==  21) * 10444) +
	(!!(MX_NUM_NODES ==  22) * 10444) +
	(!!(MX_NUM_NODES ==  23) * 10444) +
	(!!(MX_NUM_NODES ==  24) * 10444) +
	(!!(MX_NUM_NODES ==  25) * 10444) +
	(!!(MX_NUM_NODES ==  26) * 10359) +
	(!!(MX_NUM_NODES ==  27) * 10280) +
	(!!(MX_NUM_NODES ==  28) * 10204) +
	(!!(MX_NUM_NODES ==  29) * 9998) +
	(!!(MX_NUM_NODES ==  30) * 9929) +
	(!!(MX_NUM_NODES ==  31) * 9862) +
	(!!(MX_NUM_NODES ==  32) * 9799) +
	(!!(MX_NUM_NODES ==  33) * 9656) +
	(!!(MX_NUM_NODES ==  34) * 9597) +
	(!!(MX_NUM_NODES ==  35) * 9541) +
	(!!(MX_NUM_NODES ==  36) * 9487) +
	(!!(MX_NUM_NODES ==  37) * 9385) +
	(!!(MX_NUM_NODES ==  38) * 9335) +
	(!!(MX_NUM_NODES ==  39) * 9286) +
	(!!(MX_NUM_NODES ==  40) * 9240) +
	(!!(MX_NUM_NODES ==  41) * 9194) +
	(!!(MX_NUM_NODES ==  42) * 9120) +
	(!!(MX_NUM_NODES ==  43) * 9078) +
	(!!(MX_NUM_NODES ==  44) * 9037) +
	(!!(MX_NUM_NODES ==  45) * 8997) +
	(!!(MX_NUM_NODES ==  46) * 8941) +
	(!!(MX_NUM_NODES ==  47) * 8903) +
	(!!(MX_NUM_NODES ==  48) * 8867) +
	(!!(MX_NUM_NODES ==  49) * 8832) +
	(!!(MX_NUM_NODES ==  50) * 8797) +
	(!!(MX_NUM_NODES ==  51) * 8753) +
	(!!(MX_NUM_NODES ==  52) * 8721) +
	(!!(MX_NUM_NODES ==  53) * 8689) +
	(!!(MX_NUM_NODES ==  54) * 8658) +
	(!!(MX_NUM_NODES ==  55) * 8622) +
	(!!(MX_NUM_NODES ==  56) * 8593) +
	(!!(MX_NUM_NODES ==  57) * 8564) +
	(!!(MX_NUM_NODES ==  58) * 8536) +
	(!!(MX_NUM_NODES ==  59) * 8509) +
	(!!(MX_NUM_NODES ==  60) * 8478) +
	(!!(MX_NUM_NODES ==  61) * 8452) +
	(!!(MX_NUM_NODES ==  62) * 8427) +
	(!!(MX_NUM_NODES ==  63) * 8402) +
	(!!(MX_NUM_NODES ==  64) * 8377) +
	(!!(MX_NUM_NODES ==  65) * 8351) +
	(!!(MX_NUM_NODES ==  66) * 8328) +
	(!!(MX_NUM_NODES ==  67) * 8305) +
	(!!(MX_NUM_NODES ==  68) * 8283) +
	(!!(MX_NUM_NODES ==  69) * 8261) +
	(!!(MX_NUM_NODES ==  70) * 8238) +
	(!!(MX_NUM_NODES ==  71) * 8217) +
	(!!(MX_NUM_NODES ==  72) * 8196) +
	(!!(MX_NUM_NODES ==  73) * 8175) +
	(!!(MX_NUM_NODES ==  74) * 8155) +
	(!!(MX_NUM_NODES ==  75) * 8135) +
	(!!(MX_NUM_NODES ==  76) * 8116) +
	(!!(MX_NUM_NODES ==  77) * 8097) +
	(!!(MX_NUM_NODES ==  78) * 8078) +
	(!!(MX_NUM_NODES ==  79) * 8060) +
	(!!(MX_NUM_NODES ==  80) * 8042) +
	(!!(MX_NUM_NODES ==  81) * 8024) +
	(!!(MX_NUM_NODES ==  82) * 8006) +
	(!!(MX_NUM_NODES ==  83) * 7989) +
	(!!(MX_NUM_NODES ==  84) * 7972) +
	(!!(MX_NUM_NODES ==  85) * 7956) +
	(!!(MX_NUM_NODES ==  86) * 7939) +
	(!!(MX_NUM_NODES ==  87) * 7923) +
	(!!(MX_NUM_NODES ==  88) * 7907) +
	(!!(MX_NUM_NODES ==  89) * 7892) +
	(!!(MX_NUM_NODES ==  90) * 7877) +
	(!!(MX_NUM_NODES ==  91) * 7861) +
	(!!(MX_NUM_NODES ==  92) * 7846) +
	(!!(MX_NUM_NODES ==  93) * 7832) +
	(!!(MX_NUM_NODES ==  94) * 7817) +
	(!!(MX_NUM_NODES ==  95) * 7803) +
	(!!(MX_NUM_NODES ==  96) * 7789) +
	(!!(MX_NUM_NODES ==  97) * 7775) +
	(!!(MX_NUM_NODES ==  98) * 7762) +
	(!!(MX_NUM_NODES ==  99) * 7748) +
	(!!(MX_NUM_NODES == 100) * 7735) +
	(!!(MX_NUM_NODES == 101) * 7722) +
	(!!(MX_NUM_NODES == 102) * 7709) +
	(!!(MX_NUM_NODES == 103) * 7696) +
	(!!(MX_NUM_NODES == 104) * 7683) +
	(!!(MX_NUM_NODES == 105) * 7671) +
	(!!(MX_NUM_NODES == 106) * 7659) +
	(!!(MX_NUM_NODES == 107) * 7646) +
	(!!(MX_NUM_NODES == 108) * 7634) +
	(!!(MX_NUM_NODES == 109) * 7623) +
	(!!(MX_NUM_NODES == 110) * 7611) +
	(!!(MX_NUM_NODES == 111) * 7599) +
	(!!(MX_NUM_NODES == 112) * 7588) +
	(!!(MX_NUM_NODES == 113) * 7577) +
	(!!(MX_NUM_NODES == 114) * 7566) +
	(!!(MX_NUM_NODES == 115) * 7555) +
	(!!(MX_NUM_NODES == 116) * 7544) +
	(!!(MX_NUM_NODES == 117) * 7533) +
	(!!(MX_NUM_NODES == 118) * 7522) +
	(!!(MX_NUM_NODES == 119) * 7512) +
	(!!(MX_NUM_NODES == 120) * 7501) +
	(!!(MX_NUM_NODES == 121) * 7491) +
	(!!(MX_NUM_NODES == 122) * 7481) +
	(!!(MX_NUM_NODES == 123) * 7471) +
	(!!(MX_NUM_NODES == 124) * 7461) +
	(!!(MX_NUM_NODES == 125) * 7451) +
	(!!(MX_NUM_NODES == 126) * 7441) +
	(!!(MX_NUM_NODES == 127) * 7432) +
	(!!(MX_NUM_NODES == 128) * 7422) +
	(!!(MX_NUM_NODES == 129) * 7413) +
	(!!(MX_NUM_NODES == 130) * 7403) +
	(!!(MX_NUM_NODES == 131) * 7394) +
	(!!(MX_NUM_NODES == 132) * 7385) +
	(!!(MX_NUM_NODES == 133) * 7376) +
	(!!(MX_NUM_NODES == 134) * 7367) +
	(!!(MX_NUM_NODES == 135) * 7358) +
	(!!(MX_NUM_NODES == 136) * 7349) +
	(!!(MX_NUM_NODES == 137) * 7340) +
	(!!(MX_NUM_NODES == 138) * 7332) +
	(!!(MX_NUM_NODES == 139) * 7323) +
	(!!(MX_NUM_NODES == 140) * 7315) +
	(!!(MX_NUM_NODES == 141) * 7306) +
	(!!(MX_NUM_NODES == 142) * 7298) +
	(!!(MX_NUM_NODES == 143) * 7290) +
	(!!(MX_NUM_NODES == 144) * 7282) +
	(!!(MX_NUM_NODES == 145) * 7274) +
	(!!(MX_NUM_NODES == 146) * 7266) +
	(!!(MX_NUM_NODES == 147) * 7258) +
	(!!(MX_NUM_NODES == 148) * 7250) +
	(!!(MX_NUM_NODES == 149) * 7242) +
	(!!(MX_NUM_NODES == 150) * 7234) +
	(!!(MX_NUM_NODES == 151) * 7227) +
	(!!(MX_NUM_NODES == 152) * 7219) +
	(!!(MX_NUM_NODES == 153) * 7211) +
	(!!(MX_NUM_NODES == 154) * 7204) +
	(!!(MX_NUM_NODES == 155) * 7197) +
	(!!(MX_NUM_NODES == 156) * 7189) +
	(!!(MX_NUM_NODES == 157) * 7182) +
	(!!(MX_NUM_NODES == 158) * 7175) +
	(!!(MX_NUM_NODES == 159) * 7168) +
	(!!(MX_NUM_NODES == 160) * 7160) +
	(!!(MX_NUM_NODES == 161) * 7153) +
	(!!(MX_NUM_NODES == 162) * 7146) +
	(!!(MX_NUM_NODES == 163) * 7140) +
	(!!(MX_NUM_NODES == 164) * 7133) +
	(!!(MX_NUM_NODES == 165) * 7126) +
	(!!(MX_NUM_NODES == 166) * 7119) +
	(!!(MX_NUM_NODES == 167) * 7112) +
	(!!(MX_NUM_NODES == 168) * 7106) +
	(!!(MX_NUM_NODES == 169) * 7099) +
	(!!(MX_NUM_NODES == 170) * 7093) +
	(!!(MX_NUM_NODES == 171) * 7086) +
	(!!(MX_NUM_NODES == 172) * 7080) +
	(!!(MX_NUM_NODES == 173) * 7073) +
	(!!(MX_NUM_NODES == 174) * 7067) +
	(!!(MX_NUM_NODES == 175) * 7061) +
	(!!(MX_NUM_NODES == 176) * 7054) +
	(!!(MX_NUM_NODES == 177) * 7048) +
	(!!(MX_NUM_NODES == 178) * 7042) +
	(!!(MX_NUM_NODES == 179) * 7036) +
	(!!(MX_NUM_NODES == 180) * 7030) +
	(!!(MX_NUM_NODES == 181) * 7024) +
	(!!(MX_NUM_NODES == 182) * 7018) +
	(!!(MX_NUM_NODES == 183) * 7012) +
	(!!(MX_NUM_NODES == 184) * 7006) +
	(!!(MX_NUM_NODES == 185) * 7000) +
	(!!(MX_NUM_NODES == 186) * 6994) +
	(!!(MX_NUM_NODES == 187) * 6988) +
	(!!(MX_NUM_NODES == 188) * 6983) +
	(!!(MX_NUM_NODES == 189) * 6977) +
	(!!(MX_NUM_NODES == 190) * 6971) +
	(!!(MX_NUM_NODES == 191) * 6966) +
	(!!(MX_NUM_NODES == 192) * 6960) +
	(!!(MX_NUM_NODES == 193) * 6955) +
	(!!(MX_NUM_NODES == 194) * 6949) +
	(!!(MX_NUM_NODES == 195) * 6944) +
	(!!(MX_NUM_NODES == 196) * 6938) +
	(!!(MX_NUM_NODES == 197) * 6933) +
	(!!(MX_NUM_NODES == 198) * 6927) +
	(!!(MX_NUM_NODES == 199) * 6922) +
	(!!(MX_NUM_NODES == 200) * 6917) +
	(!!(MX_NUM_NODES == 201) * 6912) +
	(!!(MX_NUM_NODES == 202) * 6906) +
	(!!(MX_NUM_NODES == 203) * 6901) +
	(!!(MX_NUM_NODES == 204) * 6896) +
	(!!(MX_NUM_NODES == 205) * 6891) +
	(!!(MX_NUM_NODES == 206) * 6886) +
	(!!(MX_NUM_NODES == 207) * 6881) +
	(!!(MX_NUM_NODES == 208) * 6876) +
	(!!(MX_NUM_NODES == 209) * 6871) +
	(!!(MX_NUM_NODES == 210) * 6866) +
	(!!(MX_NUM_NODES == 211) * 6861) +
	(!!(MX_NUM_NODES == 212) * 6856) +
	(!!(MX_NUM_NODES == 213) * 6851) +
	(!!(MX_NUM_NODES == 214) * 6846) +
	(!!(MX_NUM_NODES == 215) * 6841) +
	(!!(MX_NUM_NODES == 216) * 6837) +
	(!!(MX_NUM_NODES == 217) * 6832) +
	(!!(MX_NUM_NODES == 218) * 6827) +
	(!!(MX_NUM_NODES == 219) * 6823) +
	(!!(MX_NUM_NODES == 220) * 6818) +
	(!!(MX_NUM_NODES == 221) * 6813) +
	(!!(MX_NUM_NODES == 222) * 6809) +
	(!!(MX_NUM_NODES == 223) * 6804) +
	(!!(MX_NUM_NODES == 224) * 6799) +
	(!!(MX_NUM_NODES == 225) * 6795) +
	(!!(MX_NUM_NODES == 226) * 6790) +
	(!!(MX_NUM_NODES == 227) * 6786) +
	(!!(MX_NUM_NODES == 228) * 6781) +
	(!!(MX_NUM_NODES == 229) * 6777) +
	(!!(MX_NUM_NODES == 230) * 6773) +
	(!!(MX_NUM_NODES == 231) * 6768) +
	(!!(MX_NUM_NODES == 232) * 6764) +
	(!!(MX_NUM_NODES == 233) * 6760) +
	(!!(MX_NUM_NODES == 234) * 6755) +
	(!!(MX_NUM_NODES == 235) * 6751) +
	(!!(MX_NUM_NODES == 236) * 6747) +
	(!!(MX_NUM_NODES == 237) * 6742) +
	(!!(MX_NUM_NODES == 238) * 6738) +
	(!!(MX_NUM_NODES == 239) * 6734) +
	(!!(MX_NUM_NODES == 240) * 6730) +
	(!!(MX_NUM_NODES == 241) * 6726) +
	(!!(MX_NUM_NODES == 242) * 6722) +
	(!!(MX_NUM_NODES == 243) * 6717) +
	(!!(MX_NUM_NODES == 244) * 6713) +
	(!!(MX_NUM_NODES == 245) * 6709) +
	(!!(MX_NUM_NODES == 246) * 6705) +
	(!!(MX_NUM_NODES == 247) * 6701) +
	(!!(MX_NUM_NODES == 248) * 6697) +
	(!!(MX_NUM_NODES == 249) * 6693) +
	(!!(MX_NUM_NODES == 250) * 6689) +
	(!!(MX_NUM_NODES == 251) * 6685) +
	(!!(MX_NUM_NODES == 252) * 6682) +
	(!!(MX_NUM_NODES == 253) * 6678) +
	(!!(MX_NUM_NODES == 254) * 6674) +
	(!!(MX_NUM_NODES == 255) * 6670) +
	(!!(MX_NUM_NODES == 256) * 6666) +
	0);

ASSERT_CT_STATIC(0 != MX_DISCOVERY_PTX, MX_NUM_NODES_invalid);

static const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_SIZE = (
	(!!(MX_NUM_NODES ==   2) * 1) +
	(!!(MX_NUM_NODES ==   3) * 2) +
	(!!(MX_NUM_NODES ==   4) * 3) +
	(!!(MX_NUM_NODES ==   5) * 3) +
	(!!(MX_NUM_NODES ==   6) * 4) +
	(!!(MX_NUM_NODES ==   7) * 5) +
	(!!(MX_NUM_NODES ==   8) * 5) +
	(!!(MX_NUM_NODES ==   9) * 6) +
	(!!(MX_NUM_NODES ==  10) * 6) +
	(!!(MX_NUM_NODES ==  11) * 7) +
	(!!(MX_NUM_NODES ==  12) * 8) +
	(!!(MX_NUM_NODES ==  13) * 8) +
	(!!(MX_NUM_NODES ==  14) * 8) +
	(!!(MX_NUM_NODES ==  15) * 9) +
	(!!(MX_NUM_NODES ==  16) * 9) +
	(!!(MX_NUM_NODES ==  17) * 10) +
	(!!(MX_NUM_NODES ==  18) * 10) +
	(!!(MX_NUM_NODES ==  19) * 10) +
	(!!(MX_NUM_NODES ==  20) * 10) +
	(!!(MX_NUM_NODES ==  21) * 11) +
	(!!(MX_NUM_NODES ==  22) * 11) +
	(!!(MX_NUM_NODES ==  23) * 11) +
	(!!(MX_NUM_NODES ==  24) * 11) +
	(!!(MX_NUM_NODES ==  25) * 11) +
	(!!(MX_NUM_NODES ==  26) * 12) +
	(!!(MX_NUM_NODES ==  27) * 12) +
	(!!(MX_NUM_NODES ==  28) * 12) +
	(!!(MX_NUM_NODES ==  29) * 13) +
	(!!(MX_NUM_NODES ==  30) * 13) +
	(!!(MX_NUM_NODES ==  31) * 13) +
	(!!(MX_NUM_NODES ==  32) * 13) +
	(!!(MX_NUM_NODES ==  33) * 14) +
	(!!(MX_NUM_NODES ==  34) * 14) +
	(!!(MX_NUM_NODES ==  35) * 14) +
	(!!(MX_NUM_NODES ==  36) * 15) +
	(!!(MX_NUM_NODES ==  37) * 15) +
	(!!(MX_NUM_NODES ==  38) * 15) +
	(!!(MX_NUM_NODES ==  39) * 15) +
	(!!(MX_NUM_NODES ==  40) * 15) +
	(!!(MX_NUM_NODES ==  41) * 16) +
	(!!(MX_NUM_NODES ==  42) * 16) +
	(!!(MX_NUM_NODES ==  43) * 16) +
	(!!(MX_NUM_NODES ==  44) * 16) +
	(!!(MX_NUM_NODES ==  45) * 17) +
	(!!(MX_NUM_NODES ==  46) * 17) +
	(!!(MX_NUM_NODES ==  47) * 17) +
	(!!(MX_NUM_NODES ==  48) * 17) +
	(!!(MX_NUM_NODES ==  49) * 17) +
	(!!(MX_NUM_NODES ==  50) * 18) +
	(!!(MX_NUM_NODES ==  51) * 18) +
	(!!(MX_NUM_NODES ==  52) * 18) +
	(!!(MX_NUM_NODES ==  53) * 18) +
	(!!(MX_NUM_NODES ==  54) * 18) +
	(!!(MX_NUM_NODES ==  55) * 18) +
	(!!(MX_NUM_NODES ==  56) * 19) +
	(!!(MX_NUM_NODES ==  57) * 19) +
	(!!(MX_NUM_NODES ==  58) * 19) +
	(!!(MX_NUM_NODES ==  59) * 19) +
	(!!(MX_NUM_NODES ==  60) * 19) +
	(!!(MX_NUM_NODES ==  61) * 20) +
	(!!(MX_NUM_NODES ==  62) * 20) +
	(!!(MX_NUM_NODES ==  63) * 20) +
	(!!(MX_NUM_NODES ==  64) * 20) +
	(!!(MX_NUM_NODES ==  65) * 20) +
	(!!(MX_NUM_NODES ==  66) * 20) +
	(!!(MX_NUM_NODES ==  67) * 20) +
	(!!(MX_NUM_NODES ==  68) * 21) +
	(!!(MX_NUM_NODES ==  69) * 21) +
	(!!(MX_NUM_NODES ==  70) * 21) +
	(!!(MX_NUM_NODES ==  71) * 21) +
	(!!(MX_NUM_NODES ==  72) * 21) +
	(!!(MX_NUM_NODES ==  73) * 21) +
	(!!(MX_NUM_NODES ==  74) * 21) +
	(!!(MX_NUM_NODES ==  75) * 22) +
	(!!(MX_NUM_NODES ==  76) * 22) +
	(!!(MX_NUM_NODES ==  77) * 22) +
	(!!(MX_NUM_NODES ==  78) * 22) +
	(!!(MX_NUM_NODES ==  79) * 22) +
	(!!(MX_NUM_NODES ==  80) * 22) +
	(!!(MX_NUM_NODES ==  81) * 22) +
	(!!(MX_NUM_NODES ==  82) * 23) +
	(!!(MX_NUM_NODES ==  83) * 23) +
	(!!(MX_NUM_NODES ==  84) * 23) +
	(!!(MX_NUM_NODES ==  85) * 23) +
	(!!(MX_NUM_NODES ==  86) * 23) +
	(!!(MX_NUM_NODES ==  87) * 23) +
	(!!(MX_NUM_NODES ==  88) * 23) +
	(!!(MX_NUM_NODES ==  89) * 23) +
	(!!(MX_NUM_NODES ==  90) * 24) +
	(!!(MX_NUM_NODES ==  91) * 24) +
	(!!(MX_NUM_NODES ==  92) * 24) +
	(!!(MX_NUM_NODES ==  93) * 24) +
	(!!(MX_NUM_NODES ==  94) * 24) +
	(!!(MX_NUM_NODES ==  95) * 24) +
	(!!(MX_NUM_NODES ==  96) * 24) +
	(!!(MX_NUM_NODES ==  97) * 24) +
	(!!(MX_NUM_NODES ==  98) * 25) +
	(!!(MX_NUM_NODES ==  99) * 25) +
	(!!(MX_NUM_NODES == 100) * 25) +
	(!!(MX_NUM_NODES == 101) * 25) +
	(!!(MX_NUM_NODES == 102) * 25) +
	(!!(MX_NUM_NODES == 103) * 25) +
	(!!(MX_NUM_NODES == 104) * 25) +
	(!!(MX_NUM_NODES == 105) * 25) +
	(!!(MX_NUM_NODES == 106) * 25) +
	(!!(MX_NUM_NODES == 107) * 26) +
	(!!(MX_NUM_NODES == 108) * 26) +
	(!!(MX_NUM_NODES == 109) * 26) +
	(!!(MX_NUM_NODES == 110) * 26) +
	(!!(MX_NUM_NODES == 111) * 26) +
	(!!(MX_NUM_NODES == 112) * 26) +
	(!!(MX_NUM_NODES == 113) * 26) +
	(!!(MX_NUM_NODES == 114) * 26) +
	(!!(MX_NUM_NODES == 115) * 26) +
	(!!(MX_NUM_NODES == 116) * 26) +
	(!!(MX_NUM_NODES == 117) * 27) +
	(!!(MX_NUM_NODES == 118) * 27) +
	(!!(MX_NUM_NODES == 119) * 27) +
	(!!(MX_NUM_NODES == 120) * 27) +
	(!!(MX_NUM_NODES == 121) * 27) +
	(!!(MX_NUM_NODES == 122) * 27) +
	(!!(MX_NUM_NODES == 123) * 27) +
	(!!(MX_NUM_NODES == 124) * 27) +
	(!!(MX_NUM_NODES == 125) * 27) +
	(!!(MX_NUM_NODES == 126) * 27) +
	(!!(MX_NUM_NODES == 127) * 27) +
	(!!(MX_NUM_NODES == 128) * 28) +
	(!!(MX_NUM_NODES == 129) * 28) +
	(!!(MX_NUM_NODES == 130) * 28) +
	(!!(MX_NUM_NODES == 131) * 28) +
	(!!(MX_NUM_NODES == 132) * 28) +
	(!!(MX_NUM_NODES == 133) * 28) +
	(!!(MX_NUM_NODES == 134) * 28) +
	(!!(MX_NUM_NODES == 135) * 28) +
	(!!(MX_NUM_NODES == 136) * 28) +
	(!!(MX_NUM_NODES == 137) * 28) +
	(!!(MX_NUM_NODES == 138) * 28) +
	(!!(MX_NUM_NODES == 139) * 29) +
	(!!(MX_NUM_NODES == 140) * 29) +
	(!!(MX_NUM_NODES == 141) * 29) +
	(!!(MX_NUM_NODES == 142) * 29) +
	(!!(MX_NUM_NODES == 143) * 29) +
	(!!(MX_NUM_NODES == 144) * 29) +
	(!!(MX_NUM_NODES == 145) * 29) +
	(!!(MX_NUM_NODES == 146) * 29) +
	(!!(MX_NUM_NODES == 147) * 29) +
	(!!(MX_NUM_NODES == 148) * 29) +
	(!!(MX_NUM_NODES == 149) * 29) +
	(!!(MX_NUM_NODES == 150) * 29) +
	(!!(MX_NUM_NODES == 151) * 30) +
	(!!(MX_NUM_NODES == 152) * 30) +
	(!!(MX_NUM_NODES == 153) * 30) +
	(!!(MX_NUM_NODES == 154) * 30) +
	(!!(MX_NUM_NODES == 155) * 30) +
	(!!(MX_NUM_NODES == 156) * 30) +
	(!!(MX_NUM_NODES == 157) * 30) +
	(!!(MX_NUM_NODES == 158) * 30) +
	(!!(MX_NUM_NODES == 159) * 30) +
	(!!(MX_NUM_NODES == 160) * 30) +
	(!!(MX_NUM_NODES == 161) * 30) +
	(!!(MX_NUM_NODES == 162) * 30) +
	(!!(MX_NUM_NODES == 163) * 31) +
	(!!(MX_NUM_NODES == 164) * 31) +
	(!!(MX_NUM_NODES == 165) * 31) +
	(!!(MX_NUM_NODES == 166) * 31) +
	(!!(MX_NUM_NODES == 167) * 31) +
	(!!(MX_NUM_NODES == 168) * 31) +
	(!!(MX_NUM_NODES == 169) * 31) +
	(!!(MX_NUM_NODES == 170) * 31) +
	(!!(MX_NUM_NODES == 171) * 31) +
	(!!(MX_NUM_NODES == 172) * 31) +
	(!!(MX_NUM_NODES == 173) * 31) +
	(!!(MX_NUM_NODES == 174) * 31) +
	(!!(MX_NUM_NODES == 175) * 31) +
	(!!(MX_NUM_NODES == 176) * 31) +
	(!!(MX_NUM_NODES == 177) * 32) +
	(!!(MX_NUM_NODES == 178) * 32) +
	(!!(MX_NUM_NODES == 179) * 32) +
	(!!(MX_NUM_NODES == 180) * 32) +
	(!!(MX_NUM_NODES == 181) * 32) +
	(!!(MX_NUM_NODES == 182) * 32) +
	(!!(MX_NUM_NODES == 183) * 32) +
	(!!(MX_NUM_NODES == 184) * 32) +
	(!!(MX_NUM_NODES == 185) * 32) +
	(!!(MX_NUM_NODES == 186) * 32) +
	(!!(MX_NUM_NODES == 187) * 32) +
	(!!(MX_NUM_NODES == 188) * 32) +
	(!!(MX_NUM_NODES == 189) * 32) +
	(!!(MX_NUM_NODES == 190) * 32) +
	(!!(MX_NUM_NODES == 191) * 33) +
	(!!(MX_NUM_NODES == 192) * 33) +
	(!!(MX_NUM_NODES == 193) * 33) +
	(!!(MX_NUM_NODES == 194) * 33) +
	(!!(MX_NUM_NODES == 195) * 33) +
	(!!(MX_NUM_NODES == 196) * 33) +
	(!!(MX_NUM_NODES == 197) * 33) +
	(!!(MX_NUM_NODES == 198) * 33) +
	(!!(MX_NUM_NODES == 199) * 33) +
	(!!(MX_NUM_NODES == 200) * 33) +
	(!!(MX_NUM_NODES == 201) * 33) +
	(!!(MX_NUM_NODES == 202) * 33) +
	(!!(MX_NUM_NODES == 203) * 33) +
	(!!(MX_NUM_NODES == 204) * 33) +
	(!!(MX_NUM_NODES == 205) * 33) +
	(!!(MX_NUM_NODES == 206) * 33) +
	(!!(MX_NUM_NODES == 207) * 34) +
	(!!(MX_NUM_NODES == 208) * 34) +
	(!!(MX_NUM_NODES == 209) * 34) +
	(!!(MX_NUM_NODES == 210) * 34) +
	(!!(MX_NUM_NODES == 211) * 34) +
	(!!(MX_NUM_NODES == 212) * 34) +
	(!!(MX_NUM_NODES == 213) * 34) +
	(!!(MX_NUM_NODES == 214) * 34) +
	(!!(MX_NUM_NODES == 215) * 34) +
	(!!(MX_NUM_NODES == 216) * 34) +
	(!!(MX_NUM_NODES == 217) * 34) +
	(!!(MX_NUM_NODES == 218) * 34) +
	(!!(MX_NUM_NODES == 219) * 34) +
	(!!(MX_NUM_NODES == 220) * 34) +
	(!!(MX_NUM_NODES == 221) * 34) +
	(!!(MX_NUM_NODES == 222) * 34) +
	(!!(MX_NUM_NODES == 223) * 35) +
	(!!(MX_NUM_NODES == 224) * 35) +
	(!!(MX_NUM_NODES == 225) * 35) +
	(!!(MX_NUM_NODES == 226) * 35) +
	(!!(MX_NUM_NODES == 227) * 35) +
	(!!(MX_NUM_NODES == 228) * 35) +
	(!!(MX_NUM_NODES == 229) * 35) +
	(!!(MX_NUM_NODES == 230) * 35) +
	(!!(MX_NUM_NODES == 231) * 35) +
	(!!(MX_NUM_NODES == 232) * 35) +
	(!!(MX_NUM_NODES == 233) * 35) +
	(!!(MX_NUM_NODES == 234) * 35) +
	(!!(MX_NUM_NODES == 235) * 35) +
	(!!(MX_NUM_NODES == 236) * 35) +
	(!!(MX_NUM_NODES == 237) * 35) +
	(!!(MX_NUM_NODES == 238) * 35) +
	(!!(MX_NUM_NODES == 239) * 35) +
	(!!(MX_NUM_NODES == 240) * 36) +
	(!!(MX_NUM_NODES == 241) * 36) +
	(!!(MX_NUM_NODES == 242) * 36) +
	(!!(MX_NUM_NODES == 243) * 36) +
	(!!(MX_NUM_NODES == 244) * 36) +
	(!!(MX_NUM_NODES == 245) * 36) +
	(!!(MX_NUM_NODES == 246) * 36) +
	(!!(MX_NUM_NODES == 247) * 36) +
	(!!(MX_NUM_NODES == 248) * 36) +
	(!!(MX_NUM_NODES == 249) * 36) +
	(!!(MX_NUM_NODES == 250) * 36) +
	(!!(MX_NUM_NODES == 251) * 36) +
	(!!(MX_NUM_NODES == 252) * 36) +
	(!!(MX_NUM_NODES == 253) * 36) +
	(!!(MX_NUM_NODES == 254) * 36) +
	(!!(MX_NUM_NODES == 255) * 36) +
	(!!(MX_NUM_NODES == 256) * 36) +
	0);

ASSERT_CT_STATIC(0 != MX_DISCOVERY_EXIT_SLOT_LUT_SIZE, MX_NUM_NODES_invalid);

extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_2[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_3[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_4[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_5[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_6[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_7[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_8[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_9[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_10[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_11[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_12[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_13[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_14[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_15[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_16[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_17[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_18[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_19[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_20[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_21[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_22[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_23[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_24[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_25[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_26[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_27[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_28[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_29[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_30[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_31[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_32[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_33[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_34[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_35[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_36[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_37[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_38[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_39[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_40[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_41[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_42[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_43[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_44[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_45[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_46[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_47[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_48[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_49[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_50[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_51[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_52[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_53[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_54[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_55[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_56[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_57[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_58[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_59[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_60[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_61[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_62[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_63[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_64[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_65[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_66[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_67[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_68[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_69[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_70[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_71[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_72[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_73[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_74[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_75[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_76[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_77[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_78[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_79[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_80[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_81[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_82[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_83[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_84[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_85[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_86[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_87[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_88[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_89[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_90[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_91[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_92[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_93[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_94[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_95[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_96[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_97[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_98[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_99[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_100[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_101[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_102[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_103[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_104[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_105[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_106[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_107[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_108[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_109[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_110[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_111[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_112[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_113[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_114[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_115[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_116[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_117[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_118[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_119[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_120[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_121[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_122[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_123[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_124[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_125[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_126[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_127[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_128[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_129[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_130[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_131[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_132[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_133[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_134[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_135[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_136[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_137[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_138[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_139[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_140[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_141[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_142[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_143[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_144[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_145[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_146[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_147[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_148[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_149[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_150[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_151[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_152[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_153[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_154[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_155[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_156[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_157[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_158[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_159[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_160[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_161[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_162[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_163[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_164[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_165[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_166[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_167[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_168[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_169[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_170[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_171[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_172[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_173[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_174[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_175[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_176[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_177[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_178[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_179[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_180[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_181[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_182[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_183[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_184[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_185[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_186[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_187[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_188[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_189[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_190[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_191[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_192[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_193[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_194[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_195[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_196[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_197[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_198[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_199[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_200[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_201[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_202[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_203[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_204[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_205[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_206[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_207[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_208[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_209[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_210[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_211[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_212[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_213[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_214[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_215[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_216[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_217[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_218[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_219[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_220[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_221[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_222[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_223[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_224[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_225[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_226[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_227[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_228[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_229[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_230[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_231[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_232[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_233[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_234[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_235[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_236[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_237[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_238[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_239[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_240[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_241[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_242[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_243[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_244[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_245[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_246[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_247[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_248[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_249[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_250[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_251[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_252[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_253[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_254[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_255[];
extern const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_256[];

static const uint16_t* const MX_DISCOVERY_EXIT_SLOT_LUT = (const uint16_t*)(
	(!!(MX_NUM_NODES ==   2) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_2) +
	(!!(MX_NUM_NODES ==   3) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_3) +
	(!!(MX_NUM_NODES ==   4) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_4) +
	(!!(MX_NUM_NODES ==   5) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_5) +
	(!!(MX_NUM_NODES ==   6) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_6) +
	(!!(MX_NUM_NODES ==   7) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_7) +
	(!!(MX_NUM_NODES ==   8) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_8) +
	(!!(MX_NUM_NODES ==   9) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_9) +
	(!!(MX_NUM_NODES ==  10) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_10) +
	(!!(MX_NUM_NODES ==  11) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_11) +
	(!!(MX_NUM_NODES ==  12) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_12) +
	(!!(MX_NUM_NODES ==  13) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_13) +
	(!!(MX_NUM_NODES ==  14) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_14) +
	(!!(MX_NUM_NODES ==  15) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_15) +
	(!!(MX_NUM_NODES ==  16) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_16) +
	(!!(MX_NUM_NODES ==  17) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_17) +
	(!!(MX_NUM_NODES ==  18) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_18) +
	(!!(MX_NUM_NODES ==  19) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_19) +
	(!!(MX_NUM_NODES ==  20) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_20) +
	(!!(MX_NUM_NODES ==  21) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_21) +
	(!!(MX_NUM_NODES ==  22) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_22) +
	(!!(MX_NUM_NODES ==  23) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_23) +
	(!!(MX_NUM_NODES ==  24) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_24) +
	(!!(MX_NUM_NODES ==  25) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_25) +
	(!!(MX_NUM_NODES ==  26) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_26) +
	(!!(MX_NUM_NODES ==  27) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_27) +
	(!!(MX_NUM_NODES ==  28) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_28) +
	(!!(MX_NUM_NODES ==  29) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_29) +
	(!!(MX_NUM_NODES ==  30) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_30) +
	(!!(MX_NUM_NODES ==  31) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_31) +
	(!!(MX_NUM_NODES ==  32) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_32) +
	(!!(MX_NUM_NODES ==  33) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_33) +
	(!!(MX_NUM_NODES ==  34) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_34) +
	(!!(MX_NUM_NODES ==  35) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_35) +
	(!!(MX_NUM_NODES ==  36) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_36) +
	(!!(MX_NUM_NODES ==  37) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_37) +
	(!!(MX_NUM_NODES ==  38) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_38) +
	(!!(MX_NUM_NODES ==  39) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_39) +
	(!!(MX_NUM_NODES ==  40) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_40) +
	(!!(MX_NUM_NODES ==  41) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_41) +
	(!!(MX_NUM_NODES ==  42) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_42) +
	(!!(MX_NUM_NODES ==  43) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_43) +
	(!!(MX_NUM_NODES ==  44) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_44) +
	(!!(MX_NUM_NODES ==  45) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_45) +
	(!!(MX_NUM_NODES ==  46) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_46) +
	(!!(MX_NUM_NODES ==  47) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_47) +
	(!!(MX_NUM_NODES ==  48) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_48) +
	(!!(MX_NUM_NODES ==  49) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_49) +
	(!!(MX_NUM_NODES ==  50) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_50) +
	(!!(MX_NUM_NODES ==  51) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_51) +
	(!!(MX_NUM_NODES ==  52) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_52) +
	(!!(MX_NUM_NODES ==  53) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_53) +
	(!!(MX_NUM_NODES ==  54) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_54) +
	(!!(MX_NUM_NODES ==  55) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_55) +
	(!!(MX_NUM_NODES ==  56) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_56) +
	(!!(MX_NUM_NODES ==  57) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_57) +
	(!!(MX_NUM_NODES ==  58) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_58) +
	(!!(MX_NUM_NODES ==  59) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_59) +
	(!!(MX_NUM_NODES ==  60) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_60) +
	(!!(MX_NUM_NODES ==  61) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_61) +
	(!!(MX_NUM_NODES ==  62) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_62) +
	(!!(MX_NUM_NODES ==  63) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_63) +
	(!!(MX_NUM_NODES ==  64) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_64) +
	(!!(MX_NUM_NODES ==  65) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_65) +
	(!!(MX_NUM_NODES ==  66) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_66) +
	(!!(MX_NUM_NODES ==  67) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_67) +
	(!!(MX_NUM_NODES ==  68) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_68) +
	(!!(MX_NUM_NODES ==  69) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_69) +
	(!!(MX_NUM_NODES ==  70) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_70) +
	(!!(MX_NUM_NODES ==  71) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_71) +
	(!!(MX_NUM_NODES ==  72) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_72) +
	(!!(MX_NUM_NODES ==  73) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_73) +
	(!!(MX_NUM_NODES ==  74) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_74) +
	(!!(MX_NUM_NODES ==  75) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_75) +
	(!!(MX_NUM_NODES ==  76) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_76) +
	(!!(MX_NUM_NODES ==  77) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_77) +
	(!!(MX_NUM_NODES ==  78) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_78) +
	(!!(MX_NUM_NODES ==  79) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_79) +
	(!!(MX_NUM_NODES ==  80) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_80) +
	(!!(MX_NUM_NODES ==  81) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_81) +
	(!!(MX_NUM_NODES ==  82) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_82) +
	(!!(MX_NUM_NODES ==  83) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_83) +
	(!!(MX_NUM_NODES ==  84) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_84) +
	(!!(MX_NUM_NODES ==  85) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_85) +
	(!!(MX_NUM_NODES ==  86) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_86) +
	(!!(MX_NUM_NODES ==  87) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_87) +
	(!!(MX_NUM_NODES ==  88) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_88) +
	(!!(MX_NUM_NODES ==  89) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_89) +
	(!!(MX_NUM_NODES ==  90) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_90) +
	(!!(MX_NUM_NODES ==  91) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_91) +
	(!!(MX_NUM_NODES ==  92) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_92) +
	(!!(MX_NUM_NODES ==  93) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_93) +
	(!!(MX_NUM_NODES ==  94) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_94) +
	(!!(MX_NUM_NODES ==  95) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_95) +
	(!!(MX_NUM_NODES ==  96) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_96) +
	(!!(MX_NUM_NODES ==  97) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_97) +
	(!!(MX_NUM_NODES ==  98) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_98) +
	(!!(MX_NUM_NODES ==  99) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_99) +
	(!!(MX_NUM_NODES == 100) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_100) +
	(!!(MX_NUM_NODES == 101) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_101) +
	(!!(MX_NUM_NODES == 102) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_102) +
	(!!(MX_NUM_NODES == 103) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_103) +
	(!!(MX_NUM_NODES == 104) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_104) +
	(!!(MX_NUM_NODES == 105) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_105) +
	(!!(MX_NUM_NODES == 106) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_106) +
	(!!(MX_NUM_NODES == 107) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_107) +
	(!!(MX_NUM_NODES == 108) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_108) +
	(!!(MX_NUM_NODES == 109) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_109) +
	(!!(MX_NUM_NODES == 110) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_110) +
	(!!(MX_NUM_NODES == 111) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_111) +
	(!!(MX_NUM_NODES == 112) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_112) +
	(!!(MX_NUM_NODES == 113) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_113) +
	(!!(MX_NUM_NODES == 114) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_114) +
	(!!(MX_NUM_NODES == 115) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_115) +
	(!!(MX_NUM_NODES == 116) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_116) +
	(!!(MX_NUM_NODES == 117) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_117) +
	(!!(MX_NUM_NODES == 118) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_118) +
	(!!(MX_NUM_NODES == 119) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_119) +
	(!!(MX_NUM_NODES == 120) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_120) +
	(!!(MX_NUM_NODES == 121) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_121) +
	(!!(MX_NUM_NODES == 122) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_122) +
	(!!(MX_NUM_NODES == 123) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_123) +
	(!!(MX_NUM_NODES == 124) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_124) +
	(!!(MX_NUM_NODES == 125) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_125) +
	(!!(MX_NUM_NODES == 126) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_126) +
	(!!(MX_NUM_NODES == 127) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_127) +
	(!!(MX_NUM_NODES == 128) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_128) +
	(!!(MX_NUM_NODES == 129) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_129) +
	(!!(MX_NUM_NODES == 130) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_130) +
	(!!(MX_NUM_NODES == 131) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_131) +
	(!!(MX_NUM_NODES == 132) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_132) +
	(!!(MX_NUM_NODES == 133) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_133) +
	(!!(MX_NUM_NODES == 134) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_134) +
	(!!(MX_NUM_NODES == 135) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_135) +
	(!!(MX_NUM_NODES == 136) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_136) +
	(!!(MX_NUM_NODES == 137) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_137) +
	(!!(MX_NUM_NODES == 138) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_138) +
	(!!(MX_NUM_NODES == 139) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_139) +
	(!!(MX_NUM_NODES == 140) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_140) +
	(!!(MX_NUM_NODES == 141) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_141) +
	(!!(MX_NUM_NODES == 142) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_142) +
	(!!(MX_NUM_NODES == 143) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_143) +
	(!!(MX_NUM_NODES == 144) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_144) +
	(!!(MX_NUM_NODES == 145) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_145) +
	(!!(MX_NUM_NODES == 146) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_146) +
	(!!(MX_NUM_NODES == 147) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_147) +
	(!!(MX_NUM_NODES == 148) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_148) +
	(!!(MX_NUM_NODES == 149) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_149) +
	(!!(MX_NUM_NODES == 150) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_150) +
	(!!(MX_NUM_NODES == 151) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_151) +
	(!!(MX_NUM_NODES == 152) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_152) +
	(!!(MX_NUM_NODES == 153) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_153) +
	(!!(MX_NUM_NODES == 154) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_154) +
	(!!(MX_NUM_NODES == 155) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_155) +
	(!!(MX_NUM_NODES == 156) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_156) +
	(!!(MX_NUM_NODES == 157) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_157) +
	(!!(MX_NUM_NODES == 158) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_158) +
	(!!(MX_NUM_NODES == 159) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_159) +
	(!!(MX_NUM_NODES == 160) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_160) +
	(!!(MX_NUM_NODES == 161) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_161) +
	(!!(MX_NUM_NODES == 162) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_162) +
	(!!(MX_NUM_NODES == 163) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_163) +
	(!!(MX_NUM_NODES == 164) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_164) +
	(!!(MX_NUM_NODES == 165) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_165) +
	(!!(MX_NUM_NODES == 166) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_166) +
	(!!(MX_NUM_NODES == 167) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_167) +
	(!!(MX_NUM_NODES == 168) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_168) +
	(!!(MX_NUM_NODES == 169) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_169) +
	(!!(MX_NUM_NODES == 170) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_170) +
	(!!(MX_NUM_NODES == 171) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_171) +
	(!!(MX_NUM_NODES == 172) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_172) +
	(!!(MX_NUM_NODES == 173) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_173) +
	(!!(MX_NUM_NODES == 174) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_174) +
	(!!(MX_NUM_NODES == 175) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_175) +
	(!!(MX_NUM_NODES == 176) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_176) +
	(!!(MX_NUM_NODES == 177) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_177) +
	(!!(MX_NUM_NODES == 178) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_178) +
	(!!(MX_NUM_NODES == 179) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_179) +
	(!!(MX_NUM_NODES == 180) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_180) +
	(!!(MX_NUM_NODES == 181) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_181) +
	(!!(MX_NUM_NODES == 182) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_182) +
	(!!(MX_NUM_NODES == 183) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_183) +
	(!!(MX_NUM_NODES == 184) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_184) +
	(!!(MX_NUM_NODES == 185) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_185) +
	(!!(MX_NUM_NODES == 186) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_186) +
	(!!(MX_NUM_NODES == 187) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_187) +
	(!!(MX_NUM_NODES == 188) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_188) +
	(!!(MX_NUM_NODES == 189) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_189) +
	(!!(MX_NUM_NODES == 190) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_190) +
	(!!(MX_NUM_NODES == 191) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_191) +
	(!!(MX_NUM_NODES == 192) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_192) +
	(!!(MX_NUM_NODES == 193) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_193) +
	(!!(MX_NUM_NODES == 194) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_194) +
	(!!(MX_NUM_NODES == 195) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_195) +
	(!!(MX_NUM_NODES == 196) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_196) +
	(!!(MX_NUM_NODES == 197) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_197) +
	(!!(MX_NUM_NODES == 198) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_198) +
	(!!(MX_NUM_NODES == 199) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_199) +
	(!!(MX_NUM_NODES == 200) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_200) +
	(!!(MX_NUM_NODES == 201) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_201) +
	(!!(MX_NUM_NODES == 202) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_202) +
	(!!(MX_NUM_NODES == 203) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_203) +
	(!!(MX_NUM_NODES == 204) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_204) +
	(!!(MX_NUM_NODES == 205) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_205) +
	(!!(MX_NUM_NODES == 206) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_206) +
	(!!(MX_NUM_NODES == 207) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_207) +
	(!!(MX_NUM_NODES == 208) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_208) +
	(!!(MX_NUM_NODES == 209) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_209) +
	(!!(MX_NUM_NODES == 210) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_210) +
	(!!(MX_NUM_NODES == 211) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_211) +
	(!!(MX_NUM_NODES == 212) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_212) +
	(!!(MX_NUM_NODES == 213) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_213) +
	(!!(MX_NUM_NODES == 214) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_214) +
	(!!(MX_NUM_NODES == 215) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_215) +
	(!!(MX_NUM_NODES == 216) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_216) +
	(!!(MX_NUM_NODES == 217) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_217) +
	(!!(MX_NUM_NODES == 218) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_218) +
	(!!(MX_NUM_NODES == 219) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_219) +
	(!!(MX_NUM_NODES == 220) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_220) +
	(!!(MX_NUM_NODES == 221) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_221) +
	(!!(MX_NUM_NODES == 222) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_222) +
	(!!(MX_NUM_NODES == 223) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_223) +
	(!!(MX_NUM_NODES == 224) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_224) +
	(!!(MX_NUM_NODES == 225) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_225) +
	(!!(MX_NUM_NODES == 226) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_226) +
	(!!(MX_NUM_NODES == 227) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_227) +
	(!!(MX_NUM_NODES == 228) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_228) +
	(!!(MX_NUM_NODES == 229) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_229) +
	(!!(MX_NUM_NODES == 230) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_230) +
	(!!(MX_NUM_NODES == 231) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_231) +
	(!!(MX_NUM_NODES == 232) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_232) +
	(!!(MX_NUM_NODES == 233) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_233) +
	(!!(MX_NUM_NODES == 234) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_234) +
	(!!(MX_NUM_NODES == 235) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_235) +
	(!!(MX_NUM_NODES == 236) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_236) +
	(!!(MX_NUM_NODES == 237) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_237) +
	(!!(MX_NUM_NODES == 238) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_238) +
	(!!(MX_NUM_NODES == 239) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_239) +
	(!!(MX_NUM_NODES == 240) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_240) +
	(!!(MX_NUM_NODES == 241) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_241) +
	(!!(MX_NUM_NODES == 242) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_242) +
	(!!(MX_NUM_NODES == 243) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_243) +
	(!!(MX_NUM_NODES == 244) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_244) +
	(!!(MX_NUM_NODES == 245) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_245) +
	(!!(MX_NUM_NODES == 246) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_246) +
	(!!(MX_NUM_NODES == 247) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_247) +
	(!!(MX_NUM_NODES == 248) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_248) +
	(!!(MX_NUM_NODES == 249) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_249) +
	(!!(MX_NUM_NODES == 250) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_250) +
	(!!(MX_NUM_NODES == 251) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_251) +
	(!!(MX_NUM_NODES == 252) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_252) +
	(!!(MX_NUM_NODES == 253) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_253) +
	(!!(MX_NUM_NODES == 254) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_254) +
	(!!(MX_NUM_NODES == 255) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_255) +
	(!!(MX_NUM_NODES == 256) * (uintptr_t)&MX_DISCOVERY_EXIT_SLOT_LUT_256) +
	0);

ASSERT_CT_STATIC(0 != MX_DISCOVERY_EXIT_SLOT_LUT, MX_NUM_NODES_invalid);

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



//**************************************************************************************************
//**************************************************************************************************

#endif // __MIXER_DISCOVERY_H__
