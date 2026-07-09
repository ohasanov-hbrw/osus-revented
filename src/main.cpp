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
#include "menu_shapes/shapes.hpp"

#include "cachebuilder/metadataParser.hpp"

#define NUMBER_BACKGROUND_TRIS 150
#define SIZE_BACKGROUND_TRIS 200
#define SIZE_VARIATION_BACKGROUND_TRIS 300
#define COLOR_VARIATION_BACKGROUND_TRIS 10
#define OPACITY_VARIATION_BACKGROUND_TRIS 40
#define SPEED_BACKGROUND_TRIS 10

Vector2 **backgroundTriangles;
Color *backgroundTriangleColors;
Vector2 *backgroundTriangleVelocity;

Color backgroundTriangleBase = {128, 128, 128, 128};

// Main data storage struct is first initialized here
Globals Global;
CommonMenuSizes MenuSizes;

// Overlays for platform spesific multithreading functions and datatypes
MULTITHREAD_MUTEX stateLock;
MULTITHREAD_MUTEX accessLock;
MULTITHREAD_MUTEX osuGameLock;
MULTITHREAD_MUTEX wholeRenderLock;
MULTITHREAD_MUTEX audioEngineLock;
MULTITHREAD_THREAD renderThread;

std::atomic<bool> __mutex_threads_locks[32][32];

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
    Global.OsusLogo = LoadTexture((Global.GameBinaryPath + "/resources/osus.png").c_str());
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded logo\n";
    //std::cout << (Global.GameBinaryPath + "/resources/osus.png").c_str() << std::endl;
    
    //Global.DefaultFont = LoadFont("sdmc:/3ds/resources/telegrama_render.otf");
    Global.DefaultFont = GetFontDefault();
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded font\n";


    backgroundTriangles = (Vector2**) malloc(NUMBER_BACKGROUND_TRIS * sizeof(Vector2*));
    for(int i = 0; i < NUMBER_BACKGROUND_TRIS; i++)
        backgroundTriangles[i] = (Vector2*) malloc(3 * sizeof(Vector2));
    backgroundTriangleColors = (Color*) malloc(NUMBER_BACKGROUND_TRIS * sizeof(Color));
    backgroundTriangleVelocity = (Vector2*) malloc(NUMBER_BACKGROUND_TRIS * sizeof(Vector2));


    //Generate random sized triangles that are defined counter-clockwise
    std::srand(std::time({}));
    for(int i = 0; i < NUMBER_BACKGROUND_TRIS; i++){
        backgroundTriangles[i][0] = Vector2{std::rand() % (640 * 2) - 640.0 / 2, std::rand() % (480 * 2)  - 480.0 / 2}; //Bigger than screen!
        backgroundTriangleVelocity[i] = Vector2{std::rand() % SPEED_BACKGROUND_TRIS - SPEED_BACKGROUND_TRIS / 2.0, std::rand() % SPEED_BACKGROUND_TRIS - SPEED_BACKGROUND_TRIS / 2.0}; //SPEED!!!
        int upDown = std::rand() % 2;
        if(upDown == 0){ // Triangle going to be facing up
            int size = std::rand() % SIZE_VARIATION_BACKGROUND_TRIS + SIZE_BACKGROUND_TRIS - SIZE_VARIATION_BACKGROUND_TRIS / 2;
            backgroundTriangles[i][1] = Vector2{backgroundTriangles[i][0].x + size, backgroundTriangles[i][0].y};
            backgroundTriangles[i][2] = Vector2{backgroundTriangles[i][0].x + (size / 2.0), backgroundTriangles[i][0].y - size * std::sin(M_PI / 3.0)}; //Raylib coord system is weird, down is y+
        }
        else{ // Triangle going to be facing down
            int size = std::rand() % SIZE_VARIATION_BACKGROUND_TRIS + SIZE_BACKGROUND_TRIS - SIZE_VARIATION_BACKGROUND_TRIS / 2;
            backgroundTriangles[i][1] = Vector2{backgroundTriangles[i][0].x + (size / 2.0), backgroundTriangles[i][0].y + size * std::sin(M_PI / 3.0)};
            backgroundTriangles[i][2] = Vector2{backgroundTriangles[i][0].x + size, backgroundTriangles[i][0].y};
        }
        Color tempColor = {128,128,128,128};
        tempColor.r = backgroundTriangleBase.r + (std::rand() % COLOR_VARIATION_BACKGROUND_TRIS - COLOR_VARIATION_BACKGROUND_TRIS / 2);
        tempColor.g = backgroundTriangleBase.g + (std::rand() % COLOR_VARIATION_BACKGROUND_TRIS - COLOR_VARIATION_BACKGROUND_TRIS / 2);
        tempColor.b = backgroundTriangleBase.b + (std::rand() % COLOR_VARIATION_BACKGROUND_TRIS - COLOR_VARIATION_BACKGROUND_TRIS / 2);
        tempColor.a = backgroundTriangleBase.a + (std::rand() % OPACITY_VARIATION_BACKGROUND_TRIS - OPACITY_VARIATION_BACKGROUND_TRIS / 2);
        backgroundTriangleColors[i] = tempColor;

    }            

    // First ever frame is a loading screen
    MutexLock(RENDER_BLOCK, RENDERTHREAD_ID);
    _gpu_start_drawing(Global.window);
    ClearBackground(Global.Background);
    DrawCoolBackground(backgroundTriangles, backgroundTriangleColors, backgroundTriangleVelocity, NUMBER_BACKGROUND_TRIS, 0);
    DrawTextEx(&Global.DefaultFont, TextFormat("Loading game..."), {static_cast<float>((int)Scale(10)), static_cast<float>((int)Scale(10))}, Scale(40.15), Scale(2), WHITE);
    _gpu_check_command_buffer();
    _gpu_end_drawing();
    MutexUnlock(RENDER_BLOCK, RENDERTHREAD_ID);
    
    // Playing around with shaders sometimes... Sliders are a PITA
    Global.shdrOutline = LoadShader(0, TextFormat((Global.GameBinaryPath + "/resources/shaders/glsl%i/outline.fs").c_str(), 100));
    Global.shdrTest = LoadShader(TextFormat((Global.GameBinaryPath + "/resources/shaders/glsl%i/mcosu.vsh").c_str(), 330), TextFormat((Global.GameBinaryPath + "/resources/shaders/glsl%i/mcosu.fsh").c_str(), 330));
    
    // Loading textures for the custom cursors.
    // The game crashes here if you have no skins, which duh, you SHOULD
    std::string lastPath = Global.Path;
	Global.Path = Global.GameBinaryPath + "/resources/default_skin/";
	std::vector<std::string> files = ls(".png");
	std::sort(files.begin(), files.end(), []
    (std::string_view first, std::string_view second){
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
    Global.Path = Global.GameBinaryPath + "/resources/skin/";
    files = ls(".png");
	std::sort(files.begin(), files.end(), []
    (std::string_view first, std::string_view second){
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
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded cursor\n";

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

    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded initial files and filters\n";
    
    




    // Signal readyness to the game logic loop
    Global.readyForGameLoop = true;

    // Main render loop
    while(true){
        auto t1 = std::chrono::steady_clock::now();
        double delta = getTimer() - last;
        last = getTimer();

        // While loading/initializing nothing should be rendered...
        if(Global.readyForRenderLoop){
            // Lock Mutexes to drive away multithreading-goblins
            MutexLock(RENDER_BLOCK, RENDERTHREAD_ID);
            // Texture Unloading has a high priority
            Global.CurrentState->textureOps();
            // Stop signal from main thread triggers exit
            if(Global.stop){
                MutexUnlock(RENDER_BLOCK, RENDERTHREAD_ID);
                break;
            }

            // Start drawing stuff
            _gpu_start_drawing(Global.window);

            // Dont want to be rendering stuff while loading stuff
            MutexLock(SWITCHING_STATE, RENDERTHREAD_ID);
            // Dont flash while loading, just keep the last image
            if(Global.NeedForBackgroundClear && Global.CurrentState->initializationStage != STATE_UNINITIALIZED){
                ClearBackground(Global.Background);
                DrawCoolBackground(backgroundTriangles, backgroundTriangleColors, backgroundTriangleVelocity, NUMBER_BACKGROUND_TRIS, delta / 1000.0);
            }
            // The state should know not to render while its loading
            Global.CurrentState->render();
            MutexUnlock(SWITCHING_STATE, RENDERTHREAD_ID);
            MutexUnlock(RENDER_BLOCK, RENDERTHREAD_ID);
            
            // Draw input indicator
            DrawRectangle(Scale(200), Scale(5), Scale(20), Scale(20), (Color){(unsigned char)(255 * (int)Global.Key1D), (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
            DrawRectangle(Scale(230), Scale(5), Scale(20), Scale(20), (Color){(unsigned char)(255 * (int)Global.Key2D), (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
            
            // Render mouse and its trail
            renderMouse(); 

            // Show fps and game ticks per second
            DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %.0f", avgFPS), {static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(5))}, Scale(20.05), Scale(2), GREEN);
            DrawTextEx(&Global.DefaultFont, TextFormat("TPS: %.0f", avgHZ), {static_cast<float>((int)Scale(100)), static_cast<float>((int)Scale(5))}, Scale(20.05), Scale(2), GREEN);

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

    for(int i = 0; i < NUMBER_BACKGROUND_TRIS; i++)
        free(backgroundTriangles[i]);
    free(backgroundTriangles);
    free(backgroundTriangleColors);
    free(backgroundTriangleVelocity);

    // Deinitialize GPU
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Trying to exit the rendering thread\n";
    _gpu_exit_render_thread();
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Finalized the renderthread\n";
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
    //std::cout << _os_get_free_linear_ram() << std::endl;    
    Global.linearSpaceFree = _os_get_free_linear_ram();
    
    // Convert windows paths to linux path?
    for(int i = 0; i < Global.GameBinaryPath.size(); i++) {
        if (Global.GameBinaryPath[i] == '\\')
            Global.GameBinaryPath[i] = '/';
    }

    // Get settings from an ini file if they exist
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "parsing the settings.ini file...\n";
    parseSettings();



    

    // Initialize audio engine 
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(240);

    
    buildFileMap(Global.BeatmapLocation);
    listAllMaps();
    decideNamesForSets();


    /*std::vector<SetFileMetadata> test = parseCachedSets(Global.DatabaseLocation + "/beatmapsets.db");
    for (const auto& meta : test) {
        std::cout << "Set ID: " << meta.setid << "\n";
        std::cout << "Title:  " << meta.title << "\n";
        std::cout << "Number: " << meta.number << "\n";
        std::cout << "-------------------------\n";
    }*/

    // Start the rendering loop
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Starting render loop\n";
    renderThread = _multithread_thread_create(RenderLoop);

    // Frametime statistics and cursor initialization
    double avgFrameTime;
    Global.LastFrameTime = getTimer();
    double lastFrame = getTimer();
    HideCursor();
    initMouseTrail();
    
    // Debug free memory on 3DS Systems
#ifdef THREEDS_BUILD
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free Vram: " << _os_get_free_vram() / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_ALL: " << _os_get_free_ram(MEMREGION_ALL) / 1024 << "/" << _os_get_size_ram(MEMREGION_ALL) / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_APP: " << _os_get_free_ram(MEMREGION_APPLICATION) / 1024 << "/" << _os_get_size_ram(MEMREGION_APPLICATION) / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_SYS: " << _os_get_free_ram(MEMREGION_SYSTEM) / 1024 << "/" << _os_get_size_ram(MEMREGION_SYSTEM) / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_BSE: " << _os_get_free_ram(MEMREGION_BASE) / 1024<< "/" << _os_get_size_ram(MEMREGION_BASE) / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_LIN: " << _os_get_free_linear_ram() / 1024 << "/" << Global.linearSpaceFree / 1024 << "KB" << std::endl;
#endif
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
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "exiting...\n";
    
    // Make sure that the gpu has done drawing whatever it had in its buffer... if a frame is taking more than half a second we have other problems...
    SleepInMs(500); 

    // Get control of the situation
    

    // Get control from the render thread
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "unloading current situation\n";
    Global.CurrentState->initializationStage = STATE_FORCED_EXIT;
    
    
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "locked the switching state\n";

    // Unload current state, which may involve texture operations...
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "calling unload\n";
    Global.CurrentState->unload();
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "done unloading\n";
    

    // Reset to start? Maybe?
    //Global.CurrentState.reset(new StartMenu());
    //Global.CurrentState->init();
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);

    // Signal to kill rendering thread
    SleepInMs(5);
    Global.stop = true;
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Signaling stop to render thread\n";

    _multithread_join_thread(&renderThread);
    _multithread_free_thread(&renderThread);

    //deleteGlobalVariables();

    std::cout <<  "\e[1;38;5;236m[INFO] \e[38;5;206m"  << "bye bye :3 ~!\n";
    SleepInMs(200);

    // Exit stuff
    _os_exit_program();

    return 0;
}
