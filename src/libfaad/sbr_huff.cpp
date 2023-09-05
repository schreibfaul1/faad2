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
** $Id: sbr_huff.c,v 1.21 2007/11/01 12:33:35 menno Exp $
**/
#include "neaacdec.h"

#ifdef SBR_DEC


    #include "sbr_huff.h"
    #include "sbr_syntax.h"



//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int16_t sbr_huff_dec(bitfile* ld, sbr_huff_tab t_huff) {
    uint8_t bit;
    int16_t index = 0;

    while(index >= 0) {
        bit = (uint8_t)faad_get1bit(ld);
        index = t_huff[index][bit];
    }
    return index + 64;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* table 10 */
void sbr_envelope(bitfile* ld, sbr_info* sbr, uint8_t ch) {
    uint8_t      env, band;
    int8_t       delta = 0;
    sbr_huff_tab t_huff, f_huff;

    if((sbr->L_E[ch] == 1) && (sbr->bs_frame_class[ch] == FIXFIX)) sbr->amp_res[ch] = 0;
    else
        sbr->amp_res[ch] = sbr->bs_amp_res;

    if((sbr->bs_coupling) && (ch == 1)) {
        delta = 1;
        if(sbr->amp_res[ch]) {
            t_huff = t_huffman_env_bal_3_0dB;
            f_huff = f_huffman_env_bal_3_0dB;
        }
        else {
            t_huff = t_huffman_env_bal_1_5dB;
            f_huff = f_huffman_env_bal_1_5dB;
        }
    }
    else {
        delta = 0;
        if(sbr->amp_res[ch]) {
            t_huff = t_huffman_env_3_0dB;
            f_huff = f_huffman_env_3_0dB;
        }
        else {
            t_huff = t_huffman_env_1_5dB;
            f_huff = f_huffman_env_1_5dB;
        }
    }

    for(env = 0; env < sbr->L_E[ch]; env++) {
        if(sbr->bs_df_env[ch][env] == 0) {
            if((sbr->bs_coupling == 1) && (ch == 1)) {
                if(sbr->amp_res[ch]) { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 5) << delta); }
                else { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 6) << delta); }
            }
            else {
                if(sbr->amp_res[ch]) { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 6) << delta); }
                else { sbr->E[ch][0][env] = (uint16_t)(faad_getbits(ld, 7) << delta); }
            }
            for(band = 1; band < sbr->n[sbr->f[ch][env]]; band++) { sbr->E[ch][band][env] = (sbr_huff_dec(ld, f_huff) << delta); }
        }
        else {
            for(band = 0; band < sbr->n[sbr->f[ch][env]]; band++) { sbr->E[ch][band][env] = (sbr_huff_dec(ld, t_huff) << delta); }
        }
    }
    extract_envelope_data(sbr, ch);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* table 11 */
void sbr_noise(bitfile* ld, sbr_info* sbr, uint8_t ch) {
    uint8_t      noise, band;
    int8_t       delta = 0;
    sbr_huff_tab t_huff, f_huff;

    if((sbr->bs_coupling == 1) && (ch == 1)) {
        delta = 1;
        t_huff = t_huffman_noise_bal_3_0dB;
        f_huff = f_huffman_env_bal_3_0dB;
    }
    else {
        delta = 0;
        t_huff = t_huffman_noise_3_0dB;
        f_huff = f_huffman_env_3_0dB;
    }
    for(noise = 0; noise < sbr->L_Q[ch]; noise++) {
        if(sbr->bs_df_noise[ch][noise] == 0) {
            if((sbr->bs_coupling == 1) && (ch == 1)) { sbr->Q[ch][0][noise] = (faad_getbits(ld, 5) << delta); }
            else { sbr->Q[ch][0][noise] = (faad_getbits(ld, 5) << delta); }
            for(band = 1; band < sbr->N_Q; band++) { sbr->Q[ch][band][noise] = (sbr_huff_dec(ld, f_huff) << delta); }
        }
        else {
            for(band = 0; band < sbr->N_Q; band++) { sbr->Q[ch][band][noise] = (sbr_huff_dec(ld, t_huff) << delta); }
        }
    }
    extract_noise_floor_data(sbr, ch);
}

#endif
