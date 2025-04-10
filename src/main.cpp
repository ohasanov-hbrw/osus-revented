#define SDL_MAIN_HANDLED
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
#include <filesystem>
#include "fastrender.hpp"
#include <gui.hpp>
#include "fs.hpp"
#include "state.hpp"
#include "zip.h"
#include <condition_variable>
#include <assert.h>
#include "time_util.hpp"
#include <queue>
#include "settingsParser.hpp"
//#include "SDLutils.hpp"

Globals Global;


MULTITHREAD_MUTEX stateLock;
MULTITHREAD_MUTEX accessLock;
MULTITHREAD_MUTEX osuGameLock;
MULTITHREAD_MUTEX wholeRenderLock;

MULTITHREAD_THREAD renderThread;

u32 __stacksize__= 512 * 1024;


std::condition_variable cv;

double avgFPS = Global.FPS;
double avgHZ = 1000;

double Mx = 0;
double My = 0;

int VSYNC = 0;

int avgFPSqueueNUM = 0;
int avgHZqueueNUM = 0;
double avgFPSqueueSUM = 0;
double avgHZqueueSUM = 0;
std::queue<double> avgFPSq;
std::queue<double> avgHZq;

bool dumbsleep = false;


//hello from arch!



void RenderLoop(void *){
    double last = 0;
    
    _gpu_init_render_thread();
    
    Global.OsusLogo = LoadTexture((Global.GamePath + "/resources/osus.png").c_str());
    std::cout << "Loaded logo\n";
    std::cout << (Global.GamePath + "/resources/osus.png").c_str() << std::endl;
    
    //Global.DefaultFont = LoadFont("sdmc:/3ds/resources/telegrama_render.otf");
    
    Global.DefaultFont = GetFontDefault();
    
    std::cout << "Loaded font\n";
    
    

    Global.shdrOutline = LoadShader(0, TextFormat((Global.GamePath + "/resources/shaders/glsl%i/outline.fs").c_str(), 100));

    Global.shdrTest = LoadShader(TextFormat((Global.GamePath + "/resources/shaders/glsl%i/mcosu.vsh").c_str(), 330), TextFormat((Global.GamePath + "/resources/shaders/glsl%i/mcosu.fsh").c_str(), 330));
    

    //Image cus;
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
    std::cout << "Loaded skin";

    SetTextureFilter(&Global.DefaultFont.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(&Global.cursor, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(&Global.OsusLogo, TEXTURE_FILTER_BILINEAR);

    std::cout << "Loaded all files and filters\n";


    int loc = 0;
    int lastFPS = 0;
    while(!Global.stop){ //!WindowShouldClose() and 
        auto t1 = std::chrono::steady_clock::now();
        last = getTimer();
        //rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

        //std::cout << "begin\n";
        MutexLock(RENDER_BLOCK);
        Global.CurrentState->textureOps();
        if(Global.stop){
            _gpu_start_drawing(Global.window);
            ClearBackground(Global.Background);
            DrawTextEx(&Global.DefaultFont, TextFormat("bye bye ~ :3"), {(int)ScaleCordX(10), (int)ScaleCordY(10)}, Scale(40.15), Scale(2), WHITE);
            _gpu_check_command_buffer();
            _gpu_end_drawing();
            MutexUnlock(RENDER_BLOCK);
            break;
        }
        _gpu_start_drawing(Global.window);


        if(Global.NeedForBackgroundClear)
            ClearBackground(Global.Background);
        
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->render();
        MutexUnlock(SWITCHING_STATE);
        MutexUnlock(RENDER_BLOCK);
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse(); 




        DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %.3f TPS: %.3f",  avgFPS, avgHZ), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
        
        _gpu_check_command_buffer();

        _gpu_end_drawing();
        
        #ifndef THREEDS_BUILD
            if(VSYNC == 0){
                std::chrono::duration<double, std::milli> sleepTime {std::chrono::steady_clock::now() - t1};
                unsigned int sleepTimeInt = (unsigned int)(std::max(0.0, (1000.0/Global.FPS) - (sleepTime.count())) * 800.0);
                if(!dumbsleep)
                    SleepInUs(sleepTimeInt);
                while(getTimer() - last < 1000.0/Global.FPS and getTimer() - last >= 0)
                    continue;
            }
        #endif

        std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - t1};
        double fps = (1000.0f / (elapsed.count()));
        lastFPS = fps;
        if(lastFPS > 511)
            lastFPS = 511;
        //if(elapsed.count() > 20 and VSYNC == 0)
        //    std::cout << "dropped frame with " << elapsed.count() << "ms\n";
        avgFPSq.push(fps);
        avgFPSqueueSUM += fps;
        if(avgFPSq.size() > 300){
            avgFPSqueueSUM -= avgFPSq.front();
            avgFPSq.pop();
        }
        avgFPS = avgFPSqueueSUM / (double)(avgFPSq.size());

    }
    std::cout << "trying to exit the rendering thread\n";
    _gpu_exit_render_thread();
    std::cout << "finalized the renderthread! bye bye";
	return;
}




int main(){
    _os_init_program(VSYNC);
    InitilizeLocks();

    std::cout << _os_get_free_linear_ram() << std::endl;    
    Global.linearSpaceFree = _os_get_free_linear_ram();
    

    std::cout << "parsing the settings.ini file...\n";
    parseSettings();

    

    //SDL_SetMainReady();
    Global.CurrentState = std::make_shared<MainMenu>();
    for(int i = 0; i < Global.GamePath.size(); i++) {
        if (Global.GamePath[i] == '\\')
            Global.GamePath[i] = '/';
    }
    
    //LightLock_Init(&Global.lightlock);

    std::cout << "Loaded gamepath\n";
    //SetTraceLogLevel(LOG_WARNING); //LOG_WARNING
    InitAudioDevice();
    
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetAudioStreamBufferSizeDefault(128);
    //InitWindow(640, 480, "osus - amogus");
    
    //SDL_Surface* pIcon = SDL_CreateRGBSurface(0,64,64,32,0,0,0,0);;
    //SDL_SetWindowIcon((SDL_Window*)GetWindowSDL(), pIcon);
    //SDL_FreeSurface(pIcon);
    
    //std::cout << "Global.cursor size: " << Global.cursor.width << " x " << Global.cursor.height << " y \n";
    double avgFrameTime;
    HideCursor();
    initMouseTrail();

    //std::cout << "Cursor init done\n";

    Global.LastFrameTime = getTimer();
    double lastFrame = getTimer();
    
    std::cout << "Starting render loop\n";
    std::cout << "Free Vram: " << _os_get_free_vram() << std::endl;
	std::cout << "Free M_ALL: " << _os_get_free_ram(MEMREGION_ALL) << "/" << _os_get_size_ram(MEMREGION_ALL) << std::endl;
	std::cout << "Free M_APP: " << _os_get_free_ram(MEMREGION_APPLICATION) << "/" << _os_get_size_ram(MEMREGION_APPLICATION) << std::endl;
	std::cout << "Free M_SYS: " << _os_get_free_ram(MEMREGION_SYSTEM) << "/" << _os_get_size_ram(MEMREGION_SYSTEM) << std::endl;
	std::cout << "Free M_BSE: " << _os_get_free_ram(MEMREGION_BASE) << "/" << _os_get_size_ram(MEMREGION_BASE) << std::endl;
    std::cout << "Free M_LIN: " << _os_get_free_linear_ram() << "/" << Global.linearSpaceFree << std::endl;

    
    renderThread = _multithread_thread_create(RenderLoop);


    
    while(!WindowShouldClose() and _os_should_program_run()){
        double timerXXX = getTimer();
        auto t1 = std::chrono::steady_clock::now();
        //Global.mutex.lock();
        //LightLock_Lock(&Global.lightlock);
        PollInputEvents();
        GetScale();
        GetMouse();
        GetKeys();
        _os_check_fullscreen();
        updateUpDown();
        if(_os_check_end_condition()){
            break;
        }
        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        updateMouseTrail();
        Global.CurrentState->update();
        
        //Global.mutex.unlock();
        //LightLock_Unlock(&Global.lightlock);

        std::chrono::duration<double, std::milli> sleepTime {std::chrono::steady_clock::now() - t1};
        unsigned int sleepTimeInt = (unsigned int)(std::max(0.0, (1000.0/(float)(Global.TPS)) - sleepTime.count()) * 980.0);
        if(!dumbsleep)
            SleepInUs(sleepTimeInt);
        
        while(getTimer() - timerXXX < 1000.0/(float)(Global.TPS) and getTimer() - timerXXX >= 0)
            continue;
        
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
    
    std::cout << "exiting...\n";
    //MutexLock(RENDER_BLOCK);
    SleepInMs(500); //make sure that the gpu has done drawing whatever it had in its buffer... if a frame is taking more than half a second we have other problems...
    MutexLock(ACCESSING_OBJECTS);
    std::cout << "unloading current situation\n";
    
    Global.CurrentState->initDone = 3;
    MutexUnlock(ACCESSING_OBJECTS);

    MutexLock(RENDER_BLOCK);
    MutexLock(ACCESSING_OBJECTS);
    MutexUnlock(RENDER_BLOCK);

    MutexLock(SWITCHING_STATE);
    std::cout << "locked the switching state\n";
    Global.CurrentState->unload();
    MutexUnlock(ACCESSING_OBJECTS);
    Global.CurrentState.reset(new PlayMenu());
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE);
    MutexUnlock(ACCESSING_OBJECTS);

    //Global.CurrentState->unload();
    SleepInMs(5);
    Global.stop = true;
    std::cout << "unloaded\n";
    MutexUnlock(RENDER_BLOCK);

    _multithread_join_thread(&renderThread);
    _multithread_free_thread(&renderThread);
    

    //Global.CurrentState->unload();

    std::cout << "bye bye :3 ~!\n";
    SleepInMs(1000);

    _os_exit_program();

    return 0;
}
