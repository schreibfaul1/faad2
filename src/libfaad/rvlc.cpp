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
** $Id: rvlc.c,v 1.21 2007/11/01 12:33:34 menno Exp $
**/

/* RVLC scalefactor decoding
 *
 * RVLC works like this:
 *  1. Only symmetric huffman codewords are used
 *  2. Total length of the scalefactor data is stored in the bitsream
 *  3. Scalefactors are DPCM coded
 *  4. Next to the starting value for DPCM the ending value is also stored
 *
 * With all this it is possible to read the scalefactor data from 2 sides.
 * If there is a bit error in the scalefactor data it is possible to start
 * decoding from the other end of the data, to find all but 1 scalefactor.
 */

#include "neaacdec.h"
#include "rvlc.h"
#include <stdlib.h>




/* static function declarations */

