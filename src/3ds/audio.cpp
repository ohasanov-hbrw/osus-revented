#include "audio.h"
//#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"
#include "globals.hpp"
#include <vector>
#include "soundloader.h"
#include "time_util.hpp"

void SetSoundVolume(Sound *sound, float volume){
    sound->volume = volume;
    if(sound->volume > 1.0f){
        sound->volume = 1.0f;
    }
    if(sound->volume < 0.0f){
        sound->volume = 0.0f;
    }
}
void SetSoundPan(Sound *sound, float pan){
    sound->pan = pan;
    if(sound->pan > 1.0f){
        sound->pan = 1.0f;
    }
    if(sound->pan < 0.0f){
        sound->pan = 0.0f;
    }
}
void PlaySound(Sound *sound){
    //return;
    if(sound->loaded){
        if(sound->waveBuf.status == NDSP_WBUF_PLAYING && sound->id > 1 && sound->id < 23){
            Global.channelOccupied[sound->id] = false;
            Global.soundAtChannel[sound->id] = NULL;
            sound->id = 0;
        }
        for(int i = 2; i < 23; i++){
            if(Global.channelOccupied[i]){
                //std::cout << "trying to free channel " << i << std::endl;
                if(Global.soundAtChannel[i] != NULL){
                    //std::cout << (int)Global.soundAtChannel[i]->waveBuf.status << "  -  " << Global.soundAtChannel[i] << std::endl;
                    if(Global.soundAtChannel[i]->waveBuf.status == NDSP_WBUF_DONE){
                        Global.soundAtChannel[i]->waveBuf.status = NDSP_WBUF_FREE;
                        Global.channelOccupied[i] = false;
                        Global.soundAtChannel[i] = NULL;
                        //std::cout << "freeing the channel\n";
                    }
                }
                else{
                    Global.channelOccupied[i] = false;
                    ndspChnWaveBufClear(i);
                }
            }
        }
        for(int i = 2; i < 23; i++){
            if(!Global.channelOccupied[i]){
                float mix[12];
                memset(mix, 0, sizeof(mix));
                mix[0] = sound->volume * std::max(0.0f, std::min((sound->pan) * 2.0f, 1.0f));
                mix[1] = sound->volume * std::max(0.0f, std::min((1.0f - sound->pan) * 2.0f, 1.0f));
                ndspChnWaveBufClear(i);
                ndspChnSetMix(i, mix);
                ndspSetMasterVol(1.0f);
                ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
                ndspChnSetRate(i, sound->sampleRate);
                ndspChnSetFormat(i, NDSP_FORMAT_MONO_PCM16);

                ndspChnWaveBufAdd(i, &sound->waveBuf);
                
                sound->id = i;
                Global.channelOccupied[i] = true;
                Global.soundAtChannel[i] = sound;
                Global.soundAtChannel[i]->waveBuf.status = NDSP_WBUF_PLAYING;

                //std::cout << "sound on channel " << i << std::endl;
                return;
            }
        }
        std::cout << "No channels left...\n";
    }
}

void PlayMusicStream(Music *music){
    if(!Global.MusicLoaded || !music->loaded)
        return;
    if(music->playing){
        return;
    }
    else{
        //int channel = 0;
        music->playing = true;
        music->ended = false;
        music->paused = false;
        music->command = 0x00000000;
        music->thread = threadCreate((void (*)(void*))MusicThread, music, 512*1024, 0x27, -1, false);
    }

}

void SetMusicVolume(Music *music, float volume){
    music->volume = volume;
    if(music->volume > 1.0f){
        music->volume = 1.0f;
    }
    if(music->volume < 0.0f){
        music->volume = 0.0f;
    }
    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = music->volume;
    mix[1] = music->volume;
    ndspChnSetMix(0, mix);
}

void SeekMusicStream(Music *music, float time){
    //return;
    if(!Global.MusicLoaded || !music->loaded || !music->playing)
        return;
    if(time < 0){
        time = 0;
    }
    music->seek = (u64)(time * 1000.0f);
    u8 newCommand = music->command | 0x000000F0;
    AtomicSwap(&music->command, newCommand);
    while((music->command & 0x000000F0) == 0x000000F0){
        SleepInMs(1);
    }


}

void UpdateMusicStream(Music *music){

}

float GetMusicTimePlayed(Music *music){
    int channel = 0;
    u64 timePlayed = music->playedChannels * 250 + music->offset + (ndspChnGetSamplePos(channel) * 1000) / music->sampleRate;
    timePlayed = std::max(music->lastTimePlayed, timePlayed);
    music->lastTimePlayed = timePlayed;
    
    return (float)timePlayed / 1000.0f;
}

float GetMusicTimeLength(Music *music){
    return (float)music->length / 1000.0f;
}

void StopMusicStream(Music *music){
    if(!Global.MusicLoaded)
        return;
    if(!music->playing){
        return;
    }
    else{
        int channel = 0;
        music->paused = true;
        u8 newCommand = 0x0000000F;
        music->command = newCommand;
        std::cout << "music kill command sent\n";
        if(music->playing){
            threadJoin(music->thread, U64_MAX);
            threadFree(music->thread);
            std::cout << "music thread joined\n";
        }
        for(int i = 0; i < 24; i++){
            ndspChnReset(i);
            ndspChnWaveBufClear(i);
        }
        music->playing = false;
        music->ended = true;
    }

}

bool IsMusicStreamPlaying(Music *music){
    return music->playing;
}

Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize){
    Music music = {0};
    if(Global.MusicLoaded){
        return music;
    }
    return music;
}

void MusicThread(Music *music){
    int channel = 0;
	int bufferIndex = 0;
	int sampleRate = music->sampleRate;
	int bufferSamples = music->sampleRate / 4;
	int bufferSize = bufferSamples * music->channels * sizeof(mp3d_sample_t);

    int numberOfBuffers = 2;

    music->offset = 0;
    music->playedChannels = 0;
    music->lastTimePlayed = 0;

    std::cout << "started music thread" << std::endl;

    ndspChnReset(channel);
    ndspChnWaveBufClear(channel);
	ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
	ndspChnSetRate(channel, sampleRate);
    if(music->channels == 1)
	    ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
    else
	    ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = music->volume;
    mix[1] = music->volume;

    ndspChnSetMix(channel, mix);
    ndspSetMasterVol(1.0f);

    for(int i = 0; i < numberOfBuffers; i++){
        ndspChnWaveBufAdd(channel, &music->waveBuf[i]);
    }

    while(true){
        u16 command = music->command & 0x0000000F;
        if(command == 0x0000000F){
            break;
        }
        SleepInMs(10);
        for(int i = 0; i < numberOfBuffers; i++){
            if (music->waveBuf[i].status == NDSP_WBUF_DONE) {
                music->waveBuf[i].nsamples = mp3dec_ex_read(music->decoder, (mp3d_sample_t*)music->waveBuf[i].data_vaddr, bufferSamples * music->channels) / music->channels;
                if(music->waveBuf[i].nsamples > 0){
                    int playing = -1;
                    for(int j = 0; j < numberOfBuffers; j++){
                        if(music->waveBuf[j].status == NDSP_WBUF_PLAYING){
                            playing = j;
                            break;
                        }
                    }
                    //std::cout << "queued: " << i << "  playing: " << playing << std::endl;
                    DSP_FlushDataCache(music->waveBuf[i].data_vaddr, bufferSize);
                    if(music->waveBuf[i].status == NDSP_WBUF_DONE){
                        music->playedChannels++;
                    }
                    ndspChnWaveBufAdd(channel, &music->waveBuf[i]);
                }
                else{
                    goto exitMusicThread;
                }
                
            }
        }  
        if((music->command & 0x000000F0) == 0x000000F0){
            ndspChnReset(channel);
            ndspChnWaveBufClear(channel);
            ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
            ndspChnSetRate(channel, sampleRate);
            if(music->channels == 1)
                ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
            else
                ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);
            float mix[12];
            memset(mix, 0, sizeof(mix));
            mix[0] = 1.0;
            mix[1] = 1.0;

            ndspChnSetMix(channel, mix);
            ndspSetMasterVol(1.0f);
            for (int i = 0; i < numberOfBuffers; ++i) {
                music->waveBuf[i].status = NDSP_WBUF_FREE;
            }
            mp3dec_ex_seek(music->decoder, (music->seek * music->sampleRate * music->channels) / 1000);
            for(int i = 0; i < numberOfBuffers; i++){
                if (music->waveBuf[i].status == NDSP_WBUF_FREE) {
                    music->waveBuf[i].nsamples = mp3dec_ex_read(music->decoder, (mp3d_sample_t*)music->waveBuf[i].data_pcm16, bufferSamples * music->channels) / music->channels;
                    if(music->waveBuf[i].nsamples > 0){
                        //std::cout << "preloaded buffer: " << i << std::endl;
                        DSP_FlushDataCache(music->waveBuf[i].data_pcm16, bufferSize);
                    }
                    else{
                        break;
                    }
                }
            }
            for(int i = 0; i < numberOfBuffers; i++){
                ndspChnWaveBufAdd(channel, &music->waveBuf[i]);
            }

            music->offset = music->seek;
            music->playedChannels = 0;
            music->lastTimePlayed = music->seek;
            u8 newCommand = music->command & 0xFFFFFF0F;
            music->command = newCommand;
        }
    }

    exitMusicThread:
    
    while(true){
        bool exit = true;
        for(int i = 0; i < numberOfBuffers; i++){
            if(music->waveBuf[i].status == NDSP_WBUF_PLAYING){
                exit = false;
            }
        }   
        SleepInMs(5);
        if(exit){
            break;
        }
    }
    ndspChnReset(channel);
    ndspChnWaveBufClear(channel);
    music->playing = false;
    music->ended = true;
    return;
}

Music LoadMusicStream(const char *filename){
    Music music = {0};
    music.decoder = (mp3dec_ex_t*)malloc(sizeof(mp3dec_ex_t));
    if(Global.MusicLoaded){
        return music;
    }

    FILE* file;
    file = fopen(filename,"r");
    
    if(file == NULL){
        std::cout << "CANT OPEN FILE " << filename << std::endl;
        fclose(file);
        return music;
    }

    

    fseek(file, 0, SEEK_END);
    size_t lSize = ftell(file);
    std::cout << lSize << "bytes are available\n";
    fseek(file, 0, SEEK_SET);
    music.fileSize = lSize;
    if(lSize > 1024 * 1024 * 16){
        std::cout << "TOO BIG OF A FILE TO LOAD INTO MEMORY TBH\n";
        fclose(file);
        goto skipMemory;
        
    }
    music.fileBuffer = (uint8_t *)linearAlloc(lSize * sizeof(uint8_t));
    if(music.fileBuffer == NULL){
        music.memory = false;
    }
    else{
        music.memory = true;
        std::cout << "READING MP3 MUSIC TO MEMORY!!!\n";
        if(fread(music.fileBuffer, sizeof(uint8_t), lSize, file) != lSize){
            std::cout << "CANT READ FILE " << filename << std::endl;
            free(music.fileBuffer);
            fclose(file);
            music.memory = false;
            return music;
        }
        fclose(file);
    }
    
    skipMemory:

    

    
    if(music.memory){
        if(mp3dec_ex_open_buf(music.decoder, music.fileBuffer, music.fileSize, MP3D_SEEK_TO_SAMPLE) != 0){
            std::cout << "Failed decoding music: " << filename << std::endl;
            free(music.decoder);
            linearFree(music.fileBuffer);
            music.memory = false;
            return music;
        }
    }
    else{
        if(mp3dec_ex_open(music.decoder, filename, MP3D_SEEK_TO_SAMPLE) != 0){
            std::cout << "Failed decoding music: " << filename << std::endl;
            free(music.decoder);
            return music;
        }
    }
    mp3dec_ex_seek(music.decoder, 0);
    music.length = ((music.decoder->samples * 1000) / music.decoder->info.hz) / music.decoder->info.channels;
    //std::cout << music.length << std::endl;

    std::cout << "Loaded music: " << filename << std::endl;
    music.sampleRate = music.decoder->info.hz;
    music.channels = music.decoder->info.channels;
    music.loaded = true;
    Global.MusicLoaded = true;

    int channel = 0;
	int bufferIndex = 0;
	int sampleRate = music.sampleRate;
	int bufferSamples = music.sampleRate / 4;
	int bufferSize = bufferSamples * music.channels * sizeof(mp3d_sample_t);

    int numberOfBuffers = 2;

    //std::cout << bufferSize << std::endl;
    memset(music.waveBuf, 0, sizeof(music.waveBuf));
	for (int i = 0; i < numberOfBuffers; i++) {
		music.waveBuf[i].data_vaddr = (s16*)linearAlloc(bufferSize);
        music.waveBuf[i].status = NDSP_WBUF_FREE;
	}

	ndspChnReset(channel);
    ndspChnWaveBufClear(channel);
	ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
	ndspChnSetRate(channel, sampleRate);
    if(music.channels == 1)
	    ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
    else
	    ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0;
    mix[1] = 1.0;

    ndspChnSetMix(channel, mix);
    ndspSetMasterVol(1.0f);

    for(int i = 0; i < numberOfBuffers; i++){
        if (music.waveBuf[i].status == NDSP_WBUF_FREE) {
            music.waveBuf[i].nsamples = mp3dec_ex_read(music.decoder, (mp3d_sample_t*)music.waveBuf[i].data_vaddr, bufferSamples * music.channels) / music.channels;
            if(music.waveBuf[i].nsamples > 0){
                //std::cout << "preloaded buffer: " << i << std::endl;
                DSP_FlushDataCache(music.waveBuf[i].data_vaddr, bufferSize);
            }
            else{
                break;
            }
        }
    }
    music.playing = false;
    music.paused = true;
    //music.thread = threadCreate((void (*)(void*))MusicThread, &music, 16*1024, 0x27, -1, false);
    
    //SleepInMs(2000);
    //SeekMusicStream(&music, 20.0f);
    //std::cout << "Seeked music!" << std::endl;
    //SleepInMs(10000);
    //UnloadMusicStream(&music);
    return music;
}

void UnloadSound(Sound *sound){
    if(sound->loaded){
        for(int i = 1; i < 24; i++){
            ndspChnReset(i);
            ndspChnWaveBufClear(i);
        }
        linearFree((void *)sound->waveBuf.data_vaddr);
        Global.totalNumOfSamples -= sound->sampleSize;
        sound->loaded = false;
        sound->waveBuf.nsamples = 0;
        sound->channels = 0;
        sound->sampleRate = 0;
        sound->sampleSize = 0;
        std::cout << "free sounds in dsp\n";
    }
}

void UnloadMusicStream(Music *music){
    if(Global.MusicLoaded && music->loaded){
        int numberOfBuffers = 2;
        Global.MusicLoaded = false;
        u8 newCommand = 0x0000000F;
        music->command = newCommand;
        std::cout << "music kill command sent\n";
        if(music->playing){
            threadJoin(music->thread, U64_MAX);
            threadFree(music->thread);
            std::cout << "music thread joined\n";
        }
        for(int i = 0; i < 24; i++){
            ndspChnReset(i);
            ndspChnWaveBufClear(i);
        }
        music->loaded = false;
        music->playing = false;
        mp3dec_ex_close(music->decoder);
        free(music->decoder);
        for (int i = 0; i < numberOfBuffers; i++)
		    linearFree((void*)music->waveBuf[i].data_vaddr);
        if(music->memory){
            linearFree(music->fileBuffer);
            music->memory = false;
        }
        return;
    }

}

Sound LoadSound(const char *fileName){
    Sound sound = decodeSoundFromFile(fileName);
    //std::cout << &sound << std::endl;
    //PlaySound(&sound);
    //SleepInMs(1000);
    return sound;
}

bool IsSoundReady(Sound *sound){
    return sound->loaded;
}
void InitAudioDevice(){
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspSetClippingMode(NDSP_CLIP_SOFT);
    ndspSetMasterVol(1.0f);
    Global.soundAtChannel.clear();
    Global.soundAtChannel.reserve(24);
    for(int i = 0; i < 24; i++){
        Global.channelOccupied[i] = false;
        Global.soundAtChannel.push_back(NULL);
    }
}

void SetAudioStreamBufferSizeDefault(int size){}