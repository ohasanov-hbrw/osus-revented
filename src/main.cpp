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
#include "../vendor/raylib/src/external/glfw/include/GLFW/glfw3.h"
#include <mutex>
#include "SDLutils.hpp"

double avgFPS = 144;
double avgHZ = 1000;

double Mx = 0;
double My = 0;





//hello from arch!
Globals Global;


void GameLoop(){
    while(true){
        //PollInputEvents();
        //SDLpollEvent();
        //SDLGetMouse();

        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        
        Global.CallbackMouse.x = Mx;
        Global.CallbackMouse.y = My;

        GetScale();
        

        //GetMouse();
        

        
        //updateUpDown();
        Global.CurrentState->update();
        while(getTimer() - Global.LastFrameTime < 1 and getTimer() - Global.LastFrameTime >= 0)
            continue;
        avgHZ = (avgHZ + 1000.0f / (getTimer() - Global.LastFrameTime)) / 2.0;
        //std::cout << "one more logic frame done in " << getTimer() - Global.LastFrameTime << "ms\n";
    }
}


int main() {
    //SDL_SetMainReady();
    
    //SDL_Init(SDL_INIT_EVERYTHING);
    std::mutex locktite;
    Global.CurrentState = std::make_shared<MainMenu>();
    for(int i = 0; i < Global.GamePath.size(); i++) {
        if (Global.GamePath[i] == '\\')
            Global.GamePath[i] = '/';
    }
    SetTraceLogLevel(LOG_WARNING);
    InitAudioDevice();
    
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(Global.Width, Global.Height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetAudioStreamBufferSizeDefault(128);
    

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

    /*unsigned char pixels[1 * 1 * 4];
    memset(pixels, 0x00, sizeof(pixels));
    
    GLFWimage image;
    image.width = 1;
    image.height = 1;
    image.pixels = pixels;
    
    GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);*/


    int wx, wy;
    GLFWwindow* window = (GLFWwindow*)GetWindowGL();
    std::cout << "set window\n";
    std::cout << window << "\n";
    glfwGetWindowSize(window, &wx, &wy);
    std::cout << "get window\n";
    std::cout << wx << " " << wy << "\n";
    glfwSetWindowTitle (window, "osus? osus.");
    std::cout << "set name\n";
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSwapInterval( 0 );

    /*int display = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
    ToggleFullscreen();
    SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));*/
    /*glfwSetCursor(window, cursor);

    SDL_SetCursor(Global.amog);*/

    std::thread geym(GameLoop);

    while(!WindowShouldClose()){
        lastFrame = getTimer();
        //PollInputEvents(); //IF I CALL THIS FUNCTION THE GAME BASICALLY BREAKS
        
        if (IsKeyPressed(KEY_F) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
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
        }
        
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
        GetMouse();
        GetKeys();
        //double what = getTimer();
        //updateMouseTrail();
        renderMouse(); 
        //std::cout << getTimer() - what << std::endl;
        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %.3f",  avgFPS), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("TPS: %.3f",  avgHZ), {ScaleCordX(5), ScaleCordY(35)}, Scale(15), Scale(1), GREEN);
        glfwGetWindowSize(window, &Global.glfwWindowSizeX, &Global.glfwWindowSizeY);
        glfwGetWindowPos(window, &Global.glfwWindowPosX, &Global.glfwWindowPosY);

        locktite.unlock();
        SDL_SetWindowPosition(Global.win, Global.glfwWindowPosX, Global.glfwWindowPosY);
        SDL_SetWindowSize(Global.win, Global.glfwWindowSizeX, Global.glfwWindowSizeY);
        rlDrawRenderBatchActive();
        SwapScreenBuffer();
        PollInputEvents();
        //EndDrawing();
        while(getTimer() - lastFrame < 1000.0/144.0 and getTimer() - lastFrame >= 0)
            continue;
        int x;
        int y; 
        /*SDL_GetWindowPosition(Global.win,
                           &x, &y);
        std::cout << x << " " << y << std::endl;*/
        avgFPS = (avgFPS + 1000.0f / (getTimer() - lastFrame)) / 2.0;
        //std::cout << Mx << " " << My << std::endl;
    }
    geym.detach();

    UnloadTexture(Global.OsusLogo);
    UnloadFont(Global.DefaultFont);
    UnloadShader(Global.shdrOutline);
    CloseWindow();
    //quitSDL();
}

