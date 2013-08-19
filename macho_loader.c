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
 * Main guts of the Mach-O loader for Neo-BootX.
 */

#include "genboot.h"

#define add_ptr2(x, y)      ((uintptr_t)((uintptr_t)x + (uintptr_t)y))
#define add_ptr3(x, y, z)   ((uintptr_t)((uintptr_t)x + (uintptr_t)y + (uintptr_t)z))

#define for_each_lc(lcp, head) for (uint32_t XXi = 0; XXi < head->ncmds; XXi++, lcp = (struct load_command*)add_ptr2(lcp, lcp->cmdsize))

#define for_each_section(sect, seg) sect = (struct section*)((uintptr_t)seg + sizeof(struct segment_command)); \
    for (uint32_t XXi = 0; XXi < seg->nsects; XXi++, sect++)

/**
 * macho_initialize
 *
 * Initialize the basic loader context and verify that the file
 * is indeed a Mach-O executable.
 */
loader_return_t macho_initialize(loader_context_t * ctx, void *file)
{
    mach_header_t *header = (mach_header_t *) file;

    /* We want to always perform this if a loader context is here. */
    if ((!ctx) || (!file)) {
        return kLoadInvalidParameter;
    }

    /* Verify the type of Mach-O we're about to load. */
    if (header->magic != kMachMagic) {
        return kLoadBadImage;
    }

    /* Only executables are supported. */
    if (header->filetype != kMachExecute) {
        return kLoadBadImage;
    }

    ctx->source = (uint8_t *) file;

    return kLoadSuccess;
}

/**
 * macho_set_vm_bias
 *
 * Set the virtual address bias for the specified context.
 */
loader_return_t macho_set_vm_bias(loader_context_t * ctx, uint32_t vmaddr)
{
    if (!ctx) {
        return kLoadInvalidParameter;
    }
    ctx->vm_bias = vmaddr;
    return kLoadSuccess;
}

/**
 * macho_get_entrypoint
 *
 * Get the address of the entrypoint for a specified context.
 */
loader_return_t macho_get_entrypoint(loader_context_t * ctx, uint32_t * ep)
{
    if (!ctx || !ep) {
        return kLoadInvalidParameter;
    }
    *ep = ctx->entry;
    return kLoadSuccess;
}

/**
 * macho_file_map
 *
 * Copy all of the mach-o segments to the proper addresses.
 */
loader_return_t macho_file_map(loader_context_t * ctx, uint32_t loadaddr)
{
    mach_header_t *mh = (mach_header_t *) ctx->source;
    struct load_command *lc = (struct load_command *)((mach_header_t *) mh + 1);
    uint32_t vmsize_count = 0;
    uint32_t vm_bias = ctx->vm_bias;

    for_each_lc(lc, mh) {
        /* Only segments for mapping */
        if (lc->cmd == kLoadCommandSegment) {
            struct segment_command *sc = (struct segment_command *)lc;
            uint32_t our_vmaddr;

            /* Add up the vm size */
            vmsize_count += sc->vmsize;

            /* Get the load address */
            our_vmaddr = sc->vmaddr - vm_bias;

            /* Map the binary now */
            if (sc->filesize) {
                bcopy((void *)
                      add_ptr2(ctx->source, sc->fileoff),
                      (void *)add_ptr2(loadaddr, our_vmaddr), sc->filesize);
            }

        } else if (lc->cmd == kLoadCommandUnixThread) {
            /* UnixThread has the entrypoint. */
            thread_command_t *tc = (thread_command_t *) lc;
            ctx->entry = tc->state.pc;
        }
    }

    return kLoadSuccess;
}

/**
 * macho_get_vmsize
 *
 * Copy all of the mach-o segments to the proper addresses.
 */
uint32_t macho_get_vmsize(loader_context_t * ctx)
{
    mach_header_t *mh = (mach_header_t *) ctx->source;
    struct load_command *lc = (struct load_command *)((mach_header_t *) mh + 1);
    uint32_t vmsize_count = 0;

    for_each_lc(lc, mh) {
        /* Only segments for mapping */
        if (lc->cmd == kLoadCommandSegment) {
            struct segment_command *sc = (struct segment_command *)lc;
            vmsize_count += sc->vmsize;
        }
    }

    return vmsize_count;
}
