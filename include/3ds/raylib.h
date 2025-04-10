#pragma once
#include "SDL/SDL.h"
#include "raylibDefinitions.h"
#include "rlgl.h" 
#include "audio.h"
#include "input.h"
#include "shapes.h"
#include "text.h"


bool WindowShouldClose();
unsigned char *LoadFileData(const char *fileName, int *dataSize);
void UnloadFileData(unsigned char *data);
bool TextIsEqual(const char *text1, const char *text2);
const char *GetFileExtension(const char *fileName);
bool IsFileExtension(const char *fileName, const char *ext);

Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer);