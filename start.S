/*
 * Start of Linux->Darwin bootloader
 */

#include "asm_help.h"
#include "linux_atags.h"

.arm
.code 32
.text

.align 4

/**
 * The point of this bootstrapper is to copy the kernel to the proper
 * physical address and start it.
 *
 * The known states of the registers are:
 *  r0 - #0
 *  r1 - Machine type
 *  r2 - PA of ATAGs list
 */
EnterARM(start)
    /* We are in supervisor, no interrupts. */
    cpsid   if, #0x13

    /* Verify that things are defined properly. */
    cmp     r2, #0
    beq     lol_no_atags

    /* Verify the tag header */
    ldr     r4, =ATAG_CORE
    ldr     r3, [r2, #4]
    cmp     r3, r4
    bne     lol_no_atags

    /*
     * Things look semi good, (we hope we're running at the right
     * text base, this is assumed.)
    */
    ldr     sp, =_tempstack_end
    mov     r7, #0

    /*
     * Go to core bootstrap now.
     */
    bl      corestart_main

lol_no_atags:
    /* lol no atags */
    bl      _failure

/**
 * failure
 *
 * This function is called when we fail core initialization. :(
 */
EnterARM(failure)
EnterARM(locore_halt_system)
    cpsid   if
    b       .

/**
 * locore_jump_to
 *
 * Shut down the bootloader and start the new OS image.
 */
EnterARM(locore_jump_to)
    cpsid   if, #0x13

    /* Disable L2 cache */
    mov     r9, r0
    mov     r10, r1

    mrc     p15, 0, r0, c1, c0, 1   /* read Auxiliary Control Register */
    bic     r0, r0, #0x00000002     /* disable L2 cache */
    mcr     p15, 0, r0, c1, c0, 1   /* store Auxiliary Control Register */

    /* Disable caching entirely. */
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #0x00002300 
    bic     r0, r0, #0x00000005
    bic     r0, r0, #0x00001000
    bic     r0, r0, #(1 << 2)
    mcr     p15, 0, r0, c1, c0, 0

#if 0
    /* Disable VFP/SIMD */
    mov     r0, #0x00000000
    mcr     p10, #0x7, r0, c8, c0, #0

    /* Disable I-cache */
    mrc     p15, 0, r0, c1, c0, 2
    bic     r0, r0, #0x00f00000
    mcr     p15, 0, r0, c1, c0, 2
#endif

    /* Clear caches. */
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4

    /* Disable MMU */
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #1
    mcr     p15, 0, r0, c1, c0, 0

    /* Clear prefetch buffer */
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4

    isb     sy
    dsb     sy

    /* Point of no return */
    mov     lr, r9
    mov     r0, r1
    bx      lr    
deadloop:
    b       .

.data
.align 4
/*
 * Temporary stack.
 */
_tempstack_begin:
    .space (4096), 0
_tempstack_end:

