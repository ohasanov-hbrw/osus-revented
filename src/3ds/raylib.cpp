#include "raylib.h"
#include "globals.hpp"
#include <string>
#include <cstdio>
#include <iostream>

bool WindowShouldClose(){
    return (Global.ds_kDown & KEY_START);
}

unsigned char *LoadFileData(const char *fileName, int *dataSize){
    unsigned char *data = NULL;
    *dataSize = 0;
    if (fileName != NULL){
        FILE *file = fopen(fileName, "rb");
        if (file != NULL){
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);
            if (size > 0){
                data = (unsigned char *)malloc(size*sizeof(unsigned char));
                if (data != NULL){
                    size_t count = fread(data, sizeof(unsigned char), size, file);
                    if (count > 2147483647){
                        free(data);
                        data = NULL;
                    }
                    else{
                        *dataSize = (int)count;
                    }
                }
            }
            fclose(file);
        }
    }

    return data;
}


void UnloadFileData(unsigned char *data){
    free(data);
}

bool TextIsEqual(const char *text1, const char *text2){
    bool result = false;

    if ((text1 != NULL) && (text2 != NULL)){
        if (strcmp(text1, text2) == 0) result = true;
    }

    return result;
}

const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return dot;
}

bool IsFileExtension(const char *fileName, const char *ext){
    #define MAX_FILE_EXTENSION_LENGTH  16

    bool result = false;
    const char *fileExt = GetFileExtension(fileName);

    if (fileExt != NULL){
        if (strcmp(fileExt, ext) == 0) result = true;
    }

    return result;
}

Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer){
    Image image;
    //PLACEHOLDER :)
    image = LoadImage((Global.GameBinaryPath + "/resources/sliderin.png").c_str());
    return image;
}

// Get a Color struct from hexadecimal value
Color GetColor(unsigned int hexValue){
    Color color;

    color.r = (unsigned char)(hexValue >> 24) & 0xFF;
    color.g = (unsigned char)(hexValue >> 16) & 0xFF;
    color.b = (unsigned char)(hexValue >> 8) & 0xFF;
    color.a = (unsigned char)hexValue & 0xFF;

    return color;
}

bool FileExists(const char *fileName)
{
    bool result = false;

    if (access(fileName, F_OK) != -1) result = true;

    // NOTE: Alternatively, stat() can be used instead of access()
    //#include <sys/stat.h>
    //struct stat statbuf;
    //if (stat(filename, &statbuf) == 0) result = true;

    return result;
}