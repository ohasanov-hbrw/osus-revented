#pragma once

#include <3ds.h>
#include "vshader_shbin.h"
#include "clear_shbin.h"

#include "raylibDefinitions.h"
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tex3ds.h>
#include <png.h>
#include "stb_image.h"


#include "hashtable.h"






//______________________RAYLIB___________________________



// GL blending factors
#define RL_ZERO                                 GPU_ZERO            // GL_ZERO
#define RL_ONE                                  GPU_ONE            // GL_ONE
#define RL_SRC_COLOR                            GPU_SRC_COLOR       // GL_SRC_COLOR
#define RL_ONE_MINUS_SRC_COLOR                  GPU_ONE_MINUS_SRC_COLOR       // GL_ONE_MINUS_SRC_COLOR
#define RL_SRC_ALPHA                            GPU_SRC_ALPHA       // GL_SRC_ALPHA
#define RL_ONE_MINUS_SRC_ALPHA                  GPU_ONE_MINUS_SRC_ALPHA       // GL_ONE_MINUS_SRC_ALPHA
#define RL_DST_ALPHA                            GPU_DST_ALPHA       // GL_DST_ALPHA
#define RL_ONE_MINUS_DST_ALPHA                  GPU_ONE_MINUS_DST_ALPHA       // GL_ONE_MINUS_DST_ALPHA
#define RL_DST_COLOR                            GPU_DST_COLOR       // GL_DST_COLOR
#define RL_ONE_MINUS_DST_COLOR                  GPU_ONE_MINUS_DST_COLOR       // GL_ONE_MINUS_DST_COLOR
#define RL_SRC_ALPHA_SATURATE                   GPU_SRC_ALPHA_SATURATE       // GL_SRC_ALPHA_SATURATE
#define RL_CONSTANT_COLOR                       GPU_CONSTANT_COLOR       // GL_CONSTANT_COLOR
#define RL_ONE_MINUS_CONSTANT_COLOR             GPU_ONE_MINUS_CONSTANT_COLOR       // GL_ONE_MINUS_CONSTANT_COLOR
#define RL_CONSTANT_ALPHA                       GPU_CONSTANT_ALPHA       // GL_CONSTANT_ALPHA
#define RL_ONE_MINUS_CONSTANT_ALPHA             GPU_SRC_ALPHA_SATURATE       // GL_ONE_MINUS_CONSTANT_ALPHA

// GL blending functions/equations THEY ARE MISSING
#define RL_MIN                                  GPU_BLEND_MIN       // GL_MIN
#define RL_MAX                                  GPU_BLEND_MAX       // GL_MAX
#define RL_BLEND_ADD_COLORS                     GPU_BLEND_ADD 
#define RL_BLEND_SUBTRACT_COLORS                GPU_BLEND_SUBTRACT 

#define RL_BLEND_CUSTOM_SEPARATE 0

typedef enum {
    RL_TEXTURE_FILTER_POINT = 0,        // No filter, just pixel approximation
    RL_TEXTURE_FILTER_BILINEAR,         // Linear filtering
    RL_TEXTURE_FILTER_TRILINEAR,        // Trilinear filtering (linear with mipmaps)
    RL_TEXTURE_FILTER_ANISOTROPIC_4X,   // Anisotropic filtering 4x
    RL_TEXTURE_FILTER_ANISOTROPIC_8X,   // Anisotropic filtering 8x
    RL_TEXTURE_FILTER_ANISOTROPIC_16X,  // Anisotropic filtering 16x
} rlTextureFilter;





u32 GetNextPowerOf2(u32 v);
int GetPixelDataSize(int width, int height, int format);
GPU_TEXCOLOR raylibPixel2citroPixel(int format);

void DrawCircle(float x, float y, float radius, Color color);

void DrawLineEx(Vector2 start, Vector2 stop, float size, Color color);

Texture2D LoadTextureFromImage(Image *image, bool vram = true);
void UnloadImage(Image *image);
Image ImageFromImage(Image *image, Rectangle rec);

Texture2D LoadTexture(const char *fileName);
Image LoadImage(const char *fileName);


unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount);

float GetScreenWidth();
float GetScreenHeight();

void DrawTexturePro(Texture2D *texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);

Image LoadImageFromMemory(const char *fileType, const unsigned char *fileData, int dataSize);

void ImageColorReplace(Image *image, Color color, Color replace);
Color *LoadImageColors(Image *image);
void ImageFormat(Image *image, int newFormat);   
static Vector4 *LoadImageDataNormalized(Image *image);

void SetTextureFilter(Texture2D *texture, int Filter);

void ImageColorTint(Image *image, Color color);  

void UnloadTexture(Texture2D *texture);

void ImageBlurGaussian(Image *image, int blurSize);
void ImageAlphaPremultiply(Image *image);

void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);

Color Fade(Color color, float alpha);

RenderTexture2D LoadRenderTexture(int width, int height, bool vram = true);

void BeginTextureMode(RenderTexture2D *renderTexture);

void ClearBackground(Color color);
void rlClearScreenBuffers();
void rlEnableDepthMask();

void EndTextureMode();

void rlEnableDepthTest();
void rlEnableDepthTest2();

void DrawCircleWithDepth(Vector2 center, float radius, int segments, float depth, Color color);
void DrawCircleWithDepthGrad(Vector2 center, float radius, int segments, float depth, Color color, Color color2);

void rlSetBlendMode(int mode);
void rlSetBlendFactorsSeparate(GPU_BLENDFACTOR glSrcRGB, GPU_BLENDFACTOR glDstRGB, GPU_BLENDFACTOR glSrcAlpha, GPU_BLENDFACTOR glDstAlpha, GPU_BLENDEQUATION glEqRGB, GPU_BLENDEQUATION glEqAlpha);

void rlDisableDepthTest();

void EndBlendMode();
void DrawTextureEx(Texture2D *texture, Vector2 position, float rotation, float scale, Color tint);
void DrawTextureExDepth(Texture2D *texture, Vector2 position, float depth, float scale, Color tint);

void DrawTextureRotate(Texture2D *tex, float x, float y, float s, float r, Color tint); //DEFINING HERE, UTILS.CPP :D


//NO SHADER SUPPORT
Shader LoadShader(const char *vsFileName, const char *fsFileName);
int GetShaderLocation(Shader shader, const char *uniformName);
void SetShaderValue(Shader shader, int locIndex, const void *value, int uniformType);
void BeginShaderMode(Shader shader);
void EndShaderMode();

void UnloadRenderTexture(RenderTexture2D *renderTexture);

void DrawRectangle(int posX, int posY, int width, int height, Color color);
void SetConfigFlags(unsigned int flags);
void HideCursor();

int GetFPS();

void ImageResize(Image *image, int newWidth, int newHeight);