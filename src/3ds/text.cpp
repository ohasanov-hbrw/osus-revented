#include "text.h"
#include "rlgl.h"
#include "string.h"
#include "utils.hpp"

static Font defaultFont = { 0 };

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "raymath.h"

Font GetFontDefault()
{
    return defaultFont;
}
extern void LoadFontDefault(void)
{
    #define BIT_CHECK(a,b) ((a) & (1u << (b)))

    // NOTE: Using UTF-8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // Ref: http://www.utf8-chartable.de/unicode-utf8-table.pl

    defaultFont.glyphCount = 224;   // Number of chars included in our default font
    defaultFont.glyphPadding = 0;   // Characters padding

    // Default font is directly defined here (data generated from a sprite font image)
    // This way, we reconstruct Font without creating large global variables
    // This data is automatically allocated to Stack and automatically deallocated at the end of this function
    unsigned int defaultFontData[512] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200020, 0x0001b000, 0x00000000, 0x00000000, 0x8ef92520, 0x00020a00, 0x7dbe8000, 0x1f7df45f,
        0x4a2bf2a0, 0x0852091e, 0x41224000, 0x10041450, 0x2e292020, 0x08220812, 0x41222000, 0x10041450, 0x10f92020, 0x3efa084c, 0x7d22103c, 0x107df7de,
        0xe8a12020, 0x08220832, 0x05220800, 0x10450410, 0xa4a3f000, 0x08520832, 0x05220400, 0x10450410, 0xe2f92020, 0x0002085e, 0x7d3e0281, 0x107df41f,
        0x00200000, 0x8001b000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000fbe, 0xfbf7e00f, 0x5fbf7e7d, 0x0050bee8, 0x440808a2, 0x0a142fe8, 0x50810285, 0x0050a048,
        0x49e428a2, 0x0a142828, 0x40810284, 0x0048a048, 0x10020fbe, 0x09f7ebaf, 0xd89f3e84, 0x0047a04f, 0x09e48822, 0x0a142aa1, 0x50810284, 0x0048a048,
        0x04082822, 0x0a142fa0, 0x50810285, 0x0050a248, 0x00008fbe, 0xfbf42021, 0x5f817e7d, 0x07d09ce8, 0x00008000, 0x00000fe0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000c0180,
        0xdfbf4282, 0x0bfbf7ef, 0x42850505, 0x004804bf, 0x50a142c6, 0x08401428, 0x42852505, 0x00a808a0, 0x50a146aa, 0x08401428, 0x42852505, 0x00081090,
        0x5fa14a92, 0x0843f7e8, 0x7e792505, 0x00082088, 0x40a15282, 0x08420128, 0x40852489, 0x00084084, 0x40a16282, 0x0842022a, 0x40852451, 0x00088082,
        0xc0bf4282, 0xf843f42f, 0x7e85fc21, 0x3e0900bf, 0x00000000, 0x00000004, 0x00000000, 0x000c0180, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000402, 0x41482000, 0x00000000, 0x00000800,
        0x04000404, 0x4100203c, 0x00000000, 0x00000800, 0xf7df7df0, 0x514bef85, 0xbefbefbe, 0x04513bef, 0x14414500, 0x494a2885, 0xa28a28aa, 0x04510820,
        0xf44145f0, 0x474a289d, 0xa28a28aa, 0x04510be0, 0x14414510, 0x494a2884, 0xa28a28aa, 0x02910a00, 0xf7df7df0, 0xd14a2f85, 0xbefbe8aa, 0x011f7be0,
        0x00000000, 0x00400804, 0x20080000, 0x00000000, 0x00000000, 0x00600f84, 0x20080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000f01, 0x00000000, 0x06000000, 0x24000000, 0x00000f01, 0x00000000, 0x09108000,
        0x24fa28a2, 0x00000f01, 0x00000000, 0x013e0000, 0x2242252a, 0x00000f52, 0x00000000, 0x038a8000, 0x2422222a, 0x00000f29, 0x00000000, 0x010a8000,
        0x2412252a, 0x00000f01, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000f01, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
        0x0003e000, 0x00000f00, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
        0x02bea080, 0xf0000020, 0xc452208a, 0x04202b02, 0xf8029122, 0x07f0003b, 0xe44b388e, 0x02203a02, 0x081e8a1c, 0x0411e92a, 0xf4420be0, 0x01248202,
        0xe8140414, 0x05d104ba, 0xe7c3b880, 0x00893a0a, 0x283c0e1c, 0x04500902, 0xc4400080, 0x00448002, 0xe8208422, 0x04500002, 0x80400000, 0x05200002,
        0x083e8e00, 0x04100002, 0x804003e0, 0x07000042, 0xf8008400, 0x07f00003, 0x80400000, 0x04000022, 0x00000000, 0x00000000, 0x80400000, 0x04000002,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800702, 0x1848a0c2, 0x84010000, 0x02920921, 0x01042642, 0x00005121, 0x42023f7f, 0x00291002,
        0xefc01422, 0x7efdfbf7, 0xefdfa109, 0x03bbbbf7, 0x28440f12, 0x42850a14, 0x20408109, 0x01111010, 0x28440408, 0x42850a14, 0x2040817f, 0x01111010,
        0xefc78204, 0x7efdfbf7, 0xe7cf8109, 0x011111f3, 0x2850a932, 0x42850a14, 0x2040a109, 0x01111010, 0x2850b840, 0x42850a14, 0xefdfbf79, 0x03bbbbf7,
        0x001fa020, 0x00000000, 0x00001000, 0x00000000, 0x00002070, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x08022800, 0x00012283, 0x02430802, 0x01010001, 0x8404147c, 0x20000144, 0x80048404, 0x00823f08, 0xdfbf4284, 0x7e03f7ef, 0x142850a1, 0x0000210a,
        0x50a14684, 0x528a1428, 0x142850a1, 0x03efa17a, 0x50a14a9e, 0x52521428, 0x142850a1, 0x02081f4a, 0x50a15284, 0x4a221428, 0xf42850a1, 0x03efa14b,
        0x50a16284, 0x4a521428, 0x042850a1, 0x0228a17a, 0xdfbf427c, 0x7e8bf7ef, 0xf7efdfbf, 0x03efbd0b, 0x00000000, 0x04000000, 0x00000000, 0x00000008,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200508, 0x00840400, 0x11458122, 0x00014210,
        0x00514294, 0x51420800, 0x20a22a94, 0x0050a508, 0x00200000, 0x00000000, 0x00050000, 0x08000000, 0xfefbefbe, 0xfbefbefb, 0xfbeb9114, 0x00fbefbe,
        0x20820820, 0x8a28a20a, 0x8a289114, 0x3e8a28a2, 0xfefbefbe, 0xfbefbe0b, 0x8a289114, 0x008a28a2, 0x228a28a2, 0x08208208, 0x8a289114, 0x088a28a2,
        0xfefbefbe, 0xfbefbefb, 0xfa2f9114, 0x00fbefbe, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000, 0x00000020, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00210100, 0x00000004, 0x00000000, 0x00000000, 0x14508200, 0x00001402, 0x00000000, 0x00000000,
        0x00000010, 0x00000020, 0x00000000, 0x00000000, 0xa28a28be, 0x00002228, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000,
        0xa28a28aa, 0x000022a8, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000, 0xbefbefbe, 0x00003e2f, 0x00000000, 0x00000000,
        0x00000004, 0x00002028, 0x00000000, 0x00000000, 0x80000000, 0x00003e0f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    int charsHeight = 10;
    int charsDivisor = 1;    // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically

    int charsWidth[224] = { 3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
                            7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
                            2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
                            6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
                            5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5 };

    // Re-construct image from defaultFontData and generate OpenGL texture
    //----------------------------------------------------------------------
    Image imFont = {
        .data = RL_CALLOC(128*128, 2),  // 2 bytes per pixel (gray + alpha)
        .width = 128,
        .height = 128,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA
    };

    // Fill image.data with defaultFontData (convert from bit to pixel!)
    for (int i = 0, counter = 0; i < imFont.width*imFont.height; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            if (BIT_CHECK(defaultFontData[counter], j))
            {
                // NOTE: We are unreferencing data as short, so,
                // we must consider data as little-endian order (alpha + gray)
                ((unsigned short *)imFont.data)[i + j] = 0xffff;
            }
            else ((unsigned short *)imFont.data)[i + j] = 0x00ff;
        }

        counter++;
    }

    defaultFont.texture = LoadTextureFromImage(&imFont);
    SetTextureFilter(&defaultFont.texture, RL_TEXTURE_FILTER_POINT);
    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, glyphCount
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory must be freed at end! --> Done by CloseWindow()
    defaultFont.glyphs = (GlyphInfo *)RL_MALLOC(defaultFont.glyphCount*sizeof(GlyphInfo));
    defaultFont.recs = (Rectangle *)RL_MALLOC(defaultFont.glyphCount*sizeof(Rectangle));

    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;

    for (int i = 0; i < defaultFont.glyphCount; i++)
    {
        defaultFont.glyphs[i].value = 32 + i;  // First char is 32

        defaultFont.recs[i].x = (float)currentPosX;
        defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        defaultFont.recs[i].width = (float)charsWidth[i];
        defaultFont.recs[i].height = (float)charsHeight;

        testPosX += (int)(defaultFont.recs[i].width + (float)charsDivisor);

        if (testPosX >= defaultFont.texture.width)
        {
            currentLine++;
            currentPosX = 2*charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            defaultFont.recs[i].x = (float)charsDivisor;
            defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        }
        else currentPosX = testPosX;

        // NOTE: On default font character offsets and xAdvance are not required
        defaultFont.glyphs[i].offsetX = 0;
        defaultFont.glyphs[i].offsetY = 0;
        defaultFont.glyphs[i].advanceX = 0;

        // Fill character image data from fontClear data
        defaultFont.glyphs[i].image = ImageFromImage(&imFont, defaultFont.recs[i]);
    }

    UnloadImage(&imFont);

    defaultFont.baseSize = (int)defaultFont.recs[0].height;
}

// Unload raylib default font
extern void UnloadFontDefault(void)
{
    for (int i = 0; i < defaultFont.glyphCount; i++) UnloadImage(&defaultFont.glyphs[i].image);
    UnloadTexture(&defaultFont.texture);
    RL_FREE(defaultFont.glyphs);
    RL_FREE(defaultFont.recs);
}

const char *TextFormat(const char *text, ...){
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    int requiredByteCount = vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    // If requiredByteCount is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occured
    if (requiredByteCount >= MAX_TEXT_BUFFER_LENGTH)
    {
        // Inserting "..." at the end of the string to mark as truncated
        char *truncBuffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
        sprintf(truncBuffer, "...");
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type){
    GlyphInfo *chars = NULL;
    if (fileData != NULL){
        bool genFontChars = false;
        stbtt_fontinfo fontInfo = { 0 };

        if (stbtt_InitFont(&fontInfo, (unsigned char *)fileData, 0)){
            float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);
            int ascent, descent, lineGap;
            stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
            codepointCount = (codepointCount > 0)? codepointCount : 95;
            if (codepoints == NULL){
                codepoints = (int *)malloc(codepointCount*sizeof(int));
                for (int i = 0; i < codepointCount; i++) codepoints[i] = i + 32;
                genFontChars = true;
            }
            chars = (GlyphInfo *)calloc(codepointCount, sizeof(GlyphInfo));

            for (int i = 0; i < codepointCount; i++){
                int chw = 0, chh = 0;   // Character width and height (on generation)
                int ch = codepoints[i];  // Character value to get info for
                chars[i].value = ch;
                int index = stbtt_FindGlyphIndex(&fontInfo, ch);
                if (index > 0){
                    switch (type){
                        case FONT_DEFAULT:
                        case FONT_BITMAP: chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY); break;
                        case FONT_SDF: if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, FONT_SDF_CHAR_PADDING, FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY); break;
                        default: break;
                    }

                    if (chars[i].image.data != NULL){
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                        // Load characters images
                        chars[i].image.width = chw;
                        chars[i].image.height = chh;
                        chars[i].image.mipmaps = 1;
                        chars[i].image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
                        chars[i].offsetY += (int)((float)ascent*scaleFactor);
                    }
                    if (ch == 32)
                    {
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                        Image imSpace = {
                            .data = calloc(chars[i].advanceX*fontSize, 2),
                            .width = chars[i].advanceX,
                            .height = fontSize,
                            .mipmaps = 1,
                            .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
                        };

                        chars[i].image = imSpace;
                    }

                    if (type == FONT_BITMAP){
                        for (int p = 0; p < chw*chh; p++){
                            if (((unsigned char *)chars[i].image.data)[p] < FONT_BITMAP_ALPHA_THRESHOLD) ((unsigned char *)chars[i].image.data)[p] = 0;
                            else ((unsigned char *)chars[i].image.data)[p] = 255;
                        }
                    }
                }
            }
        }
        if(genFontChars)
            free(codepoints);
    }
    return chars;
}

Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount){
    Font font = { 0 };

    char fileExtLower[16] = { 0 };
    strncpy(fileExtLower, TextToLower(fileType), 16 - 1);

    font.baseSize = fontSize;
    font.glyphCount = (codepointCount > 0)? codepointCount : 95;
    font.glyphPadding = 0;
    if (TextIsEqual(fileExtLower, ".ttf") || TextIsEqual(fileExtLower, ".otf")){
        font.glyphs = LoadFontData(fileData, dataSize, font.baseSize, codepoints, font.glyphCount, FONT_DEFAULT);
    }
    if (font.glyphs != NULL)
    {
        font.glyphPadding = FONT_TTF_DEFAULT_CHARS_PADDING;

        Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 0);
        font.texture = LoadTextureFromImage(&atlas);
        SetTextureFilter(&font.texture, RL_TEXTURE_FILTER_POINT);

        // Update glyphs[i].image to use alpha, required to be used on ImageDrawText()
        for (int i = 0; i < font.glyphCount; i++)
        {
            UnloadImage(&font.glyphs[i].image);
            font.glyphs[i].image = ImageFromImage(&atlas, font.recs[i]);
        }

        UnloadImage(&atlas);
    }
    return font;
}

Font LoadFont(const char *fileName){
    Font font = { 0 };
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);
    if (fileData != NULL){
        font = LoadFontFromMemory(GetFileExtension(fileName), fileData, dataSize, FONT_TTF_DEFAULT_SIZE, NULL, FONT_TTF_DEFAULT_NUMCHARS);
        UnloadFileData(fileData);
    }
    return font;
}

Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod){
    Image atlas = { 0 };
    if (glyphs == NULL){
        return atlas;
    }
    *glyphRecs = NULL;
    glyphCount = (glyphCount > 0)? glyphCount : 95;
    Rectangle *recs = (Rectangle *)malloc(glyphCount*sizeof(Rectangle));
    int totalWidth = 0;
    int maxGlyphWidth = 0;
    for (int i = 0; i < glyphCount; i++){
        if (glyphs[i].image.width > maxGlyphWidth) maxGlyphWidth = glyphs[i].image.width;
        totalWidth += glyphs[i].image.width + 2*padding;
    }
    int paddedFontSize = fontSize + 2*padding;
    // No need for a so-conservative atlas generation
    float totalArea = totalWidth*paddedFontSize*1.2f;
    float imageMinSize = sqrtf(totalArea);
    int imageSize = (int)powf(2, ceilf(logf(imageMinSize)/logf(2)));

    if (totalArea < ((imageSize*imageSize)/2)){
        atlas.width = imageSize;    // Atlas bitmap width
        atlas.height = imageSize/2; // Atlas bitmap height
    }
    else{
        atlas.width = imageSize;   // Atlas bitmap width
        atlas.height = imageSize;  // Atlas bitmap height
    }

    atlas.data = (unsigned char *)calloc(1, atlas.width*atlas.height);   // Create a bitmap to store characters (8 bpp)
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;

    if (packMethod == 0){
        int offsetX = padding;
        int offsetY = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < glyphCount; i++){
            // Check remaining space for glyph
            if (offsetX >= (atlas.width - glyphs[i].image.width - 2*padding)){
                offsetX = padding;
                offsetY += (fontSize + 2*padding);

                if (offsetY > (atlas.height - fontSize - padding)){
                    for(int j = i + 1; j < glyphCount; j++){
                        recs[j].x = 0;
                        recs[j].y = 0;
                        recs[j].width = 0;
                        recs[j].height = 0;
                    }
                    break;
                }
            }

            // Copy pixel data from glyph image to atlas
            for (int y = 0; y < glyphs[i].image.height; y++){
                for (int x = 0; x < glyphs[i].image.width; x++){
                    ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                }
            }
            recs[i].x = (float)offsetX;
            recs[i].y = (float)offsetY;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;
            offsetX += (glyphs[i].image.width + 2*padding);
        }
    }
    else if (packMethod == 1){
        stbrp_context *context = (stbrp_context *)malloc(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)malloc(glyphCount*sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, glyphCount);
        stbrp_rect *rects = (stbrp_rect *)malloc(glyphCount*sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < glyphCount; i++){
            rects[i].id = i;
            rects[i].w = glyphs[i].image.width + 2*padding;
            rects[i].h = glyphs[i].image.height + 2*padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, glyphCount);

        for (int i = 0; i < glyphCount; i++){
            // It returns char rectangles in atlas
            recs[i].x = rects[i].x + (float)padding;
            recs[i].y = rects[i].y + (float)padding;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;

            if (rects[i].was_packed){
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < glyphs[i].image.height; y++){
                    for (int x = 0; x < glyphs[i].image.width; x++)
                    {
                        ((unsigned char *)atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                    }
                }
            }
        }

        free(rects);
        free(nodes);
        free(context);
    }
    unsigned char *dataGrayAlpha = (unsigned char *)malloc(atlas.width*atlas.height*sizeof(unsigned char)*2); // Two channels

    for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2){
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
    }

    free(atlas.data);
    atlas.data = dataGrayAlpha;
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;

    *glyphRecs = recs;

    return atlas;
}

const char *TextToLower(const char *text){
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);
    if (text != NULL){
        for (int i = 0; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[i] != '\0'); i++){
            if ((text[i] >= 'A') && (text[i] <= 'Z')) buffer[i] = text[i] + 32;
            else buffer[i] = text[i];
        }
    }

    return buffer;
}

unsigned int TextLength(const char *text){
    unsigned int length = 0;

    if (text != NULL){
        // NOTE: Alternative: use strlen(text)

        while (*text++) length++;
    }

    return length;
}

int GetCodepointNext(const char *text, int *codepointSize){
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    *codepointSize = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0])){
        // 4 byte UTF-8 codepoint
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0])){
        // 3 byte UTF-8 codepoint */
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0])){
        // 2 byte UTF-8 codepoint
        if((ptr[1] & 0xC0) ^ 0x80) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    }
    else if (0x00 == (0x80 & ptr[0])){
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

int GetGlyphIndex(Font *font, int codepoint){
    //No support for unordered glyphs yet
    int index = 0;
    index = codepoint - 32;
    return index;
}

void DrawTextCodepoint(Font *font, int codepoint, Vector2 position, float fontSize, Color tint){
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize/font->baseSize;     // Character quad scaling factor

    // Character destination rectangle on screen
    // NOTE: We consider glyphPadding on drawing
    Rectangle dstRec = { position.x + font->glyphs[index].offsetX*scaleFactor - (float)font->glyphPadding*scaleFactor,
                      position.y + font->glyphs[index].offsetY*scaleFactor - (float)font->glyphPadding*scaleFactor,
                      (font->recs[index].width + 2.0f*font->glyphPadding)*scaleFactor,
                      (font->recs[index].height + 2.0f*font->glyphPadding)*scaleFactor };

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font->recs[index].x - (float)font->glyphPadding, font->recs[index].y - (float)font->glyphPadding,
                         font->recs[index].width + 2.0f*font->glyphPadding, font->recs[index].height + 2.0f*font->glyphPadding };

    // Draw the character texture on the screen
    DrawTexturePro(&font->texture, srcRec, dstRec, (Vector2){ 0, 0 }, 0.0f, tint);
    
}

void DrawTextEx(Font *font, const char *text, Vector2 position, float fontSize, float spacing, Color tint){
    if (font->texture.id == 0) return;

    int size = TextLength(text);    // Total size in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font->baseSize;         // Character quad scaling factor

    for (int i = 0; i < size;){
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n'){
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            int textLineSpacing = 15;
            textOffsetY += textLineSpacing;
            textOffsetX = 0.0f;
        }
        else{
            if ((codepoint != ' ') && (codepoint != '\t')){
                DrawTextCodepoint(font, codepoint, (Vector2){ position.x + textOffsetX, position.y + textOffsetY }, fontSize, tint);
            }
            if (font->glyphs[index].advanceX == 0)
                textOffsetX += ((float)font->recs[index].width*scaleFactor + spacing);
            else
                textOffsetX += ((float)font->glyphs[index].advanceX*scaleFactor + spacing);
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

Vector2 MeasureTextEx(Font *font, const char *text, float fontSize, float spacing)
{
    Vector2 textSize = { 0 };

    if ((font->texture.id == 0) || (text == NULL)) return textSize;

    int size = TextLength(text);    // Get size in bytes of text
    int tempByteCounter = 0;        // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize/(float)font->baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < size;)
    {
        byteCounter++;

        int next = 0;
        letter = GetCodepointNext(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        i += next;

        if (letter != '\n')
        {
            if (font->glyphs[index].advanceX != 0) textWidth += font->glyphs[index].advanceX;
            else textWidth += (font->recs[index].width + font->glyphs[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;

            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            int textLineSpacing = 15;
            textHeight += (float)textLineSpacing;
        }

        if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    textSize.x = tempTextWidth*scaleFactor + (float)((tempByteCounter - 1)*spacing);
    textSize.y = textHeight;

    return textSize;
}

void UnloadFontData(GlyphInfo *glyphs, int glyphCount){
    if (glyphs != NULL){
        for (int i = 0; i < glyphCount; i++) UnloadImage(&glyphs[i].image);
        free(glyphs);
    }
}

void UnloadFont(Font *font){
    UnloadFontData(font->glyphs, font->glyphCount);
    UnloadTexture(&font->texture);
    free(font->recs);
}

void DrawTextPro(Font *font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint){
    //not good but meh
    DrawTextEx(font, text, (Vector2){ position.x - origin.x, position.y - origin.y}, fontSize, spacing, tint);
}