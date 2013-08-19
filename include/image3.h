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

#ifndef _IMAGE3_H_
#define _IMAGE3_H_

#include <stdint.h>

#define kImage3Magic				'Img3'

/* Image types */
#define kImage3TypeKernel			'krnl'
#define kImage3TypeiBoot			'ibot'
#define	kImage3TypeiBSS				'ibss'
#define kImage3TypeiBEC				'ibec'
#define kImage3TypeiLLB				'illb'
#define kImage3TypeAppleLogo		'logo'
#define kImage3TypeRecoveryLogo		'recm'
#define kImage3TypeNeedService		'nsrv'
#define kImage3TypeDiags			'diag'
#define kImage3TypeTsys				'tsys'
#define kImage3TypeDeviceTree		'dtre'
#define kImage3TypeCharging0		'chg0'
#define kImage3TypeCharging1		'chg1'
#define kImage3TypeGlyphCharging	'glyC'
#define kImage3TypeGlyphPlugin		'glyP'
#define kImage3TypeCertificate		'cert'

/* Our types. */
#define kImage3TypeGenericBoot		'gbot'
#define kImage3TypeXmlDeviceTree	'xmdt'
#define kImage3TypeJsonDeviceTree	'jsdt'

/* Image3 Tags */
#define kImage3TagData				'DATA'
#define kImage3TagType				'TYPE'
#define kImage3TagCert				'CERT'
#define kImage3TagSignature			'SHSH'
#define kImage3TagBoard				'BORD'
#define kImage3TagKeyBag			'KBAG'
#define kImage3TagSecurityEpoch		'SEPO'
#define kImage3TypeVersion			'VERS'
#define kImage3TagSecurityDomain	'SDOM'
#define kImage3TagProduct			'PROD'
#define kImage3TagChip				'CHIP'

typedef struct Image3Header {
    uint32_t magic;
    uint32_t size;
    uint32_t dataSize;
} __attribute__ ((packed)) Image3Header;

typedef struct Image3ShshExtension {
    uint32_t shshOffset;
    uint32_t imageType;
} __attribute__ ((packed)) Image3ShshExtension;

typedef struct Image3RootHeader {
    Image3Header header;
    Image3ShshExtension shshExtension;
} __attribute__ ((packed)) Image3RootHeader;

extern int image3_fast_get_type(void *buffer, uint32_t * type);
extern Image3Header *image3_find_tag(void *image, uint32_t find_tag);
extern int image3_get_tag_data(void *image, uint32_t tag, void **out_data,
                               uint32_t * out_size);

#endif
