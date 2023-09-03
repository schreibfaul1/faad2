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
** $Id: sbr_e_nf.c,v 1.22 2008/03/23 23:03:29 menno Exp $
**/

#include "common.h"
#include "structs.h"

#ifdef SBR_DEC

    #include "sbr_e_nf.h"
    #include "sbr_syntax.h"
    #include <stdlib.h>

//----------------------------------------------------------------------------------------------------------------------------------------------------
void extract_envelope_data(sbr_info* sbr, uint8_t ch) {
    uint8_t l, k;

    for(l = 0; l < sbr->L_E[ch]; l++) {
        if(sbr->bs_df_env[ch][l] == 0) {
            for(k = 1; k < sbr->n[sbr->f[ch][l]]; k++) {
                sbr->E[ch][k][l] = sbr->E[ch][k - 1][l] + sbr->E[ch][k][l];
                if(sbr->E[ch][k][l] < 0) sbr->E[ch][k][l] = 0;
            }
        }
        else { /* bs_df_env == 1 */

            uint8_t g = (l == 0) ? sbr->f_prev[ch] : sbr->f[ch][l - 1];
            int16_t E_prev;

            if(sbr->f[ch][l] == g) {
                for(k = 0; k < sbr->n[sbr->f[ch][l]]; k++) {
                    if(l == 0) E_prev = sbr->E_prev[ch][k];
                    else
                        E_prev = sbr->E[ch][k][l - 1];

                    sbr->E[ch][k][l] = E_prev + sbr->E[ch][k][l];
                }
            }
            else if((g == 1) && (sbr->f[ch][l] == 0)) {
                uint8_t i;
                for(k = 0; k < sbr->n[sbr->f[ch][l]]; k++) {
                    for(i = 0; i < sbr->N_high; i++) {
                        if(sbr->f_table_res[HI_RES][i] == sbr->f_table_res[LO_RES][k]) {
                            if(l == 0) E_prev = sbr->E_prev[ch][i];
                            else
                                E_prev = sbr->E[ch][i][l - 1];
                            sbr->E[ch][k][l] = E_prev + sbr->E[ch][k][l];
                        }
                    }
                }
            }
            else if((g == 0) && (sbr->f[ch][l] == 1)) {
                uint8_t i;
                for(k = 0; k < sbr->n[sbr->f[ch][l]]; k++) {
                    for(i = 0; i < sbr->N_low; i++) {
                        if((sbr->f_table_res[LO_RES][i] <= sbr->f_table_res[HI_RES][k]) &&
                           (sbr->f_table_res[HI_RES][k] < sbr->f_table_res[LO_RES][i + 1])) {
                            if(l == 0) E_prev = sbr->E_prev[ch][i];
                            else
                                E_prev = sbr->E[ch][i][l - 1];
                            sbr->E[ch][k][l] = E_prev + sbr->E[ch][k][l];
                        }
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void extract_noise_floor_data(sbr_info* sbr, uint8_t ch) {
    uint8_t l, k;

    for(l = 0; l < sbr->L_Q[ch]; l++) {
        if(sbr->bs_df_noise[ch][l] == 0) {
            for(k = 1; k < sbr->N_Q; k++) { sbr->Q[ch][k][l] = sbr->Q[ch][k][l] + sbr->Q[ch][k - 1][l]; }
        }
        else {
            if(l == 0) {
                for(k = 0; k < sbr->N_Q; k++) { sbr->Q[ch][k][l] = sbr->Q_prev[ch][k] + sbr->Q[ch][k][0]; }
            }
            else {
                for(k = 0; k < sbr->N_Q; k++) { sbr->Q[ch][k][l] = sbr->Q[ch][k][l - 1] + sbr->Q[ch][k][l]; }
            }
        }
    }
}

#endif
