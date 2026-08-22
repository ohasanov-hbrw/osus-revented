// msaa_render_texture.h
#pragma once
#if !defined(THREEDS_BUILD) && defined(MSAA_RENDER)
#include "raylib.h"
#include "rlgl.h"

struct MSAARenderTexture {
    unsigned int resolveFbo = 0;
    unsigned int resolveTexture = 0;
    unsigned int resolveDepthRB = 0; // Added for fallback non-MSAA depth rendering
    unsigned int msaaFbo = 0;
    unsigned int msaaColorRB = 0;
    unsigned int msaaDepthRB = 0;
    int width = 0, height = 0;
    int samples = 0;
    bool valid = false;
    bool usingMSAA = false;
};

MSAARenderTexture LoadMSAARenderTexture(int width, int height, int requestedSamples = 4);
void BeginMSAARenderTexture(MSAARenderTexture& target);
void EndMSAARenderTexture(MSAARenderTexture& target);
void UnloadMSAARenderTexture(MSAARenderTexture& target);
#endif