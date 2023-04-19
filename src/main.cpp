#define WINVER 0x0500
#define _WIN32_WINNT 0x500

#define GetTickCount64() GetTickCount()

#define OPENGL_SPOOF

#define SDL_MAIN_HANDLED



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
<<<<<<< HEAD
#include "../vendor/raylib/src/external/glfw/include/GLFW/glfw3.h"
#include <mutex>
//#include "SDLutils.hpp"
=======

#include <assert.h>
//#include "SDLutils.hpp"


>>>>>>> f13b003f88aef38f55536ca83a07365f7d00b590

double avgFPS = 144;
double avgHZ = 1000;

double Mx = 0;
double My = 0;





//hello from arch!
Globals Global;


void RenderLoop(){
    double last = 0;
    
    
    while(!WindowShouldClose()){

        
        SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), GetWindowGL());
        last = getTimer();
        rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
        BeginDrawing();
        ClearBackground(Global.Background);
        Global.mutex.lock();
        Global.CurrentState->render();
        //std::cout << "gamemanager render done\n";
        if(Global.GameTextures == -1)
            Global.gameManager->unloadGameTextures();
        else if(Global.GameTextures == 1)
            Global.gameManager->loadGameTextures();
        //std::cout << "load-unload render done\n";
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        //std::cout << "Drawing mouse... " << Global.AutoMousePosition.x << " " << Global.AutoMousePosition.y << std::endl;
        renderMouse(); 
        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %.3f",  avgFPS), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("TPS: %.3f",  avgHZ), {ScaleCordX(5), ScaleCordY(35)}, Scale(15), Scale(1), GREEN);
        Global.mutex.unlock();
        rlDrawRenderBatchActive();
        SwapScreenBuffer();
        //Global.mutex.lock();
        SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), NULL);
        //Global.mutex.unlock();

        while(getTimer() - last < 1000.0/288.0 and getTimer() - last >= 0)
            continue;
        avgFPS = (avgFPS + 1000.0f / (getTimer() - last)) / 2.0;
        //std::cout << avgFPS << std::endl;
    }
    
}


int main() {
    SDL_SetMainReady();
    Global.CurrentState = std::make_shared<MainMenu>();
    for(int i = 0; i < Global.GamePath.size(); i++) {
        if (Global.GamePath[i] == '\\')
            Global.GamePath[i] = '/';
    }
    SetTraceLogLevel(LOG_WARNING);
    InitAudioDevice();
    
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetAudioStreamBufferSizeDefault(128);
    InitWindow(640, 480, "aa");
    

    Global.DefaultFont = LoadFont("resources/telegrama_render.otf");
    Global.OsusLogo = LoadTexture("resources/osus.png");
    Global.shdrOutline = LoadShader(0, TextFormat("resources/shaders/glsl%i/outline.fs", 330));

    //Image cus;
    std::string lastPath = Global.Path;
	Global.Path = "resources/default_skin/";
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
    Global.Path = "resources/skin/";
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
    GenTextureMipmaps(&Global.OsusLogo);
    GenTextureMipmaps(&Global.cursor);
    GenTextureMipmaps(&Global.DefaultFont.texture); //OPENGL1.1 DOESNT SUPPORT THIS
    SetTextureFilter(Global.DefaultFont.texture, TEXTURE_FILTER_TRILINEAR );
    double avgFrameTime;
    HideCursor();
    initMouseTrail();
    Global.LastFrameTime = getTimer();
    double lastFrame = getTimer();
    Global.GameTextures = 0;
    

    //initSDL();

    
    //
    //CORE.Window.handle;



    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSwapInterval( 0 );
    SDL_GL_SetSwapInterval(0);
    SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), NULL);

    std::thread rend(RenderLoop);

    while(!WindowShouldClose()){
        lastFrame = getTimer();
        
<<<<<<< HEAD
        /*if (IsKeyPressed(KEY_F) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
        {
            int display = GetCurrentMonitor();
            if (IsWindowFullscreen())
                SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            else
                SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            ToggleFullscreen();
            SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            if(!IsWindowFullscreen())
                SetWindowSize(640, 480); 
        }*/
        
        BeginDrawing();
        ClearBackground(Global.Background);

        locktite.lock();
        Global.CurrentState->render();
        if(Global.GameTextures == -1)
            Global.gameManager->unloadGameTextures();
        else if(Global.GameTextures == 1)
            Global.gameManager->loadGameTextures();
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        
        //double what = getTimer();
        //updateMouseTrail();
        renderMouse(); 
        //std::cout << getTimer() - what << std::endl;
        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %.3f",  avgFPS), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("TPS: %.3f",  avgHZ), {ScaleCordX(5), ScaleCordY(35)}, Scale(15), Scale(1), GREEN);

        locktite.unlock();
        rlDrawRenderBatchActive();
        SwapScreenBuffer();
        //PollInputEvents();
        //EndDrawing();
        while(getTimer() - lastFrame < 1000.0/144.0 and getTimer() - lastFrame >= 0)
=======
        /*if(getTimer() - lastFrame > 0.2)
            std::cout << "main thread waited for the mutex lock for " << getTimer() - lastFrame << " milliseconds" << std::endl;*/
        Global.mutex.lock();
        PollInputEvents();
        GetScale();
        GetMouse();
        GetKeys();
        if(IsKeyDown(SDL_SCANCODE_LALT) and IsKeyPressed(SDL_SCANCODE_RETURN)){
            ToggleFullscreen();
        }
        updateUpDown();
        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        Global.CurrentState->update();
        Global.mutex.unlock();
        while(getTimer() - lastFrame < 1000.0/750.0 and getTimer() - lastFrame >= 0)
>>>>>>> f13b003f88aef38f55536ca83a07365f7d00b590
            continue;
        avgHZ = (avgHZ + 1000.0f / (getTimer() - lastFrame)) / 2.0;

    }
    rend.join();

    UnloadTexture(Global.OsusLogo);
    UnloadFont(Global.DefaultFont);
    UnloadShader(Global.shdrOutline);
    CloseWindow();
    //quitSDL();
}

