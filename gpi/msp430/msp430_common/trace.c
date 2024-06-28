/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2018 - 2019, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					gpi/msp430/trace.c
 *
 *	@brief					MSP430 TRACE implementation
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

#include "gpi/trace.h"

#if (GPI_TRACE_MODE & GPI_TRACE_MODE_TRACE)

//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/tools.h"
#include "gpi/clocks.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************



//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************



//**************************************************************************************************
//***** Forward Declarations ***********************************************************************



//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

static Gpi_Trace_Msg			s_msg_queue[GPI_TRACE_BUFFER_ELEMENTS];
static volatile unsigned int	s_msg_queue_num_written = 0;
static volatile unsigned int	s_msg_queue_num_writing = 0;
static volatile unsigned int	s_msg_queue_num_read = 0;

//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Local Functions ****************************************************************************
/*
static void dump(const void* p, unsigned int size)
{
	static const uint8_t	digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	const uint8_t*			pc = p;

	while (size--)
	{
		while (!(IFG2 & UTXIFG1));
		U1TXBUF = digits[*pc >> 4];

		while (!(IFG2 & UTXIFG1));
		U1TXBUF = digits[*pc++ & 0x0F];
	}
}
*/
//**************************************************************************************************

#if GPI_TRACE_USE_DSR

// ATTENTION: has to be called < 0x10000 fast ticks after gpi_trace_store_msg()
// (such that gpi_tick_fast_to_hybrid() works right)
void __attribute__((interrupt(GPI_TRACE_DSR_VECTOR))) gpi_trace_isr()
{
	static unsigned int	s_num_read = 0;
	uint16_t			num_written;

	// mask IRQ
	gpi_trace_mask_dsr();

	do
	{
		Gpi_Trace_Msg	*msg, *msg_end;
		uint16_t		num_read;
		uint16_t		ts_fast;
		uint32_t		ts_hybrid;

		// enable interrupts -> allow nesting of other interrupts
		gpi_int_enable();

		// note: we use our own num_read value and expect to be always in front of s_msg_queue_num_read
		num_written = s_msg_queue_num_written;
		num_read = s_num_read;

#if !GPI_TRACE_OVERFLOW_ON_WRITE
		if (num_written - num_read > NUM_ELEMENTS(s_msg_queue))
			num_read = num_written - NUM_ELEMENTS(s_msg_queue);
		// attention: to be absolutely safe that we don't end up with corrupt timestamps if
		// overflows occur, we should do overflow handling in this routine the full, clean way
		// (i.e. with locked write access and checking num_written before). However, we forgo this
		// topic in favor of performance because we expect that TRACE overflows are seen as
		// critical issues in typical applications and hence should be avoided completely by
		// choosing a big enough queue size.
#endif

		// avoid msg_end == msg in case of full queue
		// note: we process the remaining msg in a subsequent loop iteration
		if (num_written - num_read >= NUM_ELEMENTS(s_msg_queue))
			num_written = num_read + NUM_ELEMENTS(s_msg_queue) - 1;

		msg = &s_msg_queue[num_read % NUM_ELEMENTS(s_msg_queue)];
		msg_end = &s_msg_queue[num_written % NUM_ELEMENTS(s_msg_queue)];

/*		// skip already processed messages
		while (msg != msg_end)
		{
			if (!((uint16_t)(msg->timestamp) & 1))
				break;

			if (++msg >= &s_msg_queue[NUM_ELEMENTS(s_msg_queue)])
				msg = &s_msg_queue[0];
		}
*/
		if (msg != msg_end)
		{
			// convert timestamp of first message
			ts_fast = msg->timestamp;
			ts_hybrid = gpi_tick_fast_to_hybrid(ts_fast);
			msg->timestamp = ts_hybrid | 1;

			if (++msg >= &s_msg_queue[NUM_ELEMENTS(s_msg_queue)])
				msg = &s_msg_queue[0];

			// convert subsequent messages in an efficient way
			// note: we assume that they arrived < 0x10000 fast ticks after the first message
			while (msg != msg_end)
			{
				// for some reason, compiler (msp430-gcc 4.6.3) significantly wastes registers in
				// the generated code for the following two lines. Hence we help it with handwritten
				// assembly code.
				// msg->timestamp = ts_hybrid + ((uint16_t)(msg->timestamp) - ts_fast) / (GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE);
				// msg->timestamp &= ~1ul;
				ASSERT_CT(GPI_FAST_CLOCK_RATE / GPI_HYBRID_CLOCK_RATE == 1, assembly_code_needs_update);
				__asm__
				(
					"sub	%1,	%A0		\n"
					"clr	%B0			\n"
					"add	%A2, %A0	\n"
					"addc	%B2, %B0	\n"
					"bis	#1, %A0		\n"
					: "+r"(msg->timestamp)
					: "r"(ts_fast), "r"(ts_hybrid)
					: "cc"
				);

				if (++msg >= &s_msg_queue[NUM_ELEMENTS(s_msg_queue)])
					msg = &s_msg_queue[0];
			}

			s_num_read = num_written;
		}

		// disable interrupts
		gpi_int_disable();

		// if additional msgs arrived in between: reiterate
	}
	while (num_written != s_msg_queue_num_written);

	// enable next IRQ triggering
	gpi_trace_unmask_dsr();
}

#endif	// GPI_TRACE_USE_DSR

//**************************************************************************************************
//***** Global Functions ***************************************************************************

void gpi_trace_store_msg(const char* fmt, ...)
{
	ASSERT_CT(sizeof(Gpi_Fast_Tick_Native) == sizeof(uint16_t));
	ASSERT_CT(sizeof(Gpi_Hybrid_Tick) == sizeof(uint32_t));

	unsigned int	num_writing;
	Gpi_Trace_Msg*	msg;
	int				ie;
	uint16_t		timestamp;

	ie = gpi_int_lock();
	REORDER_BARRIER();

#if GPI_TRACE_OVERFLOW_ON_WRITE
	static unsigned int	s_num_lost = 0;
	unsigned int		num_lost = 0;

	if (s_msg_queue_num_writing - s_msg_queue_num_read >= NUM_ELEMENTS(s_msg_queue))
	{
		if (s_num_lost < -2u)
			s_num_lost++;

		num_lost = -1;
    }
	else
	{
#endif
		timestamp = gpi_tick_fast_native();
		num_writing = s_msg_queue_num_writing++;

#if GPI_TRACE_OVERFLOW_ON_WRITE
		num_lost = s_num_lost;
		s_num_lost = 0;
    }
#endif

	REORDER_BARRIER();
	gpi_int_unlock(ie);

#if GPI_TRACE_OVERFLOW_ON_WRITE
	if (-1u == num_lost)
		return;
#endif

	msg = &s_msg_queue[num_writing % NUM_ELEMENTS(s_msg_queue)];

#if GPI_TRACE_USE_DSR
	msg->timestamp = timestamp & ~1;
#else
	msg->timestamp = gpi_tick_fast_to_hybrid(timestamp) | 1;
#endif

#if GPI_TRACE_OVERFLOW_ON_WRITE
	if (num_lost > 0)
	{
		msg->msg = "!!! TRACE buffer overflow, %u message(s) lost !!!\n";
		msg->var_args[0] = ++num_lost;
    }
	else
#endif
	{
		msg->msg = fmt;

		va_list varargs;
		va_start(varargs, fmt);

		int *ps = varargs, *pd = &(msg->var_args[0]);

		// we do manual loop unrolling if beneficial because this function is time critical
		// note: gpi_memcpy_dma_aligned() needs 31 + sizeof(var_args) cycles
		if (NUM_ELEMENTS(msg->var_args) <= 7)
		{
			if (NUM_ELEMENTS(msg->var_args) >= 1)
				*pd++ = *ps++;
			if (NUM_ELEMENTS(msg->var_args) >= 2)
				*pd++ = *ps++;
			if (NUM_ELEMENTS(msg->var_args) >= 3)
				*pd++ = *ps++;
			if (NUM_ELEMENTS(msg->var_args) >= 4)
				*pd++ = *ps++;
			if (NUM_ELEMENTS(msg->var_args) >= 5)
				*pd++ = *ps++;
			if (NUM_ELEMENTS(msg->var_args) >= 6)
				*pd++ = *ps++;
			if (NUM_ELEMENTS(msg->var_args) >= 7)
				*pd++ = *ps++;
        }
		else
		{
			gpi_memcpy_dma_aligned(msg->var_args, varargs, sizeof(msg->var_args));
        }

		va_end(varargs);
    }

	ie = gpi_int_lock();
	REORDER_BARRIER();

	if (s_msg_queue_num_written == num_writing)
	{
		s_msg_queue_num_written = s_msg_queue_num_writing;

#if GPI_TRACE_USE_DSR
		// trigger postprocessing IRQ
		gpi_trace_trigger_dsr();
#endif
    }

	REORDER_BARRIER();
	gpi_int_unlock(ie);
}

//**************************************************************************************************

void gpi_trace_print_all_msgs()
{
	unsigned int	num_read;
	Gpi_Trace_Msg*	msg;

	num_read = s_msg_queue_num_read;

	if (num_read == s_msg_queue_num_written)
		return;

#define CATCH_NESTED_CALLS	1
#if CATCH_NESTED_CALLS
	static volatile void*	s_outer_call = NULL;
	{
		register void*	return_address;
		register int	ie;

		return_address = __builtin_return_address(0);

		ie = gpi_int_lock();

		if (NULL != s_outer_call)
		{
			printf("\n!!! PANIC: nested (non-fast) TRACE call !!!\n");
			printf("called from %p, outer call from %p\n", return_address, s_outer_call);
			printf("system halted\n");

			// would be nice to print stack trace and enter minimalistic debug console here

			while (1);
		}

		s_outer_call = return_address;

		gpi_int_unlock(ie);
	}
#endif

#if !GPI_TRACE_OVERFLOW_ON_WRITE
	static Gpi_Trace_Msg	s_msg;
	msg = &s_msg;
#endif

	while (num_read != s_msg_queue_num_written)
	{
#if !GPI_TRACE_OVERFLOW_ON_WRITE
		gpi_memcpy_dma_aligned(msg, &s_msg_queue[num_read % NUM_ELEMENTS(s_msg_queue)], sizeof(Gpi_Trace_Msg));

		unsigned int num_open = s_msg_queue_num_writing - num_read;
		if (num_open > NUM_ELEMENTS(s_msg_queue))
		{
			num_open -= NUM_ELEMENTS(s_msg_queue);
			num_read += num_open;

			msg->msg = "!!! TRACE buffer overflow, %u message(s) lost !!!\n";
			msg->var_args[0] = num_open;
			msg->timestamp = 0;
        }
		else num_read++;

		s_msg_queue_num_read = num_read;
#else
		msg = &s_msg_queue[num_read % NUM_ELEMENTS(s_msg_queue)];
#endif
		int msg_type = -1;

		// parse internal header (see trace.h for details)
		while ('\b' == msg->msg[1])
		{
			switch (msg->msg[0])
			{
				// remove full path if requested
				// msg[2] = index of var_arg that contains __FILE__
				case 'A':
				{
					const char	**file = &(((const char**)&(msg->var_args))[msg->msg[2] - '0']);
					const char	*s = *file + strlen(*file);
					
					while (s-- != *file)
					{
						if ((*s == '/') || (*s == '\\'))
							break;
					}
					
					*file = ++s;
					
					msg->msg += 4;
					break;
                }
				
				// set message type
				case 'B':
				{
					msg_type = (unsigned char)(msg->msg[2] - '0');
					
					msg->msg += 4;
					break;
                }

				// skip command / arg				
				default:
					msg->msg += 2;
            }
        }

		if (msg_type >= 0)
		{
			static const char * const fmt[] = GPI_TRACE_TYPE_FORMAT_LUT;

			if (msg_type >= NUM_ELEMENTS(fmt))
				msg_type = 0;
						
			printf(fmt[msg_type]);
		}
		
		if (!(msg->timestamp & 1))
			printf("???.???.??? ");
		else
		{
			uint32_t	ts;
			uint16_t	s;

			ts = msg->timestamp;

			s = ts / GPI_HYBRID_CLOCK_RATE;
			ts %= GPI_HYBRID_CLOCK_RATE;
#if 0
			printf("%3u.%06lu ", s, gpi_tick_hybrid_to_us(ts));
#else
			uint16_t	ms, us;

			ts = gpi_mulu_32x16(ts, 1000);
			ms = ts / GPI_HYBRID_CLOCK_RATE;
			ts %= GPI_HYBRID_CLOCK_RATE;

			ts = gpi_mulu_32x16(ts, 1000);
			us = ts / GPI_HYBRID_CLOCK_RATE;

			printf("%3u.%03u.%03u ", s, ms, us);
#endif
        }

		vprintf(msg->msg, msg->var_args);

		if (msg_type >= 0)
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wformat-zero-length"
			
			printf(GPI_TRACE_TYPE_FORMAT_RESET);
			
			#pragma GCC diagnostic pop
        }

		printf("\n");

#if GPI_TRACE_OVERFLOW_ON_WRITE
		s_msg_queue_num_read = ++num_read;
#endif
    }

#if CATCH_NESTED_CALLS
	s_outer_call = NULL;
#endif
}

//**************************************************************************************************
//**************************************************************************************************

#endif // (TRACE_MODE & TRACE_MODE_TRACE)
