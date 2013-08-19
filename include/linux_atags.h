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

#ifndef ATAGS_H
#define ATAGS_H

#ifndef __ASSEMBLY__
#include <stdint.h>

#define u_int32_t       uint32_t
#define u_int16_t       uint16_t
#define u_int8_t        uint8_t
#endif

#define	ATAG_NONE       0x00000000
#define	ATAG_CORE       0x54410001
#define	ATAG_MEM        0x54410002
#define	ATAG_VIDEOTEXT	0x54410003
#define	ATAG_RAMDISK	0x54410004
#define	ATAG_INITRD2	0x54420005
#define	ATAG_SERIAL     0x54410006
#define	ATAG_REVISION	0x54410007
#define	ATAG_VIDEOLFB	0x54410008
#define	ATAG_CMDLINE	0x54410009
#define	ATAG_ARGS       0x54410100

#ifndef __ASSEMBLY__
/* structures for each atag */
struct atag_header {
    u_int32_t size;             /* length of tag in words including this header */
    u_int32_t tag;              /* tag type */
};

struct atag_core {
    u_int32_t flags;
    u_int32_t pagesize;
    u_int32_t rootdev;
};

struct atag_mem {
    u_int32_t size;
    u_int32_t start;
};

struct atag_videotext {
    u_int8_t x;
    u_int8_t y;
    u_int16_t video_page;
    u_int8_t video_mode;
    u_int8_t video_cols;
    u_int16_t video_ega_bx;
    u_int8_t video_lines;
    u_int8_t video_isvga;
    u_int16_t video_points;
};

struct atag_ramdisk {
    u_int32_t flags;
    u_int32_t size;
    u_int32_t start;
};

struct atag_initrd2 {
    u_int32_t start;
    u_int32_t size;
};

struct atag_serialnr {
    u_int32_t low;
    u_int32_t high;
};

struct atag_revision {
    u_int32_t rev;
};

struct atag_videolfb {
    u_int16_t lfb_width;
    u_int16_t lfb_height;
    u_int16_t lfb_depth;
    u_int16_t lfb_linelength;
    u_int32_t lfb_base;
    u_int32_t lfb_size;
    u_int8_t red_size;
    u_int8_t red_pos;
    u_int8_t green_size;
    u_int8_t green_pos;
    u_int8_t blue_size;
    u_int8_t blue_pos;
    u_int8_t rsvd_size;
    u_int8_t rsvd_pos;
};

struct atag_cmdline {
    char cmdline[1];
};

struct atag_args {
    u_int32_t argc;
    u_int32_t argv;
    u_int32_t envp;
};

struct atag {
    struct atag_header hdr;
    union {
        struct atag_core core;
        struct atag_mem mem;
        struct atag_videotext videotext;
        struct atag_ramdisk ramdisk;
        struct atag_initrd2 initrd2;
        struct atag_serialnr serialnr;
        struct atag_revision revision;
        struct atag_videolfb videolfb;
        struct atag_cmdline cmdline;
        struct atag_args args;
    } u;
};
#endif

#endif                          /* ATAGS_H */
