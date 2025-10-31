/*
    Here is the necessary "main" logic of osus!
    The main usage way of C++ was just to use a Global
    struct for most of the data storage, which also
    includes settings and whatnot.

    The libraries here should be pretty portable,
    running a very customized version of raylib 3 (?)
    and miniaudio for the main audio functions.

    Compatibility between 3DS and PC is achieved using 
    "#ifdef"s, which may not be an optimal way of doing 
    it, but since the architechtures are so different 
    I didnt really see another way of making it happen

    This has been in development since 2020, time has 
    passed... 

    with love, ohasanov :3
*/

#define SDL_MAIN_HANDLED
#define PLATFORM_DESKTOP
//#define GRAPHICS_API_OPENGL_11
//#define GRAPHICS_API_OPENGL_33

#include <math.h>
#include <vector>
#include <algorithm>
#include <utils.hpp>
#include "rlgl.h"
#include "raylib.h"
#include "globals.hpp"
#include <iostream>
#include "fastrender.hpp"
#include <gui.hpp>
#include "fs.hpp"
#include "state.hpp"
#include <assert.h>
#include "time_util.hpp"
#include <queue>
#include "settingsParser.hpp"

// Main data storage struct is first initialized here
Globals Global;

// Overlays for platform spesific multithreading functions and datatypes
MULTITHREAD_MUTEX stateLock;
MULTITHREAD_MUTEX accessLock;
MULTITHREAD_MUTEX osuGameLock;
MULTITHREAD_MUTEX wholeRenderLock;
MULTITHREAD_THREAD renderThread;

// This is mainly for the 3DS, shouldn't affect PC builds
u32 __stacksize__= 512 * 1024;

// Variables for frametime statistics
double avgFPS = Global.FPS;
double avgHZ = 1000;

// VSYNC is set on build...
// TODO: Move this to Global data and read it from settings.ini
int VSYNC = 0;

// Helpers for frametime statistics calculations
int avgFPSqueueNUM = 0;
int avgHZqueueNUM = 0;
double avgFPSqueueSUM = 0;
double avgHZqueueSUM = 0;
std::queue<double> avgFPSq;
std::queue<double> avgHZq;

// To use spinlocks, or not to use spinlocks...
bool dumbsleep = false;

// Seperate thread for rendering
void RenderLoop(void *){
    // Used for calculating frametimes
    double last = 0;
    int loc = 0;
    int lastFPS = 0;

    // Initialize GPU Functions for both platforms
    _gpu_init_render_thread();
    
    // Load some necessary files that are needed a bunch
    Global.OsusLogo = LoadTexture((Global.GamePath + "/resources/osus.png").c_str());
    std::cout << "[INFO] Loaded logo: ";
    std::cout << (Global.GamePath + "/resources/osus.png").c_str() << std::endl;
    
    //Global.DefaultFont = LoadFont("sdmc:/3ds/resources/telegrama_render.otf");
    Global.DefaultFont = GetFontDefault();
    std::cout << "[INFO] Loaded font\n";

    // First ever frame is a loading screen
    MutexLock(RENDER_BLOCK);
    _gpu_start_drawing(Global.window);
    ClearBackground(Global.Background);
    DrawTextEx(&Global.DefaultFont, TextFormat("Loading game..."), {static_cast<float>((int)Scale(10)), static_cast<float>((int)Scale(10))}, Scale(40.15), Scale(2), WHITE);
    _gpu_check_command_buffer();
    _gpu_end_drawing();
    MutexUnlock(RENDER_BLOCK);
    
    // Playing around with shaders sometimes... Sliders are a PITA
    Global.shdrOutline = LoadShader(0, TextFormat((Global.GamePath + "/resources/shaders/glsl%i/outline.fs").c_str(), 100));
    Global.shdrTest = LoadShader(TextFormat((Global.GamePath + "/resources/shaders/glsl%i/mcosu.vsh").c_str(), 330), TextFormat((Global.GamePath + "/resources/shaders/glsl%i/mcosu.fsh").c_str(), 330));
    
    // Loading textures for the custom cursors.
    // The game crashes here if you have no skins, which duh, you SHOULD
    std::string lastPath = Global.Path;
	Global.Path = Global.GamePath + "/resources/default_skin/";
	std::vector<std::string> files = ls(".png");
	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("cursortrail.png", 0) == 0)
				Global.cursorTrail = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("cursor.png", 0) == 0)
                Global.cursor = LoadTexture((Global.Path + files[i]).c_str());
		}
	}
	files.clear();
    Global.Path = Global.GamePath + "/resources/skin/";
    files = ls(".png");
	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("cursortrail.png", 0) == 0)
				Global.cursorTrail = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("cursor.png", 0) == 0)
                Global.cursor = LoadTexture((Global.Path + files[i]).c_str());
		}
	}
	files.clear();
    Global.Path = lastPath;
    std::cout << "[INFO] Loaded cursor";

    // 3DS doesn't support some fancy filters
    // TODO: Make some filter and graphics settings configurable (OGL1.1 - OGL2.2 differences)
    #ifdef THREEDS_BUILD
        SetTextureFilter(&Global.DefaultFont.texture, TEXTURE_FILTER_POINT);
    #endif
    #ifndef THREEDS_BUILD
        //GenTextureMipmaps(&Global.DefaultFont.texture);
        SetTextureFilter(&Global.DefaultFont.texture, TEXTURE_FILTER_ANISOTROPIC_16X);
    #endif

    SetTextureFilter(&Global.cursor, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(&Global.OsusLogo, TEXTURE_FILTER_BILINEAR);

    std::cout << "[INFO] Loaded initial files and filters\n";
    
    // Signal readyness to the game logic loop
    Global.readyForGameLoop = true;

    // Main render loop
    while(true){
        auto t1 = std::chrono::steady_clock::now();
        last = getTimer();

        // While loading/initializing nothing should be rendered...
        if(Global.readyForRenderLoop){
            // Lock Mutexes to drive away multithreading-goblins
            MutexLock(RENDER_BLOCK);
            // Texture Unloading has a high priority
            Global.CurrentState->textureOps();
            // Stop signal from main thread triggers exit
            if(Global.stop){
                MutexUnlock(RENDER_BLOCK);
                break;
            }

            // Start drawing stuff
            _gpu_start_drawing(Global.window);

            // Dont want to be rendering stuff while loading stuff
            MutexLock(SWITCHING_STATE);
            // Dont flash while loading, just keep the last image
            if(Global.NeedForBackgroundClear && Global.CurrentState->initDone != 0)
                ClearBackground(Global.Background);
            // The state should know not to render while its loading
            Global.CurrentState->render();
            MutexUnlock(SWITCHING_STATE);
            MutexUnlock(RENDER_BLOCK);
            
            // Draw input indicator
            DrawRectangle(GetScreenWidth() - Scale(640 - 580), GetScreenHeight() - Scale(480 - 450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
            DrawRectangle(GetScreenWidth() - Scale(640 - 610), GetScreenHeight() - Scale(480 - 450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
            
            // Render mouse and its trail
            renderMouse(); 

            // Show fps and game ticks per second
            DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %.0f TPS: %.0f",  avgFPS, avgHZ), {static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(5))}, Scale(20.05), Scale(2), GREEN);
            
            // Mainly for 3DS Debugging purposes
            _gpu_check_command_buffer();

            // End frame.
            _gpu_end_drawing();
        }
        else{
            // We still need to be able to exit even if we are loading something
            // First ever frame is a loading screen
            _gpu_start_drawing(Global.window);
            ClearBackground(Global.Background);
            DrawTextEx(&Global.DefaultFont, TextFormat("Loading game..."), {static_cast<float>((int)Scale(10)), static_cast<float>((int)Scale(10))}, Scale(40.15), Scale(2), WHITE);
            _gpu_check_command_buffer();
            _gpu_end_drawing();
            if(Global.stop){
                break;
            }
        }

        // 3DS Always uses VSYNC. Apart from VSYNC the other frame limiting type is the following
        #ifndef THREEDS_BUILD
            if(VSYNC == 0){
                std::chrono::duration<double, std::milli> sleepTime {std::chrono::steady_clock::now() - t1};
                unsigned int sleepTimeInt = (unsigned int)(std::max(0.0, (1000.0/Global.FPS) - (sleepTime.count())) * 900.0);
                // Reduce the amount of spinlocks
                if(!dumbsleep)
                    SleepInUs(sleepTimeInt);
                // Spinlock.
                while(getTimer() - last < 1000.0/Global.FPS and getTimer() - last >= 0)
                    continue;
            }
        #endif

        // Calculate Frametime and FPS
        std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - t1};
        double fps = (1000.0f / (elapsed.count()));
        lastFPS = fps;
        if(lastFPS > 511)
            lastFPS = 511;
        //if(elapsed.count() > 20 and VSYNC == 0)
        //    std::cout << "[WARN] Dropped frame with " << elapsed.count() << "ms\n";
        avgFPSq.push(fps);
        avgFPSqueueSUM += fps;
        if(avgFPSq.size() > 300){
            avgFPSqueueSUM -= avgFPSq.front();
            avgFPSq.pop();
        }
        avgFPS = avgFPSqueueSUM / (double)(avgFPSq.size());

    }

    // We need to go out with a bang! or at least say bye...
    _gpu_start_drawing(Global.window);
    ClearBackground(Global.Background);
    DrawTextEx(&Global.DefaultFont, "Bye bye!~ :3", {static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(5))}, Scale(40.05), Scale(2), WHITE);
    _gpu_check_command_buffer();
    _gpu_end_drawing();

    // Deinitialize GPU
    std::cout << "[INFO] Trying to exit the rendering thread\n";
    _gpu_exit_render_thread();
    std::cout << "[INFO] Finalized the renderthread! bye bye";
	return;
}




int main(){

    // Set flags before initializing Window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    // Initialize with VSYNC or not
    _os_init_program(VSYNC);
    // Mutexes need initializing 
    InitilizeLocks();

    // Mainly a 3DS Debug option
    std::cout << _os_get_free_linear_ram() << std::endl;    
    Global.linearSpaceFree = _os_get_free_linear_ram();
    
    // Get settings from an ini file if they exist
    std::cout << "[INFO] parsing the settings.ini file...\n";
    parseSettings();

    // Initialize audio engine 
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(240);

    // Convert windows paths to linux path?
    for(int i = 0; i < Global.GamePath.size(); i++) {
        if (Global.GamePath[i] == '\\')
            Global.GamePath[i] = '/';
    }

    // Start the rendering loop
    std::cout << "[INFO] Starting render loop\n";
    renderThread = _multithread_thread_create(RenderLoop);

    // Frametime statistics and cursor initialization
    double avgFrameTime;
    Global.LastFrameTime = getTimer();
    double lastFrame = getTimer();
    HideCursor();
    initMouseTrail();
    
    // Debug free memory on 3DS Systems
    std::cout << "[INFO] Free Vram: " << _os_get_free_vram() << std::endl;
	std::cout << "[INFO] Free M_ALL: " << _os_get_free_ram(MEMREGION_ALL) << "/" << _os_get_size_ram(MEMREGION_ALL) << std::endl;
	std::cout << "[INFO] Free M_APP: " << _os_get_free_ram(MEMREGION_APPLICATION) << "/" << _os_get_size_ram(MEMREGION_APPLICATION) << std::endl;
	std::cout << "[INFO] Free M_SYS: " << _os_get_free_ram(MEMREGION_SYSTEM) << "/" << _os_get_size_ram(MEMREGION_SYSTEM) << std::endl;
	std::cout << "[INFO] Free M_BSE: " << _os_get_free_ram(MEMREGION_BASE) << "/" << _os_get_size_ram(MEMREGION_BASE) << std::endl;
    std::cout << "[INFO] Free M_LIN: " << _os_get_free_linear_ram() << "/" << Global.linearSpaceFree << std::endl;

    // Wait for rendering thread to be ready
    while(!Global.readyForGameLoop){
        // waiting for the render loop to be initialized
        SleepInMs(100);
        if(WindowShouldClose() or !_os_should_program_run() or Global.readyForGameLoop){
            break;
        }
    }

    // Initialize the state engine
    Global.CurrentState = std::make_shared<StartMenu>();
    Global.CurrentState->init();

    // State readiness to render thread
    Global.readyForRenderLoop = true;

    // Main game loop
    while(!WindowShouldClose() and _os_should_program_run()){
        // Start timer for sleep and stats
        double timerXXX = getTimer();
        auto t1 = std::chrono::steady_clock::now();
        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        
        // Quasi "System" functions, mouse, input and whatnot
        PollInputEvents();
        GetScale();
        GetMouse();
        GetKeys();
        _os_check_fullscreen();
        updateUpDown();
        updateMouseTrail();

        // If we wanna exit, we WANT to exit
        if(_os_check_end_condition()){
            break;
        }
        
        // Run whatever the current state wants, game, menu...
        Global.CurrentState->update();
        
        // Sleep to save CPU cycles
        std::chrono::duration<double, std::milli> sleepTime {std::chrono::steady_clock::now() - t1};
        unsigned int sleepTimeInt = (unsigned int)(std::max(0.0, (1000.0/(float)(Global.TPS)) - sleepTime.count()) * 980.0);
        if(!dumbsleep)
            SleepInUs(sleepTimeInt);
        while(getTimer() - timerXXX < 1000.0/(float)(Global.TPS) and getTimer() - timerXXX >= 0)
            continue;
    
        // Calculate statistics for game speed
        std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - t1};
        double hz = (1000.0f / (elapsed.count()));
        avgHZq.push(hz);
        avgHZqueueSUM += hz;
        if(avgHZq.size() > 300){
            avgHZqueueSUM -= avgHZq.front();
            avgHZq.pop();
        }
        avgHZ = avgHZqueueSUM / (double)(avgHZq.size());
    }
    
    // Signal the end of program
    std::cout << "[INFO] exiting...\n";
    
    // Make sure that the gpu has done drawing whatever it had in its buffer... if a frame is taking more than half a second we have other problems...
    SleepInMs(500); 

    // Get control of the situation
    MutexLock(ACCESSING_OBJECTS);
    std::cout << "[INFO] unloading current situation\n";
    Global.CurrentState->initDone = 3;
    MutexUnlock(ACCESSING_OBJECTS);

    // Get control from the render thread
    MutexLock(RENDER_BLOCK);
    MutexLock(ACCESSING_OBJECTS);
    MutexUnlock(RENDER_BLOCK);
    MutexLock(SWITCHING_STATE);
    std::cout << "[INFO] locked the switching state\n";

    // Unload current state, which may involve texture operations...
    std::cout << "[INFO] calling unload\n";
    Global.CurrentState->unload();
    std::cout << "[INFO] done unloading\n";
    MutexUnlock(ACCESSING_OBJECTS);

    // Reset to start? Maybe?
    //Global.CurrentState.reset(new StartMenu());
    //Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE);
    MutexUnlock(ACCESSING_OBJECTS);

    // Signal to kill rendering thread
    SleepInMs(5);
    Global.stop = true;
    std::cout << "[INFO] Signaling stop to render thread\n";

    _multithread_join_thread(&renderThread);
    _multithread_free_thread(&renderThread);

    deleteGlobalVariables();

    std::cout << "[INFO] bye bye :3 ~!\n";
    SleepInMs(50);

    // Exit stuff
    _os_exit_program();

    return 0;
}
