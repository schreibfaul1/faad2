
#include "aac_decoder.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "libfaad/neaacdec.h"

// Declaration of the required global variables

NeAACDecHandle hAac;
NeAACDecFrameInfo frameInfo;
NeAACDecConfigurationPtr_t conf;
bool f_decoderIsInit = false;
bool f_firstCall = false;
unsigned long aacSamplerate = 0;
unsigned char aacChannels = 0;
static uint16_t validSamples = 0;


//----------------------------------------------------------------------------------------------------------------------
bool AACDecoder_IsInit(){
    return f_decoderIsInit;
}
//----------------------------------------------------------------------------------------------------------------------
bool AACDecoder_AllocateBuffers(){

    // #define MAIN       1 /* MAIN */
    // #define LC         2 /* Low Complexity (default) */
    // #define SSR        3 /* Scalable SampleRate */
    // #define LTP        4 /* Long Term Predition */
    // #define HE_AAC     5 /* High Efficiency (SBR) */
    // #define ER_LC     17 /* Error Resilient Low Complexity */
    // #define ER_LTP    19 /* Error Resilient Long Term Prediction */
    // #define LD        23 /* Low Delay */

    hAac = NeAACDecOpen();
    conf = NeAACDecGetCurrentConfiguration(hAac);

    // conf->defObjectTypeMAIN       1 /* MAIN */
    // #define LC         2 /* Low Complexity (default) */
    // #define SSR        3 /* Scalable SampleRate */
    // #define LTP        4 /* Long Term Predition */
    // #define HE_AAC     5 /* High Efficiency (SBR) */
    // #define ER_LC     17 /* Error Resilient Low Complexity */
    // #define ER_LTP    19 /* Error Resilient Long Term Prediction */
    // #define LD        23 /* Low Delay */

    hAac = NeAACDecOpen();
    conf = NeAACDecGetCurrentConfiguration(hAac);

    // conf->defObjectType:  LC         2 /* Low Complexity (default) */
    //                       SSR        3 /* Scalable SampleRate */
    //                       LTP        4 /* Long Term Predition */
    //                       HE_AAC     5 /* High Efficiency (SBR) */
    //                       ER_LC     17 /* Error Resilient Low Complexity */
    //                       ER_LTP    19 /* Error Resilient Long Term Prediction */
    //                       LD        23 /* Low Delay */

    // conf -> outputFormat: FAAD_FMT_16BIT  1 /* 16 bit integers */
    //                       FAAD_FMT_24BIT  2 /* 24 bit values packed in 32 bit integers */
    //                       FAAD_FMT_32BIT  3 /* 32 bit integers */
    //                       FAAD_FMT_FLOAT  4 /* single precision floating point */
    //                       FAAD_FMT_DOUBLE 5 /* double precision floating point */

    // conf -> useOldADTSFormat: 0 56 bit ADTS header /* default */
    //                           1 58 bit ADTS header /* old standard */

    NeAACDecSetConfiguration(hAac, conf);
    if(hAac) f_decoderIsInit = true;
    f_firstCall = false;
    return f_decoderIsInit;
}
//----------------------------------------------------------------------------------------------------------------------
void AACDecoder_FreeBuffers(){
    NeAACDecClose(hAac);
    hAac = NULL;
    f_decoderIsInit = false;
    f_firstCall = false;
}
//----------------------------------------------------------------------------------------------------------------------
uint8_t AACGetFormat(){
    return frameInfo.header_type;  // RAW        0 /* No header */
                                   // ADIF       1 /* single ADIF header at the beginning of the file */
                                   // ADTS       2 /* ADTS header at the beginning of each frame */
}
//----------------------------------------------------------------------------------------------------------------------
uint8_t AACGetSBR(){
    return frameInfo.sbr;          // NO_SBR           0 /* no SBR used in this file */
                                   // SBR_UPSAMPLED    1 /* upsampled SBR used */
                                   // SBR_DOWNSAMPLED  2 /* downsampled SBR used */
                                   // NO_SBR_UPSAMPLED 3 /* no SBR used, but file is upsampled by a factor 2 anyway */
}
//----------------------------------------------------------------------------------------------------------------------
uint8_t AACGetParametricStereo(){  // not used (0) or used (1)
    return frameInfo.ps;
}
//----------------------------------------------------------------------------------------------------------------------
int AACFindSyncWord(uint8_t *buf, int nBytes){
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------
int AACSetRawBlockParams(int copyLast, int nChans, int sampRateCore, int profile){
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------
int16_t AACGetOutputSamps(){
    return validSamples;
}
//----------------------------------------------------------------------------------------------------------------------
int AACGetBitrate(){
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------
int AACGetChannels(){
    return aacChannels;
}
//----------------------------------------------------------------------------------------------------------------------
int AACGetSampRate(){
    return aacSamplerate;
}
//----------------------------------------------------------------------------------------------------------------------
int AACGetBitsPerSample(){
    return 16;
}
//----------------------------------------------------------------------------------------------------------------------
int AACDecode(uint8_t *inbuf, int *bytesLeft, short *outbuf){
    uint8_t* ob = (uint8_t*)outbuf;
    if (f_firstCall == false){
        char err = NeAACDecInit(hAac, inbuf, *bytesLeft, &aacSamplerate, &aacChannels);
        f_firstCall = true;
        return err;
    }

    NeAACDecDecode2(hAac, &frameInfo, inbuf, *bytesLeft, (void**)&ob, 2048 * 2 * sizeof(int16_t));
    *bytesLeft -= frameInfo.bytesconsumed;
    validSamples = frameInfo.samples;
    char err = 0 - frameInfo.error;
    return err;
}
//----------------------------------------------------------------------------------------------------------------------
const char* AACGetErrorMessage(int8_t err){
    return NeAACDecGetErrorMessage(err);
}
//----------------------------------------------------------------------------------------------------------------------
