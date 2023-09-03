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
** $Id: sbr_hfgen.c,v 1.26 2007/11/01 12:33:35 menno Exp $
**/

/* High Frequency generation */
#include "neaacdec.h"

#include "structs.h"

#ifdef SBR_DEC

#include "sbr_syntax.h"
#include "sbr_hfgen.h"
#include "sbr_fbt.h"

/* static function declarations */

static void calc_prediction_coef(sbr_info *sbr, complex_t Xlow[MAX_NTSRHFG][64],
                                 complex_t *alpha_0, complex_t *alpha_1, uint8_t k);

static void calc_chirp_factors(sbr_info *sbr, uint8_t ch);
static void patch_construction(sbr_info *sbr);


void hf_generation(sbr_info *sbr, complex_t Xlow[MAX_NTSRHFG][64],
                   complex_t Xhigh[MAX_NTSRHFG][64]

                   ,uint8_t ch)
{
    uint8_t l, i, x;
    complex_t alpha_0[64], alpha_1[64];
    uint8_t offset = sbr->tHFAdj;
    uint8_t first = sbr->t_E[ch][0];
    uint8_t last = sbr->t_E[ch][sbr->L_E[ch]];

    calc_chirp_factors(sbr, ch);

    if ((ch == 0) && (sbr->Reset))
        patch_construction(sbr);

    /* calculate the prediction coefficients */

    /* actual HF generation */
    for (i = 0; i < sbr->noPatches; i++)
    {
        for (x = 0; x < sbr->patchNoSubbands[i]; x++)
        {
            int32_t a0_r, a0_i, a1_r, a1_i;
            int32_t bw, bw2;
            uint8_t q, p, k, g;

            /* find the low and high band for patching */
            k = sbr->kx + x;
            for (q = 0; q < i; q++)
            {
                k += sbr->patchNoSubbands[q];
            }
            p = sbr->patchStartSubband[i] + x;



            g = sbr->table_map_k_to_g[k];

            bw = sbr->bwArray[ch][g];
            bw2 = MUL_C(bw, bw);

            /* do the patching */
            /* with or without filtering */
            if (bw2 > 0)
            {
                int32_t temp1_r, temp2_r, temp3_r;

                int32_t temp1_i, temp2_i, temp3_i;
                calc_prediction_coef(sbr, Xlow, alpha_0, alpha_1, p);


                a0_r = MUL_C(RE(alpha_0[p]), bw);
                a1_r = MUL_C(RE(alpha_1[p]), bw2);

                a0_i = MUL_C(IM(alpha_0[p]), bw);
                a1_i = MUL_C(IM(alpha_1[p]), bw2);


            	temp2_r = QMF_RE(Xlow[first - 2 + offset][p]);
            	temp3_r = QMF_RE(Xlow[first - 1 + offset][p]);

            	temp2_i = QMF_IM(Xlow[first - 2 + offset][p]);
            	temp3_i = QMF_IM(Xlow[first - 1 + offset][p]);

				for (l = first; l < last; l++)
                {
                	temp1_r = temp2_r;
                	temp2_r = temp3_r;
                	temp3_r = QMF_RE(Xlow[l + offset][p]);

                	temp1_i = temp2_i;
                	temp2_i = temp3_i;
                    temp3_i = QMF_IM(Xlow[l + offset][p]);


                    QMF_RE(Xhigh[l + offset][k]) =
                        temp3_r
                      +(MUL_R(a0_r, temp2_r) -
                        MUL_R(a0_i, temp2_i) +
                        MUL_R(a1_r, temp1_r) -
                        MUL_R(a1_i, temp1_i));
                    QMF_IM(Xhigh[l + offset][k]) =
                        temp3_i
                      +(MUL_R(a0_i, temp2_r) +
                        MUL_R(a0_r, temp2_i) +
                        MUL_R(a1_i, temp1_r) +
                        MUL_R(a1_r, temp1_i));

                }
            } else {
                for (l = first; l < last; l++)
                {
                    QMF_RE(Xhigh[l + offset][k]) = QMF_RE(Xlow[l + offset][p]);

                    QMF_IM(Xhigh[l + offset][k]) = QMF_IM(Xlow[l + offset][p]);

                }
            }
        }
    }

    if (sbr->Reset)
    {
        limiter_frequency_table(sbr);
    }
}

typedef struct
{
    complex_t r01;
    complex_t r02;
    complex_t r11;
    complex_t r12;
    complex_t r22;
    int32_t det;
} acorr_coef;

static void auto_correlation(sbr_info *sbr, acorr_coef *ac, complex_t buffer[MAX_NTSRHFG][64],
                             uint8_t bd, uint8_t len)
{
    int32_t r01r = 0, r01i = 0, r02r = 0, r02i = 0, r11r = 0;
    int32_t temp1_r, temp1_i, temp2_r, temp2_i, temp3_r, temp3_i, temp4_r, temp4_i, temp5_r, temp5_i;
    const int32_t rel = FRAC_CONST(0.999999); // 1 / (1 + 1e-6f);
    uint32_t mask, exp;
    int32_t pow2_to_exp;

    int8_t j;
    uint8_t offset = sbr->tHFAdj;

    mask = 0;

    for (j = (offset-2); j < (len + offset); j++)
    {
        int32_t x;
        x = QMF_RE(buffer[j][bd])>>REAL_BITS;
        mask |= x ^ (x >> 31);
        x = QMF_IM(buffer[j][bd])>>REAL_BITS;
        mask |= x ^ (x >> 31);
    }

    exp = wl_min_lzc(mask);

    /* improves accuracy */
    if (exp > 0)
        exp -= 1;

    pow2_to_exp = 1<<(exp-1);

    temp2_r = (QMF_RE(buffer[offset-2][bd]) + pow2_to_exp) >> exp;
    temp2_i = (QMF_IM(buffer[offset-2][bd]) + pow2_to_exp) >> exp;
    temp3_r = (QMF_RE(buffer[offset-1][bd]) + pow2_to_exp) >> exp;
    temp3_i = (QMF_IM(buffer[offset-1][bd]) + pow2_to_exp) >> exp;
    // Save these because they are needed after loop
    temp4_r = temp2_r;
    temp4_i = temp2_i;
    temp5_r = temp3_r;
    temp5_i = temp3_i;

    for (j = offset; j < len + offset; j++)
    {
    	temp1_r = temp2_r; // temp1_r = (QMF_RE(buffer[offset-2][bd] + (1<<(exp-1))) >> exp;
    	temp1_i = temp2_i; // temp1_i = (QMF_IM(buffer[offset-2][bd] + (1<<(exp-1))) >> exp;
    	temp2_r = temp3_r; // temp2_r = (QMF_RE(buffer[offset-1][bd] + (1<<(exp-1))) >> exp;
    	temp2_i = temp3_i; // temp2_i = (QMF_IM(buffer[offset-1][bd] + (1<<(exp-1))) >> exp;
        temp3_r = (QMF_RE(buffer[j][bd]) + pow2_to_exp) >> exp;
        temp3_i = (QMF_IM(buffer[j][bd]) + pow2_to_exp) >> exp;
        r01r += MUL_R(temp3_r, temp2_r) + MUL_R(temp3_i, temp2_i);
        r01i += MUL_R(temp3_i, temp2_r) - MUL_R(temp3_r, temp2_i);
        r02r += MUL_R(temp3_r, temp1_r) + MUL_R(temp3_i, temp1_i);
        r02i += MUL_R(temp3_i, temp1_r) - MUL_R(temp3_r, temp1_i);
        r11r += MUL_R(temp2_r, temp2_r) + MUL_R(temp2_i, temp2_i);
    }

    // These are actual values in temporary variable at this point
    // temp1_r = (QMF_RE(buffer[len+offset-1-2][bd] + (1<<(exp-1))) >> exp;
    // temp1_i = (QMF_IM(buffer[len+offset-1-2][bd] + (1<<(exp-1))) >> exp;
    // temp2_r = (QMF_RE(buffer[len+offset-1-1][bd] + (1<<(exp-1))) >> exp;
    // temp2_i = (QMF_IM(buffer[len+offset-1-1][bd] + (1<<(exp-1))) >> exp;
    // temp3_r = (QMF_RE(buffer[len+offset-1][bd]) + (1<<(exp-1))) >> exp;
    // temp3_i = (QMF_IM(buffer[len+offset-1][bd]) + (1<<(exp-1))) >> exp;
    // temp4_r = (QMF_RE(buffer[offset-2][bd]) + (1<<(exp-1))) >> exp;
    // temp4_i = (QMF_IM(buffer[offset-2][bd]) + (1<<(exp-1))) >> exp;
    // temp5_r = (QMF_RE(buffer[offset-1][bd]) + (1<<(exp-1))) >> exp;
    // temp5_i = (QMF_IM(buffer[offset-1][bd]) + (1<<(exp-1))) >> exp;

    RE(ac->r12) = r01r -
        (MUL_R(temp3_r, temp2_r) + MUL_R(temp3_i, temp2_i)) +
        (MUL_R(temp5_r, temp4_r) + MUL_R(temp5_i, temp4_i));
    IM(ac->r12) = r01i -
        (MUL_R(temp3_i, temp2_r) - MUL_R(temp3_r, temp2_i)) +
        (MUL_R(temp5_i, temp4_r) - MUL_R(temp5_r, temp4_i));
    RE(ac->r22) = r11r -
        (MUL_R(temp2_r, temp2_r) + MUL_R(temp2_i, temp2_i)) +
        (MUL_R(temp4_r, temp4_r) + MUL_R(temp4_i, temp4_i));



    RE(ac->r01) = r01r;
    IM(ac->r01) = r01i;
    RE(ac->r02) = r02r;
    IM(ac->r02) = r02i;
    RE(ac->r11) = r11r;

    ac->det = MUL_R(RE(ac->r11), RE(ac->r22)) - MUL_F(rel, (MUL_R(RE(ac->r12), RE(ac->r12)) + MUL_R(IM(ac->r12), IM(ac->r12))));
}


/* calculate linear prediction coefficients using the covariance method */

static void calc_prediction_coef(sbr_info *sbr, complex_t Xlow[MAX_NTSRHFG][64],
                                 complex_t *alpha_0, complex_t *alpha_1, uint8_t k)
{
    int32_t tmp;
    acorr_coef ac;

    auto_correlation(sbr, &ac, Xlow, k, sbr->numTimeSlotsRate + 6);

    if (ac.det == 0)
    {
        RE(alpha_1[k]) = 0;
        IM(alpha_1[k]) = 0;
    } else {
        tmp = (MUL_R(RE(ac.r01), RE(ac.r12)) - MUL_R(IM(ac.r01), IM(ac.r12)) - MUL_R(RE(ac.r02), RE(ac.r11)));
        RE(alpha_1[k]) = DIV_R(tmp, ac.det);
        tmp = (MUL_R(IM(ac.r01), RE(ac.r12)) + MUL_R(RE(ac.r01), IM(ac.r12)) - MUL_R(IM(ac.r02), RE(ac.r11)));
        IM(alpha_1[k]) = DIV_R(tmp, ac.det);
    }

    if (RE(ac.r11) == 0)
    {
        RE(alpha_0[k]) = 0;
        IM(alpha_0[k]) = 0;
    } else {
        tmp = -(RE(ac.r01) + MUL_R(RE(alpha_1[k]), RE(ac.r12)) + MUL_R(IM(alpha_1[k]), IM(ac.r12)));
        RE(alpha_0[k]) = DIV_R(tmp, RE(ac.r11));
        tmp = -(IM(ac.r01) + MUL_R(IM(alpha_1[k]), RE(ac.r12)) - MUL_R(RE(alpha_1[k]), IM(ac.r12)));
        IM(alpha_0[k]) = DIV_R(tmp, RE(ac.r11));
    }

    if ((MUL_R(RE(alpha_0[k]),RE(alpha_0[k])) + MUL_R(IM(alpha_0[k]),IM(alpha_0[k])) >= REAL_CONST(16)) ||
        (MUL_R(RE(alpha_1[k]),RE(alpha_1[k])) + MUL_R(IM(alpha_1[k]),IM(alpha_1[k])) >= REAL_CONST(16)))
    {
        RE(alpha_0[k]) = 0;
        IM(alpha_0[k]) = 0;
        RE(alpha_1[k]) = 0;
        IM(alpha_1[k]) = 0;
    }
}


/* FIXED POINT: bwArray = COEF */
static int32_t mapNewBw(uint8_t invf_mode, uint8_t invf_mode_prev)
{
    switch (invf_mode)
    {
    case 1: /* LOW */
        if (invf_mode_prev == 0) /* NONE */
            return COEF_CONST(0.6);
        else
            return COEF_CONST(0.75);

    case 2: /* MID */
        return COEF_CONST(0.9);

    case 3: /* HIGH */
        return COEF_CONST(0.98);

    default: /* NONE */
        if (invf_mode_prev == 1) /* LOW */
            return COEF_CONST(0.6);
        else
            return COEF_CONST(0.0);
    }
}

/* FIXED POINT: bwArray = COEF */
static void calc_chirp_factors(sbr_info *sbr, uint8_t ch)
{
    uint8_t i;

    for (i = 0; i < sbr->N_Q; i++)
    {
        sbr->bwArray[ch][i] = mapNewBw(sbr->bs_invf_mode[ch][i], sbr->bs_invf_mode_prev[ch][i]);

        if (sbr->bwArray[ch][i] < sbr->bwArray_prev[ch][i])
            sbr->bwArray[ch][i] = MUL_F(sbr->bwArray[ch][i], FRAC_CONST(0.75)) + MUL_F(sbr->bwArray_prev[ch][i], FRAC_CONST(0.25));
        else
            sbr->bwArray[ch][i] = MUL_F(sbr->bwArray[ch][i], FRAC_CONST(0.90625)) + MUL_F(sbr->bwArray_prev[ch][i], FRAC_CONST(0.09375));

        if (sbr->bwArray[ch][i] < COEF_CONST(0.015625))
            sbr->bwArray[ch][i] = COEF_CONST(0.0);

        if (sbr->bwArray[ch][i] >= COEF_CONST(0.99609375))
            sbr->bwArray[ch][i] = COEF_CONST(0.99609375);

        sbr->bwArray_prev[ch][i] = sbr->bwArray[ch][i];
        sbr->bs_invf_mode_prev[ch][i] = sbr->bs_invf_mode[ch][i];
    }
}

static void patch_construction(sbr_info *sbr)
{
    uint8_t i, k;
    uint8_t odd, sb;
    uint8_t msb = sbr->k0;
    uint8_t usb = sbr->kx;
    uint8_t goalSbTab[] = { 21, 23, 32, 43, 46, 64, 85, 93, 128, 0, 0, 0 };
    /* (uint8_t)(2.048e6/sbr->sample_rate + 0.5); */
    uint8_t goalSb = goalSbTab[get_sr_index(sbr->sample_rate)];

    sbr->noPatches = 0;

    if (goalSb < (sbr->kx + sbr->M))
    {
        for (i = 0, k = 0; sbr->f_master[i] < goalSb; i++)
            k = i+1;
    } else {
        k = sbr->N_master;
    }

    if (sbr->N_master == 0)
    {
        sbr->noPatches = 0;
        sbr->patchNoSubbands[0] = 0;
        sbr->patchStartSubband[0] = 0;

        return;
    }

    do
    {
        uint8_t j = k + 1;

        do
        {
            j--;

            sb = sbr->f_master[j];
            odd = (sb - 2 + sbr->k0) % 2;
        } while (sb > (sbr->k0 - 1 + msb - odd));

        sbr->patchNoSubbands[sbr->noPatches] = max(sb - usb, 0);
        sbr->patchStartSubband[sbr->noPatches] = sbr->k0 - odd -
            sbr->patchNoSubbands[sbr->noPatches];

        if (sbr->patchNoSubbands[sbr->noPatches] > 0)
        {
            usb = sb;
            msb = sb;
            sbr->noPatches++;
        } else {
            msb = sbr->kx;
        }

        if (sbr->f_master[k] - sb < 3)
            k = sbr->N_master;
    } while (sb != (sbr->kx + sbr->M));

    if ((sbr->patchNoSubbands[sbr->noPatches-1] < 3) && (sbr->noPatches > 1))
    {
        sbr->noPatches--;
    }

    sbr->noPatches = min(sbr->noPatches, 5);
}

#endif
