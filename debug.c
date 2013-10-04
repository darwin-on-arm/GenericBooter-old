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
#define AMBA_UART_DR(base)      (*(volatile unsigned char *)((base) + 0x00))
#define AMBA_UART_LCRH(base)    (*(volatile unsigned char *)((base) + 0x2c))
#define AMBA_UART_CR(base)      (*(volatile unsigned char *)((base) + 0x30))
#define AMBA_UART_FR(base)      (*(volatile unsigned char *)((base) + 0x18))
#define REALVIEW_PBA8_SDRAM6_BASE               0x70000000  /* SDRAM bank 6 256MB */
#define REALVIEW_PBA8_SDRAM7_BASE               0x80000000  /* SDRAM bank 7 256MB */
#define REALVIEW_PBA8_UART0_BASE                0x10009000  /* UART 0 */

#define UART_FR_TXFE (1 << 7)
#define UART_FR_TXFF (1 << 5)

#define UART_FR_RXFE (1 << 4)
#define UART_FR_RXFF (1 << 6)

#define barrier()               __asm__ __volatile__("": : :"memory");

#define HwReg(x) *((volatile unsigned long*)(x))


/**
 * uart_putc
 *
 * Put a character to the system console.
 */
#define OMAP3_L4_PERIPH_BASE    0x49000000
#define OMAP3_UART_BASE         (OMAP3_L4_PERIPH_BASE + 0x20000)    // This is uart2
uint32_t gOmapSerialUartBase = OMAP3_UART_BASE;

#define LSR_DR          0x01                /* Data ready */
#define LSR_THRE        0x20                /* Xmit holding register empty */

#define THR     RBR
#define DLL     RBR
#define DLM     IER

/*
 * Note, on older OMAP platforms, the size of the NS16550 UARTS
 * is different, for this one it's 32-bit.
 */

#define RBR     0x0
#define IER     0x4
#define FCR     0x8
#define LCR     0xC
#define MCR     0x10
#define LSR     0x14
#define MSR     0x18
#define SCR     0x1C

static int inited_printf = 1;
void uart_putchar(int c)
{
    if (!inited_printf)
        return;

    if (c == '\n')
        uart_putchar('\r');
    
    while(!(HwReg(gOmapSerialUartBase + LSR) & LSR_THRE))
        barrier();
    
    HwReg(gOmapSerialUartBase + THR) = c;
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
