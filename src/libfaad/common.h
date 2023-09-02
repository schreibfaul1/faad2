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
** $Id: common.h,v 1.79 2015/01/26 17:48:53 knik Exp $
**/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "neaacdec.h"

/* COMPILE TIME DEFINITIONS */

#define USE_DOUBLE_PRECISION /* use double precision */
#define FIXED_POINT
#define BIG_IQ_TABLE
#define PREFER_POINTERS
#define ERROR_RESILIENCE
#define MAIN_DEC  /* Allow decoding of MAIN profile AAC */
#define SSR_DEC   /* Allow decoding of SSR profile AAC */
#define LTP_DEC   /* Allow decoding of LTP profile AAC */
#define LD_DEC    /* Allow decoding of LD profile AAC */
#define SBR_DEC
#define PS_DEC



/* LD can't do without LTP */
#ifdef LD_DEC
  #ifndef ERROR_RESILIENCE
    #define ERROR_RESILIENCE
  #endif

  #ifndef LTP_DEC
  #define LTP_DEC
  #endif
#endif




// Define LC_ONLY_DECODER if you want a pure AAC LC decoder (independant of SBR_DEC and PS_DEC)
//#define LC_ONLY_DECODER
#ifdef LC_ONLY_DECODER
  #undef LD_DEC
  #undef LTP_DEC
  #undef MAIN_DEC
  #undef SSR_DEC

  #undef ALLOW_SMALL_FRAMELENGTH
  #undef ERROR_RESILIENCE
#endif




/* FIXED POINT: No MAIN decoding */
#ifdef FIXED_POINT
# ifdef MAIN_DEC
#  undef MAIN_DEC
# endif
#endif // FIXED_POINT

#ifdef FIXED_POINT
#define DIV_R(A, B) (((int64_t)A << REAL_BITS)/B)
#define DIV_C(A, B) (((int64_t)A << COEF_BITS)/B)
#else
#define DIV_R(A, B) ((A)/(B))
#define DIV_C(A, B) ((A)/(B))
#endif


#define qmf_t complex_t
#define QMF_RE(A) RE(A)
#define QMF_IM(A) IM(A)


/* END COMPILE TIME DEFINITIONS */

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif




typedef float float32_t;




/* FIXED_POINT doesn't work with MAIN and SSR yet */
#ifdef FIXED_POINT
  #undef MAIN_DEC
  #undef SSR_DEC
#endif

#include "fixed.h"



typedef int32_t complex_t[2];
#define RE(A) A[0]
#define IM(A) A[1]


/* common functions */
uint8_t cpu_has_sse(void);
uint32_t ne_rng(uint32_t *__r1, uint32_t *__r2);
uint32_t wl_min_lzc(uint32_t x);
#ifdef FIXED_POINT
#define LOG2_MIN_INF REAL_CONST(-10000)
int32_t log2_int(uint32_t val);
int32_t log2_fix(uint32_t val);
int32_t pow2_int(int32_t val);
int32_t pow2_fix(int32_t val);
#endif
uint8_t get_sr_index(const uint32_t samplerate);
uint8_t max_pred_sfb(const uint8_t sr_index);
uint8_t max_tns_sfb(const uint8_t sr_index, const uint8_t object_type,
                    const uint8_t is_short);
uint32_t get_sample_rate(const uint8_t sr_index);
int8_t can_decode_ot(const uint8_t object_type);

void *faad_malloc(size_t size);
void faad_free(void *b);



#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2 /* PI/2 */
#define M_PI_2 1.57079632679489661923
#endif


#ifdef __cplusplus
}
#endif





#endif
