/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2005 M. Bakker, Nero AG, http://www.nero.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** The "appropriate copyright message" mentioned in section 2c of the GPLv2
** must read: "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Nero AG through Mpeg4AAClicense@nero.com.
**
** $Id: neaacdec.h,v 1.14 2012/03/02 15:29:47 knik Exp $
**/

#ifndef __NEAACDEC_H__
#define __NEAACDEC_H__

#include "common.h"
#include <inttypes.h>
#include <math.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* COMPILE TIME DEFINITIONS */
#define PREFER_POINTERS
// #define ERROR_RESILIENCE
#define LTP_DEC /* Allow decoding of LTP (long term prediction) profile AAC */
#define LD_DEC  /* Allow decoding of LD (low delay) profile AAC */
#define SBR_DEC /* Allow decoding of SBR (spectral band replication) */
#define PS_DEC  /* Allow decoding of PS (parametric stereo */

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* LD can't do without LTP */
#ifdef LD_DEC
    #ifndef ERROR_RESILIENCE
        #define ERROR_RESILIENCE
    #endif
    #ifndef LTP_DEC
        #define LTP_DEC
    #endif
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------

#define MAIN                 1 /* object types for AAC */
#define LC                   2
#define SSR                  3
#define LTP                  4
#define HE_AAC               5
#define ER_LC                17
#define ER_LTP               19
#define LD                   23
#define RAW                  0 /* header types */
#define ADIF                 1
#define ADTS                 2
#define LATM                 3
#define NO_SBR               0 /* SBR signalling */
#define SBR_UPSAMPLED        1
#define SBR_DOWNSAMPLED      2
#define NO_SBR_UPSAMPLED     3
#define FAAD_FMT_16BIT       1 /* library output formats */
#define FAAD_FMT_24BIT       2
#define FAAD_FMT_32BIT       3
#define FAAD_FMT_FLOAT       4
#define FAAD_FMT_FIXED       FAAD_FMT_FLOAT
#define FAAD_FMT_DOUBLE      5
#define LC_DEC_CAP           (1 << 0) /* Can decode LC */
#define MAIN_DEC_CAP         (1 << 1) /* Can decode MAIN */
#define LTP_DEC_CAP          (1 << 2) /* Can decode LTP */
#define LD_DEC_CAP           (1 << 3) /* Can decode LD */
#define ERROR_RESILIENCE_CAP (1 << 4) /* Can decode ER */
#define FIXED_POINT_CAP      (1 << 5) /* Fixed point */
#define FRONT_CHANNEL_CENTER (1)      /* Channel definitions */
#define FRONT_CHANNEL_LEFT   (2)
#define FRONT_CHANNEL_RIGHT  (3)
#define SIDE_CHANNEL_LEFT    (4)
#define SIDE_CHANNEL_RIGHT   (5)
#define BACK_CHANNEL_LEFT    (6)
#define BACK_CHANNEL_RIGHT   (7)
#define BACK_CHANNEL_CENTER  (8)
#define LFE_CHANNEL          (9)
#define UNKNOWN_CHANNEL      (0)
#define ER_OBJECT_START      17 /* First object type that has ER */
#define LEN_SE_ID            3  /* Bitstream */
#define LEN_TAG              4
#define LEN_BYTE             8
#define EXT_FIL              0
#define EXT_FILL_DATA        1
#define EXT_DATA_ELEMENT     2
#define EXT_DYNAMIC_RANGE    11
#define ANC_DATA             0
#define ID_SCE               0x0 /* Syntax elements */
#define ID_CPE               0x1
#define ID_CCE               0x2
#define ID_LFE               0x3
#define ID_DSE               0x4
#define ID_PCE               0x5
#define ID_FIL               0x6
#define ID_END               0x7
#define INVALID_ELEMENT_ID   255
#define ONLY_LONG_SEQUENCE   0x0
#define LONG_START_SEQUENCE  0x1
#define EIGHT_SHORT_SEQUENCE 0x2
#define LONG_STOP_SEQUENCE   0x3
#define ZERO_HCB             0
#define FIRST_PAIR_HCB       5
#define ESC_HCB              11
#define QUAD_LEN             4
#define PAIR_LEN             2
#define NOISE_HCB            13
#define INTENSITY_HCB2       14
#define INTENSITY_HCB        15
#define INVALID_SBR_ELEMENT  255
#define MAX_CHANNELS         64
#define MAX_SYNTAX_ELEMENTS  48
#define MAX_WINDOW_GROUPS    8
#define MAX_SFB              51
#define MAX_LTP_SFB          40
#define MAX_LTP_SFB_S        8
#define BYTE_NUMBIT          8
#define BYTE_NUMBIT_LD       3
#define bit2byte(a)          ((a + 7) >> BYTE_NUMBIT_LD)

/* A decode call can eat up to FAAD_MIN_STREAMSIZE bytes per decoded channel,
   so at least so much bytes per channel should be available in this stream */
#define FAAD_MIN_STREAMSIZE 768 /* 6144 bits/channel */

typedef void* NeAACDecHandle;

typedef struct mp4AudioSpecificConfig {
    unsigned char  objectTypeIndex; /* Audio Specific Info */
    unsigned char  samplingFrequencyIndex;
    unsigned long  samplingFrequency;
    unsigned char  channelsConfiguration;
    unsigned char  frameLengthFlag; /* GA Specific Info */
    unsigned char  dependsOnCoreCoder;
    unsigned short coreCoderDelay;
    unsigned char  extensionFlag;
    unsigned char  aacSectionDataResilienceFlag;
    unsigned char  aacScalefactorDataResilienceFlag;
    unsigned char  aacSpectralDataResilienceFlag;
    unsigned char  epConfig;

    char sbr_present_flag;
    char forceUpSampling;
    char downSampledSBR;
} mp4AudioSpecificConfig;

typedef struct NeAACDecConfiguration {
    unsigned char defObjectType;
    unsigned long defSampleRate;
    unsigned char outputFormat;
    unsigned char downMatrix;
    unsigned char useOldADTSFormat;
    unsigned char dontUpSampleImplicitSBR;
} NeAACDecConfiguration, *NeAACDecConfigurationPtr;

typedef struct NeAACDecFrameInfo {
    unsigned long bytesconsumed;
    unsigned long samples;
    unsigned char channels;
    unsigned char error;
    unsigned long samplerate;
    unsigned char sbr;                /* SBR: 0: off, 1: on; upsample, 2: on; downsampled, 3: off; upsampled */
    unsigned char object_type;        /* MPEG-4 ObjectType */
    unsigned char header_type;        /* AAC header type; MP4 will be signalled as RAW also */
    unsigned char num_front_channels; /* multichannel configuration */
    unsigned char num_side_channels;
    unsigned char num_back_channels;
    unsigned char num_lfe_channels;
    unsigned char channel_position[64];
    unsigned char ps; /* PS: 0: off, 1: on */
} NeAACDecFrameInfo;

typedef struct _bitfile {
    /* bit input */
    uint32_t    bufa;
    uint32_t    bufb;
    uint32_t    bits_left;
    uint32_t    buffer_size; /* size of the buffer in bytes */
    uint32_t    bytes_left;
    uint8_t     error;
    uint32_t*   tail;
    uint32_t*   start;
    const void* buffer;
} bitfile;

//----------------------------------------------------------------------------------------------------------------------------------------------------
//                                              P R O T O T Y P E S
//----------------------------------------------------------------------------------------------------------------------------------------------------
const char*              NeAACDecGetErrorMessage(unsigned char errcode);
unsigned long            NeAACDecGetCapabilities(void);
NeAACDecHandle           NeAACDecOpen(void);
NeAACDecConfigurationPtr NeAACDecGetCurrentConfiguration(NeAACDecHandle hDecoder);
unsigned char            NeAACDecSetConfiguration(NeAACDecHandle hDecoder, NeAACDecConfigurationPtr config);
long  NeAACDecInit(NeAACDecHandle hDecoder, unsigned char* buffer, unsigned long buffer_size, unsigned long* samplerate, unsigned char* channels);
char  NeAACDecInit2(NeAACDecHandle hDecoder, unsigned char* pBuffer, unsigned long SizeOfDecoderSpecificInfo, unsigned long* samplerate,
                    unsigned char* channels);
void  NeAACDecPostSeekReset(NeAACDecHandle hDecoder, long frame);
void  NeAACDecClose(NeAACDecHandle hDecoder);
void* NeAACDecDecode(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size);
void* NeAACDecDecode2(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer,
                      unsigned long sample_buffer_size);
char  NeAACDecAudioSpecificConfig(unsigned char* pBuffer, unsigned long buffer_size, mp4AudioSpecificConfig* mp4ASC);
int   NeAACDecGetVersion(const char** faad_id_string, const char** faad_copyright_string);
void     faad_initbits(bitfile* ld, const void* buffer, const uint32_t buffer_size);
void     faad_initbits_rev(bitfile* ld, void* buffer, uint32_t bits_in_buffer);
uint8_t  faad_byte_align(bitfile* ld);
uint32_t faad_get_processed_bits(bitfile* ld);
void     faad_flushbits_ex(bitfile* ld, uint32_t bits);
void     faad_rewindbits(bitfile* ld);
void     faad_resetbits(bitfile* ld, int bits);
uint8_t* faad_getbitbuffer(bitfile* ld, uint32_t bits);

//----------------------------------------------------------------------------------------------------------------------------------------------------
//                                              I N L I N E S
//----------------------------------------------------------------------------------------------------------------------------------------------------
/* circumvent memory alignment errors on ARM */
static inline uint32_t getdword(void* mem) {
    uint32_t tmp;
    ((uint8_t*)&tmp)[0] = ((uint8_t*)mem)[3];
    ((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[2];
    ((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[1];
    ((uint8_t*)&tmp)[3] = ((uint8_t*)mem)[0];
    return tmp;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* reads only n bytes from the stream instead of the standard 4 */
static /*inline*/ uint32_t getdword_n(void* mem, int n) {
    uint32_t tmp = 0;
    switch(n) {
    case 3: ((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[2];
    case 2: ((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[1];
    case 1: ((uint8_t*)&tmp)[3] = ((uint8_t*)mem)[0];
    default: break;
    }
    return tmp;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline uint32_t faad_showbits(bitfile* ld, uint32_t bits) {
    if(bits <= ld->bits_left) {
        // return (ld->bufa >> (ld->bits_left - bits)) & bitmask[bits];
        return (ld->bufa << (32 - ld->bits_left)) >> (32 - bits);
    }
    bits -= ld->bits_left;
    // return ((ld->bufa & bitmask[ld->bits_left]) << bits) | (ld->bufb >> (32 - bits));
    return ((ld->bufa & ((1 << ld->bits_left) - 1)) << bits) | (ld->bufb >> (32 - bits));
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline void faad_flushbits(bitfile* ld, uint32_t bits) {
    /* do nothing if error */
    if(ld->error != 0) return;
    if(bits < ld->bits_left) { ld->bits_left -= bits; }
    else { faad_flushbits_ex(ld, bits); }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* return next n bits (right adjusted) */
static /*inline*/ uint32_t faad_getbits(bitfile* ld, uint32_t n) {
    uint32_t ret;
    if(n == 0) return 0;
    ret = faad_showbits(ld, n);
    faad_flushbits(ld, n);
    return ret;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline uint8_t faad_get1bit(bitfile* ld) {
    uint8_t r;
    if(ld->bits_left > 0) {
        ld->bits_left--;
        r = (uint8_t)((ld->bufa >> ld->bits_left) & 1);
        return r;
    }
    /* bits_left == 0 */
    r = (uint8_t)faad_getbits(ld, 1);
    return r;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* reversed bitreading routines */
static inline uint32_t faad_showbits_rev(bitfile* ld, uint32_t bits) {
    uint8_t  i;
    uint32_t B = 0;
    if(bits <= ld->bits_left) {
        for(i = 0; i < bits; i++) {
            if(ld->bufa & (1 << (i + (32 - ld->bits_left)))) B |= (1 << (bits - i - 1));
        }
        return B;
    }
    else {
        for(i = 0; i < ld->bits_left; i++) {
            if(ld->bufa & (1 << (i + (32 - ld->bits_left)))) B |= (1 << (bits - i - 1));
        }
        for(i = 0; i < bits - ld->bits_left; i++) {
            if(ld->bufb & (1 << (i + (32 - ld->bits_left)))) B |= (1 << (bits - ld->bits_left - i - 1));
        }
        return B;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline void faad_flushbits_rev(bitfile* ld, uint32_t bits) {
    /* do nothing if error */
    if(ld->error != 0) return;
    if(bits < ld->bits_left) { ld->bits_left -= bits; }
    else {
        uint32_t tmp;
        ld->bufa = ld->bufb;
        tmp = getdword(ld->start);
        ld->bufb = tmp;
        ld->start--;
        ld->bits_left += (32 - bits);
        if(ld->bytes_left < 4) {
            ld->error = 1;
            ld->bytes_left = 0;
        }
        else { ld->bytes_left -= 4; }
        //        if (ld->bytes_left == 0)
        //            ld->no_more_reading = 1;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static /*inline*/ uint32_t faad_getbits_rev(bitfile* ld, uint32_t n) {
    uint32_t ret;

    if(n == 0) return 0;

    ret = faad_showbits_rev(ld, n);
    faad_flushbits_rev(ld, n);

    return ret;
}

#ifdef ERROR_RESILIENCE
//----------------------------------------------------------------------------------------------------------------------------------------------------
/* Modified bit reading functions for HCR */
typedef struct {
    /* bit input */
    uint32_t bufa;
    uint32_t bufb;
    int8_t   len;
} bits_t;

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline uint32_t showbits_hcr(bits_t* ld, uint8_t bits) {
    if(bits == 0) return 0;
    if(ld->len <= 32) {
        /* huffman_spectral_data_2 needs to read more than may be available, bits maybe
           > ld->len, deliver 0 than */
        if(ld->len >= bits) return ((ld->bufa >> (ld->len - bits)) & (0xFFFFFFFF >> (32 - bits)));
        else
            return ((ld->bufa << (bits - ld->len)) & (0xFFFFFFFF >> (32 - bits)));
    }
    else {
        if((ld->len - bits) < 32) { return ((ld->bufb & (0xFFFFFFFF >> (64 - ld->len))) << (bits - ld->len + 32)) | (ld->bufa >> (ld->len - bits)); }
        else { return ((ld->bufb >> (ld->len - bits - 32)) & (0xFFFFFFFF >> (32 - bits))); }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* return 1 if position is outside of buffer, 0 otherwise */
static inline int8_t flushbits_hcr(bits_t* ld, uint8_t bits) {
    ld->len -= bits;

    if(ld->len < 0) {
        ld->len = 0;
        return 1;
    }
    else { return 0; }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t getbits_hcr(bits_t* ld, uint8_t n, uint32_t* result) {
    *result = showbits_hcr(ld, n);
    return flushbits_hcr(ld, n);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t get1bit_hcr(bits_t* ld, uint8_t* result) {
    uint32_t res;
    int8_t   ret;

    ret = getbits_hcr(ld, 1, &res);
    *result = (int8_t)(res & 1);
    return ret;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
#endif // ERROR_RESILIENCE





#endif
