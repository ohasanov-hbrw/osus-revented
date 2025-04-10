#pragma once
#include "raylibDefinitions.h"

#define MAX_TEXTFORMAT_BUFFERS              4        // Maximum number of static buffers for text formatting
#define MAX_TEXT_BUFFER_LENGTH              1024
#define FONT_TTF_DEFAULT_SIZE               32      // TTF font generation default char size (char-height)
#define FONT_TTF_DEFAULT_NUMCHARS           95      // TTF font generation default charset: 95 glyphs (ASCII 32..126)
#define FONT_TTF_DEFAULT_FIRST_CHAR         32      // TTF font generation default first char for image sprite font (32-Space)
#define FONT_TTF_DEFAULT_CHARS_PADDING      4      // TTF font generation default chars padding
#define FONT_SDF_CHAR_PADDING               4      // SDF font generation char padding
#define FONT_SDF_ON_EDGE_VALUE              128      // SDF font generation on edge value
#define FONT_SDF_PIXEL_DIST_SCALE           64.0f     // SDF font generation pixel distance scale
#define FONT_BITMAP_ALPHA_THRESHOLD         80      // Bitmap (B&W) font generation alpha threshold

const char *TextFormat(const char *text, ...);
const char *TextToLower(const char *text);

Font LoadFont(const char *fileName);
Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount);
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type);
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod);

void DrawTextEx(Font *font, const char *text, Vector2 position, float fontSize, float spacing, Color tint);
unsigned int TextLength(const char *text);
int GetCodepointNext(const char *text, int *codepointSize);
int GetGlyphIndex(Font *font, int codepoint);
Vector2 MeasureTextEx(Font *font, const char *text, float fontSize, float spacing);

void UnloadFontDefault();
void LoadFontDefault();
Font GetFontDefault();

void UnloadFont(Font *font);
void UnloadFontData(GlyphInfo *glyphs, int glyphCount);
void DrawTextPro(Font *font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);