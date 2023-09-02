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
** $Id: ic_predict.h,v 1.23 2007/11/01 12:33:31 menno Exp $
**/

#ifdef MAIN_DEC

#ifndef __IC_PREDICT_H__
#define __IC_PREDICT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA      REAL_CONST(0.90625)
#define A          REAL_CONST(0.953125)


void pns_reset_pred_state(ic_stream *ics, pred_state *state);
void reset_all_predictors(pred_state *state, uint16_t frame_len);
void ic_prediction(ic_stream *ics, int32_t *spec, pred_state *state,
                   uint16_t frame_len, uint8_t sf_index);

 static const int32_t mnt_table[128] = {
    COEF_CONST(0.9531250000), COEF_CONST(0.9453125000),
    COEF_CONST(0.9375000000), COEF_CONST(0.9296875000),
    COEF_CONST(0.9257812500), COEF_CONST(0.9179687500),
    COEF_CONST(0.9101562500), COEF_CONST(0.9023437500),
    COEF_CONST(0.8984375000), COEF_CONST(0.8906250000),
    COEF_CONST(0.8828125000), COEF_CONST(0.8789062500),
    COEF_CONST(0.8710937500), COEF_CONST(0.8671875000),
    COEF_CONST(0.8593750000), COEF_CONST(0.8515625000),
    COEF_CONST(0.8476562500), COEF_CONST(0.8398437500),
    COEF_CONST(0.8359375000), COEF_CONST(0.8281250000),
    COEF_CONST(0.8242187500), COEF_CONST(0.8203125000),
    COEF_CONST(0.8125000000), COEF_CONST(0.8085937500),
    COEF_CONST(0.8007812500), COEF_CONST(0.7968750000),
    COEF_CONST(0.7929687500), COEF_CONST(0.7851562500),
    COEF_CONST(0.7812500000), COEF_CONST(0.7773437500),
    COEF_CONST(0.7734375000), COEF_CONST(0.7656250000),
    COEF_CONST(0.7617187500), COEF_CONST(0.7578125000),
    COEF_CONST(0.7539062500), COEF_CONST(0.7500000000),
    COEF_CONST(0.7421875000), COEF_CONST(0.7382812500),
    COEF_CONST(0.7343750000), COEF_CONST(0.7304687500),
    COEF_CONST(0.7265625000), COEF_CONST(0.7226562500),
    COEF_CONST(0.7187500000), COEF_CONST(0.7148437500),
    COEF_CONST(0.7109375000), COEF_CONST(0.7070312500),
    COEF_CONST(0.6992187500), COEF_CONST(0.6953125000),
    COEF_CONST(0.6914062500), COEF_CONST(0.6875000000),
    COEF_CONST(0.6835937500), COEF_CONST(0.6796875000),
    COEF_CONST(0.6796875000), COEF_CONST(0.6757812500),
    COEF_CONST(0.6718750000), COEF_CONST(0.6679687500),
    COEF_CONST(0.6640625000), COEF_CONST(0.6601562500),
    COEF_CONST(0.6562500000), COEF_CONST(0.6523437500),
    COEF_CONST(0.6484375000), COEF_CONST(0.6445312500),
    COEF_CONST(0.6406250000), COEF_CONST(0.6406250000),
    COEF_CONST(0.6367187500), COEF_CONST(0.6328125000),
    COEF_CONST(0.6289062500), COEF_CONST(0.6250000000),
    COEF_CONST(0.6210937500), COEF_CONST(0.6210937500),
    COEF_CONST(0.6171875000), COEF_CONST(0.6132812500),
    COEF_CONST(0.6093750000), COEF_CONST(0.6054687500),
    COEF_CONST(0.6054687500), COEF_CONST(0.6015625000),
    COEF_CONST(0.5976562500), COEF_CONST(0.5937500000),
    COEF_CONST(0.5937500000), COEF_CONST(0.5898437500),
    COEF_CONST(0.5859375000), COEF_CONST(0.5820312500),
    COEF_CONST(0.5820312500), COEF_CONST(0.5781250000),
    COEF_CONST(0.5742187500), COEF_CONST(0.5742187500),
    COEF_CONST(0.5703125000), COEF_CONST(0.5664062500),
    COEF_CONST(0.5664062500), COEF_CONST(0.5625000000),
    COEF_CONST(0.5585937500), COEF_CONST(0.5585937500),
    COEF_CONST(0.5546875000), COEF_CONST(0.5507812500),
    COEF_CONST(0.5507812500), COEF_CONST(0.5468750000),
    COEF_CONST(0.5429687500), COEF_CONST(0.5429687500),
    COEF_CONST(0.5390625000), COEF_CONST(0.5390625000),
    COEF_CONST(0.5351562500), COEF_CONST(0.5312500000),
    COEF_CONST(0.5312500000), COEF_CONST(0.5273437500),
    COEF_CONST(0.5273437500), COEF_CONST(0.5234375000),
    COEF_CONST(0.5195312500), COEF_CONST(0.5195312500),
    COEF_CONST(0.5156250000), COEF_CONST(0.5156250000),
    COEF_CONST(0.5117187500), COEF_CONST(0.5117187500),
    COEF_CONST(0.5078125000), COEF_CONST(0.5078125000),
    COEF_CONST(0.5039062500), COEF_CONST(0.5039062500),
    COEF_CONST(0.5000000000), COEF_CONST(0.4980468750),
    COEF_CONST(0.4960937500), COEF_CONST(0.4941406250),
    COEF_CONST(0.4921875000), COEF_CONST(0.4902343750),
    COEF_CONST(0.4882812500), COEF_CONST(0.4863281250),
    COEF_CONST(0.4843750000), COEF_CONST(0.4824218750),
    COEF_CONST(0.4804687500), COEF_CONST(0.4785156250)
};

 static const int32_t exp_table[128] = {
    COEF_CONST(0.50000000000000000000000000000000000000000000000000),
    COEF_CONST(0.25000000000000000000000000000000000000000000000000),
    COEF_CONST(0.12500000000000000000000000000000000000000000000000),
    COEF_CONST(0.06250000000000000000000000000000000000000000000000),
    COEF_CONST(0.03125000000000000000000000000000000000000000000000),
    COEF_CONST(0.01562500000000000000000000000000000000000000000000),
    COEF_CONST(0.00781250000000000000000000000000000000000000000000),
    COEF_CONST(0.00390625000000000000000000000000000000000000000000),
    COEF_CONST(0.00195312500000000000000000000000000000000000000000),
    COEF_CONST(0.00097656250000000000000000000000000000000000000000),
    COEF_CONST(0.00048828125000000000000000000000000000000000000000),
    COEF_CONST(0.00024414062500000000000000000000000000000000000000),
    COEF_CONST(0.00012207031250000000000000000000000000000000000000),
    COEF_CONST(0.00006103515625000000000000000000000000000000000000),
    COEF_CONST(0.00003051757812500000000000000000000000000000000000),
    COEF_CONST(0.00001525878906250000000000000000000000000000000000),
    COEF_CONST(0.00000762939453125000000000000000000000000000000000),
    COEF_CONST(0.00000381469726562500000000000000000000000000000000),
    COEF_CONST(0.00000190734863281250000000000000000000000000000000),
    COEF_CONST(0.00000095367431640625000000000000000000000000000000),
    COEF_CONST(0.00000047683715820312500000000000000000000000000000),
    COEF_CONST(0.00000023841857910156250000000000000000000000000000),
    COEF_CONST(0.00000011920928955078125000000000000000000000000000),
    COEF_CONST(0.00000005960464477539062500000000000000000000000000),
    COEF_CONST(0.00000002980232238769531300000000000000000000000000),
    COEF_CONST(0.00000001490116119384765600000000000000000000000000),
    COEF_CONST(0.00000000745058059692382810000000000000000000000000),
    COEF_CONST(0.00000000372529029846191410000000000000000000000000),
    COEF_CONST(0.00000000186264514923095700000000000000000000000000),
    COEF_CONST(0.00000000093132257461547852000000000000000000000000),
    COEF_CONST(0.00000000046566128730773926000000000000000000000000),
    COEF_CONST(0.00000000023283064365386963000000000000000000000000),
    COEF_CONST(0.00000000011641532182693481000000000000000000000000),
    COEF_CONST(0.00000000005820766091346740700000000000000000000000),
    COEF_CONST(0.00000000002910383045673370400000000000000000000000),
    COEF_CONST(0.00000000001455191522836685200000000000000000000000),
    COEF_CONST(0.00000000000727595761418342590000000000000000000000),
    COEF_CONST(0.00000000000363797880709171300000000000000000000000),
    COEF_CONST(0.00000000000181898940354585650000000000000000000000),
    COEF_CONST(0.00000000000090949470177292824000000000000000000000),
    COEF_CONST(0.00000000000045474735088646412000000000000000000000),
    COEF_CONST(0.00000000000022737367544323206000000000000000000000),
    COEF_CONST(0.00000000000011368683772161603000000000000000000000),
    COEF_CONST(0.00000000000005684341886080801500000000000000000000),
    COEF_CONST(0.00000000000002842170943040400700000000000000000000),
    COEF_CONST(0.00000000000001421085471520200400000000000000000000),
    COEF_CONST(0.00000000000000710542735760100190000000000000000000),
    COEF_CONST(0.00000000000000355271367880050090000000000000000000),
    COEF_CONST(0.00000000000000177635683940025050000000000000000000),
    COEF_CONST(0.00000000000000088817841970012523000000000000000000),
    COEF_CONST(0.00000000000000044408920985006262000000000000000000),
    COEF_CONST(0.00000000000000022204460492503131000000000000000000),
    COEF_CONST(0.00000000000000011102230246251565000000000000000000),
    COEF_CONST(0.00000000000000005551115123125782700000000000000000),
    COEF_CONST(0.00000000000000002775557561562891400000000000000000),
    COEF_CONST(0.00000000000000001387778780781445700000000000000000),
    COEF_CONST(0.00000000000000000693889390390722840000000000000000),
    COEF_CONST(0.00000000000000000346944695195361420000000000000000),
    COEF_CONST(0.00000000000000000173472347597680710000000000000000),
    COEF_CONST(0.00000000000000000086736173798840355000000000000000),
    COEF_CONST(0.00000000000000000043368086899420177000000000000000),
    COEF_CONST(0.00000000000000000021684043449710089000000000000000),
    COEF_CONST(0.00000000000000000010842021724855044000000000000000),
    COEF_CONST(0.00000000000000000005421010862427522200000000000000),
    COEF_CONST(0.00000000000000000002710505431213761100000000000000),
    COEF_CONST(0.00000000000000000001355252715606880500000000000000),
    COEF_CONST(0.00000000000000000000677626357803440270000000000000),
    COEF_CONST(0.00000000000000000000338813178901720140000000000000),
    COEF_CONST(0.00000000000000000000169406589450860070000000000000),
    COEF_CONST(0.00000000000000000000084703294725430034000000000000),
    COEF_CONST(0.00000000000000000000042351647362715017000000000000),
    COEF_CONST(0.00000000000000000000021175823681357508000000000000),
    COEF_CONST(0.00000000000000000000010587911840678754000000000000),
    COEF_CONST(0.00000000000000000000005293955920339377100000000000),
    COEF_CONST(0.00000000000000000000002646977960169688600000000000),
    COEF_CONST(0.00000000000000000000001323488980084844300000000000),
    COEF_CONST(0.00000000000000000000000661744490042422140000000000),
    COEF_CONST(0.00000000000000000000000330872245021211070000000000),
    COEF_CONST(0.00000000000000000000000165436122510605530000000000),
    COEF_CONST(0.00000000000000000000000082718061255302767000000000),
    COEF_CONST(0.00000000000000000000000041359030627651384000000000),
    COEF_CONST(0.00000000000000000000000020679515313825692000000000),
    COEF_CONST(0.00000000000000000000000010339757656912846000000000),
    COEF_CONST(0.00000000000000000000000005169878828456423000000000),
    COEF_CONST(0.00000000000000000000000002584939414228211500000000),
    COEF_CONST(0.00000000000000000000000001292469707114105700000000),
    COEF_CONST(0.00000000000000000000000000646234853557052870000000),
    COEF_CONST(0.00000000000000000000000000323117426778526440000000),
    COEF_CONST(0.00000000000000000000000000161558713389263220000000),
    COEF_CONST(0.00000000000000000000000000080779356694631609000000),
    COEF_CONST(0.00000000000000000000000000040389678347315804000000),
    COEF_CONST(0.00000000000000000000000000020194839173657902000000),
    COEF_CONST(0.00000000000000000000000000010097419586828951000000),
    COEF_CONST(0.00000000000000000000000000005048709793414475600000),
    COEF_CONST(0.00000000000000000000000000002524354896707237800000),
    COEF_CONST(0.00000000000000000000000000001262177448353618900000),
    COEF_CONST(0.00000000000000000000000000000631088724176809440000),
    COEF_CONST(0.00000000000000000000000000000315544362088404720000),
    COEF_CONST(0.00000000000000000000000000000157772181044202360000),
    COEF_CONST(0.00000000000000000000000000000078886090522101181000),
    COEF_CONST(0.00000000000000000000000000000039443045261050590000),
    COEF_CONST(0.00000000000000000000000000000019721522630525295000),
    COEF_CONST(0.00000000000000000000000000000009860761315262647600),
    COEF_CONST(0.00000000000000000000000000000004930380657631323800),
    COEF_CONST(0.00000000000000000000000000000002465190328815661900),
    COEF_CONST(0.00000000000000000000000000000001232595164407830900),
    COEF_CONST(0.00000000000000000000000000000000616297582203915470),
    COEF_CONST(0.00000000000000000000000000000000308148791101957740),
    COEF_CONST(0.00000000000000000000000000000000154074395550978870),
    COEF_CONST(0.00000000000000000000000000000000077037197775489434),
    COEF_CONST(0.00000000000000000000000000000000038518598887744717),
    COEF_CONST(0.00000000000000000000000000000000019259299443872359),
    COEF_CONST(0.00000000000000000000000000000000009629649721936179),
    COEF_CONST(0.00000000000000000000000000000000004814824860968090),
    COEF_CONST(0.00000000000000000000000000000000002407412430484045),
    COEF_CONST(0.00000000000000000000000000000000001203706215242022),
    COEF_CONST(0.00000000000000000000000000000000000601853107621011),
    COEF_CONST(0.00000000000000000000000000000000000300926553810506),
    COEF_CONST(0.00000000000000000000000000000000000150463276905253),
    COEF_CONST(0.00000000000000000000000000000000000075231638452626),
    COEF_CONST(0.00000000000000000000000000000000000037615819226313),
    COEF_CONST(0.00000000000000000000000000000000000018807909613157),
    COEF_CONST(0.00000000000000000000000000000000000009403954806578),
    COEF_CONST(0.00000000000000000000000000000000000004701977403289),
    COEF_CONST(0.00000000000000000000000000000000000002350988701645),
    COEF_CONST(0.00000000000000000000000000000000000001175494350822),
    COEF_CONST(0.0 /* 0000000000000000000000000000000000000587747175411 "floating point underflow" */),
    COEF_CONST(0.0)
};

#ifdef __cplusplus
}
#endif
#endif

#endif
