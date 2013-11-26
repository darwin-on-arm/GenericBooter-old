/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "genboot.h"
#include <stdarg.h>

/* Uart stuff */

#define	SDRAM_BASE								0x80000000	// spruh73i.pdf, Page:170, EMIF0 SDRAM, up to 1G

#define UART0_BASE								0x44E09000	// spruh73i.pdf, Page:171
#define UART1_BASE								0x48022000	// spruh73i.pdf, Page:172
#define UART2_BASE								0x48024000	// spruh73i.pdf, Page:172
#define UART3_BASE								0x481A6000	// spruh73i.pdf, Page:174
#define UART4_BASE								0x481A8000	// spruh73i.pdf, Page:174
#define UART5_BASE								0x481AA000	// spruh73i.pdf, Page:174

#define AMBA_UART_DR(base)			(*(volatile uint16_t *)((base) + 0x00))
#define AMBA_UART_SSR(base)     	(*(volatile uint16_t *)((base) + 0x44)) // SSR Register, spruh73i.pdf, Page:4074

#define	UART_TX_IS_FULL(base)		(AMBA_UART_SSR(base) & 1)	// SSR Register, spruh73i.pdf, Page:4074
#define	UART_QUEUE_CHAR(base, c)	(AMBA_UART_DR(base) = c)

#define barrier()               __asm__ __volatile__("": : :"memory");

/**
 * uart_putc
 *
 * Put a character to the system console.
 */
uint32_t uart_base = UART0_BASE;
static int inited_printf = 1;
void uart_putchar(int c)
{
    if (!inited_printf)
        return;

    if (c == '\n')
        uart_putchar('\r');

    while (UART_TX_IS_FULL(uart_base)) {
        /* Transmit FIFO full, wait */
        barrier();
    }

	UART_QUEUE_CHAR(uart_base, c);
}

/**
 * uart_getc
 *
 * Get a character from system input.
 */
int uart_getchar(void)
{
    return 'X';
}

static void putc_wrapper(void *p, char c)
{
    uart_putchar(c);
}

/**
 * init_debug
 *
 * Start debugging subsystems.
 */
void init_debug(void)
{
    init_printf(NULL, putc_wrapper);
    printf("debug_init()\n");
}

void __assert_func(const char *file, int line, const char *method,
                   const char *expression)
{
    panic("Assertion failed in file %s, line %d. (%s)\n",
          file, line, expression);
    while (1) ;
}

/**
 * panic
 *
 * Halt the system and explain why.
 */
#undef panic
void panic(const char *panicStr, ...)
{
    void *caller = __builtin_return_address(0);

    /* Prologue */
    printf("panic(caller 0x%08x): ", caller);

    /* Epilogue */
    va_list valist;
    va_start(valist, panicStr);
    vprintf((char *)panicStr, valist);
    va_end(valist);

    /* We are hanging here. */
    printf("\npanic: we are hanging here...\n");

    /* Halt */
    _locore_halt_system();
}
