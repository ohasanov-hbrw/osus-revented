#pragma once
#include "soundloader.h"
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"
#include "time_util.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "stb_vorbis.h"

#include "globals.hpp"

#define MAKEDWORD(a,b,c,d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))


int loadogg(const char *fileName, Sound *sound){
    FILE* file;
    file = fopen(fileName,"r");
    
    if(file == NULL){
        std::cout << "CANT OPEN FILE " << fileName << std::endl;
        fclose(file);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    int lSize = ftell(file);
    //std::cout << lSize << "bytes are available\n";
    fseek(file, 0, SEEK_SET);
    uint8_t *fileBuffer = (uint8_t *)malloc(lSize * sizeof(uint8_t));
    
    if(fread(fileBuffer, sizeof(uint8_t), lSize, file) != lSize){
        std::cout << "CANT READ FILE " << fileName << std::endl;
        free(fileBuffer);
        fclose(file);
        return 0;
    }
    fclose(file);
    //std::cout << "try to load drwav\n";

    int channels;
    int sampleRate;
    short *sampleBuffer;
    int samplesDecoded = stb_vorbis_decode_memory(fileBuffer, lSize, &channels, &sampleRate, &sampleBuffer);
    if(samplesDecoded <= 0){
        std::cout << "Failed to decode ogg\n";
        free(fileBuffer);
        return 0;
    }

    int sampleRateDivider = 1;
    if(sampleRate > 22050){
        sampleRateDivider = 2;
        //std::cout << "Halfing the sample rate\n";
    }
    int channelDivider = 1;
    if(channels == 2){
        channelDivider = 2;
        //std::cout << "Halfing the channels\n";
    }

    int maxSize = ((samplesDecoded / sampleRateDivider) / channelDivider);
    u16 *audioBuffer = (u16 *)linearAlloc(maxSize * sizeof(u16));

    /*float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0;
    mix[1] = 1.0;

    //std::cout << "New Sample Rate: " << sampleRate / sampleRateDivider << std::endl;
    //std::cout << "New SampleCount: " << maxSize << std::endl;


    ndspChnReset(0);
    ndspChnInitParams(0);
    ndspChnSetMix(0, mix);
    ndspSetMasterVol(1.0f);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, sampleRate / sampleRateDivider);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);*/

    if(channelDivider == 2 && sampleRateDivider == 2){
        for(int i = 0; i < samplesDecoded; i+=4){
            if(i / 4 > maxSize){
                break;
            }
            int mono1 = (sampleBuffer[i] + sampleBuffer[i + 1]) / 2;
            int mono2 = (sampleBuffer[i + 2] + sampleBuffer[i + 3]) / 2;
            int mono3 = (mono1 + mono2) / 2;
            mono3 = std::max(-32767, std::min(mono3, 32767));
            audioBuffer[i / 4] = (u16)mono3;
        }
    }
    else if((channelDivider == 1 && sampleRateDivider == 2) || (channelDivider == 2 && sampleRateDivider == 1)){
        for(int i = 0; i < samplesDecoded; i+=2){
            if(i / 2> maxSize){
                break;
            }
            int mono1 = (sampleBuffer[i] + sampleBuffer[i + 1]) / 2;
            mono1 = std::max(-32767, std::min(mono1, 32767));
            audioBuffer[i / 2] = (u16)mono1;
        }
    }
    else{
        for(int i = 0; i < samplesDecoded; i+=1){
            if(i / 1 > maxSize){
                break;
            }
            audioBuffer[i] = sampleBuffer[i];
        }
    }


    /*ndspWaveBuf waveBuf;
    memset(&waveBuf,0,sizeof(waveBuf));
    
    waveBuf.data_vaddr = audioBuffer;
    waveBuf.nsamples = maxSize;
    std::cout << maxSize << std::endl;

    DSP_FlushDataCache(waveBuf.data_vaddr, maxSize * sizeof(mp3d_sample_t));
    ndspChnWaveBufAdd(0, &waveBuf);
    //SleepInMs(5);
    std::cout << "Playing: " << ndspChnIsPlaying(0) << std::endl;
    std::cout << "Seq: " << ndspChnGetWaveBufSeq(0) << std::endl;
    std::cout << "SamplePos: " << ndspChnGetSamplePos(0) << std::endl;
    std::cout << "Done: " << (waveBuf.status == NDSP_WBUF_DONE) << std::endl;
    std::cout << "Paying: " << (waveBuf.status == NDSP_WBUF_PLAYING) << std::endl;
    std::cout << "Queue: " << (waveBuf.status == NDSP_WBUF_QUEUED) << std::endl;
    std::cout << "Free: " << (waveBuf.status == NDSP_WBUF_FREE) << std::endl;
    SleepInMs(1500);*/

    memset(&sound->waveBuf,0,sizeof(sound->waveBuf));
    sound->waveBuf.data_vaddr = audioBuffer;
    sound->waveBuf.nsamples = maxSize;
    DSP_FlushDataCache(sound->waveBuf.data_vaddr, maxSize * sizeof(u16));
    sound->sampleSize = maxSize;
    sound->sampleRate = sampleRate / sampleRateDivider;
    sound->channels = channels / channelDivider;
    sound->loaded = true;
    //std::cout << "loaded file\n";
    Global.totalNumOfSamples += sound->sampleSize;
    free(sampleBuffer);
    //linearFree(audioBuffer);
    free(fileBuffer);
    return 1;
}

int loadwav(const char *fileName, Sound *sound){
    FILE* file;
    file = fopen(fileName,"r");
    
    if(file == NULL){
        std::cout << "CANT OPEN FILE " << fileName << std::endl;
        fclose(file);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    int lSize = ftell(file);
    //std::cout << lSize << "bytes are available\n";
    fseek(file, 0, SEEK_SET);
    uint8_t *fileBuffer = (uint8_t *)malloc(lSize * sizeof(uint8_t));
    
    if(fread(fileBuffer, sizeof(uint8_t), lSize, file) != lSize){
        std::cout << "CANT READ FILE " << fileName << std::endl;
        free(fileBuffer);
        fclose(file);
        return 0;
    }
    fclose(file);
    //std::cout << "try to load drwav\n";

    drwav wav;
    if (!drwav_init_memory(&wav, fileBuffer, lSize, NULL)) {
        std::cout << "Error: Could not open wav\n";
        free(fileBuffer);
        return 0;
    }

    if(wav.totalPCMFrameCount < 100){
        std::cout << "Too small of a file..?\n";
        free(fileBuffer);
        return 0;
    }

    int sampleRate = wav.sampleRate;
    //std::cout << "Sample Rate: " << sampleRate << std::endl;
    //std::cout << "SampleCount: " << wav.totalPCMFrameCount << std::endl;

    drwav_int16* pDecodedInterleavedPCMFrames = (drwav_int16 *)malloc(wav.totalPCMFrameCount * wav.channels * sizeof(drwav_int16));
    size_t numberOfSamplesActuallyDecoded = drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pDecodedInterleavedPCMFrames);

    int sampleRateDivider = 1;
    if(sampleRate > 22050){
        sampleRateDivider = 2;
        //std::cout << "Halfing the sample rate\n";
    }
    int channelDivider = 1;
    if(wav.channels == 2){
        channelDivider = 2;
        //std::cout << "Halfing the channels\n";
    }

    int maxSize = ((numberOfSamplesActuallyDecoded / sampleRateDivider) / channelDivider);
    drwav_int16 *audioBuffer = (drwav_int16 *)linearAlloc(maxSize * sizeof(drwav_int16));

    /*float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0;
    mix[1] = 1.0;

    std::cout << "New Sample Rate: " << sampleRate / sampleRateDivider << std::endl;
    std::cout << "New SampleCount: " << maxSize << std::endl;


    ndspChnReset(0);
    ndspChnInitParams(0);
    ndspChnSetMix(0, mix);
    ndspSetMasterVol(1.0f);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, sampleRate / sampleRateDivider);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);*/

    if(channelDivider == 2 && sampleRateDivider == 2){
        for(int i = 0; i < numberOfSamplesActuallyDecoded; i+=4){
            if(i / 4 > maxSize){
                break;
            }
            int mono1 = (pDecodedInterleavedPCMFrames[i] + pDecodedInterleavedPCMFrames[i + 1]) / 2;
            int mono2 = (pDecodedInterleavedPCMFrames[i + 2] + pDecodedInterleavedPCMFrames[i + 3]) / 2;
            int mono3 = (mono1 + mono2) / 2;
            mono3 = std::max(-32767, std::min(mono3, 32767));
            audioBuffer[i / 4] = (drwav_int16)mono3;
        }
    }
    else if((channelDivider == 1 && sampleRateDivider == 2) || (channelDivider == 2 && sampleRateDivider == 1)){
        for(int i = 0; i < numberOfSamplesActuallyDecoded; i+=2){
            if(i / 2> maxSize){
                break;
            }
            int mono1 = (pDecodedInterleavedPCMFrames[i] + pDecodedInterleavedPCMFrames[i + 1]) / 2;
            mono1 = std::max(-32767, std::min(mono1, 32767));
            audioBuffer[i / 2] = (drwav_int16)mono1;
        }
    }
    else{
        for(int i = 0; i < numberOfSamplesActuallyDecoded; i+=1){
            if(i / 1 > maxSize){
                break;
            }
            audioBuffer[i] = pDecodedInterleavedPCMFrames[i];
        }
    }


    /*ndspWaveBuf waveBuf;
    memset(&waveBuf,0,sizeof(waveBuf));
    
    waveBuf.data_vaddr = audioBuffer;
    waveBuf.nsamples = maxSize;
    std::cout << maxSize << std::endl;

    DSP_FlushDataCache(waveBuf.data_vaddr, maxSize * sizeof(mp3d_sample_t));
    //ndspChnWaveBufAdd(0, &waveBuf);
    //SleepInMs(5);
    std::cout << "Playing: " << ndspChnIsPlaying(0) << std::endl;
    std::cout << "Seq: " << ndspChnGetWaveBufSeq(0) << std::endl;
    std::cout << "SamplePos: " << ndspChnGetSamplePos(0) << std::endl;
    std::cout << "Done: " << (waveBuf.status == NDSP_WBUF_DONE) << std::endl;
    std::cout << "Paying: " << (waveBuf.status == NDSP_WBUF_PLAYING) << std::endl;
    std::cout << "Queue: " << (waveBuf.status == NDSP_WBUF_QUEUED) << std::endl;
    std::cout << "Free: " << (waveBuf.status == NDSP_WBUF_FREE) << std::endl;
    //SleepInMs(1500);*/

    memset(&sound->waveBuf,0,sizeof(sound->waveBuf));
    sound->waveBuf.data_vaddr = audioBuffer;
    sound->waveBuf.nsamples = maxSize;
    DSP_FlushDataCache(sound->waveBuf.data_vaddr, maxSize * sizeof(drwav_int16));
    sound->sampleSize = maxSize;
    sound->sampleRate = sampleRate / sampleRateDivider;
    sound->channels = wav.channels / channelDivider;
    sound->loaded = true;
    //std::cout << "loaded file\n";
    Global.totalNumOfSamples += sound->sampleSize;
    free(pDecodedInterleavedPCMFrames);
    //linearFree(audioBuffer);
    free(fileBuffer);
    drwav_uninit(&wav);
    return 1;
}

int loadmp3(const char *fileName, Sound *sound){
    
    //std::cout << fileName << std::endl;
    FILE* file;
    file = fopen(fileName,"r");
    
    if(file == NULL){
        std::cout << "CANT OPEN FILE " << fileName << std::endl;
        fclose(file);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    int lSize = ftell(file);
    std::cout << lSize << "bytes are available\n";
    fseek(file, 0, SEEK_SET);
    uint8_t *fileBuffer = (uint8_t *)malloc(lSize * sizeof(uint8_t));
    
    if(fread(fileBuffer, sizeof(uint8_t), lSize, file) != lSize){
        std::cout << "CANT READ FILE " << fileName << std::endl;
        free(fileBuffer);
        fclose(file);
        return 0;
    }
    fclose(file);
    //std::cout << "try to load mp3dec\n";

    mp3dec_ex_t* decoder = (mp3dec_ex_t*)malloc(sizeof(mp3dec_ex_t));
    int error = mp3dec_ex_open_buf(decoder, fileBuffer, lSize, MP3D_SEEK_TO_SAMPLE);
    std::cout << error << std::endl;
    if(error){
        std::cout << "CANT OPEN MP3 FILE????\n";
        free(fileBuffer);
        free(decoder);
        return 0;
    }
    //std::cout << "OK" << std::endl;
    
    int samplesReadAtOneTime = 512;
    mp3dec_ex_seek(decoder, 0);

    //std::cout << "seek to 0\n";

    mp3d_sample_t *sampleBuffer = (mp3d_sample_t *)malloc(sizeof(mp3d_sample_t)*samplesReadAtOneTime);
    int totalRead = 0;
    while(true){
        size_t read = mp3dec_ex_read(decoder, sampleBuffer, samplesReadAtOneTime);
        //std::cout << read << std::endl;
        totalRead += read;
        if(read == 0){
            //std::cout << "nothing left to read\n";
            break;
        }
        else if(read != samplesReadAtOneTime){
            //std::cout << "nearing eof?\n";
        }
    }
    
    //std::cout << fileName << std::endl;
    //std::cout << "Samples: " << totalRead << std::endl;
    //std::cout << "Channels: " << decoder->info.channels << std::endl;
    //std::cout << "SampleRate: " << decoder->info.hz << std::endl;

    int sampleRateDivider = 1;
    if(decoder->info.hz > 22050){
        sampleRateDivider = 2;
        //std::cout << "Halfing the sample rate\n";
    }
    int channelDivider = 1;
    if(decoder->info.channels == 2){
        channelDivider = 2;
        //std::cout << "Halfing the channels\n";
    }
    
    /*float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0;
    mix[1] = 1.0;

    
    ndspChnReset(0);
    ndspChnInitParams(0);
    ndspChnSetMix(0, mix);
    ndspSetMasterVol(1.0f);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, decoder->info.hz / sampleRateDivider);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);*/

    int maxSize = ((totalRead / sampleRateDivider) / channelDivider);
    mp3d_sample_t *audioBuffer = (mp3d_sample_t *)linearAlloc(maxSize * sizeof(mp3d_sample_t));
    int bufferOffset = 0;
    int offset = 0;
    //std::cout << "Allocating audio buffer" << std::endl;
    //std::cout << (maxSize * sizeof(mp3d_sample_t) / 1024) << " kb allocated" << std::endl;
    mp3dec_ex_seek(decoder, 0);
    while(true){
        if(offset >= totalRead){
            break;
        }
        size_t read = mp3dec_ex_read(decoder, sampleBuffer, samplesReadAtOneTime);
        if(channelDivider == 2 && sampleRateDivider == 2){
            int tempBufferOffset = 0;
            for(int i = 0; i < read; i+=4){
                if(i / 4 > maxSize){
                    break;
                }
                int mono1 = ((int)sampleBuffer[i] + (int)sampleBuffer[i + 1]) / 2;
                int mono2 = ((int)sampleBuffer[i + 2] + (int)sampleBuffer[i + 3]) / 2;
                int mono3 = (mono1 + mono2) / 2;
                mono3 = std::max(-32767, std::min(mono3, 32767));
                audioBuffer[bufferOffset + i / 4] = (mp3d_sample_t)mono3;
                tempBufferOffset = std::max(tempBufferOffset, bufferOffset + i / 4 + 1);
            }
            bufferOffset = tempBufferOffset;
        }
        else if((channelDivider == 1 && sampleRateDivider == 2) || (channelDivider == 2 && sampleRateDivider == 1)){
            int tempBufferOffset = 0;
            for(int i = 0; i < read; i+=2){
                if(i / 2 > maxSize){
                    break;
                }
                int mono1 = ((u32)sampleBuffer[i] + (u32)sampleBuffer[i + 1]) / 2;
                mono1 = std::max(-32767, std::min(mono1, 32767));
                audioBuffer[bufferOffset + i / 2] = (mp3d_sample_t)mono1;
                tempBufferOffset = std::max(tempBufferOffset, bufferOffset + i / 2 + 1);
            }
            bufferOffset = tempBufferOffset;
        }
        else{
            int tempBufferOffset = 0;
            for(int i = 0; i < read; i+=1){
                if(i / 1 > maxSize){
                    break;
                }
                audioBuffer[bufferOffset + i] = sampleBuffer[i];
                tempBufferOffset = std::max(tempBufferOffset, bufferOffset + i + 1);
            }
            bufferOffset = tempBufferOffset;
        }
        
        offset += read;
        if(read == 0){
            std::cout << "nothing left to read\n";
            break;
        }
        else if(read != samplesReadAtOneTime){
            std::cout << "nearing eof?\n";
        }
    }

    std::cout << offset << std::endl;

    /*ndspWaveBuf waveBuf;
    memset(&waveBuf,0,sizeof(waveBuf));
    
    waveBuf.data_vaddr = audioBuffer;
    waveBuf.nsamples = maxSize;
    DSP_FlushDataCache(waveBuf.data_vaddr, maxSize * sizeof(mp3d_sample_t));
    //ndspChnWaveBufAdd(0, &waveBuf);
    //SleepInMs(5);
    std::cout << "Playing: " << ndspChnIsPlaying(0) << std::endl;
    std::cout << "Seq: " << ndspChnGetWaveBufSeq(0) << std::endl;
    std::cout << "SamplePos: " << ndspChnGetSamplePos(0) << std::endl;
    std::cout << "Done: " << (waveBuf.status == NDSP_WBUF_DONE) << std::endl;
    std::cout << "Paying: " << (waveBuf.status == NDSP_WBUF_PLAYING) << std::endl;
    std::cout << "Queue: " << (waveBuf.status == NDSP_WBUF_QUEUED) << std::endl;
    std::cout << "Free: " << (waveBuf.status == NDSP_WBUF_FREE) << std::endl;
    //SleepInMs(1500);*/

    memset(&sound->waveBuf,0,sizeof(sound->waveBuf));
    sound->waveBuf.data_vaddr = audioBuffer;
    DSP_FlushDataCache(sound->waveBuf.data_vaddr, maxSize * sizeof(mp3d_sample_t));
    sound->waveBuf.nsamples = maxSize;
    sound->sampleSize = maxSize;
    sound->sampleRate = decoder->info.hz / sampleRateDivider;
    sound->channels = decoder->info.channels / channelDivider;
    sound->loaded = true;
    //std::cout << "loaded file\n";
    Global.totalNumOfSamples += sound->sampleSize;
    mp3dec_ex_close(decoder);
    free(fileBuffer);
    free(decoder);
    free(sampleBuffer);
    //linearFree(audioBuffer);
    return 1;
}

Sound decodeSoundFromFile(const char *fileName){
    Sound sound = { 0 };
    sound.loaded = false;
    //std::cout << "Loading " << fileName << std::endl;
    FILE* file;
    file = fopen(fileName,"r");
    
    if(file == NULL){
        std::cout << "CANT OPEN FILE " << fileName << std::endl;
        fclose(file);
        return sound;
    }

    //std::cout << "loaded file for reading\n";
    fseek(file, 0, SEEK_END);
    int lSize = ftell(file);
    //std::cout << lSize << "bytes are available\n";
    fseek(file, 0, SEEK_SET);

    u32 tag;
    //std::cout << "reading tag\n";
    if(fread(&tag, 1, 4, file) != 4){
        std::cout << "CANT READ FILE " << fileName << std::endl;
        fclose(file);
        return sound;
    }
    //std::cout << "read tag\n";
    fclose(file);
    //std::cout << "closed file\n";

    if(tag == (u32)MAKEDWORD('O','g','g','S')){
        std::cout << "ogg file: " << fileName << std::endl;
        loadogg(fileName, &sound);
    } 
    else if(tag == (u32)MAKEDWORD('R','I','F','F')){
        std::cout << "wav file: " << fileName << std::endl;
        loadwav(fileName, &sound);
    }
    else if(loadmp3(fileName, &sound) == 1){
        std::cout << "mp3 file: " << fileName << std::endl;
    }
    else{
        std::cout << "no. file: " << fileName << std::endl;
    }
    std::cout << Global.totalNumOfSamples * 2 << " bytes loaded total in audio memory" << std::endl;
    return sound;
}