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
** $Id: output.c,v 1.47 2009/01/26 23:51:15 menno Exp $
**/
#include "neaacdec.h"
#include "output.h"
#include "common.h"
#include "structs.h"

#define DM_MUL FRAC_CONST(0.3203772410170407)    // 1/(1+sqrt(2) + 1/sqrt(2))
#define RSQRT2 FRAC_CONST(0.7071067811865475244) // 1/sqrt(2)

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int32_t get_sample(int32_t** input, uint8_t channel, uint16_t sample, uint8_t down_matrix, uint8_t up_matrix, uint8_t* internal_channel) {
    if(up_matrix == 1) return input[internal_channel[0]][sample];

    if(!down_matrix) return input[internal_channel[channel]][sample];

    if(channel == 0) {
        int32_t C = MUL_F(input[internal_channel[0]][sample], RSQRT2);
        int32_t L_S = MUL_F(input[internal_channel[3]][sample], RSQRT2);
        int32_t cum = input[internal_channel[1]][sample] + C + L_S;
        return MUL_F(cum, DM_MUL);
    }
    else {
        int32_t C = MUL_F(input[internal_channel[0]][sample], RSQRT2);
        int32_t R_S = MUL_F(input[internal_channel[4]][sample], RSQRT2);
        int32_t cum = input[internal_channel[2]][sample] + C + R_S;
        return MUL_F(cum, DM_MUL);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void* output_to_PCM(NeAACDecStruct* hDecoder, int32_t** input, void* sample_buffer, uint8_t channels, uint16_t frame_len, uint8_t format) {
    uint8_t  ch;
    uint16_t i;
    int16_t* short_sample_buffer = (int16_t*)sample_buffer;
    int32_t* int_sample_buffer = (int32_t*)sample_buffer;

    /* Copy output to a standard PCM buffer */
    for(ch = 0; ch < channels; ch++) {
        switch(format) {
        case FAAD_FMT_16BIT:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                if(tmp >= 0) {
                    tmp += (1 << (REAL_BITS - 1));
                    if(tmp >= REAL_CONST(32767)) { tmp = REAL_CONST(32767); }
                }
                else {
                    tmp += -(1 << (REAL_BITS - 1));
                    if(tmp <= REAL_CONST(-32768)) { tmp = REAL_CONST(-32768); }
                }
                tmp >>= REAL_BITS;
                short_sample_buffer[(i * channels) + ch] = (int16_t)tmp;
            }
            break;
        case FAAD_FMT_24BIT:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                if(tmp >= 0) {
                    tmp += (1 << (REAL_BITS - 9));
                    tmp >>= (REAL_BITS - 8);
                    if(tmp >= 8388607) { tmp = 8388607; }
                }
                else {
                    tmp += -(1 << (REAL_BITS - 9));
                    tmp >>= (REAL_BITS - 8);
                    if(tmp <= -8388608) { tmp = -8388608; }
                }
                int_sample_buffer[(i * channels) + ch] = (int32_t)tmp;
            }
            break;
        case FAAD_FMT_32BIT:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                if(tmp >= 0) {
                    tmp += (1 << (16 - REAL_BITS - 1));
                    tmp <<= (16 - REAL_BITS);
                }
                else {
                    tmp += -(1 << (16 - REAL_BITS - 1));
                    tmp <<= (16 - REAL_BITS);
                }
                int_sample_buffer[(i * channels) + ch] = (int32_t)tmp;
            }
            break;
        case FAAD_FMT_FIXED:
            for(i = 0; i < frame_len; i++) {
                int32_t tmp = get_sample(input, ch, i, hDecoder->downMatrix, hDecoder->upMatrix, hDecoder->internal_channel);
                int_sample_buffer[(i * channels) + ch] = (int32_t)tmp;
            }
            break;
        }
    }

    return sample_buffer;
}
