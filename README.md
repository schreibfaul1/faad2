

Install: (VS Code) <br>
Test environment: an aac file is converted into a wav file
- Clone Git Repository
- compile **`g++ -g ./src/*.c src/*/*.c -o a.out`**
- execute **`./a.out`**


**Freeware Advanced Audio (AAC) Decoder including SBR decoding**

FAAD2 is a HE, LC, MAIN and LTP profile, MPEG2 and MPEG-4 AAC decoder.
FAAD2 includes code for SBR (HE AAC) decoding.
FAAD2 is licensed under the GPL.


__________
COPYRIGHTS

For FAAD2 the following license applies:

******************************************************************************
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
******************************************************************************


Please note that the use of this software may require the payment of
patent royalties. You need to consider this issue before you start
building derivative works. We are not warranting or indemnifying you in
any way for patent royalities! YOU ARE SOLELY RESPONSIBLE FOR YOUR OWN
ACTIONS!


___________________
DIRECTORY STRUCTURE

faad2 - top level directory.

   docs - API documentation.

   frontend - command line frontend to the FAAD2 library, also supports
              MPEG-4 file decoding.

   include - inlude file for the FAAD2 library.

   libfaad - the FAAD2 AAC decoder library including SBR.

      codebook - Huffman codebooks.

   project/msvc - Visual Studio 2017 project files.
