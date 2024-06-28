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
 *	@file					mixer_discovery.c
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
//***** Trace Settings *****************************************************************************



//**************************************************************************************************
//**** Includes ************************************************************************************

#include "mixer_discovery.h"

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

// p_test = 0.333333

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_2[] =
	{ 9 };
	// E[density] = 2, exit_slot_max = 7

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_3[] =
	{ 9, 11 };
	// E[density] = 3, exit_slot_max = 9

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_4[] =
	{ 9, 11, 12 };
	// E[density] = 4, exit_slot_max = 11

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_5[] =
	{ 9, 12, 14 };
	// E[density] = 5, exit_slot_max = 13

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_6[] =
	{ 10, 12, 14, 15 };
	// E[density] = 6, exit_slot_max = 15

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_7[] =
	{ 10, 12, 14, 16, 17 };
	// E[density] = 7, exit_slot_max = 17

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_8[] =
	{ 10, 13, 14, 16, 19 };
	// E[density] = 8, exit_slot_max = 19

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_9[] =
	{ 10, 13, 14, 16, 19, 21 };
	// E[density] = 9, exit_slot_max = 21

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_10[] =
	{ 10, 13, 15, 16, 19, 23 };
	// E[density] = 10, exit_slot_max = 23

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_11[] =
	{ 10, 13, 15, 16, 19, 23, 25 };
	// E[density] = 10, exit_slot_max = 25

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_12[] =
	{ 10, 13, 15, 16, 19, 23, 26, 27 };
	// E[density] = 10, exit_slot_max = 27

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_13[] =
	{ 10, 13, 15, 16, 19, 23, 26, 29 };
	// E[density] = 10, exit_slot_max = 29

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_14[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31 };
	// E[density] = 10, exit_slot_max = 31

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_15[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 33 };
	// E[density] = 10, exit_slot_max = 33

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_16[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 35 };
	// E[density] = 10, exit_slot_max = 35

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_17[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 37 };
	// E[density] = 10, exit_slot_max = 37

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_18[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 39 };
	// E[density] = 10, exit_slot_max = 39

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_19[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 41 };
	// E[density] = 10, exit_slot_max = 41

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_20[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 43 };
	// E[density] = 10, exit_slot_max = 43

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_21[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 43, 45 };
	// E[density] = 10, exit_slot_max = 45

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_22[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 43, 47 };
	// E[density] = 10, exit_slot_max = 47

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_23[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 43, 49 };
	// E[density] = 10, exit_slot_max = 49

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_24[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 43, 50 };
	// E[density] = 10, exit_slot_max = 50

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_25[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 43, 50 };
	// E[density] = 10, exit_slot_max = 50

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_26[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 42, 50, 52 };
	// E[density] = 10, exit_slot_max = 52

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_27[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 42, 49, 54 };
	// E[density] = 10, exit_slot_max = 54

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_28[] =
	{ 10, 13, 15, 16, 19, 23, 26, 31, 36, 42, 49, 56 };
	// E[density] = 10, exit_slot_max = 56

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_29[] =
	{ 10, 13, 15, 16, 19, 22, 26, 30, 35, 41, 48, 56, 58 };
	// E[density] = 11, exit_slot_max = 58

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_30[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 35, 41, 48, 56, 60 };
	// E[density] = 11, exit_slot_max = 60

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_31[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 35, 41, 47, 55, 62 };
	// E[density] = 11, exit_slot_max = 62

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_32[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 35, 41, 47, 55, 64 };
	// E[density] = 11, exit_slot_max = 64

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_33[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 35, 40, 47, 54, 63, 66 };
	// E[density] = 12, exit_slot_max = 66

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_34[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 35, 40, 46, 54, 62, 68 };
	// E[density] = 12, exit_slot_max = 68

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_35[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 34, 40, 46, 53, 62, 70 };
	// E[density] = 12, exit_slot_max = 70

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_36[] =
	{ 10, 13, 15, 17, 19, 22, 26, 30, 34, 40, 46, 53, 62, 71, 72 };
	// E[density] = 12, exit_slot_max = 72

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_37[] =
	{ 11, 13, 15, 17, 19, 22, 26, 30, 34, 39, 46, 53, 61, 70, 74 };
	// E[density] = 13, exit_slot_max = 74

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_38[] =
	{ 11, 13, 15, 17, 19, 22, 26, 30, 34, 39, 45, 52, 60, 70, 76 };
	// E[density] = 13, exit_slot_max = 76

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_39[] =
	{ 11, 13, 15, 17, 19, 22, 26, 30, 34, 39, 45, 52, 60, 70, 78 };
	// E[density] = 13, exit_slot_max = 78

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_40[] =
	{ 11, 13, 15, 17, 19, 22, 26, 30, 34, 39, 45, 52, 60, 69, 80 };
	// E[density] = 13, exit_slot_max = 80

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_41[] =
	{ 11, 13, 15, 17, 19, 22, 26, 30, 34, 39, 45, 52, 60, 69, 79, 82 };
	// E[density] = 13, exit_slot_max = 82

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_42[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 34, 39, 45, 51, 59, 68, 78, 84 };
	// E[density] = 14, exit_slot_max = 84

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_43[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 34, 39, 44, 51, 59, 68, 78, 86 };
	// E[density] = 14, exit_slot_max = 86

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_44[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 34, 39, 44, 51, 58, 67, 77, 88 };
	// E[density] = 14, exit_slot_max = 88

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_45[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 34, 39, 44, 51, 58, 67, 77, 89, 90 };
	// E[density] = 14, exit_slot_max = 90

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_46[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 34, 38, 44, 50, 58, 66, 76, 88, 92 };
	// E[density] = 15, exit_slot_max = 92

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_47[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 34, 38, 44, 50, 58, 66, 76, 87, 94 };
	// E[density] = 15, exit_slot_max = 94

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_48[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 33, 38, 44, 50, 57, 66, 76, 87, 96 };
	// E[density] = 15, exit_slot_max = 96

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_49[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 33, 38, 44, 50, 57, 66, 75, 86, 98 };
	// E[density] = 15, exit_slot_max = 98

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_50[] =
	{ 11, 14, 15, 17, 20, 22, 26, 29, 33, 38, 44, 50, 57, 65, 75, 86, 99, 100 };
	// E[density] = 15, exit_slot_max = 100

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_51[] =
	{ 11, 14, 16, 17, 20, 22, 26, 29, 33, 38, 43, 50, 57, 65, 74, 85, 98, 102 };
	// E[density] = 16, exit_slot_max = 102

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_52[] =
	{ 11, 14, 16, 17, 20, 22, 26, 29, 33, 38, 43, 49, 57, 65, 74, 85, 97, 104 };
	// E[density] = 16, exit_slot_max = 104

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_53[] =
	{ 11, 14, 16, 17, 20, 22, 26, 29, 33, 38, 43, 49, 56, 64, 74, 84, 97, 106 };
	// E[density] = 16, exit_slot_max = 106

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_54[] =
	{ 11, 14, 16, 17, 20, 22, 26, 29, 33, 38, 43, 49, 56, 64, 73, 84, 96, 108 };
	// E[density] = 16, exit_slot_max = 108

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_55[] =
	{ 11, 14, 16, 17, 20, 22, 26, 29, 33, 38, 43, 49, 56, 64, 73, 84, 96, 110 };
	// E[density] = 17, exit_slot_max = 110

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_56[] =
	{ 11, 14, 16, 17, 20, 22, 26, 29, 33, 38, 43, 49, 56, 64, 73, 83, 95, 109, 112 };
	// E[density] = 17, exit_slot_max = 112

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_57[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 38, 43, 49, 56, 63, 72, 83, 95, 108, 114 };
	// E[density] = 17, exit_slot_max = 114

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_58[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 38, 43, 49, 55, 63, 72, 82, 94, 108, 116 };
	// E[density] = 17, exit_slot_max = 116

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_59[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 43, 49, 55, 63, 72, 82, 94, 107, 118 };
	// E[density] = 17, exit_slot_max = 118

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_60[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 43, 48, 55, 63, 72, 82, 93, 107, 120 };
	// E[density] = 18, exit_slot_max = 120

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_61[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 55, 63, 71, 81, 93, 106, 121, 122 };
	// E[density] = 18, exit_slot_max = 122

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_62[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 55, 62, 71, 81, 92, 106, 121, 124 };
	// E[density] = 18, exit_slot_max = 124

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_63[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 55, 62, 71, 81, 92, 105, 120, 126 };
	// E[density] = 18, exit_slot_max = 126

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_64[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 55, 62, 71, 80, 92, 105, 119, 128 };
	// E[density] = 18, exit_slot_max = 128

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_65[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 54, 62, 70, 80, 91, 104, 119, 130 };
	// E[density] = 19, exit_slot_max = 130

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_66[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 54, 62, 70, 80, 91, 104, 118, 132 };
	// E[density] = 19, exit_slot_max = 132

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_67[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 54, 62, 70, 80, 91, 103, 118, 134 };
	// E[density] = 19, exit_slot_max = 134

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_68[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 54, 61, 70, 79, 90, 103, 117, 133, 136 };
	// E[density] = 19, exit_slot_max = 136

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_69[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 48, 54, 61, 70, 79, 90, 102, 117, 133, 138 };
	// E[density] = 19, exit_slot_max = 138

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_70[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 54, 61, 69, 79, 90, 102, 116, 132, 140 };
	// E[density] = 20, exit_slot_max = 140

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_71[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 54, 61, 69, 79, 89, 102, 116, 132, 142 };
	// E[density] = 20, exit_slot_max = 142

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_72[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 54, 61, 69, 78, 89, 101, 115, 131, 144 };
	// E[density] = 20, exit_slot_max = 144

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_73[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 53, 61, 69, 78, 89, 101, 115, 130, 146 };
	// E[density] = 20, exit_slot_max = 146

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_74[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 53, 60, 69, 78, 88, 100, 114, 130, 148 };
	// E[density] = 20, exit_slot_max = 148

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_75[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 53, 60, 68, 78, 88, 100, 114, 129, 147, 150 };
	// E[density] = 21, exit_slot_max = 150

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_76[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 53, 60, 68, 77, 88, 100, 113, 129, 146, 152 };
	// E[density] = 21, exit_slot_max = 152

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_77[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 42, 47, 53, 60, 68, 77, 87, 99, 113, 128, 146, 154 };
	// E[density] = 21, exit_slot_max = 154

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_78[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 41, 47, 53, 60, 68, 77, 87, 99, 112, 128, 145, 156 };
	// E[density] = 21, exit_slot_max = 156

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_79[] =
	{ 11, 14, 16, 17, 20, 23, 26, 29, 33, 37, 41, 47, 53, 60, 68, 77, 87, 99, 112, 127, 144, 158 };
	// E[density] = 21, exit_slot_max = 158

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_80[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 33, 37, 41, 47, 53, 60, 68, 77, 87, 98, 112, 127, 144, 160 };
	// E[density] = 21, exit_slot_max = 160

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_81[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 37, 41, 47, 53, 60, 67, 76, 86, 98, 111, 126, 143, 162 };
	// E[density] = 22, exit_slot_max = 162

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_82[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 37, 41, 47, 53, 59, 67, 76, 86, 98, 111, 126, 143, 162, 164 };
	// E[density] = 22, exit_slot_max = 164

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_83[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 37, 41, 47, 53, 59, 67, 76, 86, 97, 110, 125, 142, 161, 166 };
	// E[density] = 22, exit_slot_max = 166

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_84[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 37, 41, 46, 52, 59, 67, 76, 86, 97, 110, 125, 142, 161, 168 };
	// E[density] = 22, exit_slot_max = 168

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_85[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 37, 41, 46, 52, 59, 67, 76, 85, 97, 110, 124, 141, 160, 170 };
	// E[density] = 22, exit_slot_max = 170

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_86[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 37, 41, 46, 52, 59, 67, 75, 85, 97, 109, 124, 140, 159, 172 };
	// E[density] = 23, exit_slot_max = 172

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_87[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 59, 67, 75, 85, 96, 109, 123, 140, 159, 174 };
	// E[density] = 23, exit_slot_max = 174

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_88[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 59, 66, 75, 85, 96, 109, 123, 139, 158, 176 };
	// E[density] = 23, exit_slot_max = 176

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_89[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 59, 66, 75, 85, 96, 108, 123, 139, 157, 178 };
	// E[density] = 23, exit_slot_max = 178

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_90[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 59, 66, 75, 84, 95, 108, 122, 138, 157, 178, 180 };
	// E[density] = 23, exit_slot_max = 180

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_91[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 59, 66, 75, 84, 95, 108, 122, 138, 156, 177, 182 };
	// E[density] = 24, exit_slot_max = 182

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_92[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 58, 66, 74, 84, 95, 107, 121, 137, 156, 176, 184 };
	// E[density] = 24, exit_slot_max = 184

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_93[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 58, 66, 74, 84, 95, 107, 121, 137, 155, 176, 186 };
	// E[density] = 24, exit_slot_max = 186

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_94[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 58, 66, 74, 84, 94, 107, 121, 137, 155, 175, 188 };
	// E[density] = 24, exit_slot_max = 188

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_95[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 58, 66, 74, 83, 94, 106, 120, 136, 154, 174, 190 };
	// E[density] = 24, exit_slot_max = 190

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_96[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 58, 65, 74, 83, 94, 106, 120, 136, 153, 174, 192 };
	// E[density] = 24, exit_slot_max = 192

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_97[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 52, 58, 65, 74, 83, 94, 106, 120, 135, 153, 173, 194 };
	// E[density] = 25, exit_slot_max = 194

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_98[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73, 83, 94, 106, 119, 135, 152, 172, 195, 196 };
	// E[density] = 25, exit_slot_max = 196

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_99[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73, 83, 93, 105, 119, 134, 152, 172, 194, 198 };
	// E[density] = 25, exit_slot_max = 198

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_100[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73, 83, 93, 105, 119, 134, 151, 171, 194, 200 };
	// E[density] = 25, exit_slot_max = 200

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_101[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73, 82, 93, 105, 118, 134, 151, 171, 193, 202 };
	// E[density] = 25, exit_slot_max = 202

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_102[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73, 82, 93, 105, 118, 133, 150, 170, 192, 204 };
	// E[density] = 25, exit_slot_max = 204

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_103[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73, 82, 92, 104, 118, 133, 150, 169, 191, 206 };
	// E[density] = 26, exit_slot_max = 206

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_104[] =
	{ 11, 14, 16, 18, 20, 23, 26, 29, 32, 36, 41, 45, 51, 57, 65, 73, 82, 92, 104, 117, 132, 150, 169, 191, 208 };
	// E[density] = 26, exit_slot_max = 208

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_105[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 41, 45, 51, 57, 64, 73, 82, 92, 104, 117, 132, 149, 168, 190, 210 };
	// E[density] = 26, exit_slot_max = 210

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_106[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 82, 92, 104, 117, 132, 149, 168, 189, 212 };
	// E[density] = 26, exit_slot_max = 212

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_107[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 92, 103, 116, 131, 148, 167, 189, 213, 214 };
	// E[density] = 26, exit_slot_max = 214

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_108[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 91, 103, 116, 131, 148, 167, 188, 213, 216 };
	// E[density] = 27, exit_slot_max = 216

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_109[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 91, 103, 116, 131, 147, 166, 188, 212, 218 };
	// E[density] = 27, exit_slot_max = 218

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_110[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 91, 103, 116, 130, 147, 166, 187, 211, 220 };
	// E[density] = 27, exit_slot_max = 220

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_111[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 91, 102, 115, 130, 146, 165, 186, 210, 222 };
	// E[density] = 27, exit_slot_max = 222

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_112[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 91, 102, 115, 130, 146, 165, 186, 210, 224 };
	// E[density] = 27, exit_slot_max = 224

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_113[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 81, 91, 102, 115, 129, 146, 164, 185, 209, 226 };
	// E[density] = 27, exit_slot_max = 226

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_114[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 72, 80, 90, 102, 115, 129, 145, 164, 185, 208, 228 };
	// E[density] = 28, exit_slot_max = 228

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_115[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 51, 57, 64, 71, 80, 90, 102, 114, 129, 145, 163, 184, 208, 230 };
	// E[density] = 28, exit_slot_max = 230

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_116[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 57, 64, 71, 80, 90, 101, 114, 128, 145, 163, 184, 207, 232 };
	// E[density] = 28, exit_slot_max = 232

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_117[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 57, 63, 71, 80, 90, 101, 114, 128, 144, 162, 183, 206, 233, 234 };
	// E[density] = 28, exit_slot_max = 234

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_118[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 80, 90, 101, 114, 128, 144, 162, 182, 206, 232, 236 };
	// E[density] = 28, exit_slot_max = 236

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_119[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 80, 90, 101, 113, 127, 143, 162, 182, 205, 231, 238 };
	// E[density] = 28, exit_slot_max = 238

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_120[] =
	{ 11, 15, 16, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 80, 89, 101, 113, 127, 143, 161, 181, 204, 230, 240 };
	// E[density] = 29, exit_slot_max = 240

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_121[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 79, 89, 100, 113, 127, 143, 161, 181, 204, 230, 242 };
	// E[density] = 29, exit_slot_max = 242

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_122[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 79, 89, 100, 113, 127, 142, 160, 180, 203, 229, 244 };
	// E[density] = 29, exit_slot_max = 244

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_123[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 79, 89, 100, 112, 126, 142, 160, 180, 203, 228, 246 };
	// E[density] = 29, exit_slot_max = 246

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_124[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 71, 79, 89, 100, 112, 126, 142, 159, 179, 202, 227, 248 };
	// E[density] = 29, exit_slot_max = 248

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_125[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 70, 79, 89, 100, 112, 126, 141, 159, 179, 201, 227, 250 };
	// E[density] = 29, exit_slot_max = 250

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_126[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 70, 79, 89, 99, 112, 125, 141, 159, 178, 201, 226, 252 };
	// E[density] = 30, exit_slot_max = 252

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_127[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 70, 79, 88, 99, 111, 125, 141, 158, 178, 200, 225, 254 };
	// E[density] = 30, exit_slot_max = 254

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_128[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 70, 79, 88, 99, 111, 125, 140, 158, 178, 200, 225, 253, 256 };
	// E[density] = 30, exit_slot_max = 256

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_129[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 63, 70, 79, 88, 99, 111, 125, 140, 157, 177, 199, 224, 252, 258 };
	// E[density] = 30, exit_slot_max = 258

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_130[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 62, 70, 78, 88, 99, 111, 124, 140, 157, 177, 199, 223, 251, 260 };
	// E[density] = 30, exit_slot_max = 260

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_131[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 62, 70, 78, 88, 99, 111, 124, 139, 157, 176, 198, 223, 251, 262 };
	// E[density] = 30, exit_slot_max = 262

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_132[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 62, 70, 78, 88, 98, 110, 124, 139, 156, 176, 198, 222, 250, 264 };
	// E[density] = 31, exit_slot_max = 264

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_133[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 62, 70, 78, 88, 98, 110, 124, 139, 156, 175, 197, 222, 249, 266 };
	// E[density] = 31, exit_slot_max = 266

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_134[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 62, 70, 78, 87, 98, 110, 123, 139, 156, 175, 197, 221, 248, 268 };
	// E[density] = 31, exit_slot_max = 268

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_135[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 45, 50, 56, 62, 70, 78, 87, 98, 110, 123, 138, 155, 174, 196, 220, 248, 270 };
	// E[density] = 31, exit_slot_max = 270

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_136[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 44, 50, 56, 62, 70, 78, 87, 98, 110, 123, 138, 155, 174, 196, 220, 247, 272 };
	// E[density] = 31, exit_slot_max = 272

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_137[] =
	{ 11, 15, 17, 18, 20, 23, 26, 29, 32, 36, 40, 44, 50, 55, 62, 69, 78, 87, 98, 109, 123, 138, 155, 174, 195, 219, 246, 274 };
	// E[density] = 31, exit_slot_max = 274

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_138[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 50, 55, 62, 69, 78, 87, 97, 109, 123, 137, 154, 173, 195, 219, 246, 276 };
	// E[density] = 31, exit_slot_max = 276

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_139[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 50, 55, 62, 69, 78, 87, 97, 109, 122, 137, 154, 173, 194, 218, 245, 275, 278 };
	// E[density] = 32, exit_slot_max = 278

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_140[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 50, 55, 62, 69, 77, 87, 97, 109, 122, 137, 154, 172, 194, 217, 244, 275, 280 };
	// E[density] = 32, exit_slot_max = 280

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_141[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 50, 55, 62, 69, 77, 87, 97, 109, 122, 137, 153, 172, 193, 217, 244, 274, 282 };
	// E[density] = 32, exit_slot_max = 282

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_142[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 62, 69, 77, 86, 97, 108, 122, 136, 153, 172, 193, 216, 243, 273, 284 };
	// E[density] = 32, exit_slot_max = 284

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_143[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 62, 69, 77, 86, 97, 108, 121, 136, 153, 171, 192, 216, 242, 272, 286 };
	// E[density] = 32, exit_slot_max = 286

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_144[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 62, 69, 77, 86, 97, 108, 121, 136, 152, 171, 192, 215, 242, 271, 288 };
	// E[density] = 32, exit_slot_max = 288

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_145[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 62, 69, 77, 86, 96, 108, 121, 136, 152, 171, 191, 215, 241, 271, 290 };
	// E[density] = 33, exit_slot_max = 290

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_146[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 62, 69, 77, 86, 96, 108, 121, 135, 152, 170, 191, 214, 240, 270, 292 };
	// E[density] = 33, exit_slot_max = 292

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_147[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 69, 77, 86, 96, 108, 121, 135, 151, 170, 190, 214, 240, 269, 294 };
	// E[density] = 33, exit_slot_max = 294

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_148[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 69, 77, 86, 96, 107, 120, 135, 151, 169, 190, 213, 239, 269, 296 };
	// E[density] = 33, exit_slot_max = 296

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_149[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 69, 77, 86, 96, 107, 120, 135, 151, 169, 190, 213, 239, 268, 298 };
	// E[density] = 33, exit_slot_max = 298

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_150[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 77, 86, 96, 107, 120, 134, 151, 169, 189, 212, 238, 267, 300 };
	// E[density] = 33, exit_slot_max = 300

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_151[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 96, 107, 120, 134, 150, 168, 189, 212, 237, 266, 299, 302 };
	// E[density] = 34, exit_slot_max = 302

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_152[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 107, 120, 134, 150, 168, 188, 211, 237, 266, 298, 304 };
	// E[density] = 34, exit_slot_max = 304

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_153[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 107, 119, 134, 150, 168, 188, 211, 236, 265, 297, 306 };
	// E[density] = 34, exit_slot_max = 306

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_154[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 106, 119, 133, 149, 167, 188, 210, 236, 264, 297, 308 };
	// E[density] = 34, exit_slot_max = 308

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_155[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 106, 119, 133, 149, 167, 187, 210, 235, 264, 296, 310 };
	// E[density] = 34, exit_slot_max = 310

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_156[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 106, 119, 133, 149, 167, 187, 209, 235, 263, 295, 312 };
	// E[density] = 34, exit_slot_max = 312

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_157[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 106, 119, 133, 149, 166, 186, 209, 234, 262, 294, 314 };
	// E[density] = 34, exit_slot_max = 314

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_158[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 106, 118, 133, 148, 166, 186, 208, 234, 262, 294, 316 };
	// E[density] = 35, exit_slot_max = 316

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_159[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 95, 106, 118, 132, 148, 166, 186, 208, 233, 261, 293, 318 };
	// E[density] = 35, exit_slot_max = 318

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_160[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85, 94, 106, 118, 132, 148, 165, 185, 208, 233, 261, 292, 320 };
	// E[density] = 35, exit_slot_max = 320

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_161[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 36, 39, 44, 49, 55, 61, 68, 76, 84, 94, 105, 118, 132, 148, 165, 185, 207, 232, 260, 291, 322 };
	// E[density] = 35, exit_slot_max = 322

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_162[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 61, 68, 76, 84, 94, 105, 118, 132, 147, 165, 185, 207, 232, 259, 291, 324 };
	// E[density] = 35, exit_slot_max = 324

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_163[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 61, 68, 75, 84, 94, 105, 118, 131, 147, 165, 184, 206, 231, 259, 290, 325, 326 };
	// E[density] = 35, exit_slot_max = 326

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_164[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 61, 68, 75, 84, 94, 105, 117, 131, 147, 164, 184, 206, 231, 258, 289, 324, 328 };
	// E[density] = 36, exit_slot_max = 328

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_165[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 61, 68, 75, 84, 94, 105, 117, 131, 147, 164, 184, 205, 230, 258, 289, 323, 330 };
	// E[density] = 36, exit_slot_max = 330

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_166[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 61, 67, 75, 84, 94, 105, 117, 131, 146, 164, 183, 205, 230, 257, 288, 322, 332 };
	// E[density] = 36, exit_slot_max = 332

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_167[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 84, 94, 105, 117, 131, 146, 163, 183, 205, 229, 256, 287, 322, 334 };
	// E[density] = 36, exit_slot_max = 334

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_168[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 84, 94, 104, 117, 130, 146, 163, 182, 204, 229, 256, 287, 321, 336 };
	// E[density] = 36, exit_slot_max = 336

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_169[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 84, 93, 104, 117, 130, 146, 163, 182, 204, 228, 255, 286, 320, 338 };
	// E[density] = 36, exit_slot_max = 338

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_170[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 84, 93, 104, 116, 130, 145, 163, 182, 203, 228, 255, 285, 319, 340 };
	// E[density] = 36, exit_slot_max = 340

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_171[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 84, 93, 104, 116, 130, 145, 162, 181, 203, 227, 254, 285, 319, 342 };
	// E[density] = 37, exit_slot_max = 342

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_172[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 83, 93, 104, 116, 130, 145, 162, 181, 203, 227, 254, 284, 318, 344 };
	// E[density] = 37, exit_slot_max = 344

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_173[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 83, 93, 104, 116, 129, 145, 162, 181, 202, 226, 253, 283, 317, 346 };
	// E[density] = 37, exit_slot_max = 346

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_174[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 83, 93, 104, 116, 129, 144, 161, 180, 202, 226, 253, 283, 316, 348 };
	// E[density] = 37, exit_slot_max = 348

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_175[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 83, 93, 104, 116, 129, 144, 161, 180, 201, 225, 252, 282, 316, 350 };
	// E[density] = 37, exit_slot_max = 350

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_176[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 83, 93, 103, 115, 129, 144, 161, 180, 201, 225, 252, 281, 315, 352 };
	// E[density] = 37, exit_slot_max = 352

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_177[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 75, 83, 93, 103, 115, 129, 144, 161, 180, 201, 224, 251, 281, 314, 352, 354 };
	// E[density] = 37, exit_slot_max = 354

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_178[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 49, 54, 60, 67, 74, 83, 93, 103, 115, 129, 144, 160, 179, 200, 224, 251, 280, 313, 351, 356 };
	// E[density] = 38, exit_slot_max = 356

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_179[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 67, 74, 83, 92, 103, 115, 128, 143, 160, 179, 200, 224, 250, 280, 313, 350, 358 };
	// E[density] = 38, exit_slot_max = 358

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_180[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 67, 74, 83, 92, 103, 115, 128, 143, 160, 179, 200, 223, 249, 279, 312, 349, 360 };
	// E[density] = 38, exit_slot_max = 360

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_181[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 67, 74, 83, 92, 103, 115, 128, 143, 160, 178, 199, 223, 249, 278, 311, 348, 362 };
	// E[density] = 38, exit_slot_max = 362

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_182[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 67, 74, 83, 92, 103, 115, 128, 143, 159, 178, 199, 222, 249, 278, 311, 347, 364 };
	// E[density] = 38, exit_slot_max = 364

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_183[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 67, 74, 83, 92, 103, 114, 128, 143, 159, 178, 199, 222, 248, 277, 310, 347, 366 };
	// E[density] = 38, exit_slot_max = 366

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_184[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 67, 74, 82, 92, 102, 114, 128, 142, 159, 177, 198, 221, 248, 277, 309, 346, 368 };
	// E[density] = 38, exit_slot_max = 368

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_185[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 66, 74, 82, 92, 102, 114, 127, 142, 159, 177, 198, 221, 247, 276, 309, 345, 370 };
	// E[density] = 39, exit_slot_max = 370

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_186[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 66, 74, 82, 92, 102, 114, 127, 142, 158, 177, 198, 221, 247, 276, 308, 344, 372 };
	// E[density] = 39, exit_slot_max = 372

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_187[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 66, 74, 82, 92, 102, 114, 127, 142, 158, 177, 197, 220, 246, 275, 307, 344, 374 };
	// E[density] = 39, exit_slot_max = 374

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_188[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 44, 48, 54, 60, 66, 74, 82, 92, 102, 114, 127, 142, 158, 176, 197, 220, 246, 274, 307, 343, 376 };
	// E[density] = 39, exit_slot_max = 376

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_189[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 43, 48, 54, 60, 66, 74, 82, 91, 102, 114, 127, 141, 158, 176, 197, 219, 245, 274, 306, 342, 378 };
	// E[density] = 39, exit_slot_max = 378

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_190[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 43, 48, 54, 60, 66, 74, 82, 91, 102, 113, 127, 141, 157, 176, 196, 219, 245, 273, 305, 341, 380 };
	// E[density] = 39, exit_slot_max = 380

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_191[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 43, 48, 54, 60, 66, 74, 82, 91, 102, 113, 126, 141, 157, 175, 196, 219, 244, 273, 305, 341, 381, 382 };
	// E[density] = 40, exit_slot_max = 382

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_192[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 43, 48, 54, 60, 66, 74, 82, 91, 102, 113, 126, 141, 157, 175, 196, 218, 244, 272, 304, 340, 380, 384 };
	// E[density] = 40, exit_slot_max = 384

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_193[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 43, 48, 54, 59, 66, 74, 82, 91, 101, 113, 126, 141, 157, 175, 195, 218, 243, 272, 304, 339, 379, 386 };
	// E[density] = 40, exit_slot_max = 386

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_194[] =
	{ 11, 15, 17, 18, 21, 23, 26, 29, 32, 35, 39, 43, 48, 54, 59, 66, 73, 82, 91, 101, 113, 126, 140, 157, 175, 195, 218, 243, 271, 303, 338, 378, 388 };
	// E[density] = 40, exit_slot_max = 388

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_195[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 82, 91, 101, 113, 126, 140, 156, 174, 195, 217, 243, 271, 302, 338, 377, 390 };
	// E[density] = 40, exit_slot_max = 390

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_196[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 82, 91, 101, 113, 126, 140, 156, 174, 194, 217, 242, 270, 302, 337, 377, 392 };
	// E[density] = 40, exit_slot_max = 392

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_197[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 82, 91, 101, 113, 125, 140, 156, 174, 194, 217, 242, 270, 301, 336, 376, 394 };
	// E[density] = 40, exit_slot_max = 394

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_198[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 91, 101, 112, 125, 140, 156, 174, 194, 216, 241, 269, 301, 336, 375, 396 };
	// E[density] = 41, exit_slot_max = 396

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_199[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 91, 101, 112, 125, 139, 155, 173, 193, 216, 241, 269, 300, 335, 374, 398 };
	// E[density] = 41, exit_slot_max = 398

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_200[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 91, 101, 112, 125, 139, 155, 173, 193, 215, 240, 268, 299, 334, 373, 400 };
	// E[density] = 41, exit_slot_max = 400

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_201[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 101, 112, 125, 139, 155, 173, 193, 215, 240, 268, 299, 334, 373, 402 };
	// E[density] = 41, exit_slot_max = 402

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_202[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 101, 112, 125, 139, 155, 173, 193, 215, 240, 267, 298, 333, 372, 404 };
	// E[density] = 41, exit_slot_max = 404

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_203[] =
	{ 11, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 100, 112, 125, 139, 155, 172, 192, 214, 239, 267, 298, 332, 371, 406 };
	// E[density] = 41, exit_slot_max = 406

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_204[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 100, 112, 124, 139, 154, 172, 192, 214, 239, 266, 297, 332, 370, 408 };
	// E[density] = 41, exit_slot_max = 408

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_205[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 100, 112, 124, 138, 154, 172, 192, 214, 238, 266, 297, 331, 370, 410 };
	// E[density] = 42, exit_slot_max = 410

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_206[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 100, 111, 124, 138, 154, 172, 191, 213, 238, 265, 296, 330, 369, 412 };
	// E[density] = 42, exit_slot_max = 412

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_207[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 66, 73, 81, 90, 100, 111, 124, 138, 154, 171, 191, 213, 238, 265, 296, 330, 368, 411, 414 };
	// E[density] = 42, exit_slot_max = 414

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_208[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 73, 81, 90, 100, 111, 124, 138, 154, 171, 191, 213, 237, 265, 295, 329, 367, 410, 416 };
	// E[density] = 42, exit_slot_max = 416

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_209[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 73, 81, 90, 100, 111, 124, 138, 153, 171, 191, 212, 237, 264, 295, 329, 367, 409, 418 };
	// E[density] = 42, exit_slot_max = 418

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_210[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 73, 81, 90, 100, 111, 124, 138, 153, 171, 190, 212, 236, 264, 294, 328, 366, 408, 420 };
	// E[density] = 42, exit_slot_max = 420

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_211[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 73, 81, 90, 100, 111, 123, 137, 153, 170, 190, 212, 236, 263, 293, 327, 365, 407, 422 };
	// E[density] = 42, exit_slot_max = 422

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_212[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 73, 81, 90, 100, 111, 123, 137, 153, 170, 190, 211, 236, 263, 293, 327, 364, 407, 424 };
	// E[density] = 43, exit_slot_max = 424

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_213[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 73, 81, 90, 100, 111, 123, 137, 153, 170, 189, 211, 235, 262, 292, 326, 364, 406, 426 };
	// E[density] = 43, exit_slot_max = 426

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_214[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 111, 123, 137, 152, 170, 189, 211, 235, 262, 292, 326, 363, 405, 428 };
	// E[density] = 43, exit_slot_max = 428

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_215[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 123, 137, 152, 170, 189, 210, 235, 261, 291, 325, 362, 404, 430 };
	// E[density] = 43, exit_slot_max = 430

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_216[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 123, 137, 152, 169, 189, 210, 234, 261, 291, 324, 362, 403, 432 };
	// E[density] = 43, exit_slot_max = 432

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_217[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 123, 136, 152, 169, 188, 210, 234, 261, 290, 324, 361, 403, 434 };
	// E[density] = 43, exit_slot_max = 434

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_218[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 123, 136, 152, 169, 188, 210, 233, 260, 290, 323, 360, 402, 436 };
	// E[density] = 43, exit_slot_max = 436

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_219[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 122, 136, 152, 169, 188, 209, 233, 260, 289, 323, 360, 401, 438 };
	// E[density] = 43, exit_slot_max = 438

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_220[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 122, 136, 151, 168, 188, 209, 233, 259, 289, 322, 359, 400, 440 };
	// E[density] = 44, exit_slot_max = 440

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_221[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 122, 136, 151, 168, 187, 209, 232, 259, 288, 322, 358, 399, 442 };
	// E[density] = 44, exit_slot_max = 442

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_222[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 122, 136, 151, 168, 187, 208, 232, 259, 288, 321, 358, 399, 444 };
	// E[density] = 44, exit_slot_max = 444

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_223[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 122, 136, 151, 168, 187, 208, 232, 258, 288, 320, 357, 398, 444, 446 };
	// E[density] = 44, exit_slot_max = 446

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_224[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 99, 110, 122, 135, 151, 168, 187, 208, 231, 258, 287, 320, 356, 397, 443, 448 };
	// E[density] = 44, exit_slot_max = 448

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_225[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 59, 65, 72, 80, 89, 98, 109, 122, 135, 150, 167, 186, 207, 231, 257, 287, 319, 356, 396, 442, 450 };
	// E[density] = 44, exit_slot_max = 450

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_226[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 80, 89, 98, 109, 122, 135, 150, 167, 186, 207, 231, 257, 286, 319, 355, 396, 441, 452 };
	// E[density] = 44, exit_slot_max = 452

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_227[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 80, 89, 98, 109, 121, 135, 150, 167, 186, 207, 230, 257, 286, 318, 355, 395, 440, 454 };
	// E[density] = 45, exit_slot_max = 454

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_228[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 80, 88, 98, 109, 121, 135, 150, 167, 186, 207, 230, 256, 285, 318, 354, 394, 439, 456 };
	// E[density] = 45, exit_slot_max = 456

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_229[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 80, 88, 98, 109, 121, 135, 150, 167, 185, 206, 230, 256, 285, 317, 353, 394, 439, 458 };
	// E[density] = 45, exit_slot_max = 458

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_230[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 80, 88, 98, 109, 121, 135, 150, 166, 185, 206, 229, 255, 284, 317, 353, 393, 438, 460 };
	// E[density] = 45, exit_slot_max = 460

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_231[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 80, 88, 98, 109, 121, 134, 149, 166, 185, 206, 229, 255, 284, 316, 352, 392, 437, 462 };
	// E[density] = 45, exit_slot_max = 462

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_232[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 79, 88, 98, 109, 121, 134, 149, 166, 185, 206, 229, 255, 283, 316, 351, 391, 436, 464 };
	// E[density] = 45, exit_slot_max = 464

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_233[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 79, 88, 98, 109, 121, 134, 149, 166, 184, 205, 228, 254, 283, 315, 351, 391, 435, 466 };
	// E[density] = 45, exit_slot_max = 466

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_234[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 79, 88, 98, 109, 121, 134, 149, 166, 184, 205, 228, 254, 283, 315, 350, 390, 434, 468 };
	// E[density] = 46, exit_slot_max = 468

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_235[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 72, 79, 88, 98, 108, 120, 134, 149, 165, 184, 205, 228, 253, 282, 314, 350, 389, 434, 470 };
	// E[density] = 46, exit_slot_max = 470

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_236[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 65, 71, 79, 88, 98, 108, 120, 134, 149, 165, 184, 204, 227, 253, 282, 314, 349, 389, 433, 472 };
	// E[density] = 46, exit_slot_max = 472

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_237[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 48, 53, 58, 64, 71, 79, 88, 98, 108, 120, 134, 148, 165, 184, 204, 227, 253, 281, 313, 349, 388, 432, 474 };
	// E[density] = 46, exit_slot_max = 474

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_238[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 53, 58, 64, 71, 79, 88, 97, 108, 120, 133, 148, 165, 183, 204, 227, 252, 281, 313, 348, 387, 431, 476 };
	// E[density] = 46, exit_slot_max = 476

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_239[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 53, 58, 64, 71, 79, 88, 97, 108, 120, 133, 148, 165, 183, 204, 227, 252, 280, 312, 347, 387, 431, 478 };
	// E[density] = 46, exit_slot_max = 478

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_240[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 53, 58, 64, 71, 79, 88, 97, 108, 120, 133, 148, 165, 183, 203, 226, 252, 280, 312, 347, 386, 430, 479, 480 };
	// E[density] = 46, exit_slot_max = 480

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_241[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 88, 97, 108, 120, 133, 148, 164, 183, 203, 226, 251, 280, 311, 346, 385, 429, 478, 482 };
	// E[density] = 46, exit_slot_max = 482

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_242[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 88, 97, 108, 120, 133, 148, 164, 182, 203, 226, 251, 279, 311, 346, 385, 428, 477, 484 };
	// E[density] = 47, exit_slot_max = 484

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_243[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 88, 97, 108, 120, 133, 148, 164, 182, 203, 225, 251, 279, 310, 345, 384, 428, 476, 486 };
	// E[density] = 47, exit_slot_max = 486

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_244[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 108, 120, 133, 147, 164, 182, 202, 225, 250, 278, 310, 345, 383, 427, 475, 488 };
	// E[density] = 47, exit_slot_max = 488

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_245[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 108, 119, 133, 147, 164, 182, 202, 225, 250, 278, 309, 344, 383, 426, 474, 490 };
	// E[density] = 47, exit_slot_max = 490

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_246[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 107, 119, 132, 147, 163, 182, 202, 224, 250, 278, 309, 344, 382, 425, 473, 492 };
	// E[density] = 47, exit_slot_max = 492

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_247[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 107, 119, 132, 147, 163, 181, 202, 224, 249, 277, 308, 343, 382, 425, 473, 494 };
	// E[density] = 47, exit_slot_max = 494

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_248[] =
	{ 12, 15, 17, 19, 21, 23, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 107, 119, 132, 147, 163, 181, 201, 224, 249, 277, 308, 342, 381, 424, 472, 496 };
	// E[density] = 47, exit_slot_max = 496

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_249[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 107, 119, 132, 147, 163, 181, 201, 224, 249, 276, 307, 342, 380, 423, 471, 498 };
	// E[density] = 48, exit_slot_max = 498

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_250[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 107, 119, 132, 147, 163, 181, 201, 223, 248, 276, 307, 341, 380, 422, 470, 500 };
	// E[density] = 48, exit_slot_max = 500

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_251[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 97, 107, 119, 132, 146, 163, 181, 201, 223, 248, 276, 306, 341, 379, 422, 469, 502 };
	// E[density] = 48, exit_slot_max = 502

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_252[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 79, 87, 96, 107, 119, 132, 146, 162, 180, 200, 223, 248, 275, 306, 340, 379, 421, 468, 504 };
	// E[density] = 48, exit_slot_max = 504

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_253[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 78, 87, 96, 107, 119, 132, 146, 162, 180, 200, 222, 247, 275, 306, 340, 378, 420, 468, 506 };
	// E[density] = 48, exit_slot_max = 506

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_254[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 78, 87, 96, 107, 118, 131, 146, 162, 180, 200, 222, 247, 274, 305, 339, 377, 420, 467, 508 };
	// E[density] = 48, exit_slot_max = 508

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_255[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 78, 87, 96, 107, 118, 131, 146, 162, 180, 200, 222, 247, 274, 305, 339, 377, 419, 466, 510 };
	// E[density] = 48, exit_slot_max = 510

const uint16_t MX_DISCOVERY_EXIT_SLOT_LUT_256[] =
	{ 12, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39, 43, 47, 52, 58, 64, 71, 78, 87, 96, 107, 118, 131, 146, 162, 180, 199, 222, 246, 274, 304, 338, 376, 418, 465, 512 };
	// E[density] = 49, exit_slot_max = 512

//**************************************************************************************************
//***** Local Functions ****************************************************************************



//**************************************************************************************************
//***** Global Functions ***************************************************************************



//**************************************************************************************************
//**************************************************************************************************
