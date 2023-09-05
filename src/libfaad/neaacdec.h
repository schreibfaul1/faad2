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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* LD can't do without LTP */
#ifdef LD_DEC
    #ifndef ERROR_RESILIENCE
        #define ERROR_RESILIENCE
    #endif
    #ifndef LTP_DEC
        #define LTP_DEC
    #endif
#endif

typedef int32_t      complex_t[2];
typedef void*        NeAACDecHandle;
typedef const int8_t (*ps_huff_tab)[2];
typedef const int8_t (*sbr_huff_tab)[2];
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
#define MAX_NTSRHFG          40     /* MAX_NTSRHFG: maximum of number_time_slots * rate + HFGen. 16*2+8 */
#define MAX_NTSR             32     /* max number_time_slots * rate */
#define MAX_M                49     /* MAX_M: maximum value for M */
#define MAX_L_E              5      /* MAX_L_E: maximum value for L_E */
#define DRC_REF_LEVEL        20 * 4 /* -20 dB */
#define NUM_ERROR_MESSAGES   34
#define ZERO_HCB             0
#define FIRST_PAIR_HCB       5
#define ESC_HCB              11
#define QUAD_LEN             4
#define PAIR_LEN             2
#define NOISE_HCB            13
#define INTENSITY_HCB2       14
#define INTENSITY_HCB        15
#define IQ_TABLE_SIZE        1026
#define NUM_CB               6
#define NUM_CB_ER            22
#define MAX_CB               32
#define VCB11_FIRST          16
#define VCB11_LAST           31
#define NOISE_OFFSET         90
#define NEGATE_IPD_MASK      (0x1000)
#define T_HFGEN              8
#define T_HFADJ              2
#define EXT_SBR_DATA         13
#define EXT_SBR_DATA_CRC     14
#define FIXFIX               0
#define FIXVAR               1
#define VARFIX               2
#define VARVAR               3
#define LO_RES               0
#define HI_RES               1
#define NO_TIME_SLOTS_960    15
#define NO_TIME_SLOTS        16
#define RATE                 2
#define ESC_VAL              7

#define NOISE_FLOOR_OFFSET 6

#define EPS                 (1) /* smallest number available in fixed point */
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
#define FRAC_CONST(A)       (((A) == 1.00) ? ((int32_t)FRAC_MAX) : (((A) >= 0) ? ((int32_t)((A) * (FRAC_PRECISION) + 0.5)) : ((int32_t)((A) * (FRAC_PRECISION)-0.5))))
#define DECAY_SLOPE         FRAC_CONST(0.05)
#define COEF_SQRT2          COEF_CONST(1.4142135623731)
#define Q2_BITS             22
#define Q2_PRECISION        (1 << Q2_BITS)
#define Q2_CONST(A)         (((A) >= 0) ? ((int32_t)((A) * (Q2_PRECISION) + 0.5)) : ((int32_t)((A) * (Q2_PRECISION)-0.5)))
#define MUL_R(A, B)         (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (REAL_BITS - 1))) >> REAL_BITS) /* multiply with real shift */
#define MUL_C(A, B)         (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (COEF_BITS - 1))) >> COEF_BITS) /* multiply with coef shift */
#define _MulHigh(A, B)      (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (FRAC_SIZE - 1))) >> FRAC_SIZE) /* multiply with fractional shift */
#define MUL_F(A, B)         (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (FRAC_BITS - 1))) >> FRAC_BITS)
#define MUL_Q2(A, B)        (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (Q2_BITS - 1))) >> Q2_BITS)
#define MUL_SHIFT6(A, B)    (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (6 - 1))) >> 6)
#define MUL_SHIFT23(A, B)   (int32_t)(((int64_t)(A) * (int64_t)(B) + (1 << (23 - 1))) >> 23)
#define RE(A)               A[0]
#define IM(A)               A[1]
#define DIV_R(A, B)         (((int64_t)A << REAL_BITS) / B)
#define DIV_C(A, B)         (((int64_t)A << COEF_BITS) / B)
#define QMF_RE(A)           RE(A)
#define QMF_IM(A)           IM(A)
#define DM_MUL              FRAC_CONST(0.3203772410170407)    // 1/(1+sqrt(2) + 1/sqrt(2))
#define RSQRT2              FRAC_CONST(0.7071067811865475244) // 1/sqrt(2)
#define segmentWidth(cb)    min(maxCwLen[cb], ics->length_of_longest_codeword)
#define DIV(A, B)           (((int64_t)A << REAL_BITS) / B)
#define bit_set(A, B)       ((A) & (1 << (B)))

#define step(shift)                                  \
    if((0x40000000l >> shift) + root <= value) {     \
        value -= (0x40000000l >> shift) + root;      \
        root = (root >> 1) | (0x40000000l >> shift); \
    }                                                \
    else { root = root >> 1; }

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

#include "structs.h"
#include "tables.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                              P R O T O T Y P E S
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const char*                NeAACDecGetErrorMessage(unsigned char errcode);
unsigned long              NeAACDecGetCapabilities(void);
NeAACDecHandle             NeAACDecOpen(void);
NeAACDecConfigurationPtr_t NeAACDecGetCurrentConfiguration(NeAACDecHandle hDecoder);
unsigned char              NeAACDecSetConfiguration(NeAACDecHandle hDecoder, NeAACDecConfigurationPtr_t config);
long                       NeAACDecInit(NeAACDecHandle hDecoder, unsigned char* buffer, unsigned long buffer_size, unsigned long* samplerate, unsigned char* channels);
char                       NeAACDecInit2(NeAACDecHandle hDecoder, unsigned char* pBuffer, unsigned long SizeOfDecoderSpecificInfo, unsigned long* samplerate, unsigned char* channels);
void                       NeAACDecPostSeekReset(NeAACDecHandle hDecoder, long frame);
void                       NeAACDecClose(NeAACDecHandle hDecoder);
void*                      NeAACDecDecode(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size);
void*                      NeAACDecDecode2(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer, unsigned long sample_buffer_size);
char                       NeAACDecAudioSpecificConfig(unsigned char* pBuffer, unsigned long buffer_size, mp4AudioSpecificConfig* mp4ASC);
int                        NeAACDecGetVersion(const char** faad_id_string, const char** faad_copyright_string);
void                       faad_initbits(bitfile* ld, const void* buffer, const uint32_t buffer_size);
void                       faad_initbits_rev(bitfile* ld, void* buffer, uint32_t bits_in_buffer);
uint8_t                    faad_byte_align(bitfile* ld);
uint32_t                   faad_get_processed_bits(bitfile* ld);
void                       faad_flushbits_ex(bitfile* ld, uint32_t bits);
void                       faad_rewindbits(bitfile* ld);
void                       faad_resetbits(bitfile* ld, int bits);
uint8_t*                   faad_getbitbuffer(bitfile* ld, uint32_t bits);
uint32_t                   ne_rng(uint32_t* __r1, uint32_t* __r2);
uint32_t                   wl_min_lzc(uint32_t x);
int32_t                    log2_int(uint32_t val);
int32_t                    log2_fix(uint32_t val);
int32_t                    pow2_int(int32_t val);
int32_t                    pow2_fix(int32_t val);
uint8_t                    get_sr_index(const uint32_t samplerate);
uint8_t                    max_pred_sfb(const uint8_t sr_index);
uint8_t                    max_tns_sfb(const uint8_t sr_index, const uint8_t object_type, const uint8_t is_short);
uint32_t                   get_sample_rate(const uint8_t sr_index);
int8_t                     can_decode_ot(const uint8_t object_type);
void*                      faad_malloc(size_t size);
void                       faad_free(void* b);
void                       tns_decode_frame(ic_stream* ics, tns_info* tns, uint8_t sr_index, uint8_t object_type, int32_t* spec, uint16_t frame_len);
void                       tns_encode_frame(ic_stream* ics, tns_info* tns, uint8_t sr_index, uint8_t object_type, int32_t* spec, uint16_t frame_len);
uint16_t                   ps_data(ps_info* ps, bitfile* ld, uint8_t* header);  /* ps_syntax.c */
ps_info*                   ps_init(uint8_t sr_index, uint8_t numTimeSlotsRate); /* ps_dec.c */
void                       ps_free(ps_info* ps);
uint8_t                    ps_decode(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64]);
sbr_info*                  sbrDecodeInit(uint16_t framelength, uint8_t id_aac, uint32_t sample_rate, uint8_t downSampledSBR);
void                       sbrDecodeEnd(sbr_info* sbr);
void                       sbrReset(sbr_info* sbr);
uint8_t                    sbrDecodeCoupleFrame(sbr_info* sbr, int32_t* left_chan, int32_t* right_chan, const uint8_t just_seeked, const uint8_t downSampledSBR);
uint8_t                    sbrDecodeSingleFrame(sbr_info* sbr, int32_t* channel, const uint8_t just_seeked, const uint8_t downSampledSBR);
uint8_t                    sbrDecodeSingleFramePS(sbr_info* sbr, int32_t* left_channel, int32_t* right_channel, const uint8_t just_seeked, const uint8_t downSampledSBR);
uint8_t                    allocate_single_channel(NeAACDecStruct* hDecoder, uint8_t channel, uint8_t output_channels);
uint8_t                    window_grouping_info(NeAACDecStruct* hDecoder, ic_stream* ics);
uint8_t                    reconstruct_channel_pair(NeAACDecStruct* hDecoder, ic_stream* ics1, ic_stream* ics2, element* cpe, int16_t* spec_data1, int16_t* spec_data2);
uint8_t                    reconstruct_single_channel(NeAACDecStruct* hDecoder, ic_stream* ics, element* sce, int16_t* spec_data);
uint8_t                    envelope_time_border_vector(sbr_info* sbr, uint8_t ch);
void                       noise_floor_time_border_vector(sbr_info* sbr, uint8_t ch);
uint8_t                    qmf_start_channel(uint8_t bs_start_freq, uint8_t bs_samplerate_mode, uint32_t sample_rate);
uint8_t                    qmf_stop_channel(uint8_t bs_stop_freq, uint32_t sample_rate, uint8_t k0);
uint8_t                    master_frequency_table_fs0(sbr_info* sbr, uint8_t k0, uint8_t k2, uint8_t bs_alter_scale);
uint8_t                    master_frequency_table(sbr_info* sbr, uint8_t k0, uint8_t k2, uint8_t bs_freq_scale, uint8_t bs_alter_scale);
uint8_t                    derived_frequency_table(sbr_info* sbr, uint8_t bs_xover_band, uint8_t k2);
void                       limiter_frequency_table(sbr_info* sbr);
int8_t                     GASpecificConfig(bitfile* ld, mp4AudioSpecificConfig* mp4ASC, program_config* pce);
uint8_t                    adts_frame(adts_header* adts, bitfile* ld);
void                       get_adif_header(adif_header* adif, bitfile* ld);
void                       raw_data_block(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, bitfile* ld, program_config* pce, drc_info* drc);
uint8_t                    reordered_spectral_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld, int16_t* spectral_data);
uint32_t                   faad_latm_frame(latm_header* latm, bitfile* ld);
qmfa_info*                 qmfa_init(uint8_t channels);
void                       qmfa_end(qmfa_info* qmfa);
qmfs_info*                 qmfs_init(uint8_t channels);
void                       qmfs_end(qmfs_info* qmfs);
void                       sbr_qmf_analysis_32(sbr_info* sbr, qmfa_info* qmfa, const int32_t* input, complex_t X[MAX_NTSRHFG][64], uint8_t offset, uint8_t kx);
void                       sbr_qmf_synthesis_32(sbr_info* sbr, qmfs_info* qmfs, complex_t X[MAX_NTSRHFG][64], int32_t* output);
void                       sbr_qmf_synthesis_64(sbr_info* sbr, qmfs_info* qmfs, complex_t X[MAX_NTSRHFG][64], int32_t* output);
void                       cfftf(cfft_info* cfft, complex_t* c);
void                       cfftb(cfft_info* cfft, complex_t* c);
cfft_info*                 cffti(uint16_t n);
void                       cfftu(cfft_info* cfft);
void                       is_decode(ic_stream* ics, ic_stream* icsr, int32_t* l_spec, int32_t* r_spec, uint16_t frame_len);
static void                passf2pos(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa);
static void                passf2neg(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa);
static void                passf3(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const int8_t isign);
static void                passf4pos(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3);
static void                passf4neg(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3);
static void                passf5(const uint16_t ido, const uint16_t l1, const complex_t* cc, complex_t* ch, const complex_t* wa1, const complex_t* wa2, const complex_t* wa3, const complex_t* wa4, const int8_t isign);
static void                cffti1(uint16_t n, complex_t* wa, uint16_t* ifac);
drc_info*                  drc_init(int32_t cut, int32_t boost);
void                       drc_end(drc_info* drc);
void                       drc_decode(drc_info* drc, int32_t* spec);
static void                tns_decode_coef(uint8_t order, uint8_t coef_res_bits, uint8_t coef_compress, uint8_t* coef, int32_t* a);
static void                tns_ar_filter(int32_t* spectrum, uint16_t size, int8_t inc, int32_t* lpc, uint8_t order);
static void                tns_ma_filter(int32_t* spectrum, uint16_t size, int8_t inc, int32_t* lpc, uint8_t order);
void                       dct4_kernel(int32_t* in_real, int32_t* in_imag, int32_t* out_real, int32_t* out_imag);
void                       DCT4_32(int32_t* y, int32_t* x);
void                       DST4_32(int32_t* y, int32_t* x);
static void                decode_sce_lfe(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, bitfile* ld, uint8_t id_syn_ele);
static void                decode_cpe(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, bitfile* ld, uint8_t id_syn_ele);
static uint8_t             single_lfe_channel_element(NeAACDecStruct* hDecoder, bitfile* ld, uint8_t channel, uint8_t* tag);
static uint8_t             channel_pair_element(NeAACDecStruct* hDecoder, bitfile* ld, uint8_t channel, uint8_t* tag);
static uint16_t            data_stream_element(NeAACDecStruct* hDecoder, bitfile* ld);
static uint8_t             program_config_element(program_config* pce, bitfile* ld);
static uint8_t             fill_element(NeAACDecStruct* hDecoder, bitfile* ld, drc_info* drc, uint8_t sbr_ele);
static uint8_t             individual_channel_stream(NeAACDecStruct* hDecoder, element* ele, bitfile* ld, ic_stream* ics, uint8_t scal_flag, int16_t* spec_data);
static uint8_t             ics_info(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld, uint8_t common_window);
static uint8_t             section_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld);
static uint8_t             scale_factor_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld);
static uint8_t             spectral_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld, int16_t* spectral_data);
static uint16_t            extension_payload(bitfile* ld, drc_info* drc, uint16_t count);
static uint8_t             pulse_data(ic_stream* ics, pulse_info* pul, bitfile* ld);
static void                tns_data(ic_stream* ics, tns_info* tns, bitfile* ld);
#ifdef LTP_DEC
static uint8_t ltp_data(NeAACDecStruct* hDecoder, ic_stream* ics, ltp_info* ltp, bitfile* ld);
#endif
static uint8_t        adts_fixed_header(adts_header* adts, bitfile* ld);
static void           adts_variable_header(adts_header* adts, bitfile* ld);
static void           adts_error_check(adts_header* adts, bitfile* ld);
static uint8_t        dynamic_range_info(bitfile* ld, drc_info* drc);
static uint8_t        excluded_channels(bitfile* ld, drc_info* drc);
static uint8_t        side_info(NeAACDecStruct* hDecoder, element* ele, bitfile* ld, ic_stream* ics, uint8_t scal_flag);
static void*          aac_frame_decode(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer2, unsigned long sample_buffer_size);
static void           create_channel_config(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo);
void*                 output_to_PCM(NeAACDecStruct* hDecoder, int32_t** input, void* samplebuffer, uint8_t channels, uint16_t frame_len, uint8_t format);
fb_info*              filter_bank_init(uint16_t frame_len);
void                  filter_bank_end(fb_info* fb);
void                  filter_bank_ltp(fb_info* fb, uint8_t window_sequence, uint8_t window_shape, uint8_t window_shape_prev, int32_t* in_data, int32_t* out_mdct, uint8_t object_type, uint16_t frame_len);
void                  ifilter_bank(fb_info* fb, uint8_t window_sequence, uint8_t window_shape, uint8_t window_shape_prev, int32_t* freq_in, int32_t* time_out, int32_t* overlap, uint8_t object_type, uint16_t frame_len);
int8_t                AudioSpecificConfig2(uint8_t* pBuffer, uint32_t buffer_size, mp4AudioSpecificConfig* mp4ASC, program_config* pce, uint8_t short_form);
int8_t                AudioSpecificConfigFromBitfile(bitfile* ld, mp4AudioSpecificConfig* mp4ASC, program_config* pce, uint32_t bsize, uint8_t short_form);
mdct_info*            faad_mdct_init(uint16_t N);
void                  faad_mdct_end(mdct_info* mdct);
void                  faad_imdct(mdct_info* mdct, int32_t* X_in, int32_t* X_out);
void                  faad_mdct(mdct_info* mdct, int32_t* X_in, int32_t* X_out);
static inline void    huffman_sign_bits(bitfile* ld, int16_t* sp, uint8_t len);
static inline uint8_t huffman_getescape(bitfile* ld, int16_t* sp);
static uint8_t        huffman_2step_quad(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_2step_quad_sign(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_2step_pair(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_2step_pair_sign(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_binary_quad(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_binary_quad_sign(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_binary_pair(uint8_t cb, bitfile* ld, int16_t* sp);
static uint8_t        huffman_binary_pair_sign(uint8_t cb, bitfile* ld, int16_t* sp);
static int16_t        huffman_codebook(uint8_t i);
static void           vcb11_check_LAV(uint8_t cb, int16_t* sp);
int8_t                huffman_scale_factor(bitfile* ld);
uint8_t               huffman_spectral_data(uint8_t cb, bitfile* ld, int16_t* sp);
#ifdef ERROR_RESILIENCE
int8_t huffman_spectral_data_2(uint8_t cb, bits_t* ld, int16_t* sp);
#endif
static void      gen_rand_vector(int32_t* spec, int16_t scale_factor, uint16_t size, uint8_t sub, uint32_t* __r1, uint32_t* __r2);
void             pns_decode(ic_stream* ics_left, ic_stream* ics_right, int32_t* spec_left, int32_t* spec_right, uint16_t frame_len, uint8_t channel_pair, uint8_t object_type, uint32_t* __r1, uint32_t* __r2);
void             ms_decode(ic_stream* ics, ic_stream* icsr, int32_t* l_spec, int32_t* r_spec, uint16_t frame_len);
static void      ps_data_decode(ps_info* ps);
static hyb_info* hybrid_init(uint8_t numTimeSlotsRate);
static void      channel_filter2(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid);
static void inline DCT3_4_unscaled(int32_t* y, int32_t* x);
static void          channel_filter8(hyb_info* hyb, uint8_t frame_len, const int32_t* filter, complex_t* buffer, complex_t** X_hybrid);
static void          hybrid_analysis(hyb_info* hyb, complex_t X[32][64], complex_t X_hybrid[32][32], uint8_t use34, uint8_t numTimeSlotsRate);
static void          hybrid_synthesis(hyb_info* hyb, complex_t X[32][64], complex_t X_hybrid[32][32], uint8_t use34, uint8_t numTimeSlotsRate);
static int8_t        delta_clip(int8_t i, int8_t min, int8_t max);
static void          delta_decode(uint8_t enable, int8_t* index, int8_t* index_prev, uint8_t dt_flag, uint8_t nr_par, uint8_t stride, int8_t min_index, int8_t max_index);
static void          delta_modulo_decode(uint8_t enable, int8_t* index, int8_t* index_prev, uint8_t dt_flag, uint8_t nr_par, uint8_t stride, int8_t and_modulo);
static void          map20indexto34(int8_t* index, uint8_t bins);
static void          ps_data_decode(ps_info* ps);
static void          ps_decorrelate(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64], complex_t X_hybrid_left[32][32], complex_t X_hybrid_right[32][32]);
static void          ps_mix_phase(ps_info* ps, complex_t X_left[38][64], complex_t X_right[38][64], complex_t X_hybrid_left[32][32], complex_t X_hybrid_right[32][32]);
static int16_t       real_to_int16(int32_t sig_in);
uint8_t              is_ltp_ot(uint8_t object_type);
void                 lt_prediction(ic_stream* ics, ltp_info* ltp, int32_t* spec, int16_t* lt_pred_stat, fb_info* fb, uint8_t win_shape, uint8_t win_shape_prev, uint8_t sr_index, uint8_t object_type, uint16_t frame_len);
void                 lt_update_state(int16_t* lt_pred_stat, int32_t* time, int32_t* overlap, uint16_t frame_len, uint8_t object_type);
static uint16_t      ps_extension(ps_info* ps, bitfile* ld, const uint8_t ps_extension_id, const uint16_t num_bits_left);
static void          huff_data(bitfile* ld, const uint8_t dt, const uint8_t nr_par, ps_huff_tab t_huff, ps_huff_tab f_huff, int8_t* par);
static inline int8_t ps_huff_dec(bitfile* ld, ps_huff_tab t_huff);
uint8_t              pulse_decode(ic_stream* ics, int16_t* spec_coef, uint16_t framelen);
static uint8_t       sbr_save_prev_data(sbr_info* sbr, uint8_t ch);
static void          sbr_save_matrix(sbr_info* sbr, uint8_t ch);
void                 sbr_envelope(bitfile* ld, sbr_info* sbr, uint8_t ch);
void                 sbr_noise(bitfile* ld, sbr_info* sbr, uint8_t ch);
static uint8_t       rvlc_decode_sf_forward(ic_stream* ics, bitfile* ld_sf, bitfile* ld_esc, uint8_t* is_used);
static int8_t        rvlc_huffman_sf(bitfile* ld_sf, bitfile* ld_esc, int8_t direction);
static int8_t        rvlc_huffman_esc(bitfile* ld_esc, int8_t direction);
uint8_t              rvlc_scale_factor_data(ic_stream* ics, bitfile* ld);
uint8_t              rvlc_decode_scale_factors(ic_stream* ics, bitfile* ld);
void                 hf_generation(sbr_info* sbr, complex_t Xlow[MAX_NTSRHFG][64], complex_t Xhigh[MAX_NTSRHFG][64], uint8_t ch);
uint8_t              hf_adjustment(sbr_info* sbr, complex_t Xsbr[MAX_NTSRHFG][64], uint8_t ch);
void                 extract_envelope_data(sbr_info* sbr, uint8_t ch);
void                 extract_noise_floor_data(sbr_info* sbr, uint8_t ch);
static int32_t       find_bands(uint8_t warp, uint8_t bands, uint8_t a0, uint8_t a1);
static uint8_t       estimate_current_envelope(sbr_info* sbr, sbr_hfadj_info* adj, complex_t Xsbr[MAX_NTSRHFG][64], uint8_t ch);
static void          calculate_gain(sbr_info* sbr, sbr_hfadj_info* adj, uint8_t ch);
static void          hf_assembly(sbr_info* sbr, sbr_hfadj_info* adj, complex_t Xsbr[MAX_NTSRHFG][64], uint8_t ch);
static void          calc_prediction_coef(sbr_info* sbr, complex_t Xlow[MAX_NTSRHFG][64], complex_t* alpha_0, complex_t* alpha_1, uint8_t k);
static void          calc_chirp_factors(sbr_info* sbr, uint8_t ch);
static void          patch_construction(sbr_info* sbr);
static void          sbr_header(bitfile* ld, sbr_info* sbr);
static uint8_t       calc_sbr_tables(sbr_info* sbr, uint8_t start_freq, uint8_t stop_freq, uint8_t samplerate_mode, uint8_t freq_scale, uint8_t alter_scale, uint8_t xover_band);
static uint8_t       sbr_data(bitfile* ld, sbr_info* sbr);
static uint16_t      sbr_extension(bitfile* ld, sbr_info* sbr, uint8_t bs_extension_id, uint16_t num_bits_left);
static uint8_t       sbr_single_channel_element(bitfile* ld, sbr_info* sbr);
static uint8_t       sbr_channel_pair_element(bitfile* ld, sbr_info* sbr);
static uint8_t       sbr_grid(bitfile* ld, sbr_info* sbr, uint8_t ch);
static void          sbr_dtdf(bitfile* ld, sbr_info* sbr, uint8_t ch);
static void          invf_mode(bitfile* ld, sbr_info* sbr, uint8_t ch);
static void          sinusoidal_coding(bitfile* ld, sbr_info* sbr, uint8_t ch);
static uint8_t       middleBorder(sbr_info* sbr, uint8_t ch);
static uint8_t       quant_to_spec(NeAACDecStruct* hDecoder, ic_stream* ics, int16_t* quant_data, int32_t* spec_data, uint16_t frame_len);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                              I N L I N E S
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* circumvent memory alignment errors on ARM */
static inline uint32_t getdword(void* mem) {
    uint32_t tmp;
    ((uint8_t*)&tmp)[0] = ((uint8_t*)mem)[3];
    ((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[2];
    ((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[1];
    ((uint8_t*)&tmp)[3] = ((uint8_t*)mem)[0];
    return tmp;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline uint32_t faad_showbits(bitfile* ld, uint32_t bits) {
    if(bits <= ld->bits_left) {
        // return (ld->bufa >> (ld->bits_left - bits)) & bitmask[bits];
        return (ld->bufa << (32 - ld->bits_left)) >> (32 - bits);
    }
    bits -= ld->bits_left;
    // return ((ld->bufa & bitmask[ld->bits_left]) << bits) | (ld->bufb >> (32 - bits));
    return ((ld->bufa & ((1 << ld->bits_left) - 1)) << bits) | (ld->bufb >> (32 - bits));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline void faad_flushbits(bitfile* ld, uint32_t bits) {
    /* do nothing if error */
    if(ld->error != 0) return;
    if(bits < ld->bits_left) { ld->bits_left -= bits; }
    else { faad_flushbits_ex(ld, bits); }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* return next n bits (right adjusted) */
static /*inline*/ uint32_t faad_getbits(bitfile* ld, uint32_t n) {
    uint32_t ret;
    if(n == 0) return 0;
    ret = faad_showbits(ld, n);
    faad_flushbits(ld, n);
    return ret;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static /*inline*/ uint32_t faad_getbits_rev(bitfile* ld, uint32_t n) {
    uint32_t ret;

    if(n == 0) return 0;

    ret = faad_showbits_rev(ld, n);
    faad_flushbits_rev(ld, n);

    return ret;
}

#ifdef ERROR_RESILIENCE
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* return 1 if position is outside of buffer, 0 otherwise */
static inline int8_t flushbits_hcr(bits_t* ld, uint8_t bits) {
    ld->len -= bits;

    if(ld->len < 0) {
        ld->len = 0;
        return 1;
    }
    else { return 0; }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t getbits_hcr(bits_t* ld, uint8_t n, uint32_t* result) {
    *result = showbits_hcr(ld, n);
    return flushbits_hcr(ld, n);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t get1bit_hcr(bits_t* ld, uint8_t* result) {
    uint32_t res;
    int8_t   ret;

    ret = getbits_hcr(ld, 1, &res);
    *result = (int8_t)(res & 1);
    return ret;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // ERROR_RESILIENCE

static inline int8_t is_intensity(ic_stream* ics, uint8_t group, uint8_t sfb) {
    switch(ics->sfb_cb[group][sfb]) {
    case INTENSITY_HCB: return 1;
    case INTENSITY_HCB2: return -1;
    default: return 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t invert_intensity(ic_stream* ics, uint8_t group, uint8_t sfb) {
    if(ics->ms_mask_present == 1) return (1 - 2 * ics->ms_used[group][sfb]);
    return 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* Complex multiplication */
inline void ComplexMult(int32_t* y1, int32_t* y2, int32_t x1, int32_t x2, int32_t c1, int32_t c2) {
    *y1 = (_MulHigh(x1, c1) + _MulHigh(x2, c2)) << (FRAC_SIZE - FRAC_BITS);
    *y2 = (_MulHigh(x2, c1) - _MulHigh(x1, c2)) << (FRAC_SIZE - FRAC_BITS);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static inline uint8_t is_noise(ic_stream* ics, uint8_t group, uint8_t sfb) {
    if(ics->sfb_cb[group][sfb] == NOISE_HCB) return 1;
    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
