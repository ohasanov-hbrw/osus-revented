#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylibDefinitions.h"
#include <iostream>

//SOUND NEEDS TO BE IMPLEMENTED. IM ONLY IMPLEMENTING GPU RN

void SetSoundVolume(Sound *sound, float volume);
void SetSoundPan(Sound *sound, float pan);
void PlaySound(Sound *sound);
void PlayMusicStream(Music *music);
void SetMusicVolume(Music *music, float volume);
void SeekMusicStream(Music *music, float time);
void UpdateMusicStream(Music *music);
float GetMusicTimePlayed(Music *music);
float GetMusicTimeLength(Music *music);
void StopMusicStream(Music *music);
bool IsMusicStreamPlaying(Music *music);
Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize);
void UnloadSound(Sound *sound);
void UnloadMusicStream(Music *music);
Sound LoadSound(const char *fileName);
bool IsSoundReady(Sound *sound);
void InitAudioDevice();
void SetAudioStreamBufferSizeDefault(int size);
Music LoadMusicStream(const char *filename);
void MusicThread(Music *music);