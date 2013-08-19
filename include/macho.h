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
 * Mach-O loader for Neo-BootX, a Linux->Darwin bootloader.
 */

#ifndef _MACHO_H_
#define _MACHO_H_

#include <stdint.h>

/*
 * The Mach header is at the beginning of all 32-bit MachO files.
 *
 * This isn't the 64-bit version you're looking for by the way.
 */
typedef struct __mach_header {
    uint32_t magic;             /* Mach file magic */
    uint32_t cputype;           /* CPU type the executable was made for */
    uint32_t cpusubtype;        /* CPU subtype */
    uint32_t filetype;          /* File type, ie: dylinker, object, ... */
    uint32_t ncmds;             /* Number of load commands */
    uint32_t sizeofcmds;        /* Size of the load commands */
    uint32_t flags;
} __attribute__ ((packed)) mach_header_t;

#define kMachMagic      0xfeedface  /* Mach magic number */
#define kMachCigam      0xcefaedfe  /* Endian swapped. */

/* 
 * The following enumerated type describes the filetype field in the
 * mach header.
 */
typedef enum {
    kMachObject = 1,
    kMachExecute = 2,
    kMachFvmLibrary = 3,
    kMachCoreDump = 4,
    kMachPreload = 5,
    kMachDylib = 6,
    kMachDylinker = 7,
    kMachBundle = 8,
    kMachDylibStub = 9,
    kMachDsym = 10,
    kMachKextBundle = 11,
} kMachOFileType;

/*
 * The following describes the ARM thread state.
 */
#if defined(__arm__) || defined(__target_arm__)
typedef struct __arm_thread_state {
    uint32_t r[13];             /* General purpose registers */
    uint32_t sp;                /* ARM stack pointer */
    uint32_t lr;                /* Link register */
    uint32_t pc;                /* Program counter */
    uint32_t cpsr;              /* Current program state register */
} __attribute__ ((packed)) arm_thread_state_t;
#endif

/*
 * Load commands are used to denote various parts of a Mach-O file,
 * such as segments or the initial program start state.
 */
#if defined(__arm__) || defined(__target_arm__)
typedef struct __arm_thread_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t flavor;
    uint32_t count;
    arm_thread_state_t state;
} __attribute__ ((packed)) thread_command_t;
#else
#error The architecture you are compiling for is not supported yet.
#endif

typedef enum {
    kLoadCommandSegment = 1,
    kLoadCommandSymtab = 2,
    kLoadCommandSymSeg = 3,
    kLoadCommandThread = 4,
    kLoadCommandUnixThread = 5,
    kLoadCommandFvmLib = 6,
    kLoadCommandFvmIdent = 7,
    kLoadCommandIdentity = 8,
    kLoadCommandFvmFile = 9,
    kLoadCommandPrePage = 10,
    kLoadCommandDsymtab = 11,
    kLoadCommandLoadDylib = 12,
    kLoadCommandIdDylib = 13,
    kLoadCommandLoadDylink = 14,
    kLoadCommandIdDylink = 15,
    kLoadCommandPrebound = 16,
} kMachOLoadCommands;

/* 
 * These are used for segment names.
 */
#define kSegDataName    "__DATA"
#define kSegTextName    "__TEXT"

struct load_command {
    uint32_t cmd;               /* type of load command */
    uint32_t cmdsize;           /* total size of command in bytes */
} __attribute__ ((packed));

struct segment_command {        /* for 32-bit architectures */
    uint32_t cmd;               /* LC_SEGMENT */
    uint32_t cmdsize;           /* includes sizeof section structs */
    char segname[16];           /* segment name */
    uint32_t vmaddr;            /* memory address of this segment */
    uint32_t vmsize;            /* memory size of this segment */
    uint32_t fileoff;           /* file offset of this segment */
    uint32_t filesize;          /* amount to map from the file */
    uint32_t maxprot;           /* maximum VM protection */
    uint32_t initprot;          /* initial VM protection */
    uint32_t nsects;            /* number of sections in segment */
    uint32_t flags;             /* flags */
} __attribute__ ((packed));

struct section {                /* for 32-bit architectures */
    char sectname[16];          /* name of this section */
    char segname[16];           /* segment this section goes in */
    uint32_t addr;              /* memory address of this section */
    uint32_t size;              /* size in bytes of this section */
    uint32_t offset;            /* file offset of this section */
    uint32_t align;             /* section alignment (power of 2) */
    uint32_t reloff;            /* file offset of relocation entries */
    uint32_t nreloc;            /* number of relocation entries */
    uint32_t flags;             /* flags (section type and attributes) */
    uint32_t reserved1;         /* reserved (for offset or index) */
    uint32_t reserved2;         /* reserved (for count or sizeof) */
} __attribute__ ((packed));

#endif
