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
/*
 * Mach-O Loader for Neo-BootX.
 */

#ifndef _MACHO_LOADER_H_
#define _MACHO_LOADER_H_

#include "macho.h"

/*
 * The global loader context is used to keep track of the loaded mach-o
 * file and related properties such as VM bias and so on.
 */
typedef struct __loader_context {
    uint8_t *source;            /* Original file source */
    uint8_t *load_addr;         /* Load address */
    uint32_t entry;             /* Entrypoint */
    uint32_t vm_bias;           /* VM address the file is based to */
    uint32_t vm_size;           /* Size of the object after mapping. */
} loader_context_t;

/*
 * These are the error codes returned by the loader for core operations.
 */
typedef enum {
    kLoadSuccess = 0,
    kLoadFailure = -1,
    kLoadBadImage = -2,
    kLoadMalformedSection = -3,
    kLoadWrongArchitecture = -4,
    kLoadUnexpectedError = -5,
    kLoadInvalidParameter = -6,
    kLoadBadContext = -7,
    kLoadThatImageSucks = 0xBEEF
} loader_return_t;

/*
 * API functions.
 */
loader_return_t macho_initialize(loader_context_t * ctx, void *file);

loader_return_t macho_set_vm_bias(loader_context_t * ctx, uint32_t vmaddr);

loader_return_t macho_get_entrypoint(loader_context_t * ctx, uint32_t * ep);

loader_return_t macho_file_map(loader_context_t * ctx, uint32_t loadaddr);

uint32_t macho_get_vmsize(loader_context_t * ctx);

#endif
