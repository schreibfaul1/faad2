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
** $Id: neaacdec.h,v 1.14 2012/03/02 15:29:47 knik Exp $
**/

#ifndef __NEAACDEC_H__
#define __NEAACDEC_H__

#include <inttypes.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include "common.h"

/* COMPILE TIME DEFINITIONS */
#define PREFER_POINTERS
//#define ERROR_RESILIENCE
#define LTP_DEC  /* Allow decoding of LTP (long term prediction) profile AAC */
#define LD_DEC   /* Allow decoding of LD (low delay) profile AAC */
#define SBR_DEC  /* Allow decoding of SBR (spectral band replication) */
#define PS_DEC   /* Allow decoding of PS (parametric stereo */

//----------------------------------------------------------------------------------------------------------------------------------------------------
/* LD can't do without LTP */
#ifdef LD_DEC
    #ifndef ERROR_RESILIENCE
        #define ERROR_RESILIENCE
    #endif
    #ifndef LTP_DEC
        #define LTP_DEC
    #endif
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------

/* object types for AAC */
#define MAIN   1
#define LC     2
#define SSR    3
#define LTP    4
#define HE_AAC 5
#define ER_LC  17
#define ER_LTP 19
#define LD     23

/* header types */
#define RAW  0
#define ADIF 1
#define ADTS 2
#define LATM 3

/* SBR signalling */
#define NO_SBR           0
#define SBR_UPSAMPLED    1
#define SBR_DOWNSAMPLED  2
#define NO_SBR_UPSAMPLED 3

/* library output formats */
#define FAAD_FMT_16BIT  1
#define FAAD_FMT_24BIT  2
#define FAAD_FMT_32BIT  3
#define FAAD_FMT_FLOAT  4
#define FAAD_FMT_FIXED  FAAD_FMT_FLOAT
#define FAAD_FMT_DOUBLE 5

/* Capabilities */
#define LC_DEC_CAP           (1 << 0) /* Can decode LC */
#define MAIN_DEC_CAP         (1 << 1) /* Can decode MAIN */
#define LTP_DEC_CAP          (1 << 2) /* Can decode LTP */
#define LD_DEC_CAP           (1 << 3) /* Can decode LD */
#define ERROR_RESILIENCE_CAP (1 << 4) /* Can decode ER */
#define FIXED_POINT_CAP      (1 << 5) /* Fixed point */

/* Channel definitions */
#define FRONT_CHANNEL_CENTER (1)
#define FRONT_CHANNEL_LEFT   (2)
#define FRONT_CHANNEL_RIGHT  (3)
#define SIDE_CHANNEL_LEFT    (4)
#define SIDE_CHANNEL_RIGHT   (5)
#define BACK_CHANNEL_LEFT    (6)
#define BACK_CHANNEL_RIGHT   (7)
#define BACK_CHANNEL_CENTER  (8)
#define LFE_CHANNEL          (9)
#define UNKNOWN_CHANNEL      (0)

/* A decode call can eat up to FAAD_MIN_STREAMSIZE bytes per decoded channel,
   so at least so much bytes per channel should be available in this stream */
#define FAAD_MIN_STREAMSIZE 768 /* 6144 bits/channel */

typedef void* NeAACDecHandle;

typedef struct mp4AudioSpecificConfig {
    unsigned char objectTypeIndex; /* Audio Specific Info */
    unsigned char samplingFrequencyIndex;
    unsigned long samplingFrequency;
    unsigned char channelsConfiguration;
    unsigned char  frameLengthFlag; /* GA Specific Info */
    unsigned char  dependsOnCoreCoder;
    unsigned short coreCoderDelay;
    unsigned char  extensionFlag;
    unsigned char  aacSectionDataResilienceFlag;
    unsigned char  aacScalefactorDataResilienceFlag;
    unsigned char  aacSpectralDataResilienceFlag;
    unsigned char  epConfig;

    char sbr_present_flag;
    char forceUpSampling;
    char downSampledSBR;
} mp4AudioSpecificConfig;

typedef struct NeAACDecConfiguration {
    unsigned char defObjectType;
    unsigned long defSampleRate;
    unsigned char outputFormat;
    unsigned char downMatrix;
    unsigned char useOldADTSFormat;
    unsigned char dontUpSampleImplicitSBR;
} NeAACDecConfiguration, *NeAACDecConfigurationPtr;

typedef struct NeAACDecFrameInfo {
    unsigned long bytesconsumed;
    unsigned long samples;
    unsigned char channels;
    unsigned char error;
    unsigned long samplerate;
    unsigned char sbr; /* SBR: 0: off, 1: on; upsample, 2: on; downsampled, 3: off; upsampled */
    unsigned char object_type; /* MPEG-4 ObjectType */
    unsigned char header_type; /* AAC header type; MP4 will be signalled as RAW also */
    unsigned char num_front_channels; /* multichannel configuration */
    unsigned char num_side_channels;
    unsigned char num_back_channels;
    unsigned char num_lfe_channels;
    unsigned char channel_position[64];
    unsigned char ps; /* PS: 0: off, 1: on */
} NeAACDecFrameInfo;

const char*              NeAACDecGetErrorMessage(unsigned char errcode);
unsigned long            NeAACDecGetCapabilities(void);
NeAACDecHandle           NeAACDecOpen(void);
NeAACDecConfigurationPtr NeAACDecGetCurrentConfiguration(NeAACDecHandle hDecoder);
unsigned char            NeAACDecSetConfiguration(NeAACDecHandle hDecoder, NeAACDecConfigurationPtr config);
long                     NeAACDecInit(NeAACDecHandle hDecoder, unsigned char* buffer, unsigned long buffer_size, unsigned long* samplerate, unsigned char* channels);
char                     NeAACDecInit2(NeAACDecHandle hDecoder, unsigned char* pBuffer, unsigned long SizeOfDecoderSpecificInfo, unsigned long* samplerate, unsigned char* channels);
void                     NeAACDecPostSeekReset(NeAACDecHandle hDecoder, long frame);
void                     NeAACDecClose(NeAACDecHandle hDecoder);
void*                    NeAACDecDecode(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size);
void*                    NeAACDecDecode2(NeAACDecHandle hDecoder, NeAACDecFrameInfo* hInfo, unsigned char* buffer, unsigned long buffer_size, void** sample_buffer, unsigned long sample_buffer_size);
char                     NeAACDecAudioSpecificConfig(unsigned char* pBuffer, unsigned long buffer_size, mp4AudioSpecificConfig* mp4ASC);
int                      NeAACDecGetVersion(const char** faad_id_string, const char** faad_copyright_string);



#endif
