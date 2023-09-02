
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "aac_decoder.h"


// Declaration of the required global variables

uint8_t inBuffer[4096];
int16_t outBuffer[2048 * 2 * sizeof(int16_t)];

unsigned long  bytesRead, pcmSize;
unsigned long  samplerate = 0;
unsigned long  byterate = 0;
unsigned char  channels = 0;
unsigned long  sumSamples = 0;

//const char* aacFileName = "sample1.aac";  // stereo 44100Hz
const char* aacFileName = "sample2.aac";  // stereo 48000Hz
//const char* aacFileName = "sample3.aac";  // stereo 44100Hz, SBR
//const char* aacFileName = "sample4.aac";  // stereo 44100Hz, SBR + PS
const char* wavFileName = "out.wav";
uint16_t    validSamples = 0;

int main() {
    FILE *aacFile = NULL;
    aacFile = fopen(aacFileName, "r");
    if(aacFile == NULL) {
        printf("aac file not found\n");
        exit(1);
    }

    FILE *wavFile = NULL;
    wavFile = fopen("out.wav", "w");

    bool ab = AACDecoder_AllocateBuffers();


    // Read the first aac frame to determine the parameters
    bytesRead = fread(inBuffer, 1, sizeof(inBuffer), aacFile);
    printf("first bytesRead %ld\n", bytesRead);

    int ibs = sizeof(inBuffer);
    int err = AACDecode(inBuffer, &ibs, outBuffer);

    if(err != 0){
        printf("error %d -> exit\n", err);
        return 0;
    }
    samplerate = AACGetSampRate();
    channels = AACGetChannels();
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
        int bytesLeft = bytesRead;
    //    printf("bytesLeft %i, bytesRead %li\n", bytesLeft, bytesRead);
        int16_t err = AACDecode(inBuffer, &bytesLeft, outBuffer);
    //    printf("bytesLeft %i, bytesRead %li\n", bytesLeft, bytesRead);
    //    printf("err %i \n", err);
        bytesconsumed = bytesRead - bytesLeft;
    //    printf("bytesconsumed %i \n", bytesconsumed);
        pos += bytesconsumed;
        if(bytesconsumed == 0) break;
        validSamples = AACGetOutputSamps();
    //    printf("samples %d\n", validSamples);
        fwrite(outBuffer, 1, validSamples * channels, wavFile);
        if(loopidx == 1){
            printf("Parametric Stereo %d (0 off, 1 on)\n", AACGetParametricStereo());
            printf("AAC Header Type %d (0 RAW, 1 ADIF, 2 ADTS)\n", AACGetFormat());
            printf("Spectral Band Replication %d (0 no SBR, 1 upsampled SBR, 2 downsampled SBR, 3 no SBR but upsampled)\n",AACGetSBR());
        }
        if(err) printf("error: %s", AACGetErrorMessage(err));
        loopidx++;
        sumSamples += validSamples * channels;
    }

    printf("ready\n");
    AACDecoder_FreeBuffers();
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