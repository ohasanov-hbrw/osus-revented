#pragma once
#include <atomic>
// we need to add some mutex management stuff here with atomics, so that one thread cannot unlock twice, or lock twice

#ifdef THREEDS_BUILD
    #define OSUS_MAX_TEXTURE_WIDTH 256
    #define OSUS_MAX_TEXTURE_HEIGHT 128
    #define OSUS_DEFAULT_DIVIDER 4
#endif
#ifndef THREEDS_BUILD
    #define OSUS_MAX_TEXTURE_WIDTH 2048
    #define OSUS_MAX_TEXTURE_HEIGHT 2048
    #define OSUS_DEFAULT_DIVIDER 1
#endif


#ifndef THREEDS_BUILD
    #include <thread>
    #include "raylib.h"
    #include <mutex>
    typedef std::mutex MULTITHREAD_MUTEX;
    typedef Vector2 INPUT_TOUCHSCREEN;
    typedef void GPU_RENDER_TARGET;
    typedef int64_t i64;
    typedef int32_t i32;
    typedef int16_t i16;
    typedef int8_t i8;

    typedef uint64_t u64;
    typedef uint32_t u32;
    typedef uint16_t u16;
    typedef uint8_t u8;

    typedef float f32;
    typedef double f64;

    #define MEMREGION_ALL 0
    #define MEMREGION_APPLICATION 0
    #define MEMREGION_SYSTEM 0
    #define MEMREGION_BASE 0


    //void DrawTextureExDepth(Texture2D *texture, Vector2 position, float depth, float scale, Color tint);
    void DrawTextureRotate(Texture2D *tex, float x, float y, float s, float r, Color color);

    typedef std::thread MULTITHREAD_THREAD;

#endif

#ifdef THREEDS_BUILD
    #include <3ds.h>
    #include "SDL/SDL.h"
    #include "raylibDefinitions.h"
    typedef LightLock MULTITHREAD_MUTEX;
    typedef touchPosition INPUT_TOUCHSCREEN;
    typedef C3D_RenderTarget GPU_RENDER_TARGET;
    typedef Thread MULTITHREAD_THREAD;
#endif


void _multithread_mutex_init(MULTITHREAD_MUTEX * mutex);
void _multithread_mutex_lock(MULTITHREAD_MUTEX * mutex);
void _multithread_mutex_unlock(MULTITHREAD_MUTEX * mutex);

MULTITHREAD_THREAD _multithread_thread_create(void(void*));

void _multithread_join_thread(MULTITHREAD_THREAD *);
void _multithread_free_thread(MULTITHREAD_THREAD *);

bool _music_check_if_ended(Music *music);

long long int _os_get_free_vram();

long long int _os_get_free_ram(int region);
long long int _os_get_size_ram(int region);

long long int _os_get_free_linear_ram();

bool _os_should_program_run();

void _gpu_start_drawing(GPU_RENDER_TARGET * target);
void _gpu_end_drawing(bool batch = true);

void _gpu_init_render_thread();

void _gpu_exit_render_thread();

void _gpu_check_command_buffer();

void _os_check_fullscreen();

bool _os_check_end_condition();

void _os_init_program(bool VSYNC);

void _os_exit_program();

void DrawTextureCenter(Texture2D *tex, float x, float y, float s, Color color);


