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
** $Id: specrec.c,v 1.63 2010/06/04 20:47:56 menno Exp $
**/

/*
  Spectral reconstruction:
   - grouping/sectioning
   - inverse quantization
   - applying scalefactors
*/
#include "neaacdec.h"

#include "filtbank.h"
#include "iq_table.h"
#include "is.h"
#include "lt_predict.h"
#include "ms.h"
#include "pns.h"
#include <stdlib.h>
#include <string.h>

/* static function declarations */
static uint8_t quant_to_spec(NeAACDecStruct* hDecoder, ic_stream* ics, int16_t* quant_data, int32_t* spec_data, uint16_t frame_len);

#ifdef LD_DEC
static const uint8_t num_swb_512_window[] = {0, 0, 0, 36, 36, 37, 31, 31, 0, 0, 0, 0};
static const uint8_t num_swb_480_window[] = {0, 0, 0, 35, 35, 37, 30, 30, 0, 0, 0, 0};
#endif

static const uint8_t num_swb_960_window[] = {40, 40, 45, 49, 49, 49, 46, 46, 42, 42, 42, 40};

static const uint8_t num_swb_1024_window[] = {41, 41, 47, 49, 49, 51, 47, 47, 43, 43, 43, 40};

static const uint8_t num_swb_128_window[] = {12, 12, 12, 14, 14, 14, 15, 15, 15, 15, 15, 15};

static const uint16_t swb_offset_1024_96[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,
                                              56,  64,  72,  80,  88,  96,  108, 120, 132, 144, 156, 172, 188, 212,
                                              240, 276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024};

static const uint16_t swb_offset_128_96[] = {0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128};

static const uint16_t swb_offset_1024_64[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  64,
                                              72,  80,  88,  100, 112, 124, 140, 156, 172, 192, 216, 240, 268, 304, 344, 384,
                                              424, 464, 504, 544, 584, 624, 664, 704, 744, 784, 824, 864, 904, 944, 984, 1024};

static const uint16_t swb_offset_128_64[] = {0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128};

static const uint16_t swb_offset_1024_48[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,  56,  64,  72,  80,  88,
                                              96,  108, 120, 132, 144, 160, 176, 196, 216, 240, 264, 292, 320, 352, 384, 416, 448,
                                              480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 1024};

#ifdef LD_DEC
static const uint16_t swb_offset_512_48[] = {0,  4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  68,  76, 84,
                                             92, 100, 112, 124, 136, 148, 164, 184, 208, 236, 268, 300, 332, 364, 396, 428, 460, 512};

static const uint16_t swb_offset_480_48[] = {0,  4,  8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  64,  72,  80,
                                             88, 96, 108, 120, 132, 144, 156, 172, 188, 212, 240, 272, 304, 336, 368, 400, 432, 480};
#endif

static const uint16_t swb_offset_128_48[] = {0, 4, 8, 12, 16, 20, 28, 36, 44, 56, 68, 80, 96, 112, 128};

static const uint16_t swb_offset_1024_32[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,  56,  64,  72,  80,  88,  96,
                                              108, 120, 132, 144, 160, 176, 196, 216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512,
                                              544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024};

#ifdef LD_DEC
static const uint16_t swb_offset_512_32[] = {0,  4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  64,  72,  80,  88,
                                             96, 108, 120, 132, 144, 160, 176, 192, 212, 236, 260, 288, 320, 352, 384, 416, 448, 480, 512};

static const uint16_t swb_offset_480_32[] = {0,  4,  8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  72,  80,
                                             88, 96, 104, 112, 124, 136, 148, 164, 180, 200, 224, 256, 288, 320, 352, 384, 416, 448, 480};
#endif

static const uint16_t swb_offset_1024_24[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  52,  60,  68,  76,
                                              84,  92,  100, 108, 116, 124, 136, 148, 160, 172, 188, 204, 220, 240, 260, 284,
                                              308, 336, 364, 396, 432, 468, 508, 552, 600, 652, 704, 768, 832, 896, 960, 1024};

#ifdef LD_DEC
static const uint16_t swb_offset_512_24[] = {0,  4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  52,  60,  68,  80,
                                             92, 104, 120, 140, 164, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512};

static const uint16_t swb_offset_480_24[] = {0,  4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  52,  60,  68, 80,
                                             92, 104, 120, 140, 164, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480};
#endif

static const uint16_t swb_offset_128_24[] = {0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 64, 76, 92, 108, 128};

static const uint16_t swb_offset_1024_16[] = {0,   8,   16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  100, 112, 124,
                                              136, 148, 160, 172, 184, 196, 212, 228, 244, 260, 280, 300, 320, 344, 368,
                                              396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832, 896, 960, 1024};

static const uint16_t swb_offset_128_16[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 60, 72, 88, 108, 128};

static const uint16_t swb_offset_1024_8[] = {0,   12,  24,  36,  48,  60,  72,  84,  96,  108, 120, 132, 144, 156, 172, 188, 204, 220, 236, 252, 268,
                                             288, 308, 328, 348, 372, 396, 420, 448, 476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024};

static const uint16_t swb_offset_128_8[] = {0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 60, 72, 88, 108, 128};

static const uint16_t* swb_offset_1024_window[] = {
    swb_offset_1024_96, /* 96000 */
    swb_offset_1024_96, /* 88200 */
    swb_offset_1024_64, /* 64000 */
    swb_offset_1024_48, /* 48000 */
    swb_offset_1024_48, /* 44100 */
    swb_offset_1024_32, /* 32000 */
    swb_offset_1024_24, /* 24000 */
    swb_offset_1024_24, /* 22050 */
    swb_offset_1024_16, /* 16000 */
    swb_offset_1024_16, /* 12000 */
    swb_offset_1024_16, /* 11025 */
    swb_offset_1024_8   /* 8000  */
};

#ifdef LD_DEC
static const uint16_t* swb_offset_512_window[] = {
    0,                 /* 96000 */
    0,                 /* 88200 */
    0,                 /* 64000 */
    swb_offset_512_48, /* 48000 */
    swb_offset_512_48, /* 44100 */
    swb_offset_512_32, /* 32000 */
    swb_offset_512_24, /* 24000 */
    swb_offset_512_24, /* 22050 */
    0,                 /* 16000 */
    0,                 /* 12000 */
    0,                 /* 11025 */
    0                  /* 8000  */
};

static const uint16_t* swb_offset_480_window[] = {
    0,                 /* 96000 */
    0,                 /* 88200 */
    0,                 /* 64000 */
    swb_offset_480_48, /* 48000 */
    swb_offset_480_48, /* 44100 */
    swb_offset_480_32, /* 32000 */
    swb_offset_480_24, /* 24000 */
    swb_offset_480_24, /* 22050 */
    0,                 /* 16000 */
    0,                 /* 12000 */
    0,                 /* 11025 */
    0                  /* 8000  */
};
#endif

static const uint16_t* swb_offset_128_window[] = {
    swb_offset_128_96, /* 96000 */
    swb_offset_128_96, /* 88200 */
    swb_offset_128_64, /* 64000 */
    swb_offset_128_48, /* 48000 */
    swb_offset_128_48, /* 44100 */
    swb_offset_128_48, /* 32000 */
    swb_offset_128_24, /* 24000 */
    swb_offset_128_24, /* 22050 */
    swb_offset_128_16, /* 16000 */
    swb_offset_128_16, /* 12000 */
    swb_offset_128_16, /* 11025 */
    swb_offset_128_8   /* 8000  */
};

#define bit_set(A, B) ((A) & (1 << (B)))


//----------------------------------------------------------------------------------------------------------------------------------------------------
/*
  - determine the number of windows in a window_sequence named num_windows
  - determine the number of window_groups named num_window_groups
  - determine the number of windows in each group named window_group_length[g]
  - determine the total number of scalefactor window bands named num_swb for the actual window type
  - determine swb_offset[swb], the offset of the first coefficient in scalefactor window band named swb of the window actually used
  - determine sect_sfb_offset[g][section],the offset of the first coefficient in section named section. This offset depends on window_sequence and
    scale_factor_grouping and is needed to decode the spectral_data().
*/
uint8_t window_grouping_info(NeAACDecStruct* hDecoder, ic_stream* ics) {
    uint8_t i, g;

    uint8_t sf_index = hDecoder->sf_index;

    switch(ics->window_sequence) {
    case ONLY_LONG_SEQUENCE:
    case LONG_START_SEQUENCE:
    case LONG_STOP_SEQUENCE:
        ics->num_windows = 1;
        ics->num_window_groups = 1;
        ics->window_group_length[ics->num_window_groups - 1] = 1;
#ifdef LD_DEC
        if(hDecoder->object_type == LD) {
            if(hDecoder->frameLength == 512) ics->num_swb = num_swb_512_window[sf_index];
            else /* if (hDecoder->frameLength == 480) */
                ics->num_swb = num_swb_480_window[sf_index];
        }
        else {
#endif
            if(hDecoder->frameLength == 1024) ics->num_swb = num_swb_1024_window[sf_index];
            else /* if (hDecoder->frameLength == 960) */
                ics->num_swb = num_swb_960_window[sf_index];
#ifdef LD_DEC
        }
#endif
        if(ics->max_sfb > ics->num_swb) { return 32; }
        /* preparation of sect_sfb_offset for long blocks also copy the last value! */
#ifdef LD_DEC
        if(hDecoder->object_type == LD) {
            if(hDecoder->frameLength == 512) {
                for(i = 0; i < ics->num_swb; i++) {
                    ics->sect_sfb_offset[0][i] = swb_offset_512_window[sf_index][i];
                    ics->swb_offset[i] = swb_offset_512_window[sf_index][i];
                }
            }
            else /* if (hDecoder->frameLength == 480) */ {
                for(i = 0; i < ics->num_swb; i++) {
                    ics->sect_sfb_offset[0][i] = swb_offset_480_window[sf_index][i];
                    ics->swb_offset[i] = swb_offset_480_window[sf_index][i];
                }
            }
            ics->sect_sfb_offset[0][ics->num_swb] = hDecoder->frameLength;
            ics->swb_offset[ics->num_swb] = hDecoder->frameLength;
            ics->swb_offset_max = hDecoder->frameLength;
        }
        else {
#endif
            for(i = 0; i < ics->num_swb; i++) {
                ics->sect_sfb_offset[0][i] = swb_offset_1024_window[sf_index][i];
                ics->swb_offset[i] = swb_offset_1024_window[sf_index][i];
            }
            ics->sect_sfb_offset[0][ics->num_swb] = hDecoder->frameLength;
            ics->swb_offset[ics->num_swb] = hDecoder->frameLength;
            ics->swb_offset_max = hDecoder->frameLength;
#ifdef LD_DEC
        }
#endif
        return 0;
    case EIGHT_SHORT_SEQUENCE:
        ics->num_windows = 8;
        ics->num_window_groups = 1;
        ics->window_group_length[ics->num_window_groups - 1] = 1;
        ics->num_swb = num_swb_128_window[sf_index];
        if(ics->max_sfb > ics->num_swb) { return 32; }
        for(i = 0; i < ics->num_swb; i++) ics->swb_offset[i] = swb_offset_128_window[sf_index][i];
        ics->swb_offset[ics->num_swb] = hDecoder->frameLength / 8;
        ics->swb_offset_max = hDecoder->frameLength / 8;
        for(i = 0; i < ics->num_windows - 1; i++) {
            if(bit_set(ics->scale_factor_grouping, 6 - i) == 0) {
                ics->num_window_groups += 1;
                ics->window_group_length[ics->num_window_groups - 1] = 1;
            }
            else { ics->window_group_length[ics->num_window_groups - 1] += 1; }
        }
        /* preparation of sect_sfb_offset for short blocks */
        for(g = 0; g < ics->num_window_groups; g++) {
            uint16_t width;
            uint8_t  sect_sfb = 0;
            uint16_t offset = 0;
            for(i = 0; i < ics->num_swb; i++) {
                if(i + 1 == ics->num_swb) { width = (hDecoder->frameLength / 8) - swb_offset_128_window[sf_index][i]; }
                else { width = swb_offset_128_window[sf_index][i + 1] - swb_offset_128_window[sf_index][i]; }
                width *= ics->window_group_length[g];
                ics->sect_sfb_offset[g][sect_sfb++] = offset;
                offset += width;
            }
            ics->sect_sfb_offset[g][sect_sfb] = offset;
        }
        return 0;
    default: return 32;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* iquant() *
/* output = sign(input)*abs(input)^(4/3) */
/**/
static inline int32_t iquant(int16_t q, const int32_t* tab, uint8_t* error) {
    static const int32_t errcorr[] = {REAL_CONST(0),         REAL_CONST(1.0 / 8.0), REAL_CONST(2.0 / 8.0),
                                      REAL_CONST(3.0 / 8.0), REAL_CONST(4.0 / 8.0), REAL_CONST(5.0 / 8.0),
                                      REAL_CONST(6.0 / 8.0), REAL_CONST(7.0 / 8.0), REAL_CONST(0)};
    int32_t              x1, x2;

    int16_t sgn = 1;

    if(q < 0) {
        q = -q;
        sgn = -1;
    }

    if(q < IQ_TABLE_SIZE) {

#ifdef IQUANT_PRINT // #define IQUANT_PRINT
        // printf("0x%.8X\n", sgn * tab[q]);
        printf("%d\n", sgn * tab[q]);
#endif
        return sgn * tab[q];
    }
    if(q >= 8192) {
        *error = 17;
        return 0;
    }
    /* linear interpolation */
    x1 = tab[q >> 3];
    x2 = tab[(q >> 3) + 1];
    return sgn * 16 * (MUL_R(errcorr[q & 7], (x2 - x1)) + x1);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* quant_to_spec: perform dequantisation and scaling and in case of short block it also does the deinterleaving */
/* For ONLY_LONG_SEQUENCE windows (num_window_groups = 1, window_group_length[0] = 1) the spectral data is in ascending spectral order.
  For the EIGHT_SHORT_SEQUENCE window, the spectral order depends on the grouping in the following manner:
  - Groups are ordered sequentially
  - Within a group, a scalefactor band consists of the spectral data of all grouped SHORT_WINDOWs for the associated scalefactor window band. To
    clarify via example, the length of a group is in the range of one to eight SHORT_WINDOWs.
  - If there are eight groups each with length one (num_window_groups = 8, window_group_length[0..7] = 1), the result is a sequence of eight spectra,
    each in ascending spectral order.
  - If there is only one group with length eight (num_window_groups = 1, window_group_length[0] = 8), the result is that spectral data of all eight
    SHORT_WINDOWs is interleaved by scalefactor window bands.
  - Within a scalefactor window band, the coefficients are in ascending spectral order.
*/
static uint8_t quant_to_spec(NeAACDecStruct* hDecoder, ic_stream* ics, int16_t* quant_data, int32_t* spec_data, uint16_t frame_len) {
    static const int32_t pow2_table[] = {
        COEF_CONST(1.0), COEF_CONST(1.1892071150027210667174999705605), /* 2^0.25 */
        COEF_CONST(1.4142135623730950488016887242097),                  /* 2^0.5 */
        COEF_CONST(1.6817928305074290860622509524664)                   /* 2^0.75 */
    };
    const int32_t* tab = iq_table;
    uint8_t  g, sfb, win;
    uint16_t width, bin, k, gindex, wa, wb;
    uint8_t  error = 0; /* Init error flag */
    k = 0;
    gindex = 0;
    for(g = 0; g < ics->num_window_groups; g++) {
        uint16_t j = 0;
        uint16_t gincrease = 0;
        uint16_t win_inc = ics->swb_offset[ics->num_swb];
        for(sfb = 0; sfb < ics->num_swb; sfb++) {
            int32_t exp, frac;
            width = ics->swb_offset[sfb + 1] - ics->swb_offset[sfb];
            /* this could be scalefactor for IS or PNS, those can be negative or bigger then 255 */
            /* just ignore them */
            if(ics->scale_factors[g][sfb] < 0 || ics->scale_factors[g][sfb] > 255) {
                exp = 0;
                frac = 0;
            }
            else {
                /* ics->scale_factors[g][sfb] must be between 0 and 255 */
                exp = (ics->scale_factors[g][sfb] /* - 100 */) >> 2;
                /* frac must always be > 0 */
                frac = (ics->scale_factors[g][sfb] /* - 100 */) & 3;
            }
            exp -= 25;
            /* IMDCT pre-scaling */
            if(hDecoder->object_type == LD) { exp -= 6 /*9*/; }
            else {
                if(ics->window_sequence == EIGHT_SHORT_SEQUENCE) exp -= 4 /*7*/;
                else
                    exp -= 7 /*10*/;
            }
            wa = gindex + j;
            for(win = 0; win < ics->window_group_length[g]; win++) {
                for(bin = 0; bin < width; bin += 4) {
                    int32_t iq0 = iquant(quant_data[k + 0], tab, &error);
                    int32_t iq1 = iquant(quant_data[k + 1], tab, &error);
                    int32_t iq2 = iquant(quant_data[k + 2], tab, &error);
                    int32_t iq3 = iquant(quant_data[k + 3], tab, &error);
                    wb = wa + bin;
                    if(exp < 0) {
                        spec_data[wb + 0] = iq0 >>= -exp;
                        spec_data[wb + 1] = iq1 >>= -exp;
                        spec_data[wb + 2] = iq2 >>= -exp;
                        spec_data[wb + 3] = iq3 >>= -exp;
                    }
                    else {
                        spec_data[wb + 0] = iq0 <<= exp;
                        spec_data[wb + 1] = iq1 <<= exp;
                        spec_data[wb + 2] = iq2 <<= exp;
                        spec_data[wb + 3] = iq3 <<= exp;
                    }
                    if(frac != 0) {
                        spec_data[wb + 0] = MUL_C(spec_data[wb + 0], pow2_table[frac]);
                        spec_data[wb + 1] = MUL_C(spec_data[wb + 1], pow2_table[frac]);
                        spec_data[wb + 2] = MUL_C(spec_data[wb + 2], pow2_table[frac]);
                        spec_data[wb + 3] = MUL_C(spec_data[wb + 3], pow2_table[frac]);
                    }
                    gincrease += 4;
                    k += 4;
                }
                wa += win_inc;
            }
            j += width;
        }
        gindex += gincrease;
    }
    return error;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void faad_free(void* b); 

uint8_t allocate_single_channel(NeAACDecStruct* hDecoder, uint8_t channel, uint8_t output_channels) {
    int mul = 1;

#ifdef LTP_DEC
    if(is_ltp_ot(hDecoder->object_type)) {
        /* allocate the state only when needed */
        if(hDecoder->lt_pred_stat[channel] != NULL) {
            faad_free(hDecoder->lt_pred_stat[channel]);
            hDecoder->lt_pred_stat[channel] = NULL;
        }

        hDecoder->lt_pred_stat[channel] = (int16_t*)faad_malloc(hDecoder->frameLength * 4 * sizeof(int16_t));
        memset(hDecoder->lt_pred_stat[channel], 0, hDecoder->frameLength * 4 * sizeof(int16_t));
    }
#endif

    if(hDecoder->time_out[channel] != NULL) {
        faad_free(hDecoder->time_out[channel]);
        hDecoder->time_out[channel] = NULL;
    }

    {
        mul = 1;
#ifdef SBR_DEC
        hDecoder->sbr_alloced[hDecoder->fr_ch_ele] = 0;
        if((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) {
            /* SBR requires 2 times as much output data */
            mul = 2;
            hDecoder->sbr_alloced[hDecoder->fr_ch_ele] = 1;
        }
#endif
        hDecoder->time_out[channel] = (int32_t*)faad_malloc(mul * hDecoder->frameLength * sizeof(int32_t));
        memset(hDecoder->time_out[channel], 0, mul * hDecoder->frameLength * sizeof(int32_t));
    }

#if(defined(PS_DEC))
    if(output_channels == 2) {
        if(hDecoder->time_out[channel + 1] != NULL) {
            faad_free(hDecoder->time_out[channel + 1]);
            hDecoder->time_out[channel + 1] = NULL;
        }

        hDecoder->time_out[channel + 1] = (int32_t*)faad_malloc(mul * hDecoder->frameLength * sizeof(int32_t));
        memset(hDecoder->time_out[channel + 1], 0, mul * hDecoder->frameLength * sizeof(int32_t));
    }
#endif

    if(hDecoder->fb_intermed[channel] != NULL) {
        faad_free(hDecoder->fb_intermed[channel]);
        hDecoder->fb_intermed[channel] = NULL;
    }

    hDecoder->fb_intermed[channel] = (int32_t*)faad_malloc(hDecoder->frameLength * sizeof(int32_t));
    memset(hDecoder->fb_intermed[channel], 0, hDecoder->frameLength * sizeof(int32_t));

    return 0;
}

static uint8_t allocate_channel_pair(NeAACDecStruct* hDecoder, uint8_t channel, uint8_t paired_channel) {
    int mul = 1;



#ifdef LTP_DEC
    if(is_ltp_ot(hDecoder->object_type)) {
        /* allocate the state only when needed */
        if(hDecoder->lt_pred_stat[channel] == NULL) {
            hDecoder->lt_pred_stat[channel] = (int16_t*)faad_malloc(hDecoder->frameLength * 4 * sizeof(int16_t));
            memset(hDecoder->lt_pred_stat[channel], 0, hDecoder->frameLength * 4 * sizeof(int16_t));
        }
        if(hDecoder->lt_pred_stat[paired_channel] == NULL) {
            hDecoder->lt_pred_stat[paired_channel] = (int16_t*)faad_malloc(hDecoder->frameLength * 4 * sizeof(int16_t));
            memset(hDecoder->lt_pred_stat[paired_channel], 0, hDecoder->frameLength * 4 * sizeof(int16_t));
        }
    }
#endif

    if(hDecoder->time_out[channel] == NULL) {
        mul = 1;
#ifdef SBR_DEC
        hDecoder->sbr_alloced[hDecoder->fr_ch_ele] = 0;
        if((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) {
            /* SBR requires 2 times as much output data */
            mul = 2;
            hDecoder->sbr_alloced[hDecoder->fr_ch_ele] = 1;
        }
#endif
        hDecoder->time_out[channel] = (int32_t*)faad_malloc(mul * hDecoder->frameLength * sizeof(int32_t));
        memset(hDecoder->time_out[channel], 0, mul * hDecoder->frameLength * sizeof(int32_t));
    }
    if(hDecoder->time_out[paired_channel] == NULL) {
        hDecoder->time_out[paired_channel] = (int32_t*)faad_malloc(mul * hDecoder->frameLength * sizeof(int32_t));
        memset(hDecoder->time_out[paired_channel], 0, mul * hDecoder->frameLength * sizeof(int32_t));
    }

    if(hDecoder->fb_intermed[channel] == NULL) {
        hDecoder->fb_intermed[channel] = (int32_t*)faad_malloc(hDecoder->frameLength * sizeof(int32_t));
        memset(hDecoder->fb_intermed[channel], 0, hDecoder->frameLength * sizeof(int32_t));
    }
    if(hDecoder->fb_intermed[paired_channel] == NULL) {
        hDecoder->fb_intermed[paired_channel] = (int32_t*)faad_malloc(hDecoder->frameLength * sizeof(int32_t));
        memset(hDecoder->fb_intermed[paired_channel], 0, hDecoder->frameLength * sizeof(int32_t));
    }

    return 0;
}

uint8_t reconstruct_single_channel(NeAACDecStruct* hDecoder, ic_stream* ics, element* sce, int16_t* spec_data) {
    uint8_t retval;
    int     output_channels;
    int32_t spec_coef[1024];

#ifdef PROFILE
    int64_t count = faad_get_ts();
#endif

    /* always allocate 2 channels, PS can always "suddenly" turn up */

    if(hDecoder->ps_used[hDecoder->fr_ch_ele]) output_channels = 2;
    else
        output_channels = 1;

    if(hDecoder->element_output_channels[hDecoder->fr_ch_ele] == 0) {
        /* element_output_channels not set yet */
        hDecoder->element_output_channels[hDecoder->fr_ch_ele] = output_channels;
    }
    else if(hDecoder->element_output_channels[hDecoder->fr_ch_ele] != output_channels) {
        /* element inconsistency */

        /* this only happens if PS is actually found but not in the first frame
         * this means that there is only 1 bitstream element!
         */

        /* reset the allocation */
        hDecoder->element_alloced[hDecoder->fr_ch_ele] = 0;

        hDecoder->element_output_channels[hDecoder->fr_ch_ele] = output_channels;

        // return 21;
    }

    if(hDecoder->element_alloced[hDecoder->fr_ch_ele] == 0) {
        retval = allocate_single_channel(hDecoder, sce->channel, output_channels);
        if(retval > 0) return retval;

        hDecoder->element_alloced[hDecoder->fr_ch_ele] = 1;
    }

    /* sanity check, CVE-2018-20199, CVE-2018-20360 */
    if(!hDecoder->time_out[sce->channel]) return 15;
    if(output_channels > 1 && !hDecoder->time_out[sce->channel + 1]) return 15;
    if(!hDecoder->fb_intermed[sce->channel]) return 15;

    /* dequantisation and scaling */
    retval = quant_to_spec(hDecoder, ics, spec_data, spec_coef, hDecoder->frameLength);
    if(retval > 0) return retval;

#ifdef PROFILE
    count = faad_get_ts() - count;
    hDecoder->requant_cycles += count;
#endif

    /* pns decoding */
    pns_decode(ics, NULL, spec_coef, NULL, hDecoder->frameLength, 0, hDecoder->object_type, &(hDecoder->__r1), &(hDecoder->__r2));



#ifdef LTP_DEC
    if(is_ltp_ot(hDecoder->object_type)) {
    #ifdef LD_DEC
        if(hDecoder->object_type == LD) {
            if(ics->ltp.data_present) {
                if(ics->ltp.lag_update) hDecoder->ltp_lag[sce->channel] = ics->ltp.lag;
            }
            ics->ltp.lag = hDecoder->ltp_lag[sce->channel];
        }
    #endif

        /* long term prediction */
        lt_prediction(ics, &(ics->ltp), spec_coef, hDecoder->lt_pred_stat[sce->channel], hDecoder->fb, ics->window_shape,
                      hDecoder->window_shape_prev[sce->channel], hDecoder->sf_index, hDecoder->object_type, hDecoder->frameLength);
    }
#endif

    /* tns decoding */
    tns_decode_frame(ics, &(ics->tns), hDecoder->sf_index, hDecoder->object_type, spec_coef, hDecoder->frameLength);

    /* drc decoding */
#ifdef APPLY_DRC
    if(hDecoder->drc->present) {
        if(!hDecoder->drc->exclude_mask[sce->channel] || !hDecoder->drc->excluded_chns_present) drc_decode(hDecoder->drc, spec_coef);
    }
#endif
    /* filter bank */

    ifilter_bank(hDecoder->fb, ics->window_sequence, ics->window_shape, hDecoder->window_shape_prev[sce->channel], spec_coef,
                 hDecoder->time_out[sce->channel], hDecoder->fb_intermed[sce->channel], hDecoder->object_type, hDecoder->frameLength);

    /* save window shape for next frame */
    hDecoder->window_shape_prev[sce->channel] = ics->window_shape;

#ifdef LTP_DEC
    if(is_ltp_ot(hDecoder->object_type)) {
        lt_update_state(hDecoder->lt_pred_stat[sce->channel], hDecoder->time_out[sce->channel], hDecoder->fb_intermed[sce->channel],
                        hDecoder->frameLength, hDecoder->object_type);
    }
#endif

#ifdef SBR_DEC
    if(((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) && hDecoder->sbr_alloced[hDecoder->fr_ch_ele]) {
        int ele = hDecoder->fr_ch_ele;
        int ch = sce->channel;

        /* following case can happen when forceUpSampling == 1 */
        if(hDecoder->sbr[ele] == NULL) {
            hDecoder->sbr[ele] =
                sbrDecodeInit(hDecoder->frameLength, hDecoder->element_id[ele], 2 * get_sample_rate(hDecoder->sf_index), hDecoder->downSampledSBR

                );
        }
        if(!hDecoder->sbr[ele]) return 19;

        if(sce->ics1.window_sequence == EIGHT_SHORT_SEQUENCE)
            hDecoder->sbr[ele]->maxAACLine = 8 * min(sce->ics1.swb_offset[max(sce->ics1.max_sfb - 1, 0)], sce->ics1.swb_offset_max);
        else
            hDecoder->sbr[ele]->maxAACLine = min(sce->ics1.swb_offset[max(sce->ics1.max_sfb - 1, 0)], sce->ics1.swb_offset_max);

            /* check if any of the PS tools is used */
    #if(defined(PS_DEC))
        if(hDecoder->ps_used[ele] == 0) {
    #endif
            retval = sbrDecodeSingleFrame(hDecoder->sbr[ele], hDecoder->time_out[ch], hDecoder->postSeekResetFlag, hDecoder->downSampledSBR);
    #if(defined(PS_DEC))
        }
        else {
            retval = sbrDecodeSingleFramePS(hDecoder->sbr[ele], hDecoder->time_out[ch], hDecoder->time_out[ch + 1], hDecoder->postSeekResetFlag,
                                            hDecoder->downSampledSBR);
        }
    #endif
        if(retval > 0) return retval;
    }
    else if(((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) && !hDecoder->sbr_alloced[hDecoder->fr_ch_ele]) { return 23; }
#endif

    /* copy L to R when no PS is used */
#if(defined(PS_DEC))
    if((hDecoder->ps_used[hDecoder->fr_ch_ele] == 0) && (hDecoder->element_output_channels[hDecoder->fr_ch_ele] == 2)) {
        int ele = hDecoder->fr_ch_ele;
        int ch = sce->channel;
        int frame_size = (hDecoder->sbr_alloced[ele]) ? 2 : 1;
        frame_size *= hDecoder->frameLength * sizeof(int32_t);

        memcpy(hDecoder->time_out[ch + 1], hDecoder->time_out[ch], frame_size);
    }
#endif

    return 0;
}

uint8_t reconstruct_channel_pair(NeAACDecStruct* hDecoder, ic_stream* ics1, ic_stream* ics2, element* cpe, int16_t* spec_data1, int16_t* spec_data2) {
    uint8_t retval;
    int32_t spec_coef1[1024];
    int32_t spec_coef2[1024];

#ifdef PROFILE
    int64_t count = faad_get_ts();
#endif
    if(hDecoder->element_alloced[hDecoder->fr_ch_ele] != 2) {
        retval = allocate_channel_pair(hDecoder, cpe->channel, (uint8_t)cpe->paired_channel);
        if(retval > 0) return retval;

        hDecoder->element_alloced[hDecoder->fr_ch_ele] = 2;
    }

    /* sanity check, CVE-2018-20199, CVE-2018-20360 */
    if(!hDecoder->time_out[cpe->channel] || !hDecoder->time_out[cpe->paired_channel]) return 15;
    if(!hDecoder->fb_intermed[cpe->channel] || !hDecoder->fb_intermed[cpe->paired_channel]) return 15;

    /* dequantisation and scaling */
    retval = quant_to_spec(hDecoder, ics1, spec_data1, spec_coef1, hDecoder->frameLength);
    if(retval > 0) return retval;
    retval = quant_to_spec(hDecoder, ics2, spec_data2, spec_coef2, hDecoder->frameLength);
    if(retval > 0) return retval;

#ifdef PROFILE
    count = faad_get_ts() - count;
    hDecoder->requant_cycles += count;
#endif

    /* pns decoding */
    if(ics1->ms_mask_present) {
        pns_decode(ics1, ics2, spec_coef1, spec_coef2, hDecoder->frameLength, 1, hDecoder->object_type, &(hDecoder->__r1), &(hDecoder->__r2));
    }
    else { pns_decode(ics1, ics2, spec_coef1, spec_coef2, hDecoder->frameLength, 0, hDecoder->object_type, &(hDecoder->__r1), &(hDecoder->__r2)); }

    /* mid/side decoding */
    ms_decode(ics1, ics2, spec_coef1, spec_coef2, hDecoder->frameLength);

#if 0
    {
        int i;
        for (i = 0; i < 1024; i++)
        {
            //printf("%d\n", spec_coef1[i]);
            printf("0x%.8X\n", spec_coef1[i]);
        }
        for (i = 0; i < 1024; i++)
        {
            //printf("%d\n", spec_coef2[i]);
            printf("0x%.8X\n", spec_coef2[i]);
        }
    }
#endif

    /* intensity stereo decoding */
    is_decode(ics1, ics2, spec_coef1, spec_coef2, hDecoder->frameLength);

#ifdef LTP_DEC
    if(is_ltp_ot(hDecoder->object_type)) {
        ltp_info* ltp1 = &(ics1->ltp);
        ltp_info* ltp2 = (cpe->common_window) ? &(ics2->ltp2) : &(ics2->ltp);
    #ifdef LD_DEC
        if(hDecoder->object_type == LD) {
            if(ltp1->data_present) {
                if(ltp1->lag_update) hDecoder->ltp_lag[cpe->channel] = ltp1->lag;
            }
            ltp1->lag = hDecoder->ltp_lag[cpe->channel];
            if(ltp2->data_present) {
                if(ltp2->lag_update) hDecoder->ltp_lag[cpe->paired_channel] = ltp2->lag;
            }
            ltp2->lag = hDecoder->ltp_lag[cpe->paired_channel];
        }
    #endif
        /* long term prediction */
        lt_prediction(ics1, ltp1, spec_coef1, hDecoder->lt_pred_stat[cpe->channel], hDecoder->fb, ics1->window_shape,
                      hDecoder->window_shape_prev[cpe->channel], hDecoder->sf_index, hDecoder->object_type, hDecoder->frameLength);
        lt_prediction(ics2, ltp2, spec_coef2, hDecoder->lt_pred_stat[cpe->paired_channel], hDecoder->fb, ics2->window_shape,
                      hDecoder->window_shape_prev[cpe->paired_channel], hDecoder->sf_index, hDecoder->object_type, hDecoder->frameLength);
    }
#endif
    /* tns decoding */
    tns_decode_frame(ics1, &(ics1->tns), hDecoder->sf_index, hDecoder->object_type, spec_coef1, hDecoder->frameLength);
    tns_decode_frame(ics2, &(ics2->tns), hDecoder->sf_index, hDecoder->object_type, spec_coef2, hDecoder->frameLength);
    /* drc decoding */
#if APPLY_DRC
    if(hDecoder->drc->present) {
        if(!hDecoder->drc->exclude_mask[cpe->channel] || !hDecoder->drc->excluded_chns_present) drc_decode(hDecoder->drc, spec_coef1);
        if(!hDecoder->drc->exclude_mask[cpe->paired_channel] || !hDecoder->drc->excluded_chns_present) drc_decode(hDecoder->drc, spec_coef2);
    }
#endif
    /* filter bank */

    ifilter_bank(hDecoder->fb, ics1->window_sequence, ics1->window_shape, hDecoder->window_shape_prev[cpe->channel], spec_coef1,
                 hDecoder->time_out[cpe->channel], hDecoder->fb_intermed[cpe->channel], hDecoder->object_type, hDecoder->frameLength);
    ifilter_bank(hDecoder->fb, ics2->window_sequence, ics2->window_shape, hDecoder->window_shape_prev[cpe->paired_channel], spec_coef2,
                 hDecoder->time_out[cpe->paired_channel], hDecoder->fb_intermed[cpe->paired_channel], hDecoder->object_type, hDecoder->frameLength);

    /* save window shape for next frame */
    hDecoder->window_shape_prev[cpe->channel] = ics1->window_shape;
    hDecoder->window_shape_prev[cpe->paired_channel] = ics2->window_shape;

#ifdef LTP_DEC
    if(is_ltp_ot(hDecoder->object_type)) {
        lt_update_state(hDecoder->lt_pred_stat[cpe->channel], hDecoder->time_out[cpe->channel], hDecoder->fb_intermed[cpe->channel],
                        hDecoder->frameLength, hDecoder->object_type);
        lt_update_state(hDecoder->lt_pred_stat[cpe->paired_channel], hDecoder->time_out[cpe->paired_channel],
                        hDecoder->fb_intermed[cpe->paired_channel], hDecoder->frameLength, hDecoder->object_type);
    }
#endif

#ifdef SBR_DEC
    if(((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) && hDecoder->sbr_alloced[hDecoder->fr_ch_ele]) {
        int ele = hDecoder->fr_ch_ele;
        int ch0 = cpe->channel;
        int ch1 = cpe->paired_channel;

        /* following case can happen when forceUpSampling == 1 */
        if(hDecoder->sbr[ele] == NULL) {
            hDecoder->sbr[ele] =
                sbrDecodeInit(hDecoder->frameLength, hDecoder->element_id[ele], 2 * get_sample_rate(hDecoder->sf_index), hDecoder->downSampledSBR

                );
        }
        if(!hDecoder->sbr[ele]) return 19;

        if(cpe->ics1.window_sequence == EIGHT_SHORT_SEQUENCE)
            hDecoder->sbr[ele]->maxAACLine = 8 * min(cpe->ics1.swb_offset[max(cpe->ics1.max_sfb - 1, 0)], cpe->ics1.swb_offset_max);
        else
            hDecoder->sbr[ele]->maxAACLine = min(cpe->ics1.swb_offset[max(cpe->ics1.max_sfb - 1, 0)], cpe->ics1.swb_offset_max);

        retval = sbrDecodeCoupleFrame(hDecoder->sbr[ele], hDecoder->time_out[ch0], hDecoder->time_out[ch1], hDecoder->postSeekResetFlag,
                                      hDecoder->downSampledSBR);
        if(retval > 0) return retval;
    }
    else if(((hDecoder->sbr_present_flag == 1) || (hDecoder->forceUpSampling == 1)) && !hDecoder->sbr_alloced[hDecoder->fr_ch_ele]) { return 23; }
#endif

    return 0;
}
