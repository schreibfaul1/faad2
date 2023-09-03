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
** $Id: fixed.h,v 1.32 2007/11/01 12:33:30 menno Exp $
**/

#ifndef __FIXED_H__
#define __FIXED_H__

#ifdef __cplusplus
extern "C" {
#endif

#define COEF_BITS      28
#define COEF_PRECISION (1 << COEF_BITS)
#define REAL_BITS      14 // MAXIMUM OF 14 FOR FIXED POINT SBR
#define REAL_PRECISION (1 << REAL_BITS)
#define FRAC_SIZE      32 /* frac is a 32 bit integer */
#define FRAC_BITS      31
#define FRAC_PRECISION ((uint32_t)(1 << FRAC_BITS))
#define FRAC_MAX       0x7FFFFFFF
#define REAL_CONST(A)  (((A) >= 0) ? ((int32_t)((A) * (REAL_PRECISION) + 0.5)) : ((int32_t)((A) * (REAL_PRECISION)-0.5)))
#define COEF_CONST(A)  (((A) >= 0) ? ((int32_t)((A) * (COEF_PRECISION) + 0.5)) : ((int32_t)((A) * (COEF_PRECISION)-0.5)))
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

/* Complex multiplication */
static inline void ComplexMult(int32_t* y1, int32_t* y2, int32_t x1, int32_t x2, int32_t c1, int32_t c2) {
    *y1 = (_MulHigh(x1, c1) + _MulHigh(x2, c2)) << (FRAC_SIZE - FRAC_BITS);
    *y2 = (_MulHigh(x2, c1) - _MulHigh(x1, c2)) << (FRAC_SIZE - FRAC_BITS);
}

#ifdef __cplusplus
}
#endif
#endif
