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

#ifndef _GENBOOT_H_
#define _GENBOOT_H_

#include <stdint.h>
#include <string.h>

#include "boot_args.h"
#include "device_tree.h"
#include "linux_atags.h"
#include "printf.h"
#include "tlsf.h"
#include "malloc.h"
#include "image3.h"
#include "macho.h"
#include "macho_loader.h"
#include "memory_regions.h"
#include "jsmn.h"
#include "xml.h"

#include <assert.h>

extern boot_args gBootArgs;
extern uint32_t ramdisk_base;
extern uint32_t ramdisk_size;

/* Booter functions */
extern void panic(const char *panicStr, ...);
extern void init_debug(void);

extern void _locore_halt_system(void);

/* Pretty panics. */
#define __STRINGIFY(x) #x
#define LINE_NUMBER(x) __STRINGIFY(x)
#define PANIC_LOCATION __FILE__ ":" LINE_NUMBER(__LINE__)
#define panic(ex, ...) \
	(panic)(# ex "@" PANIC_LOCATION, ## __VA_ARGS__)

/* Machine. */
#define MACH_TYPE_REALVIEW_PBA8		1897

/* BSD functions */
extern void bzero(void *block, size_t nbytes);
extern void bcopy(void *src, void *dst, size_t nbytes);

/* Bootload Darwin kernel */
extern void start_darwin(void);

/* Kernel start routine */
typedef void (*kernel_start) (void *bootArgs, uint32_t ttbr);
extern void _locore_jump_to(kernel_start * startRoutine, void *bootArgs);

#endif
