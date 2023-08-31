#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "libfaad/neaacdec.h"

// Declaration of the required global variables
NeAACDecHandle hAac = NeAACDecOpen();
NeAACDecFrameInfo frameInfo;
NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(hAac);
uint8_t inBuffer[4096];
uint8_t outBuffer[4096 * 2];
uint8_t* ob = outBuffer;
unsigned long bytesRead, pcmSize;
unsigned long  samplerate = 0;
uint8_t channels = 0;
const char* aacFileName = "sample_48000.aac";
const char* wavFileName = "out.wav";

int main() {

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
	printf("bytesRead %ld\n", bytesRead);

    NeAACDecSetConfiguration(hAac, conf);
    char err = NeAACDecInit(hAac, inBuffer, sizeof(inBuffer), &samplerate, &channels);

	if(err != 0){
		printf("error %d -> exit\n", err);
		return 0;
	}

	printf("samplerate = %li\n", samplerate);
	printf("channels = %i\n", channels);

	const char* sr;
	if(samplerate == 44100)      sr = "\x44\xAC\x00\x00";
	else if(samplerate == 48000) sr = "\x80\xBB\x00\x00";
	else if(samplerate == 22050) sr = "\x22\x56\x00\x00";
	else {printf("wrong samplerate\n"); return 0;}

	const char* ch;
	if(channels == 1) ch = "\x01\x00";
	else if(channels == 2) ch = "\x02\x00";
	else {printf("wrong channels\n"); return 0;}

	// Write the WAV header
	fwrite("RIFF", 4, 1, wavFile);              // Chunk ID
	fwrite("\x00\x00\x00\x00", 4, 1, wavFile);  // Chunk Size (will be filled later)
	fwrite("WAVE", 4, 1, wavFile);              // Format
	fwrite("fmt ", 4, 1, wavFile);              // Subchunk 1 ID
	fwrite("\x10\x00\x00\x00", 4, 1, wavFile);  // Subchunk 1 Size
	fwrite("\x01\x00", 2, 1, wavFile);          // AudioFormat
	fwrite(ch        , 2, 1, wavFile);          // NumChannels
	fwrite(sr        , 4, 1, wavFile);          // Sample Rate
	fwrite("\x00\x00\x00\x00", 4, 1, wavFile);  // Byte Rate
	fwrite("\x04\x00", 2, 1, wavFile);          // Block Align
	fwrite("\x10\x00", 2, 1, wavFile);          // Bits Per Sample
	fwrite("data", 4, 1, wavFile);              // Subchunk 2 ID
	fwrite("\x00\x00\x00\x00", 4, 1, wavFile);  // Subchunk 2 Size (will be filled later)


	// decode file
	size_t pos = 0;
	int loopidx = 0;
	uint16_t bytesconsumed = 0;
	while(1){
		fseek(aacFile, pos, SEEK_SET);
		bytesRead = fread(inBuffer, 1, sizeof(inBuffer), aacFile);
		if(bytesRead < 1) break;
		NeAACDecDecode2(hAac, &frameInfo, inBuffer, sizeof(inBuffer), (void**)&ob, sizeof(outBuffer));
		bytesconsumed = frameInfo.bytesconsumed;
    	// printf("bytesconsumed %i \n", bytesconsumed);
		pos += bytesconsumed;
		if(bytesconsumed == 0) break;
		// printf("samples %ld\n",frameInfo.samples);
		fwrite(ob, 1, frameInfo.samples * frameInfo.channels, wavFile);
		if(loopidx == 1){
			printf("Parametric Stereo %d (0 off, 1 on)\n", frameInfo.ps);
			printf("AAC Header Type %d (0 raw, 1 ADIF, 2 ADTS)\n",frameInfo.header_type);
			printf("Spectral Band Replication %d (0 no SBR, 1 upsampled SBR, 2 downsampled SBR, 3 no SBR but upsampled)\n",frameInfo.sbr);
		}
		if(frameInfo.error) printf("err %s\n", NeAACDecGetErrorMessage(frameInfo.error));
		loopidx++;
	}

	printf("ready\n");
	fclose(aacFile);
	fclose(wavFile);
	return 0;
}