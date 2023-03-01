#define WINVER 0x0500
#define _WIN32_WINNT 0x500

#define GetTickCount64() GetTickCount()

#define OPENGL_SPOOF

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
double Mx = 0;
double My = 0;





//hello from arch!
Globals Global;

static void cursorCall(GLFWwindow* window, double xpos, double ypos){
    Mx = xpos;
    My = ypos;
    //std::cout << "callback\n";
}

void GameLoop(){
    //while(true){
        //PollInputEvents();
        
        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        
        Global.CallbackMouse.x = Mx;
        Global.CallbackMouse.y = My;

        GetScale();
        GetMouse();
        GetKeys();

        updateMouseTrail();
        updateUpDown();
        Global.CurrentState->update();
        while(getTimer() - Global.LastFrameTime < 1 and getTimer() - Global.LastFrameTime >= 0)
            continue;
        //std::cout << "one more logic frame done in " << getTimer() - Global.LastFrameTime << "ms\n";
    //}
}



/*
if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
#if !defined(PLATFORM_WEB)
static void WindowMaximizeCallback(GLFWwindow* window, int maximized);                     // GLFW3 Window Maximize Callback, runs when window is maximized
#endif
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                      // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowFocusCallback(GLFWwindow *window, int focused);                          // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window
// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                            // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);       // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
#endif
*/


int main() {

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

    SetTargetFPS(Global.FPS);
    Global.DefaultFont = LoadFont("resources/telegrama_render.otf");
    Global.OsusLogo = LoadTexture("resources/osus.png");
    Global.shdrOutline = LoadShader(0, TextFormat("resources/shaders/glsl%i/outline.fs", 330));

    
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
    

    //CORE.Window.handle;
    int wx, wy;
    GLFWwindow* window = (GLFWwindow*)GetWindowGL();
    std::cout << "set window\n";
    std::cout << window << "\n";
    glfwGetWindowSize(window, &wx, &wy);
    std::cout << "get window\n";
    std::cout << wx << " " << wy << "\n";
    glfwSetWindowTitle (window, "osus?");
    std::cout << "set name\n";
    //glfwSetCursorPosCallback(window, NULL);
    std::cout << "set to null\n";
    glfwSetCursorPosCallback(window, cursorCall);
    std::cout << "set callback\n";

    //std::thread geym(GameLoop);

    while(!WindowShouldClose()){
        while(getTimer() - lastFrame < 1000.0/144.0 and getTimer() - lastFrame >= 0){
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
            PollInputEvents();
            GameLoop();
            //glfwPollEvents();
        }
        lastFrame = getTimer();
        BeginDrawing();
        ClearBackground(Global.Background);
        Global.CurrentState->render();
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse();
        /*DrawTextEx(Global.DefaultFont, TextFormat("FPS: %.3f",  1000.0/Timer), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("Rough MS per game loop: %.3f",  Timer/(double)num), {ScaleCordX(5), ScaleCordY(35)}, Scale(15), Scale(1), GREEN);*/
        rlDrawRenderBatchActive();
        SwapScreenBuffer();
        //std::cout << Mx << " " << My << std::endl;
    }
    //geym.detach();
    UnloadTexture(Global.OsusLogo);
    UnloadFont(Global.DefaultFont);
    UnloadShader(Global.shdrOutline);
    CloseWindow();
}

