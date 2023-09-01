#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "libfaad/neaacdec.h"



// Declaration of the required global variables
NeAACDecHandle hAac;
NeAACDecFrameInfo frameInfo;
NeAACDecConfigurationPtr conf;
uint8_t inBuffer[4096];
uint8_t outBuffer[4096 * 2];
uint8_t* ob = outBuffer;
unsigned long  bytesRead, pcmSize;
unsigned long  samplerate = 0;
unsigned long  byterate = 0;
unsigned char  channels = 0;
unsigned long  sumSamples = 0;

//const char* aacFileName = "sample1.aac";  // stereo 44100Hz
//const char* aacFileName = "sample2.aac";  // stereo 48000Hz
//const char* aacFileName = "sample3.aac";  // stereo 44100Hz, SBR
const char* aacFileName = "sample4.aac";  // stereo 44100Hz, SBR + PS

const char* wavFileName = "out.wav";

int main() {
    hAac = NeAACDecOpen();
    conf = NeAACDecGetCurrentConfiguration(hAac);
    FILE *aacFile = NULL;
    aacFile = fopen(aacFileName, "r");
    if(aacFile == NULL) {
        printf("aac file not found\n");
        exit(1);
    }

    FILE *wavFile = NULL;
    wavFile = fopen("out.wav", "w");

    // Read the first aac frame to determine the parameters
    bytesRead = fread(inBuffer, 1, sizeof(inBuffer), aacFile);
    printf("first bytesRead %ld\n", bytesRead);

    NeAACDecSetConfiguration(hAac, conf);
    char err = NeAACDecInit(hAac, inBuffer, sizeof(inBuffer), &samplerate, &channels);

    if(err != 0){
        printf("error %d -> exit\n", err);
        return 0;
    }

    printf("samplerate = %li\n", samplerate);
    printf("channels = %i\n", channels);

    byterate = samplerate * channels * 16  / 8;

    // Write the WAV header
    fwrite("RIFF",             4, 1, wavFile);  // Chunk ID
    fwrite("\x00\x00\x00\x00", 4, 1, wavFile);  // Chunk Size (will be filled later)
    fwrite("WAVE",             4, 1, wavFile);  // Format
    fwrite("fmt ",             4, 1, wavFile);  // Subchunk 1 ID
    fwrite("\x10\x00\x00\x00", 4, 1, wavFile);  // Subchunk 1 Size
    fwrite("\x01\x00",         2, 1, wavFile);  // AudioFormat
    fwrite(&channels,          2, 1, wavFile);  // NumChannels
    fwrite(&samplerate,        4, 1, wavFile);  // Sample Rate
    fwrite(&byterate,          4, 1, wavFile);  // Byte Rate
    fwrite("\x02\x00",         2, 1, wavFile);  // Block Align
    fwrite("\x10\x00",         2, 1, wavFile);  // Bits Per Sample
    fwrite("data",             4, 1, wavFile);  // Subchunk 2 ID
    fwrite("\x00\x00\x00\x00", 4, 1, wavFile);  // Subchunk 2 Size (will be filled later)


    // decode file
    size_t pos = 0;
    int i = 0;
    int loopidx = 0;
    uint16_t bytesconsumed = 0;
    while(1){
        fseek(aacFile, pos, SEEK_SET);
        bytesRead = fread(inBuffer, 1, sizeof(inBuffer), aacFile);
        if(bytesRead < 1) break;
        NeAACDecDecode2(hAac, &frameInfo, inBuffer, sizeof(inBuffer), (void**)&ob, sizeof(outBuffer));
        bytesconsumed = frameInfo.bytesconsumed;
    //      printf("bytesconsumed %i \n", bytesconsumed);
        pos += bytesconsumed;
        if(bytesconsumed == 0) break;
    //     printf("samples %ld\n", frameInfo.samples);
        fwrite(ob, 1, frameInfo.samples * frameInfo.channels, wavFile);
        if(loopidx == 1){
            printf("Parametric Stereo %d (0 off, 1 on)\n", frameInfo.ps);
            printf("AAC Header Type %d (0 raw, 1 ADIF, 2 ADTS)\n",frameInfo.header_type);
            printf("Spectral Band Replication %d (0 no SBR, 1 upsampled SBR, 2 downsampled SBR, 3 no SBR but upsampled)\n",frameInfo.sbr);
        }
        if(frameInfo.error) printf("err %s\n", NeAACDecGetErrorMessage(frameInfo.error));
        loopidx++;
        sumSamples += frameInfo.samples * frameInfo.channels;
    }

    printf("ready\n");
    NeAACDecClose(hAac);
    printf("%ld bytes written\n", sumSamples);
    fseek(wavFile, 40, SEEK_SET);
    fwrite(&sumSamples, 4, 1, wavFile);  // Subchunk 2 Size
    sumSamples += 34;
    fseek(wavFile, 4, SEEK_SET);
    fwrite(&sumSamples, 4, 1, wavFile);  // Subchunk 1 Size
    fclose(aacFile);
    fclose(wavFile);
    return 0;
}