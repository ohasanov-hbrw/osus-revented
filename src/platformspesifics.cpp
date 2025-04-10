#pragma once
#include "platformspesifics.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "rlgl.h"
#include "time_util.hpp"

void _multithread_mutex_init(MULTITHREAD_MUTEX * mutex){
    #ifdef THREEDS_BUILD
        LightLock_Init(mutex);
    #endif
    #ifndef THREEDS_BUILD
        //literally not needed????
    #endif
}
void _multithread_mutex_lock(MULTITHREAD_MUTEX * mutex){
    #ifdef THREEDS_BUILD
        LightLock_Lock(mutex);
    #endif
    #ifndef THREEDS_BUILD
        mutex->lock();
    #endif
}
void _multithread_mutex_unlock(MULTITHREAD_MUTEX * mutex){
    #ifdef THREEDS_BUILD
        LightLock_Unlock(mutex);
    #endif
    #ifndef THREEDS_BUILD
        mutex->unlock();
    #endif
}

MULTITHREAD_THREAD _multithread_thread_create(void func(void*)){
    #ifdef THREEDS_BUILD
        return threadCreate(func, NULL, 1*1024*1024, 0x29, -1, false);
    #endif
    #ifndef THREEDS_BUILD
        void * arguments = NULL;
        return std::thread(func, arguments);
    #endif
}

void _multithread_join_thread(MULTITHREAD_THREAD * thread){
    #ifdef THREEDS_BUILD
        threadJoin(*thread, U64_MAX);
    #endif
    #ifndef THREEDS_BUILD
        thread->join();
    #endif
}

void _multithread_free_thread(MULTITHREAD_THREAD * thread){
    #ifdef THREEDS_BUILD
        threadFree(*thread);
    #endif
    #ifndef THREEDS_BUILD
        std::cout << "IDK HOW TO FREE THREADS IN STDLIB?????" << std::endl;
    #endif
}

bool _music_check_if_ended(Music *music){
    #ifdef THREEDS_BUILD
        return music->ended;
    #endif
    #ifndef THREEDS_BUILD
        return (GetMusicTimeLength(music) - GetMusicTimePlayed(music) < 0.05f);
    #endif
}

long long int _os_get_free_vram(){
    #ifdef THREEDS_BUILD
        return vramSpaceFree();
    #endif
    #ifndef THREEDS_BUILD
        return -37;
    #endif
}

long long int _os_get_free_ram(int region){
    #ifdef THREEDS_BUILD
        return osGetMemRegionFree((MemRegion)region);
    #endif
    #ifndef THREEDS_BUILD
        return -37;
    #endif
}

long long int _os_get_size_ram(int region){
    #ifdef THREEDS_BUILD
        return osGetMemRegionSize((MemRegion)region);
    #endif
    #ifndef THREEDS_BUILD
        return -37;
    #endif
}

long long int _os_get_free_linear_ram(){
    #ifdef THREEDS_BUILD
        return linearSpaceFree();
    #endif
    #ifndef THREEDS_BUILD
        return -37;
    #endif
}

bool _os_should_program_run(){
    #ifdef THREEDS_BUILD
        return aptMainLoop();
    #endif
    #ifndef THREEDS_BUILD
        return true;
    #endif
}


void _gpu_start_drawing(GPU_RENDER_TARGET * target){
    #ifdef THREEDS_BUILD
        C2D_Prepare();
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_SceneBegin(target);
    #endif
    #ifndef THREEDS_BUILD
        rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
        BeginDrawing();
    #endif
}

void _gpu_end_drawing(bool batch){
    #ifdef THREEDS_BUILD
        C2D_Flush();  //test
        C3D_FrameEnd(0);
    #endif
    #ifndef THREEDS_BUILD
        if(batch){
            rlDrawRenderBatchActive();
            SDL_GL_SwapWindow((SDL_Window*)GetWindowSDL());
        }
        else{
            EndDrawing();
        }
    #endif
}

void _gpu_init_render_thread(){
    #ifdef THREEDS_BUILD
        C3D_Init(0x100000);//C3D_DEFAULT_CMDBUF_SIZE);
        C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
        
        std::cout << "WAITING FOR 0.5SECS\n";

        SleepInUs(1*500*1000);

        std::cout << "C2D INIT\n";
        
        
        if(Global.useTopScreen){
            Global.window = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
        }
        else{
            Global.window = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
        } 
        Global.gpu_currentRenderTarget = Global.window;

        C2D_Prepare();
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_SceneBegin(Global.window);
        C2D_SetTintMode(C2D_TintMult);
        EndBlendMode();
        ClearBackground(Global.Background);
        C2D_Flush();  //test
        C3D_FrameEnd(0);

        LoadFontDefault(); //raylib does this automatically normally...

        Global.legacyRender = true;
        Global.polygonalRendering = false;

    #endif
    #ifndef THREEDS_BUILD
        SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), GetWindowGL());
        std::cout << "Render make gl current\n";
        BeginDrawing();
        ClearBackground(Global.Background);
        rlDrawRenderBatchActive();
        SDL_GL_SwapWindow((SDL_Window*)GetWindowSDL());

        std::cout << "Clear first bg\n";
    #endif
}

void _gpu_exit_render_thread(){
    #ifdef THREEDS_BUILD
        UnloadTexture(&Global.OsusLogo);
        UnloadTexture(&Global.cursor);
        //UnloadFont(&Global.DefaultFont);
        UnloadFontDefault(); //again, normally this is done by raylib automagically...

        C2D_Fini();
        C3D_Fini();
    #endif
    #ifndef THREEDS_BUILD
        UnloadTexture(&Global.OsusLogo);
        UnloadTexture(&Global.cursor);
    #endif
}

void _gpu_check_command_buffer(){
    #ifdef THREEDS_BUILD
        if(C3D_GetCmdBufUsage() > 0.8f){
            std::cout << "NEARLY OVERFLOWING THE COMMAND BUFFER, BEWARE: " << C3D_GetCmdBufUsage() * 100.0f << "%\n";
        }
    #endif
    #ifndef THREEDS_BUILD
        return;
    #endif
}

void _os_check_fullscreen(){
    #ifdef THREEDS_BUILD
        return;
    #endif
    #ifndef THREEDS_BUILD
        if(IsKeyDown(SDL_SCANCODE_LALT) and IsKeyPressed(SDL_SCANCODE_RETURN)){
            ToggleFullscreen();
        }
    #endif
}

bool _os_check_end_condition(){
    #ifdef THREEDS_BUILD
        if(IsKeyDown(KEY_START)){
            return true;
        }
        return false;
    #endif
    #ifndef THREEDS_BUILD
        if(IsKeyDown(SDL_SCANCODE_LALT) and IsKeyPressed(SDL_SCANCODE_F4)){
            return true;
        }
        return false;
    #endif
}

void _os_init_program(bool VSYNC){
    #ifdef THREEDS_BUILD
        osSetSpeedupEnable(true);
        gfxInitDefault();
        aptSetSleepAllowed(true);
        consoleGetDefault()->fg = 23;
        
        if(Global.useTopScreen){
            consoleInit(GFX_BOTTOM, NULL);
        }
        else{
            consoleInit(GFX_TOP, NULL);
        }

        //consoleGetDefault()->flags &= ~CONSOLE_COLOR_BOLD;
        //consoleGetDefault()->flags |= CONSOLE_COLOR_FAINT;
        consoleGetDefault()->fg = 23;
        std::cout << "Loaded gpu\n";
    #endif
    #ifndef THREEDS_BUILD
        SDL_SetMainReady();
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(640, 480, "osus - amogus");
        
        SDL_Surface* pIcon = SDL_CreateRGBSurface(0,64,64,32,0,0,0,0);;
        SDL_SetWindowIcon((SDL_Window*)GetWindowSDL(), pIcon);
        SDL_FreeSurface(pIcon);

        SDL_GL_SetSwapInterval(VSYNC);
        std::cout << "SetVsync\n";
        SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), NULL);
    #endif
}
void _os_exit_program(){
    #ifdef THREEDS_BUILD
        gfxExit();
    #endif
    #ifndef THREEDS_BUILD
        SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), GetWindowGL());
        CloseWindow();
    #endif
}

#ifdef THREEDS_BUILD
    //need to put this into the raylib folder tbh
    void DrawTextureCenter(Texture2D *tex, float x, float y, float s, Color color){
        if(tex->id == 0)
            return;
        //DrawTextureEx(tex, ScaleCords(GetRaylibOrigin({x,y,tex.width*s,tex.height*s})), 0, Scale(s), color);
        C2D_ImageTint c2dTint;
        //Maybe needs fixing, idk how blend and alpha work
        c2dTint.corners[0] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
        c2dTint.corners[1] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
        c2dTint.corners[2] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
        c2dTint.corners[3] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
        /*if(tex.width == 48){
            std::cout << tex.subtex.width << tex.subtex.height << std::endl;
        }*/
        C2D_Prepare();
        C2D_DrawImageAt(C2D_Image{&tex->tex, &tex->subtex}, ScaleCordX(x - ((tex->width * s) / 2.0f)), ScaleCordY(y - ((tex->height * s) / 2.0f)), 0.0f, &c2dTint, Scale(s * tex->baseScale), Scale(s * tex->baseScale));
        C2D_Flush();  //test
    }
#endif

#ifndef THREEDS_BUILD
    //These stuff are going into raylib to be implemented :D

    //void DrawTextureExDepth(Texture2D *texture, Vector2 position, float depth, float scale, Color tint){
    //    std::cout << "WARNING: The function \"DrawTextureExDepth\" is not supported on the PC architecture. This means that the graphics will seem broken since there are some things that are using this function, as it was called. To solve this issue, check the code and see if you can do something about it I guess. " << std::endl;
    //    std::cout << "TIP: This function is currently mainly used for the slider generation, maybe turn on the polygon based renderer by setting \"polygonalRendering\" to \"true\"" << std::endl;
    //    return;
    //}
//
    void DrawTextureRotate(Texture2D *tex, float x, float y, float s, float r, Color color){
        DrawTexturePro(tex, Rectangle{0,0,tex->width,tex->height}, Rectangle{ScaleCordX(x),ScaleCordY(y),Scale(tex->width*s),Scale(tex->height*s)}, Vector2{Scale(tex->width*s/2.0f), Scale(tex->height*s/2.0f)}, r, color);
    }

    void DrawTextureCenter(Texture2D *tex, float x, float y, float s, Color color){
        //Make the necessary calculations inside the function
        DrawTextureEx(tex, ScaleCords(GetRaylibOrigin({x,y,tex->width*s,tex->height*s})), 0, Scale(s), color);
    }
    
#endif

