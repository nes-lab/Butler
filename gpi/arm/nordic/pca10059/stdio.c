/***************************************************************************************************
 ***************************************************************************************************
 *
 *	Copyright (c) 2021, Networked Embedded Systems Lab, TU Dresden
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
 *	@file					gpi/arm/nordic/pca10059/stdio.c
 *
 *	@brief					platform specific stdio implementation (CRT internal functions)
 *
 *	@version				$Id$
 *	@date					TODO
 *
 *	@author					Fabian Mager
 *
 ***************************************************************************************************

 	@details

	TODO

 **************************************************************************************************/
//***** Trace Settings *****************************************************************************



//**************************************************************************************************
//**** Includes ************************************************************************************

#include "gpi/tools.h"
#include "gpi/platform_spec.h"
#include "gpi/platform.h"
#include "gpi/resource_check.h"

#include <nrf.h>

#include <stdio.h>

GPI_RESOURCE_RESERVE_SHARED(NRF_UARTE, 0);

//**************************************************************************************************
//***** Local Defines and Consts *******************************************************************



//**************************************************************************************************
//***** Local Typedefs and Class Declarations ******************************************************



//**************************************************************************************************
//***** Forward Declarations ***********************************************************************



//**************************************************************************************************
//***** Local (Static) Variables *******************************************************************

// RAM area
// NOTE: do not used __RAM_segment_start__ and __RAM_segment_end__, as these symbols are
// specific for the build environment (SEGGER Embedded Studio with SEGGER Linker)
// NOTE: MBR occupies the first flash page (address 0 - 0xFFF) and also reserves the lowest 8 bytes
// of RAM (0x20000000 - 0x20000007).
static void * const RAM_SEGMENT_START = (void*)0x20000008;
static void * const RAM_SEGMENT_END   = (void*)0x20040000;

//**************************************************************************************************
//***** Global Variables ***************************************************************************



//**************************************************************************************************
//***** Local Functions ****************************************************************************

// core output function
// NOTE: inline to enable optimized usage inside the adapter functions (see below)
static inline void gpi_uart_write(const void *s, unsigned int len)
{
	// test if data is in RAM (EasyDMA cannot access flash)
	ASSERT(s >= (void*)RAM_SEGMENT_START && s < (void*)RAM_SEGMENT_END);

	if (len < 1)
		return;

	// flush registers before activating DMA
	// this could be relevant when function gets inlined and highly optimized
	REORDER_BARRIER();

	// setup DMA
	NRF_UARTE0->TXD.PTR = (uintptr_t)s;
	NRF_UARTE0->TXD.MAXCNT = len;

	// flush store buffer writes from CPU pipeline
	// NOTE: This is not really necessary here because it has been done implicitly for sure
	// due to the short pipeline length of the Cortex-M4. We do it anyway to keep the code clean.
	__DMB();

	// wait until previous transmission has finished
	// NOTE: TXSTARTED is used as a marker for open transmissions
	if (NRF_UARTE0->EVENTS_TXSTARTED)
	{
		NRF_UARTE0->EVENTS_TXSTARTED = 0;
		while (!(NRF_UARTE0->EVENTS_ENDTX));
	}

	// start TX
	NRF_UARTE0->EVENTS_ENDTX = 0;
	NRF_UARTE0->TASKS_STARTTX = 1;

	// wait until TX has been started and TXD.PTR and TXD.MAXCNT can be accessed again
	// NOTE: TXD.PTR and TXD.MAXCNT are double-buffered (see spec. 4413_417 v1.2 page 511)
	while (!(NRF_UARTE0->EVENTS_TXSTARTED));
}

//**************************************************************************************************
//***** Global Functions ***************************************************************************

// putchar() / puts()
// ATTENTION: the simple implementations are not reentrant

#if GPI_ARCH_IS_OS(NONE)

#if GPI_ARCH_IS_CRT(SEGGER2)

// The documentation of the SEGGER RTL says that the function to provide is __SEGGER_RTL_stdout_putc().
// However, we have not seen it (maybe it refers to a different version of the library), and
// inspecting the code reveals that putchar(), puts() etc. call the following function
// (if Library I/O is set to STD).
int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *stream, const char *s, unsigned len)
{
	static char			crlf[] = "\r\n";	// do not use const char, must be in RAM for sure
	const char* const	end = &s[len];
	const char			*r;
	unsigned int		l;

	// avoid "variable unused" warning
	(void)stream;

	// TXSTARTED is used as a marker for open transmissions in gpi_uart_write()
	NRF_UARTE0->EVENTS_TXSTARTED = 0;

	// if data is not in RAM, we must copy it because EasyDMA has no access to flash area
	if (s < (char*)RAM_SEGMENT_START || s >= (char*)RAM_SEGMENT_END)
	{
		char c;
		for (l = len; l-- > 0;)
		{
			c = *s++;
			if (c == '\n')
				gpi_uart_write(&crlf, 2);
			else gpi_uart_write(&c, 1);
		}
	}

	else
	{
		// split s into segments separated by \n
		for (r = s; r != end;)
		{
			for (; r != end; r++)
			{
				if (*r == '\n')
					break;
			}

			// write current segment
			l = (uintptr_t)r - (uintptr_t)s;
			if (l > 0)
				gpi_uart_write(s, l);

			// write newline sequence
			if (r != end)
			{
				gpi_uart_write(&crlf, 2);
				r++;
			}

			// next segment
			s = r;
		}
	}

	// wait until transmission has finished (so data buffer can be released for sure)
	// NOTE: TXSTARTED is used as a marker for open transmissions
	if (NRF_UARTE0->EVENTS_TXSTARTED)
		while (!(NRF_UARTE0->EVENTS_ENDTX));

	// return value seems undocumented so far,
	// we guess that it should be len or -1 in case of error
	return len;
}

#elif GPI_ARCH_IS_CRT(SEGGER1)

// The (old) RTL versions call __putchar(), with an additional proprietary parameter.
// NOTE: function is declared as weak to allow the application to provide a different
// implementation (e.g. from Segger RTT) without causing conflicts
// ATTENTION: thumb_crt0.s contains a weak definition of __putchar redirecting to debug_putchar.
// Hence, to ensure that putchar redirects here it is not safe to declare __putchar alone
// (if the definition shall be weak), as this would not safely overwrite the definition from
// thumb_crt0.s. Instead we provide (non-weak) debug_putchar() (to catch thumb_crt0.s) plus
// weak __putchar(). This is somewhat dirty as debug_putchar() is meant to be the low-level 
// debug output routine and should not be overwritten in general.

int __putchar(int c, __printf_tag_ptr file) __attribute__((weak, alias("debug_putchar")));

int debug_putchar(int c, __printf_tag_ptr file)
{
	uint8_t			buf[2];
	uint_fast8_t	len = 0;

	// avoid "variable unused" warning
	(void)file;

	// copy data to RAM buffer, convert "\n" to "\r\n"
	if (c == '\n')
		buf[len++] = '\r';
	buf[len++] = c;

	// TXSTARTED is used as a marker for open transmissions in gpi_uart_write()
	NRF_UARTE0->EVENTS_TXSTARTED = 0;

	gpi_uart_write(buf, len);

	// wait until transmission has finished
	while (!(NRF_UARTE0->EVENTS_ENDTX));

	return c;
}

#endif	// GPI_ARCH_IS_CRT(...)

#endif	// GPI_ARCH_IS_OS(NONE)

//**************************************************************************************************
// getchar()
// ATTENTION: implementations are very simple and not reentrant

#if GPI_ARCH_IS_OS(NONE)

void gpi_stdin_flush()
{
	uint8_t	t[8];

	NRF_UARTE0->RXD.PTR = (uintptr_t)&t;
	NRF_UARTE0->RXD.MAXCNT = 8;

	NRF_UARTE0->EVENTS_ENDRX = 0;
	NRF_UARTE0->TASKS_FLUSHRX = 1;
	while (!(NRF_UARTE0->EVENTS_ENDRX));
}

// NOTE: function is declared as weak to allow the application to provide a different
// implementation without causing conflicts
int __attribute__((weak)) getchar()
{
	uint8_t	c;

	NRF_UARTE0->RXD.PTR = (uintptr_t)&c;
	NRF_UARTE0->RXD.MAXCNT = 1;

	NRF_UARTE0->EVENTS_ENDRX = 0;
	NRF_UARTE0->TASKS_STARTRX = 1;
	while (!(NRF_UARTE0->EVENTS_ENDRX));

	return c;
}

// getsn()
#include "gpi/stdio_getsn.c"

#endif	// GPI_ARCH_IS_OS(NONE)

//**************************************************************************************************
//**************************************************************************************************
