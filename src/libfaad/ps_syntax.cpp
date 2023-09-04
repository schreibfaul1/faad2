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
** $Id: ps_syntax.c,v 1.11 2007/11/01 12:33:33 menno Exp $
**/
#include "neaacdec.h"


#ifdef PS_DEC


/* type definitaions */




/* static function declarations */
static uint16_t      ps_extension(ps_info* ps, bitfile* ld, const uint8_t ps_extension_id, const uint16_t num_bits_left);
static void          huff_data(bitfile* ld, const uint8_t dt, const uint8_t nr_par, ps_huff_tab t_huff, ps_huff_tab f_huff, int8_t* par);
static inline int8_t ps_huff_dec(bitfile* ld, ps_huff_tab t_huff);


//----------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t ps_data(ps_info* ps, bitfile* ld, uint8_t* header) {
    uint8_t  tmp, n;
    uint16_t bits = (uint16_t)faad_get_processed_bits(ld);

    *header = 0;

    /* check for new PS header */
    if(faad_get1bit(ld)) {
        *header = 1;
        ps->header_read = 1;
        ps->use34hybrid_bands = 0;
        /* Inter-channel Intensity Difference (IID) parameters enabled */
        ps->enable_iid = (uint8_t)faad_get1bit(ld);
        if(ps->enable_iid) {
            ps->iid_mode = (uint8_t)faad_getbits(ld, 3);
            ps->nr_iid_par = nr_iid_par_tab[ps->iid_mode];
            ps->nr_ipdopd_par = nr_ipdopd_par_tab[ps->iid_mode];
            if(ps->iid_mode == 2 || ps->iid_mode == 5) ps->use34hybrid_bands = 1;
            /* IPD freq res equal to IID freq res */
            ps->ipd_mode = ps->iid_mode;
        }
        /* Inter-channel Coherence (ICC) parameters enabled */
        ps->enable_icc = (uint8_t)faad_get1bit(ld);
        if(ps->enable_icc) {
            ps->icc_mode = (uint8_t)faad_getbits(ld, 3);
            ps->nr_icc_par = nr_icc_par_tab[ps->icc_mode];
            if(ps->icc_mode == 2 || ps->icc_mode == 5) ps->use34hybrid_bands = 1;
        }
        /* PS extension layer enabled */
        ps->enable_ext = (uint8_t)faad_get1bit(ld);
    }
    /* we are here, but no header has been read yet */
    if(ps->header_read == 0) {
        ps->ps_data_available = 0;
        return 1;
    }
    ps->frame_class = (uint8_t)faad_get1bit(ld);
    tmp = (uint8_t)faad_getbits(ld, 2);
    ps->num_env = num_env_tab[ps->frame_class][tmp];
    if(ps->frame_class) {
        for(n = 1; n < ps->num_env + 1; n++) { ps->border_position[n] = (uint8_t)faad_getbits(ld, 5) + 1; }
    }
    if(ps->enable_iid) {
        for(n = 0; n < ps->num_env; n++) {
            ps->iid_dt[n] = (uint8_t)faad_get1bit(ld);
            /* iid_data */
            if(ps->iid_mode < 3) { huff_data(ld, ps->iid_dt[n], ps->nr_iid_par, t_huff_iid_def, f_huff_iid_def, ps->iid_index[n]); }
            else { huff_data(ld, ps->iid_dt[n], ps->nr_iid_par, t_huff_iid_fine, f_huff_iid_fine, ps->iid_index[n]); }
        }
    }
    if(ps->enable_icc) {
        for(n = 0; n < ps->num_env; n++) {
            ps->icc_dt[n] = (uint8_t)faad_get1bit(ld);

            /* icc_data */
            huff_data(ld, ps->icc_dt[n], ps->nr_icc_par, t_huff_icc, f_huff_icc, ps->icc_index[n]);
        }
    }
    if(ps->enable_ext) {
        uint16_t num_bits_left;
        uint16_t cnt = (uint16_t)faad_getbits(ld, 4);
        if(cnt == 15) { cnt += (uint16_t)faad_getbits(ld, 8); }
        num_bits_left = 8 * cnt;
        while(num_bits_left > 7) {
            uint8_t ps_extension_id = (uint8_t)faad_getbits(ld, 2);
            num_bits_left -= 2;
            num_bits_left -= ps_extension(ps, ld, ps_extension_id, num_bits_left);
        }
        faad_getbits(ld, num_bits_left);
    }
    bits = (uint16_t)faad_get_processed_bits(ld) - bits;
    ps->ps_data_available = 1;
    return bits;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static uint16_t ps_extension(ps_info* ps, bitfile* ld, const uint8_t ps_extension_id, const uint16_t num_bits_left) {
    uint8_t  n;
    uint16_t bits = (uint16_t)faad_get_processed_bits(ld);

    if(ps_extension_id == 0) {
        ps->enable_ipdopd = (uint8_t)faad_get1bit(ld);
        if(ps->enable_ipdopd) {
            for(n = 0; n < ps->num_env; n++) {
                ps->ipd_dt[n] = (uint8_t)faad_get1bit(ld);
                /* ipd_data */
                huff_data(ld, ps->ipd_dt[n], ps->nr_ipdopd_par, t_huff_ipd, f_huff_ipd, ps->ipd_index[n]);
                ps->opd_dt[n] = (uint8_t)faad_get1bit(ld);
                /* opd_data */
                huff_data(ld, ps->opd_dt[n], ps->nr_ipdopd_par, t_huff_opd, f_huff_opd, ps->opd_index[n]);
            }
        }
        faad_get1bit(ld);
    }
    /* return number of bits read */
    bits = (uint16_t)faad_get_processed_bits(ld) - bits;
    return bits;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* read huffman data coded in either the frequency or the time direction */
static void huff_data(bitfile* ld, const uint8_t dt, const uint8_t nr_par, ps_huff_tab t_huff, ps_huff_tab f_huff, int8_t* par) {
    uint8_t n;

    if(dt) {
        /* coded in time direction */
        for(n = 0; n < nr_par; n++) { par[n] = ps_huff_dec(ld, t_huff); }
    }
    else {
        /* coded in frequency direction */
        par[0] = ps_huff_dec(ld, f_huff);

        for(n = 1; n < nr_par; n++) { par[n] = ps_huff_dec(ld, f_huff); }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
static inline int8_t ps_huff_dec(bitfile* ld, ps_huff_tab t_huff) { /* binary search huffman decoding */
    uint8_t bit;
    int16_t index = 0;

    while(index >= 0) {
        bit = (uint8_t)faad_get1bit(ld);
        index = t_huff[index][bit];
    }

    return index + 31;
}

#endif
