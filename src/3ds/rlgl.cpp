#include "rlgl.h"
#include "globals.hpp"
#include "time_util.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "external/stb_image_resize.h"

u32 GetNextPowerOf2(u32 v){
    u32 result = 1;
    while(true){
        if(result >= v)
            break;
        result *= 2;
    }
    return result;
}

int GetPixelDataSize(int width, int height, int format){
    int dataSize = 0;       // Size in bytes
    int bpp = 32;            // Bits per pixel
    switch (format){
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        default: break;
    }

    dataSize = width*height*bpp/8;  // Total data size in bytes
    return dataSize;
}

GPU_TEXCOLOR raylibPixel2citroPixel(int format){
	switch (format){
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: return GPU_L8;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: return GPU_LA8;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: return GPU_RGB565;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: return GPU_RGBA5551;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: return GPU_RGBA4;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: return GPU_RGBA8;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: return GPU_RGB8;
        default: return GPU_RGBA8;
    }
}

void DrawCircle(float x, float y, float radius, Color color){
	//C2D_DrawCircleSolid(x, y, 0.0f, radius, C2D_Color32(color.r, color.g, color.b, color.a));
    DrawCircleWithDepth((Vector2){x, y}, radius, 16, 0,color);
    C2D_Flush();  //test
	//may need to add a queue for this (3ds magick circle bad) also check depth
}

void DrawLineEx(Vector2 start, Vector2 stop, float size, Color color){
	C2D_DrawLine(start.x, start.y, C2D_Color32(color.r, color.g, color.b, color.a), stop.x, stop.y, C2D_Color32(color.r, color.g, color.b, color.a), size, 0.0f);
	C2D_Flush();  //test
    //check depth...
}

float GetScreenWidth(){
	if(Global.useTopScreen)
		return 400.0f;
	return 320.0f;
}

float GetScreenHeight(){
	return 240.0f;
}


typedef uint32_t (*readFunc) (const void*);
typedef void 	 (*writeFunc)(void*, uint32_t);

static size_t _determineBPP(GPU_TEXCOLOR format){
	switch (format){
		case GPU_RGBA8:  return 4;

		case GPU_RGB8:   return 3;

		case GPU_RGBA5551:
		case GPU_RGB565:
		case GPU_RGBA4:
		case GPU_LA8:    return 2;

		case GPU_A8:
		case GPU_LA4:    return 1;

		default:         return 4;
	}
}

uint32_t _readRGBA8(const void* data){
    //std::cout << *(uint32_t*)data << std::endl;
	return *(uint32_t*)data;
}

uint32_t _readRGB8(const void* data){
	uint8_t  *clr = (uint8_t*)data;

	uint8_t r = clr[0];
	uint8_t g = clr[1];
	uint8_t b = clr[2];
	uint8_t a = 255;

	return (r | (g << 8) | (b << 16) | (a << 24));
}

uint32_t _readLA8(const void* data){

	uint8_t  *clr = (uint8_t*)data;

	uint8_t l = clr[0];
	uint8_t a = clr[1];

	return (l | (l << 8) | (l << 16) | (a << 24));
}

uint32_t _readA8(const void* data){
	uint8_t  *clr = (uint8_t*)data;

	uint8_t l = 255;
	uint8_t a = clr[0];

	return (l | (l << 8) | (l << 16) | (a << 24));
}

void _writeRGBA4(void *data, uint32_t color){
	uint16_t *dst = (uint16_t*)data;
	uint8_t  *clr = (uint8_t*)&color;

	uint8_t r = clr[0] >> 4;
	uint8_t g = clr[1] >> 4;
	uint8_t b = clr[2] >> 4;
	uint8_t a = clr[3] >> 4;

	*dst = (a | (b << 4) | (g << 8) | (r << 12));
}

void _writeRGB565(void *data, uint32_t color){
	uint16_t *dst = (uint16_t*)data;
	uint8_t  *clr = (uint8_t*)&color;

	uint8_t r = clr[0] >> 3;
	uint8_t g = clr[1] >> 2;
	uint8_t b = clr[2] >> 3;

	*dst = (b | (g << 5) | (r << 11));
}

void _writeLA4(void *data, uint32_t color){
	uint8_t *dst = (uint8_t*)data;
	uint8_t *clr = (uint8_t*)&color;

	uint8_t l = clr[0] >> 4;
	uint8_t a = clr[3] >> 4;

	*dst = (a | (l << 4));
}

void _writeLA8(void *data, uint32_t color){
	uint16_t *dst = (uint16_t*)data;
	uint8_t  *clr = (uint8_t*)&color;

	uint8_t l = clr[0];
	uint8_t a = clr[3];

	*dst = (a | (l << 8));
}

void _writeA8(void *data, uint32_t color){
	uint8_t *dst = (uint8_t*)data;
	uint8_t *clr = (uint8_t*)&color;

	*dst = clr[3];
}


//Borrowed from citra
static inline uint32_t _mortonInterleave(uint32_t x, uint32_t y)
{
    static uint32_t xlut[] = {0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15};
    static uint32_t ylut[] = {0x00, 0x02, 0x08, 0x0a, 0x20, 0x22, 0x28, 0x2a};

    return xlut[x % 8] + ylut[y % 8];
}

#define BLOCK_HEIGHT 8

static inline uint32_t _getMortonOffset(uint32_t x, uint32_t y)
{
	uint32_t coarse_x = x & ~7;

	u32 i = _mortonInterleave(x, y);

	uint32_t offset = coarse_x * BLOCK_HEIGHT;

	return (i + offset);
}

static GPU_TEXCOLOR _determineHardwareFormat(int format){
	switch(format){	
		case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: //return GPU_RGBA8;
		case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: return GPU_RGBA4;

		case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
		case PIXELFORMAT_UNCOMPRESSED_R5G6B5:  return GPU_RGB565;

		case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
		case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: return GPU_LA4;

		default: return GPU_RGBA4;
	}
}

static inline readFunc _determineReadFunction(int format, uint8_t *bpp){
	switch (format) {
		case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *bpp = 1; return _readA8;
		case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *bpp = 2; return _readLA8;
		case PIXELFORMAT_UNCOMPRESSED_R8G8B8: *bpp = 3; return _readRGB8;
		case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *bpp = 4; return _readRGBA8;
		default:return NULL;
	}
}

static inline writeFunc _determineWriteFunction(GPU_TEXCOLOR format){
	switch (format) {
		case GPU_RGBA4:  return _writeRGBA4;
		case GPU_RGB565: return _writeRGB565;
		case GPU_LA8:    return _writeLA8;
		case GPU_LA4:    return _writeLA4;
		case GPU_A8:     return _writeA8;
		default:         return NULL;
	}
}

Texture2D LoadTextureFromImage(Image *image, bool vram){
    Texture2D texture = { 0 };
	texture.id = 0;
    if ((image->width != 0) && (image->height != 0)){
        texture.id = 1;

		texture.subtex.width = static_cast<u16>(image->width);
        texture.subtex.height = static_cast<u16>(image->height);

        u32 w_pow2 = GetNextPowerOf2(texture.subtex.width);
		u32 h_pow2 = GetNextPowerOf2(texture.subtex.height);
        //std::cout << "t w " << w_pow2 << " h " << h_pow2 << std::endl;

		texture.subtex.left = 0.f;
		texture.subtex.top = 1.f;
		texture.subtex.right = (static_cast<float>(image->width) /static_cast<float>(w_pow2));
		texture.subtex.bottom = (1.0 - (static_cast<float>(image->height) / static_cast<float>(h_pow2)));
        
        //std::cout << "s w " << texture.subtex.width << " h " << texture.subtex.height << std::endl;


        //std::cout << "TEXTURE INIT IN GPU VRAM\n";
        //SleepInMs(200);

        vram = false;

		if(vram)
            texture.id = C3D_TexInitVRAM(&texture.tex, static_cast<u16>(w_pow2), static_cast<u16>(h_pow2), _determineHardwareFormat(image->format));
        else
            texture.id = C3D_TexInit(&texture.tex, static_cast<u16>(w_pow2), static_cast<u16>(h_pow2), _determineHardwareFormat(image->format));

        //std::cout << "TEXTURE INIT IN GPU DONE\n";
        //SleepInMs(200);
		//memset(texture.tex.data, 0, texture.tex.size);
        //GSPGPU_FlushDataCache(data, texture.tex.width * texture.tex.height * GetPixelDataSize(1, 1, image.format));
        //std::cout << "MEMSET DONE\n";
        //C3D_TexFlush(&texture.tex);
        //std::cout << _determineBPP(_determineHardwareFormat(image->format)) << " Bpp - " << texture.tex.size << " bytes allocated" << std::endl;
        //SleepInMs(100);
        //C3D_TexLoadImage(&texture.tex, image.data, GPU_TEXFACE_2D, 1);

        C3D_TexFlush(&texture.tex);

		/*for (u32 x = 0; x < image.width; x++) {
			for (u32 y = 0; y < image.height; y++) {
				u32 dst_pos = ((((y >> 3) * (w_pow2 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * GetPixelDataSize(1, 1, image.format);
				//u32 dst_pos = (y * w_pow2 + x) * GetPixelDataSize(1, 1, image.format);
                //u32 dst_pos = (y * w_pow2 + x) * GetPixelDataSize(1, 1, image.format);
                //dst_pos = get_morton_offset(x, y, GetPixelDataSize(1, 1, image.format));
                u32 src_pos = (y * image.width + x) * GetPixelDataSize(1, 1, image.format);
				//memcpy(&(static_cast<u8 *>(texture.tex.data))[dst_pos], &(static_cast<u8 *>(image.data))[src_pos], GetPixelDataSize(1, 1, image.format));
                
                //if(y == image.height - 1)
                //    std::cout << texture.tex.size << " " << dst_pos << std::endl;

                if(dst_pos + GetPixelDataSize(1, 1, image.format) - 1 >= w_pow2 * h_pow2 * GetPixelDataSize(1, 1, image.format) or dst_pos + GetPixelDataSize(1, 1, image.format) - 1 >= texture.tex.size){
                    std::cout << "DST ERRER??????\n";
                    SleepInMs(2000);
                }

                if(src_pos + GetPixelDataSize(1, 1, image.format) - 1 >= image.width * image.height * GetPixelDataSize(1, 1, image.format)){
                    std::cout << "SRC ERRER??????\n";
                    SleepInMs(2000);
                }
                if(GetPixelDataSize(1, 1, image.format) == 4){
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+0] = (static_cast<u8 *>(image.data))[src_pos+3];
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+1] = (static_cast<u8 *>(image.data))[src_pos+2];
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+2] = (static_cast<u8 *>(image.data))[src_pos+1];
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+3] = (static_cast<u8 *>(image.data))[src_pos+0];
                }
                else if(GetPixelDataSize(1, 1, image.format) == 3){
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+0] = (static_cast<u8 *>(image.data))[src_pos+2];
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+1] = (static_cast<u8 *>(image.data))[src_pos+1];
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+2] = (static_cast<u8 *>(image.data))[src_pos+0];
                }
                else if(GetPixelDataSize(1, 1, image.format) == 2){
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+0] = (static_cast<u8 *>(image.data))[src_pos+1];
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+1] = (static_cast<u8 *>(image.data))[src_pos+0];
                }
                else{
                    (static_cast<u8 *>(texture.tex.data))[dst_pos+0] = (static_cast<u8 *>(image.data))[src_pos+0];
                }
                C3D_TexFlush(&texture.tex);
			}



		}*/
        
        uint8_t offset_bpp = GetPixelDataSize(1, 1, image->format);
        readFunc readPixel   = _determineReadFunction(image->format, &offset_bpp);
	    writeFunc writePixel = _determineWriteFunction(_determineHardwareFormat(image->format));

        uint32_t texel, offset, output_y, coarse_y;
        void *tiled_output = texture.tex.data;

        if(vram){
            tiled_output = linearAlloc(texture.tex.size + 5);
            if(!tiled_output){
                texture.id = 0;
                return texture;
            }
        }
        //std::cout << "amogus:" << offset_bpp << std::endl;

        for(size_t x = 0; x < texture.tex.size; x++){
            ((uint8_t*)tiled_output)[x] = 0;
        }

        GSPGPU_FlushDataCache(tiled_output, texture.tex.size);

        for(int y = 0; y < image->height; y++){
            output_y = image->height - 1 - (y);
            coarse_y = output_y & ~7;
            for(int x = 0; x < image->width; x++){
                offset = (_getMortonOffset(x, output_y) + coarse_y * texture.tex.width) * _determineBPP(_determineHardwareFormat(image->format));
                texel = readPixel(image->data + (((x + ((image->height - y - 1) * image->width)) * offset_bpp)));
                //texel = 0xFF00FFFF;
                writePixel(tiled_output + offset, texel);
            }
        }

        GSPGPU_FlushDataCache(tiled_output, texture.tex.size);
        if(vram){
            GX_TextureCopy((u32*)tiled_output, 0, (u32*)texture.tex.data, 0, texture.tex.size, 8);
            linearFree(tiled_output);
        }

		//std::cout << "TEXTURE COPY DONE\n";
        //SleepInMs(100);
		
		texture.tex.border = 0x00000000;
        C3D_TexSetWrap(&texture.tex, GPU_MIRRORED_REPEAT , GPU_MIRRORED_REPEAT );
        C3D_TexFlush(&texture.tex);
        //C3D_TexFlush(&texture.tex);
    }

    texture.width = image->width; //texture.subtex.width;
    texture.height = image->height; //texture.subtex.height;
    texture.mipmaps = image->mipmaps;
    texture.format = image->format;
    texture.baseScale = 1;
    //C3D_TexSetFilter(&texture.tex, GPU_LINEAR, GPU_LINEAR);
    C2D_Flush(); 
    return texture;
}

void ImageResize(Image *image, int newWidth, int newHeight){
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Check if we can use a fast path on image scaling
    // It can be for 8 bit per channel images with 1 to 4 channels per pixel
    if ((image->format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8))
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *output = (unsigned char *)malloc(newWidth*newHeight*bytesPerPixel);

        //switch (image->format)
        //{
        //    case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)1); break;
        //    case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)2); break;
        //    case PIXELFORMAT_UNCOMPRESSED_R8G8B8: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)3); break;
        //    case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)4); break;
        //    default: break;
        //}

        switch (image->format)
        {
            case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: stbir__resize_arbitrary(NULL, (unsigned char *)image->data, image->width, image->height, 0,output, newWidth, newHeight, 0,0,0,1,1,NULL,1,-1,0, STBIR_TYPE_UINT8, STBIR_FILTER_MITCHELL, STBIR_FILTER_MITCHELL,STBIR_EDGE_ZERO, STBIR_EDGE_ZERO, STBIR_COLORSPACE_LINEAR); break;
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: stbir__resize_arbitrary(NULL, (unsigned char *)image->data, image->width, image->height, 0,output, newWidth, newHeight, 0,0,0,1,1,NULL,2,-1,0, STBIR_TYPE_UINT8, STBIR_FILTER_MITCHELL, STBIR_FILTER_MITCHELL,STBIR_EDGE_ZERO, STBIR_EDGE_ZERO, STBIR_COLORSPACE_LINEAR); break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8: stbir__resize_arbitrary(NULL, (unsigned char *)image->data, image->width, image->height, 0,output, newWidth, newHeight, 0,0,0,1,1,NULL,3,-1,0, STBIR_TYPE_UINT8, STBIR_FILTER_MITCHELL, STBIR_FILTER_MITCHELL,STBIR_EDGE_ZERO, STBIR_EDGE_ZERO, STBIR_COLORSPACE_LINEAR); break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: stbir__resize_arbitrary(NULL, (unsigned char *)image->data, image->width, image->height, 0,output, newWidth, newHeight, 0,0,0,1,1,NULL,4,-1,0, STBIR_TYPE_UINT8, STBIR_FILTER_MITCHELL, STBIR_FILTER_MITCHELL,STBIR_EDGE_ZERO, STBIR_EDGE_ZERO, STBIR_COLORSPACE_LINEAR); break;
            default: break;
        }

        

        free(image->data);
        image->data = output;
        image->width = newWidth;
        image->height = newHeight;
    }
    else{
        // Get data as Color pixels array to work with it
        Color *pixels = LoadImageColors(image);
        Color *output = (Color *)malloc(newWidth*newHeight*sizeof(Color));

        // NOTE: Color data is cast to (unsigned char *), there shouldn't been any problem...
        //stbir_resize_uint8_linear((unsigned char *)pixels, image->width, image->height, 0, (unsigned char *)output, newWidth, newHeight, 0, (stbir_pixel_layout)4);
        stbir__resize_arbitrary(NULL, (unsigned char *)pixels, image->width, image->height, 0,output, newWidth, newHeight, 0,0,0,1,1,NULL,4,STBIR_FLAG_ALPHA_PREMULTIPLIED,0, STBIR_TYPE_UINT8, STBIR_FILTER_MITCHELL, STBIR_FILTER_MITCHELL,STBIR_EDGE_ZERO, STBIR_EDGE_ZERO, STBIR_COLORSPACE_LINEAR);
        int format = image->format;

        free(pixels);
        free(image->data);

        image->data = output;
        image->width = newWidth;
        image->height = newHeight;
        image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        ImageFormat(image, format);  // Reformat 32bit RGBA image to original format
    }
}

Texture2D LoadTexture(const char *fileName){
    //SleepInMs(200);
    Texture2D texture = { 0 };
    //std::cout << "I " << fileName << std::endl;
    Image image = LoadImage(fileName);
    if(image.width <= 1 || image.height <= 1){
        texture.id = 0;
        UnloadImage(&image);
        return texture;
    }
    //SleepInMs(200);
    int divider = 4;
    while(true){
        if(image.width / divider > 128 or image.height / divider > 128){
            divider += 1;
            std::cout << "TOO BIG OF AN IMAGE!" << std::endl;
            
        }
        else{
            break;
        }
    }
    std::cout << "RESIZING TO" << (int)(image.height / divider) << " x " << (int)(image.width / divider) << std::endl;
    //std::cout << "resize" << std::endl;
    //SleepInMs(200);
    int initialW = (int)image.width;
    int initialH = (int)image.height;
    int resizeW = (int)(image.width / divider);
    int resizeH = (int)(image.height / divider);
    ImageResize(&image, resizeW, resizeH);
    //std::cout << "T " << image.width << " " << image.height << std::endl;
    
    if (image.data != NULL){
        //std::cout << "loadtex" << std::endl;
        //SleepInMs(200);
        texture = LoadTextureFromImage(&image);
        texture.id = 1;
        texture.baseScale = divider;
        texture.width = initialW;
        texture.height = initialH;
        //std::cout << "unloadim" << std::endl;
        //SleepInMs(200);
        UnloadImage(&image);
    }
    //C3D_TexSetFilter(&texture.tex, GPU_LINEAR, GPU_LINEAR);
    //C3D_TexSetWrap(&texture.tex, GPU_MIRRORED_REPEAT, GPU_MIRRORED_REPEAT);
    C2D_Flush();
    return texture;
}

Image LoadImage(const char *fileName){
    Image image = { 0 };
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);
    //std::cout << "ISize:" << dataSize << std::endl;
    // Loading image from memory data
    if (fileData != NULL){
        image = LoadImageFromMemory(GetFileExtension(fileName), fileData, dataSize);
        free(fileData);
    }
    return image;
}

void UnloadImage(Image *image){
	free(image->data);
}

Image ImageFromImage(Image *image, Rectangle rec){
    Image result = { 0 };
    int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
    result.width = (int)rec.width;
    result.height = (int)rec.height;
    result.data = calloc((int)rec.width*(int)rec.height*bytesPerPixel, 1);
    result.format = image->format;
    result.mipmaps = 1;
    for (int y = 0; y < (int)rec.height; y++){
        memcpy(((unsigned char *)result.data) + y*(int)rec.width*bytesPerPixel, ((unsigned char *)image->data) + ((y + (int)rec.y)*image->width + (int)rec.x)*bytesPerPixel, (int)rec.width*bytesPerPixel);
    }
    return result;
}

void DrawTexturePro(Texture2D *texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint){
    // Check if texture is valid
    if (texture->id > 0){
		C2D_Image image;
		image.tex = &texture->tex;
		Tex3DS_SubTexture subTex = texture->subtex;
		

		int x = source.x / texture->baseScale;
		int y = source.y / texture->baseScale;
		int endX = (source.x + source.width) / texture->baseScale;
		int endY = (source.y + source.height) / texture->baseScale;

        if (x != endX){
            int deltaX  = endX - x;
            float texRL = subTex.left - subTex.right;
            subTex.left    = subTex.left - (float)texRL / subTex.width * x;
            subTex.right   = subTex.left - (float)texRL / subTex.width * deltaX;
            subTex.width   = deltaX;
        }
        if (y != endY)
        {
            float texTB = subTex.top - subTex.bottom;
            int deltaY  = endY - y;
            subTex.top     = subTex.top - (float)texTB / subTex.height * y;
            subTex.bottom  = subTex.top - (float)texTB / subTex.height * deltaY;
            subTex.height  = deltaY;
        }


		image.subtex = &subTex;
		
		C2D_ImageTint c2dTint;
		//Maybe needs fixing, idk how blend and alpha work
		c2dTint.corners[0] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
		c2dTint.corners[1] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
		c2dTint.corners[2] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
		c2dTint.corners[3] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};

		float scaleX = (dest.width / source.width) * texture->baseScale;
		float scaleY = (dest.height / source.height) * texture->baseScale;

		//ADD ROTATION :) I CANT BE F*CKIN BOTHERED
        C2D_Prepare();
		C2D_DrawImageAtRotated(image, dest.x - origin.x + (dest.width / 2.0), dest.y - origin.y + (dest.height / 2.0), 0, 0, &c2dTint, scaleX, scaleY);
        C2D_Flush();  //test
    }
}

Image LoadImageFromMemory(const char *fileType, const unsigned char *fileData, int dataSize){
    Image image = { 0 };
    if ((false)
        || (strcmp(fileType, ".png") == 0) || (strcmp(fileType, ".PNG") == 0)
        || (strcmp(fileType, ".bmp") == 0) || (strcmp(fileType, ".BMP") == 0)
        || (strcmp(fileType, ".jpg") == 0) || (strcmp(fileType, ".jpeg") == 0)
        || (strcmp(fileType, ".JPG") == 0) || (strcmp(fileType, ".JPEG") == 0)){
        if (fileData != NULL){
            int comp = 0;
            image.data = stbi_load_from_memory(fileData, dataSize, &image.width, &image.height, &comp, 0);

            if (image.data != NULL){
                image.mipmaps = 1;

                if (comp == 1) image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
                else if (comp == 2) image.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
                else if (comp == 3) image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
                else if (comp == 4) image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
                else image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
            }
        }
    }
    return image;
}

void ImageColorReplace(Image *image, Color color, Color replace){
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Color *pixels = LoadImageColors(image);

    for (int i = 0; i < image->width * image->height; i++){
        if ((pixels[i].r == color.r) &&
            (pixels[i].g == color.g) &&
            (pixels[i].b == color.b) &&
            (pixels[i].a == color.a))
        {
            pixels[i].r = replace.r;
            pixels[i].g = replace.g;
            pixels[i].b = replace.b;
            pixels[i].a = replace.a;
        }
    }

    int format = image->format;
    free(image->data);
    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

Color *LoadImageColors(Image *image){
    if ((image->width == 0) || (image->height == 0)) return NULL;

    Color *pixels = (Color *)malloc(image->width*image->height*sizeof(Color));
	for (int i = 0, k = 0; i < image->width*image->height; i++)
	{
		switch (image->format)
		{
			case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
			{
				pixels[i].r = ((unsigned char *)image->data)[i];
				pixels[i].g = ((unsigned char *)image->data)[i];
				pixels[i].b = ((unsigned char *)image->data)[i];
				pixels[i].a = 255;

			} break;
			case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
			{
				pixels[i].r = ((unsigned char *)image->data)[k];
				pixels[i].g = ((unsigned char *)image->data)[k];
				pixels[i].b = ((unsigned char *)image->data)[k];
				pixels[i].a = ((unsigned char *)image->data)[k + 1];

				k += 2;
			} break;
			case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
			{
				unsigned short pixel = ((unsigned short *)image->data)[i];

				pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
				pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
				pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
				pixels[i].a = (unsigned char)((pixel & 0b0000000000000001)*255);

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
			{
				unsigned short pixel = ((unsigned short *)image->data)[i];

				pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
				pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
				pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
				pixels[i].a = 255;

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
			{
				unsigned short pixel = ((unsigned short *)image->data)[i];

				pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
				pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
				pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
				pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
			{
				pixels[i].r = ((unsigned char *)image->data)[k];
				pixels[i].g = ((unsigned char *)image->data)[k + 1];
				pixels[i].b = ((unsigned char *)image->data)[k + 2];
				pixels[i].a = ((unsigned char *)image->data)[k + 3];

				k += 4;
			} break;
			case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
			{
				pixels[i].r = (unsigned char)((unsigned char *)image->data)[k];
				pixels[i].g = (unsigned char)((unsigned char *)image->data)[k + 1];
				pixels[i].b = (unsigned char)((unsigned char *)image->data)[k + 2];
				pixels[i].a = 255;

				k += 3;
			} break;
			default: break;
		}
	}

    return pixels;
}

void ImageFormat(Image *image, int newFormat){
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((newFormat != 0) && (image->format != newFormat)){
		Vector4 *pixels = LoadImageDataNormalized(image);     // Supports 8 to 32 bit per channel

		free(image->data);      // WARNING! We loose mipmaps data --> Regenerated at the end...
		image->data = NULL;
		image->format = newFormat;

		switch (image->format)
		{
			case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
			{
				image->data = (unsigned char *)malloc(image->width*image->height*sizeof(unsigned char));

				for (int i = 0; i < image->width*image->height; i++)
				{
					((unsigned char *)image->data)[i] = (unsigned char)((pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f)*255.0f);
				}

			} break;
			case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
			{
				image->data = (unsigned char *)malloc(image->width*image->height*2*sizeof(unsigned char));

				for (int i = 0, k = 0; i < image->width*image->height*2; i += 2, k++)
				{
					((unsigned char *)image->data)[i] = (unsigned char)((pixels[k].x*0.299f + (float)pixels[k].y*0.587f + (float)pixels[k].z*0.114f)*255.0f);
					((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].w*255.0f);
				}

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
			{
				image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));

				unsigned char r = 0;
				unsigned char g = 0;
				unsigned char b = 0;

				for (int i = 0; i < image->width*image->height; i++)
				{
					r = (unsigned char)(round(pixels[i].x*31.0f));
					g = (unsigned char)(round(pixels[i].y*63.0f));
					b = (unsigned char)(round(pixels[i].z*31.0f));

					((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
				}

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
			{
				image->data = (unsigned char *)malloc(image->width*image->height*3*sizeof(unsigned char));

				for (int i = 0, k = 0; i < image->width*image->height*3; i += 3, k++)
				{
					((unsigned char *)image->data)[i] = (unsigned char)(pixels[k].x*255.0f);
					((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].y*255.0f);
					((unsigned char *)image->data)[i + 2] = (unsigned char)(pixels[k].z*255.0f);
				}
			} break;
			case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
			{
				image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));

				unsigned char r = 0;
				unsigned char g = 0;
				unsigned char b = 0;
				unsigned char a = 0;

				for (int i = 0; i < image->width*image->height; i++)
				{
					r = (unsigned char)(round(pixels[i].x*31.0f));
					g = (unsigned char)(round(pixels[i].y*31.0f));
					b = (unsigned char)(round(pixels[i].z*31.0f));
					a = (pixels[i].w > ((float)50.0f/255.0f))? 1 : 0;

					((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
				}

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
			{
				image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));

				unsigned char r = 0;
				unsigned char g = 0;
				unsigned char b = 0;
				unsigned char a = 0;

				for (int i = 0; i < image->width*image->height; i++)
				{
					r = (unsigned char)(round(pixels[i].x*15.0f));
					g = (unsigned char)(round(pixels[i].y*15.0f));
					b = (unsigned char)(round(pixels[i].z*15.0f));
					a = (unsigned char)(round(pixels[i].w*15.0f));

					((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;
				}

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
			{
				image->data = (unsigned char *)malloc(image->width*image->height*4*sizeof(unsigned char));

				for (int i = 0, k = 0; i < image->width*image->height*4; i += 4, k++)
				{
					((unsigned char *)image->data)[i] = (unsigned char)(pixels[k].x*255.0f);
					((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].y*255.0f);
					((unsigned char *)image->data)[i + 2] = (unsigned char)(pixels[k].z*255.0f);
					((unsigned char *)image->data)[i + 3] = (unsigned char)(pixels[k].w*255.0f);
				}
			} break;
			default: break;
		}

		free(pixels);
		pixels = NULL;

		// In case original image had mipmaps, generate mipmaps for formatted image
		// NOTE: Original mipmaps are replaced by new ones, if custom mipmaps were used, they are lost
		if (image->mipmaps > 1){
		}
    }
}

static Vector4 *LoadImageDataNormalized(Image *image)
{
    Vector4 *pixels = (Vector4 *)malloc(image->width*image->height*sizeof(Vector4));
	for (int i = 0, k = 0; i < image->width*image->height; i++)
	{
		switch (image->format)
		{
			case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
			{
				pixels[i].x = (float)((unsigned char *)image->data)[i]/255.0f;
				pixels[i].y = (float)((unsigned char *)image->data)[i]/255.0f;
				pixels[i].z = (float)((unsigned char *)image->data)[i]/255.0f;
				pixels[i].w = 1.0f;

			} break;
			case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
			{
				pixels[i].x = (float)((unsigned char *)image->data)[k]/255.0f;
				pixels[i].y = (float)((unsigned char *)image->data)[k]/255.0f;
				pixels[i].z = (float)((unsigned char *)image->data)[k]/255.0f;
				pixels[i].w = (float)((unsigned char *)image->data)[k + 1]/255.0f;

				k += 2;
			} break;
			case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
			{
				unsigned short pixel = ((unsigned short *)image->data)[i];

				pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
				pixels[i].y = (float)((pixel & 0b0000011111000000) >> 6)*(1.0f/31);
				pixels[i].z = (float)((pixel & 0b0000000000111110) >> 1)*(1.0f/31);
				pixels[i].w = ((pixel & 0b0000000000000001) == 0)? 0.0f : 1.0f;

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
			{
				unsigned short pixel = ((unsigned short *)image->data)[i];

				pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
				pixels[i].y = (float)((pixel & 0b0000011111100000) >> 5)*(1.0f/63);
				pixels[i].z = (float)(pixel & 0b0000000000011111)*(1.0f/31);
				pixels[i].w = 1.0f;

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
			{
				unsigned short pixel = ((unsigned short *)image->data)[i];

				pixels[i].x = (float)((pixel & 0b1111000000000000) >> 12)*(1.0f/15);
				pixels[i].y = (float)((pixel & 0b0000111100000000) >> 8)*(1.0f/15);
				pixels[i].z = (float)((pixel & 0b0000000011110000) >> 4)*(1.0f/15);
				pixels[i].w = (float)(pixel & 0b0000000000001111)*(1.0f/15);

			} break;
			case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
			{
				pixels[i].x = (float)((unsigned char *)image->data)[k]/255.0f;
				pixels[i].y = (float)((unsigned char *)image->data)[k + 1]/255.0f;
				pixels[i].z = (float)((unsigned char *)image->data)[k + 2]/255.0f;
				pixels[i].w = (float)((unsigned char *)image->data)[k + 3]/255.0f;

				k += 4;
			} break;
			case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
			{
				pixels[i].x = (float)((unsigned char *)image->data)[k]/255.0f;
				pixels[i].y = (float)((unsigned char *)image->data)[k + 1]/255.0f;
				pixels[i].z = (float)((unsigned char *)image->data)[k + 2]/255.0f;
				pixels[i].w = 1.0f;

				k += 3;
			} break;
			default: break;
		}
    }

    return pixels;
}

void SetTextureFilter(Texture2D *texture, int Filter){
	if(Filter == RL_TEXTURE_FILTER_POINT){
		C3D_TexSetFilter(&texture->tex, GPU_NEAREST, GPU_NEAREST);
        //C3D_TexSetFilterMipmap(&texture.tex, GPU_LINEAR);
        C3D_TexFlush(&texture->tex);
        C2D_Flush();
	}
	else{
		C3D_TexSetFilter(&texture->tex, GPU_LINEAR, GPU_LINEAR);
        //3D_TexSetFilterMipmap(&texture.tex, GPU_NEAREST);
        C3D_TexFlush(&texture->tex);
        C2D_Flush();
	}
}

void ImageColorTint(Image *image, Color color){
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Color *pixels = LoadImageColors(image);

    float cR = (float)color.r/255;
    float cG = (float)color.g/255;
    float cB = (float)color.b/255;
    float cA = (float)color.a/255;

    for (int i = 0; i < image->width * image->height; i++)
    {
        unsigned char r = (unsigned char)(((float)pixels[i].r/255*cR)*255.0f);
        unsigned char g = (unsigned char)(((float)pixels[i].g/255*cG)*255.0f);
        unsigned char b = (unsigned char)(((float)pixels[i].b/255*cB)*255.0f);
        unsigned char a = (unsigned char)(((float)pixels[i].a/255*cA)*255.0f);

        pixels[i].r = r;
        pixels[i].g = g;
        pixels[i].b = b;
        pixels[i].a = a;
    }

    int format = image->format;
    free(image->data);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

void UnloadTexture(Texture2D *texture){
    if(texture->id == 0)
        return;
    C3D_TexFlush(&texture->tex);
	C3D_TexDelete(&texture->tex);
    C2D_Flush();
    //C3D_TexFlush(&texture.tex);
	texture->id = 0;
    texture->baseScale = 1;
}

void ImageBlurGaussian(Image *image, int blurSize) {
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    ImageAlphaPremultiply(image);

    Color *pixels = LoadImageColors(image);

    // Loop switches between pixelsCopy1 and pixelsCopy2
    Vector4 *pixelsCopy1 = (Vector4 *)malloc((image->height)*(image->width)*sizeof(Vector4));
    Vector4 *pixelsCopy2 = (Vector4 *)malloc((image->height)*(image->width)*sizeof(Vector4));

    for (int i = 0; i < (image->height)*(image->width); i++) {
        pixelsCopy1[i].x = pixels[i].r;
        pixelsCopy1[i].y = pixels[i].g;
        pixelsCopy1[i].z = pixels[i].b;
        pixelsCopy1[i].w = pixels[i].a;
    }

    // Repeated convolution of rectangular window signal by itself converges to a gaussian distribution
    for (int j = 0; j < 4; j++) {
        // Horizontal motion blur
        for (int row = 0; row < image->height; row++)
        {
            float avgR = 0.0f;
            float avgG = 0.0f;
            float avgB = 0.0f;
            float avgAlpha = 0.0f;
            int convolutionSize = blurSize;

            for (int i = 0; i < blurSize; i++)
            {
                avgR += pixelsCopy1[row*image->width + i].x;
                avgG += pixelsCopy1[row*image->width + i].y;
                avgB += pixelsCopy1[row*image->width + i].z;
                avgAlpha += pixelsCopy1[row*image->width + i].w;
            }

            for (int x = 0; x < image->width; x++)
            {
                if (x-blurSize-1 >= 0)
                {
                    avgR -= pixelsCopy1[row*image->width + x-blurSize-1].x;
                    avgG -= pixelsCopy1[row*image->width + x-blurSize-1].y;
                    avgB -= pixelsCopy1[row*image->width + x-blurSize-1].z;
                    avgAlpha -= pixelsCopy1[row*image->width + x-blurSize-1].w;
                    convolutionSize--;
                }

                if (x+blurSize < image->width)
                {
                    avgR += pixelsCopy1[row*image->width + x+blurSize].x;
                    avgG += pixelsCopy1[row*image->width + x+blurSize].y;
                    avgB += pixelsCopy1[row*image->width + x+blurSize].z;
                    avgAlpha += pixelsCopy1[row*image->width + x+blurSize].w;
                    convolutionSize++;
                }

                pixelsCopy2[row*image->width + x].x = avgR/convolutionSize;
                pixelsCopy2[row*image->width + x].y = avgG/convolutionSize;
                pixelsCopy2[row*image->width + x].z = avgB/convolutionSize;
                pixelsCopy2[row*image->width + x].w = avgAlpha/convolutionSize;
            }
        }

        // Vertical motion blur
        for (int col = 0; col < image->width; col++)
        {
            float avgR = 0.0f;
            float avgG = 0.0f;
            float avgB = 0.0f;
            float avgAlpha = 0.0f;
            int convolutionSize = blurSize;

            for (int i = 0; i < blurSize; i++)
            {
                avgR += pixelsCopy2[i*image->width + col].x;
                avgG += pixelsCopy2[i*image->width + col].y;
                avgB += pixelsCopy2[i*image->width + col].z;
                avgAlpha += pixelsCopy2[i*image->width + col].w;
            }

            for (int y = 0; y < image->height; y++)
            {
                if (y-blurSize-1 >= 0)
                {
                    avgR -= pixelsCopy2[(y-blurSize-1)*image->width + col].x;
                    avgG -= pixelsCopy2[(y-blurSize-1)*image->width + col].y;
                    avgB -= pixelsCopy2[(y-blurSize-1)*image->width + col].z;
                    avgAlpha -= pixelsCopy2[(y-blurSize-1)*image->width + col].w;
                    convolutionSize--;
                }
                if (y+blurSize < image->height)
                {
                    avgR += pixelsCopy2[(y+blurSize)*image->width + col].x;
                    avgG += pixelsCopy2[(y+blurSize)*image->width + col].y;
                    avgB += pixelsCopy2[(y+blurSize)*image->width + col].z;
                    avgAlpha += pixelsCopy2[(y+blurSize)*image->width + col].w;
                    convolutionSize++;
                }

                pixelsCopy1[y*image->width + col].x = (unsigned char) (avgR/convolutionSize);
                pixelsCopy1[y*image->width + col].y = (unsigned char) (avgG/convolutionSize);
                pixelsCopy1[y*image->width + col].z = (unsigned char) (avgB/convolutionSize);
                pixelsCopy1[y*image->width + col].w = (unsigned char) (avgAlpha/convolutionSize);
            }
        }
    }

    // Reverse premultiply
    for (int i = 0; i < (image->width)*(image->height); i++)
    {
        if (pixelsCopy1[i].w == 0.0f)
        {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
            pixels[i].a = 0;
        }
        else if (pixelsCopy1[i].w <= 255.0f)
        {
            float alpha = (float)pixelsCopy1[i].w/255.0f;
            pixels[i].r = (unsigned char)((float)pixelsCopy1[i].x/alpha);
            pixels[i].g = (unsigned char)((float)pixelsCopy1[i].y/alpha);
            pixels[i].b = (unsigned char)((float)pixelsCopy1[i].z/alpha);
            pixels[i].a = (unsigned char) pixelsCopy1[i].w;
        }
    }

    int format = image->format;
    free(image->data);
    free(pixelsCopy1);
    free(pixelsCopy2);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

void ImageAlphaPremultiply(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    float alpha = 0.0f;
    Color *pixels = LoadImageColors(image);

    for (int i = 0; i < image->width*image->height; i++)
    {
        if (pixels[i].a == 0)
        {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
        }
        else if (pixels[i].a < 255)
        {
            alpha = (float)pixels[i].a/255.0f;
            pixels[i].r = (unsigned char)((float)pixels[i].r*alpha);
            pixels[i].g = (unsigned char)((float)pixels[i].g*alpha);
            pixels[i].b = (unsigned char)((float)pixels[i].b*alpha);
        }
    }

    free(image->data);

    int format = image->format;
    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

void DrawRectangleRec(Rectangle rec, Color color){
	C2D_DrawRectSolid(rec.x, rec.y, 0.0f, rec.width, rec.height, C2D_Color32(color.r, color.g, color.b, color.a));
    C2D_Flush();  //test
}

void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color){
	if ((lineThick > rec.width) || (lineThick > rec.height)){
        if (rec.width > rec.height) lineThick = rec.height/2;
        else if (rec.width < rec.height) lineThick = rec.width/2;
    }

    // When rec = { x, y, 8.0f, 6.0f } and lineThick = 2, the following
    // four rectangles are drawn ([T]op, [B]ottom, [L]eft, [R]ight):
    //
    //   TTTTTTTT
    //   TTTTTTTT
    //   LL    RR
    //   LL    RR
    //   BBBBBBBB
    //   BBBBBBBB
    //

    Rectangle top = { rec.x, rec.y, rec.width, lineThick };
    Rectangle bottom = { rec.x, rec.y - lineThick + rec.height, rec.width, lineThick };
    Rectangle left = { rec.x, rec.y + lineThick, lineThick, rec.height - lineThick*2.0f };
    Rectangle right = { rec.x - lineThick + rec.width, rec.y + lineThick, lineThick, rec.height - lineThick*2.0f };

    DrawRectangleRec(top, color);
    DrawRectangleRec(bottom, color);
    DrawRectangleRec(left, color);
    DrawRectangleRec(right, color);

}

Color Fade(Color color, float alpha){
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    return (Color){ color.r, color.g, color.b, (unsigned char)(255.0f*alpha) };
}

RenderTexture2D LoadRenderTexture(int width, int height, bool vram){
    RenderTexture2D target = { 0 };
    target.id = 1; // Load an empty framebuffer
    //std::cout << "shit.\n";
    //TEST
    //target.id = 0;
    //return target;
    if (target.id > 0){
        target.texture.id = 1;

		target.texture.subtex.width = static_cast<u16>(width);
        target.texture.subtex.height = static_cast<u16>(height);

        u32 w_pow2 = GetNextPowerOf2(target.texture.subtex.width);
		u32 h_pow2 = GetNextPowerOf2(target.texture.subtex.height);

        //std::cout << w_pow2 << " w_pow2  -  ";
        //std::cout << h_pow2 << " h_pow2  -  ";
        
		target.texture.subtex.left = 0.f;
		target.texture.subtex.top = 1.f;
		target.texture.subtex.right = (target.texture.subtex.width /static_cast<float>(w_pow2));
		target.texture.subtex.bottom = (1.0 - (target.texture.subtex.height / static_cast<float>(h_pow2)));
        
        if(vramSpaceFree() <=  w_pow2 * h_pow2 * 2){
            vram = false;
        }
		if(vram)
            target.texture.id = C3D_TexInitVRAM(&target.texture.tex, static_cast<u16>(w_pow2), static_cast<u16>(h_pow2), _determineHardwareFormat(PIXELFORMAT_UNCOMPRESSED_R4G4B4A4));
        else
            target.texture.id = C3D_TexInit(&target.texture.tex, static_cast<u16>(w_pow2), static_cast<u16>(h_pow2), _determineHardwareFormat(PIXELFORMAT_UNCOMPRESSED_R4G4B4A4));
        //std::cout << "texinitdone.\n";
		if(target.texture.id == 0){
            target.id = 0;
            std::cout << "TEXTURE INIT FAILED, MAYBE RUNNING OUT OF MEMORYY" << "bytes\n";
            return target;
        }
        std::cout << target.texture.tex.size << " bytes for the texture, free: " << vramSpaceFree() << "bytes\n";
		target.texture.tex.border = 0x00000000;
		C3D_TexSetWrap(&target.texture.tex, GPU_MIRRORED_REPEAT  , GPU_MIRRORED_REPEAT  );
        C3D_TexSetFilter(&target.texture.tex, GPU_LINEAR, GPU_LINEAR);
        C3D_TexFlush(&target.texture.tex);

        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;
        target.texture.mipmaps = 0;

        target.renderTarget = C3D_RenderTargetCreateFromTex(&target.texture.tex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH16);
        //std::cout << "rendertarget done.\n";
    }
    return target;
}

void BeginTextureMode(RenderTexture2D *renderTexture){
    C2D_Flush(); 
    //C2D_Prepare();
    C2D_Prepare();
    C2D_SceneBegin(renderTexture->renderTarget);
    Global.gpu_currentRenderTarget = renderTexture->renderTarget;
}

void ClearBackground(Color color){
    C2D_TargetClear(Global.gpu_currentRenderTarget, C2D_Color32(color.r, color.g, color.b, color.a));
    C2D_Flush();  //test
}

void rlClearScreenBuffers(){
    //not needed after clearbackground
    //C3D_RenderTargetClear(Global.gpu_currentRenderTarget, C3D_CLEAR_ALL, C2D_Color32(0, 0, 0, 0), 0);
    //C3D_RenderTargetClear(Global.window, C3D_CLEAR_ALL, C2D_Color32(0, 0, 0, 0), 0);
}

void rlEnableDepthMask(){
    //this isnt even needed afaik?????
}

void EndTextureMode(){
    C2D_Flush(); 
    //C2D_Prepare();
    C2D_Prepare();
    C2D_SceneBegin(Global.window);
    Global.gpu_currentRenderTarget = Global.window;
}

void rlEnableDepthTest(){
    C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
    C2D_Flush(); 
}

void rlEnableDepthTest2(){
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C2D_Flush(); 
}

void DrawCircleWithDepth(Vector2 center, float radius, int segments, float depth, Color color){
    //C2D_DrawCircleSolid(center.x, center.y, depth, radius, C2D_Color32(color.r, color.g, color.b, color.a));
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    float startAngle = 0;
    float endAngle = 360;
    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - 0.5f/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    startAngle = startAngle + 360 / segments;
    endAngle = endAngle + 360 / segments;


    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;
    for (int i = 0; i < segments; i++){
        //rlColor4ub(color.r, color.g, color.b, color.a);

        //rlVertex2f(center.x, center.y);
        //rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);
        //rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
        u32 c = C2D_Color32(color.r, color.g, color.b, color.a);
        C2D_DrawTriangle(center.x, center.y, c, center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius, c, center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius, c, depth);
        C2D_Flush(); 
        angle += stepLength;
    }



    C2D_Flush();  //test
}
void DrawCircleWithDepthGrad(Vector2 center, float radius, int segments, float depth, Color color, Color color2){
    //C2D_DrawCircleSolid(center.x, center.y, depth, radius, C2D_Color32(color.r, color.g, color.b, color.a));
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    float startAngle = 0;
    float endAngle = 360;
    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - 0.5f/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    startAngle = startAngle + 360 / segments;
    endAngle = endAngle + 360 / segments;


    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;
    for (int i = 0; i < segments; i++){
        //rlColor4ub(color.r, color.g, color.b, color.a);

        //rlVertex2f(center.x, center.y);
        //rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);
        //rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
        u32 c = C2D_Color32(color2.r, color2.g, color2.b, color2.a);
        u32 cs = C2D_Color32(color.r, color.g, color.b, color.a);
        C2D_DrawTriangle(center.x, center.y, cs, center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius, c, center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius, c, depth);
        C2D_Flush(); 
        angle += stepLength;
    }
    C2D_Flush();  //test
}

void rlSetBlendMode(int mode){
    //dummy func
}

void rlSetBlendFactorsSeparate(GPU_BLENDFACTOR glSrcRGB, GPU_BLENDFACTOR glDstRGB, GPU_BLENDFACTOR glSrcAlpha, GPU_BLENDFACTOR glDstAlpha, GPU_BLENDEQUATION glEqRGB, GPU_BLENDEQUATION glEqAlpha){
    C3D_AlphaBlend(glEqRGB, glEqAlpha, glSrcRGB, glDstRGB, glSrcAlpha, glDstAlpha);	
    C2D_Flush(); 
}

void rlDisableDepthTest(){
    C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_ALL);
    C2D_Flush(); 
}

void EndBlendMode(){
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);	
    C2D_Flush(); 
}

void DrawTextureEx(Texture2D *texture, Vector2 position, float rotation, float scale, Color tint){
    if(texture->id == 0)
        return;
    C2D_Image image;
    image.tex = &texture->tex;
    image.subtex = &texture->subtex;
    C2D_ImageTint c2dTint;
    //Maybe needs fixing, idk how blend and alpha work
    c2dTint.corners[0] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[1] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[2] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[3] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};

    if(rotation == 0.0f){
        C2D_Prepare();
        C2D_DrawImageAt(image, position.x, position.y, 0.0f, &c2dTint, scale * texture->baseScale, scale * texture->baseScale);
        //std::cout << texture->baseScale << std::endl;
        //if(texture.width != 120)
            //std::cout << "texdraw: " << texture.width << std::endl;
    }
    else{
        std::cout << "nah bro no rotation in drawex\n"; //maybe use the function in drawtexturepro??? hmmm
    }
    C2D_Flush();  //test
}

void DrawTextureExDepth(Texture2D *texture, Vector2 position, float depth, float scale, Color tint){
    if(texture->id == 0)
        return;
    C2D_Image image;
    image.tex = &texture->tex;
    image.subtex = &texture->subtex;
    C2D_ImageTint c2dTint;
    //Maybe needs fixing, idk how blend and alpha work
    c2dTint.corners[0] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[1] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[2] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[3] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};

    C2D_Prepare();
    C2D_DrawImageAt(image, position.x, position.y, depth, &c2dTint, scale * texture->baseScale, scale * texture->baseScale);
    C2D_Flush();  //test
}

void DrawTextureRotate(Texture2D *tex, float x, float y, float s, float r, Color tint){
    if(tex->id == 0)
        return;
    C2D_Image image;
    image.tex = &tex->tex;
    image.subtex = &tex->subtex;
    C2D_ImageTint c2dTint;
    //Maybe needs fixing, idk how blend and alpha work
    c2dTint.corners[0] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[1] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[2] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};
    c2dTint.corners[3] = {C2D_Color32(tint.r, tint.g, tint.b, tint.a), 1.0f};

    //DUNNO IF ROTATION IS GIVEN IN RADIANS BUT YEAH! FIRE IN THE HOLE!!
    C2D_Prepare();
    C2D_DrawImageAtRotated(image, ScaleCordX(x), ScaleCordY(y), 0, DEG2RAD*r, &c2dTint, Scale(s * tex->baseScale), Scale(s * tex->baseScale));
    C2D_Flush();  //test
}

void UnloadRenderTexture(RenderTexture2D *renderTexture){
    if(renderTexture->id == 0)
        return;
    C3D_RenderTargetDelete(renderTexture->renderTarget);
    C2D_Flush();
    UnloadTexture(&renderTexture->texture);
    C2D_Flush();
    renderTexture->texture.id = 0;
    renderTexture->id = 0;
}

void DrawRectangle(int posX, int posY, int width, int height, Color color){
    C2D_DrawRectSolid(posX, posY, 0.0f, width, height, C2D_Color32(color.r, color.g, color.b, color.a));
    C2D_Flush();  //test
}

int GetFPS(){
    return 0;
}

Shader LoadShader(const char *vsFileName, const char *fsFileName){
    Shader shader = { 0 };
    return shader;
}
int GetShaderLocation(Shader shader, const char *uniformName){
    return 0;
}
void SetShaderValue(Shader shader, int locIndex, const void *value, int uniformType){}
void BeginShaderMode(Shader shader){}
void EndShaderMode(){}

void SetConfigFlags(unsigned int flags){}
void HideCursor(){}