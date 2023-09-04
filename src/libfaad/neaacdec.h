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

typedef int32_t complex_t[2];

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
#define TNS_MAX_ORDER        20
#define EXTENSION_ID_PS      2
#define MAX_PS_ENVELOPES     5
#define NO_ALLPASS_LINKS     3
#define MAX_NTSRHFG          40 /* MAX_NTSRHFG: maximum of number_time_slots * rate + HFGen. 16*2+8 */
#define MAX_NTSR             32 /* max number_time_slots * rate */
#define MAX_M                49 /* MAX_M: maximum value for M */
#define MAX_L_E              5  /* MAX_L_E: maximum value for L_E */

#define bit2byte(a)         ((a + 7) >> BYTE_NUMBIT_LD)
#define FAAD_MIN_STREAMSIZE 768 /* 6144 bits/channel */
#define COEF_BITS           28
#define COEF_PRECISION      (1 << COEF_BITS)
#define REAL_BITS           14 // MAXIMUM OF 14 FOR FIXED POINT SBR
#define REAL_PRECISION      (1 << REAL_BITS)
#define FRAC_SIZE           32 /* frac is a 32 bit integer */
#define FRAC_BITS           31
#define FRAC_PRECISION      ((uint32_t)(1 << FRAC_BITS))
#define FRAC_MAX            0x7FFFFFFF
#define REAL_CONST(A)       (((A) >= 0) ? ((int32_t)((A) * (REAL_PRECISION) + 0.5)) : ((int32_t)((A) * (REAL_PRECISION)-0.5)))
#define LOG2_MIN_INF        REAL_CONST(-10000)
#define COEF_CONST(A)       (((A) >= 0) ? ((int32_t)((A) * (COEF_PRECISION) + 0.5)) : ((int32_t)((A) * (COEF_PRECISION)-0.5)))
#define FRAC_CONST(A) \
    (((A) == 1.00) ? ((int32_t)FRAC_MAX) : (((A) >= 0) ? ((int32_t)((A) * (FRAC_PRECISION) + 0.5)) : ((int32_t)((A) * (FRAC_PRECISION)-0.5))))
#define Q2_BITS           22
#define Q2_PRECISION      (1 << Q2_BITS)
#define Q2_CONST(A)       (((A) >= 0) ? ((int32_t)((A) * (Q2_PRECISION) + 0.5)) : ((int32_t)((A) * (Q2_PRECISION)-0.5)))
#define MUL_R(A, B)       (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (REAL_BITS - 1))) >> REAL_BITS) /* multiply with real shift */
#define MUL_C(A, B)       (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (COEF_BITS - 1))) >> COEF_BITS) /* multiply with coef shift */
#define _MulHigh(A, B)    (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (FRAC_SIZE - 1))) >> FRAC_SIZE) /* multiply with fractional shift */
#define MUL_F(A, B)       (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (FRAC_BITS - 1))) >> FRAC_BITS)
#define MUL_Q2(A, B)      (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (Q2_BITS - 1))) >> Q2_BITS)
#define MUL_SHIFT6(A, B)  (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (6 - 1))) >> 6)
#define MUL_SHIFT23(A, B) (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (23 - 1))) >> 23)
#define RE(A)             A[0]
#define IM(A)             A[1]
#define DIV_R(A, B)       (((int64_t)A << REAL_BITS) / B)
#define DIV_C(A, B)       (((int64_t)A << COEF_BITS) / B)
#define QMF_RE(A)         RE(A)
#define QMF_IM(A)         IM(A)

#ifndef max
    #define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2 /* PI/2 */
    #define M_PI_2 1.57079632679489661923
#endif

typedef void* NeAACDecHandle;

#include "structs.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------
//                                              P R O T O T Y P E S
//----------------------------------------------------------------------------------------------------------------------------------------------------

const char*                NeAACDecGetErrorMessage(unsigned char errcode);
unsigned long              NeAACDecGetCapabilities(void);
NeAACDecHandle             NeAACDecOpen(void);
NeAACDecConfigurationPtr_t NeAACDecGetCurrentConfiguration(NeAACDecHandle hDecoder);
unsigned char              NeAACDecSetConfiguration(NeAACDecHandle hDecoder, NeAACDecConfigurationPtr_t config);
long     NeAACDecInit(NeAACDecHandle hDecoder, unsigned char* buffer, unsigned long buffer_size, unsigned long* samplerate, unsigned char* channels);
char     NeAACDecInit2(NeAACDecHandle hDecoder, unsigned char* pBuffer, unsigned long SizeOfDecoderSpecificInfo, unsigned long* samplerate,
                       unsigned char* channels);
void     NeAACDecPostSeekReset(NeAACDecHandle hDecoder, long frame);
void     NeAACDecClose(NeAACDecHandle hDecoder);
void*    NeAACDecDecode(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size);
void*    NeAACDecDecode2(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer,
                         unsigned long sample_buffer_size);
char     NeAACDecAudioSpecificConfig(unsigned char* pBuffer, unsigned long buffer_size, mp4AudioSpecificConfig* mp4ASC);
int      NeAACDecGetVersion(const char** faad_id_string, const char** faad_copyright_string);
void     faad_initbits(bitfile* ld, const void* buffer, const uint32_t buffer_size);
void     faad_initbits_rev(bitfile* ld, void* buffer, uint32_t bits_in_buffer);
uint8_t  faad_byte_align(bitfile* ld);
uint32_t faad_get_processed_bits(bitfile* ld);
void     faad_flushbits_ex(bitfile* ld, uint32_t bits);
void     faad_rewindbits(bitfile* ld);
void     faad_resetbits(bitfile* ld, int bits);
uint8_t* faad_getbitbuffer(bitfile* ld, uint32_t bits);
uint32_t ne_rng(uint32_t* __r1, uint32_t* __r2);
uint32_t wl_min_lzc(uint32_t x);

int32_t   log2_int(uint32_t val);
int32_t   log2_fix(uint32_t val);
int32_t   pow2_int(int32_t val);
int32_t   pow2_fix(int32_t val);
uint8_t   get_sr_index(const uint32_t samplerate);
uint8_t   max_pred_sfb(const uint8_t sr_index);
uint8_t   max_tns_sfb(const uint8_t sr_index, const uint8_t object_type, const uint8_t is_short);
uint32_t  get_sample_rate(const uint8_t sr_index);
int8_t    can_decode_ot(const uint8_t object_type);
void*     faad_malloc(size_t size);
void      faad_free(void* b);
void      tns_decode_frame(ic_stream* ics, tns_info* tns, uint8_t sr_index, uint8_t object_type, int32_t* spec, uint16_t frame_len);
void      tns_encode_frame(ic_stream* ics, tns_info* tns, uint8_t sr_index, uint8_t object_type, int32_t* spec, uint16_t frame_len);
uint16_t  ps_data(ps_info* ps, bitfile* ld, uint8_t* header);  /* ps_syntax.c */
ps_info*  ps_init(uint8_t sr_index, uint8_t numTimeSlotsRate); /* ps_dec.c */
void      ps_free(ps_info* ps);
uint8_t   ps_decode(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64]);
sbr_info* sbrDecodeInit(uint16_t framelength, uint8_t id_aac, uint32_t sample_rate, uint8_t downSampledSBR);
void      sbrDecodeEnd(sbr_info* sbr);
void      sbrReset(sbr_info* sbr);
uint8_t   sbrDecodeCoupleFrame(sbr_info* sbr, int32_t* left_chan, int32_t* right_chan, const uint8_t just_seeked, const uint8_t downSampledSBR);
uint8_t   sbrDecodeSingleFrame(sbr_info* sbr, int32_t* channel, const uint8_t just_seeked, const uint8_t downSampledSBR);
uint8_t sbrDecodeSingleFramePS(sbr_info* sbr, int32_t* left_channel, int32_t* right_channel, const uint8_t just_seeked, const uint8_t downSampledSBR);
uint8_t allocate_single_channel(NeAACDecStruct* hDecoder, uint8_t channel, uint8_t output_channels);
uint8_t window_grouping_info(NeAACDecStruct* hDecoder, ic_stream* ics);
uint8_t reconstruct_channel_pair(NeAACDecStruct* hDecoder, ic_stream* ics1, ic_stream* ics2, element* cpe, int16_t* spec_data1, int16_t* spec_data2);
uint8_t reconstruct_single_channel(NeAACDecStruct* hDecoder, ic_stream* ics, element* sce, int16_t* spec_data);
uint8_t envelope_time_border_vector(sbr_info* sbr, uint8_t ch);
void    noise_floor_time_border_vector(sbr_info* sbr, uint8_t ch);
uint8_t qmf_start_channel(uint8_t bs_start_freq, uint8_t bs_samplerate_mode, uint32_t sample_rate);
uint8_t qmf_stop_channel(uint8_t bs_stop_freq, uint32_t sample_rate, uint8_t k0);
uint8_t master_frequency_table_fs0(sbr_info* sbr, uint8_t k0, uint8_t k2, uint8_t bs_alter_scale);
uint8_t master_frequency_table(sbr_info* sbr, uint8_t k0, uint8_t k2, uint8_t bs_freq_scale, uint8_t bs_alter_scale);
uint8_t derived_frequency_table(sbr_info* sbr, uint8_t bs_xover_band, uint8_t k2);
void    limiter_frequency_table(sbr_info* sbr);
int8_t  GASpecificConfig(bitfile* ld, mp4AudioSpecificConfig* mp4ASC, program_config* pce);
uint8_t adts_frame(adts_header* adts, bitfile* ld);
void    get_adif_header(adif_header* adif, bitfile* ld);
void    raw_data_block(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, bitfile* ld, program_config* pce, drc_info* drc);
uint8_t reordered_spectral_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld, int16_t* spectral_data);
uint32_t   faad_latm_frame(latm_header* latm, bitfile* ld);
qmfa_info* qmfa_init(uint8_t channels);
void       qmfa_end(qmfa_info* qmfa);
qmfs_info* qmfs_init(uint8_t channels);
void       qmfs_end(qmfs_info* qmfs);
void       sbr_qmf_analysis_32(sbr_info* sbr, qmfa_info* qmfa, const int32_t* input, complex_t X[MAX_NTSRHFG][64], uint8_t offset, uint8_t kx);
void       sbr_qmf_synthesis_32(sbr_info* sbr, qmfs_info* qmfs, complex_t X[MAX_NTSRHFG][64], int32_t* output);
void       sbr_qmf_synthesis_64(sbr_info* sbr, qmfs_info* qmfs, complex_t X[MAX_NTSRHFG][64], int32_t* output);

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
