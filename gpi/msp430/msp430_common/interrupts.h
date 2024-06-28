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
 *	@file					gpi/msp430/interrupts.h
 *
 *	@brief					basic interrupt handling
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

#ifndef __GPI_MSP430_INTERRUPTS_H__
#define __GPI_MSP430_INTERRUPTS_H__

//**************************************************************************************************
//***** Includes ***********************************************************************************



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



#ifdef __cplusplus
	}
#endif

//**************************************************************************************************
//***** Implementations of Inline Functions ********************************************************

// available intrinsics: __dint (-> dint, nop), __eint (-> eint)

static inline void	gpi_int_enable()	{ __eint();	}
static inline void	gpi_int_disable()	{ __dint();	}

//**************************************************************************************************

static inline __attribute__((always_inline)) int gpi_int_lock()
{
	register int r;

	REORDER_BARRIER();
	
	__asm__ volatile
	(
		"mov	r2, %0			\n"
		"dint					\n"
		"bic	#0xFFF7, %0		\n"
			// notice: replaces nop which is typically added after dint
			// notice: don't use "and #8, %0" since it is not faster but touches status bits
		: "=r"(r)
	);

	REORDER_BARRIER();
	
	return r;
}

//**************************************************************************************************

static inline __attribute__((always_inline)) void gpi_int_unlock(int r)
{
	REORDER_BARRIER();
	
	__asm__ volatile
	(
		// notice: we expect r as it has been returned by gpi_int_lock(). On one hand, we could
		// AND it here with 0x0008 for sure since side effects to SR bits are critical. On the
		// other hand, if r is damaged then we also don't know if bit 3 is still valid. So we
		// decide to expect the whole value to be untouched and save the extra instruction.
		"bis	%0, r2"
		:
		: "r"(r)
	);

	REORDER_BARRIER();
}

//**************************************************************************************************
//**************************************************************************************************

static inline void __attribute__((always_inline)) gpi_atomic_or(volatile unsigned int *p, unsigned int mask)
{
	__asm__ ("bis %1, %0" : "=m"(*p) : "g"(mask));
}

//**************************************************************************************************

static inline void __attribute__((always_inline)) gpi_atomic_and(volatile unsigned int *p, unsigned int mask)
{
	__asm__ ("and %1, %0" : "=m"(*p) : "g"(mask) : "cc");
}

//**************************************************************************************************

static inline void __attribute__((always_inline)) gpi_atomic_set(volatile unsigned int *p, unsigned int mask)
{
	gpi_atomic_or(p, mask);
}

//**************************************************************************************************

static inline void __attribute__((always_inline)) gpi_atomic_clear(volatile unsigned int *p, unsigned int mask)
{
	__asm__ ("bic %1, %0" : "=m"(*p) : "g"(mask));
}

//**************************************************************************************************
//**************************************************************************************************

static ALWAYS_INLINE void gpi_atomic_write_8(volatile uint8_t *p, uint8_t data)
{
	// NOTE: this function does not implement REORDER_BARRIERs

	__asm__ volatile ("mov.b %1, %0" : "=m"(*p) : "r"(data));
}

//**************************************************************************************************

static ALWAYS_INLINE void gpi_atomic_write_16(volatile uint16_t *p, uint16_t data)
{
	// NOTE: this function does not implement REORDER_BARRIERs

	__asm__ volatile ("mov.w %1, %0" : "=m"(*p) : "r"(data));
}

//**************************************************************************************************

static ALWAYS_INLINE void gpi_atomic_write_32(volatile uint32_t *p, uint32_t data)
{
	register int ie = gpi_int_lock();

	*p = data;
	
	gpi_int_unlock(ie);
}

//**************************************************************************************************

#define gpi_atomic_write(p, data)															\
	do {																					\
		ASSERT_CT(sizeof(*(p)) <= 4, gpi_atomic_write_size_overflow);						\
		switch (sizeof(*(p))) {																\
			case 1:	gpi_atomic_write_8((volatile uint8_t*)(p),   (uint8_t)(data));	break;	\
			case 2:	gpi_atomic_write_16((volatile uint16_t*)(p), (uint16_t)(data));	break;	\
			case 4:	gpi_atomic_write_32((volatile uint32_t*)(p), (uint32_t)(data));	break;	\
			default: assert(0);																\
		}																					\
	} while (0)

//**************************************************************************************************
//**************************************************************************************************

#endif // __GPI_MSP430_INTERRUPTS_H__
