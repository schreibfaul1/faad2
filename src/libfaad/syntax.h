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
** $Id: syntax.h,v 1.60 2009/01/26 23:51:17 menno Exp $
**/

#ifndef __SYNTAX_H__
#define __SYNTAX_H__


#include "neaacdec.h"




int8_t GASpecificConfig(bitfile* ld, mp4AudioSpecificConfig* mp4ASC, program_config* pce);

uint8_t adts_frame(adts_header* adts, bitfile* ld);
void    get_adif_header(adif_header* adif, bitfile* ld);
void    raw_data_block(NeAACDecStruct* hDecoder, NeAACDecFrameInfo* hInfo, bitfile* ld, program_config* pce, drc_info* drc);
uint8_t reordered_spectral_data(NeAACDecStruct* hDecoder, ic_stream* ics, bitfile* ld, int16_t* spectral_data);

uint32_t faad_latm_frame(latm_header* latm, bitfile* ld);

#endif
