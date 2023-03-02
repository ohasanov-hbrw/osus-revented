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
#include <mutex>


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
    while(true){
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
    }
}

void UnloadTextures(){
    std::cout << "UnloadingTextures" << std::endl;
    Global.GameTextures = 0;
    UnloadTexture(Global.gameManager->hitCircleOverlay);
    UnloadTexture(Global.gameManager->selectCircle);
    UnloadTexture(Global.gameManager->hitCircle);
    UnloadTexture(Global.gameManager->sliderscorepoint);
    UnloadTexture(Global.gameManager->approachCircle);
    UnloadTexture(Global.gameManager->hit300);
    UnloadTexture(Global.gameManager->hit100);
    UnloadTexture(Global.gameManager->hit50);
    UnloadTexture(Global.gameManager->hit0);
    UnloadTexture(Global.gameManager->sliderb);
    UnloadTexture(Global.gameManager->sliderin);
    UnloadTexture(Global.gameManager->sliderout);
    UnloadTexture(Global.gameManager->reverseArrow);
    UnloadTexture(Global.gameManager->spinnerBottom);
    UnloadTexture(Global.gameManager->spinnerTop);
    UnloadTexture(Global.gameManager->spinnerCircle);
    UnloadTexture(Global.gameManager->spinnerApproachCircle);
    UnloadTexture(Global.gameManager->spinnerMetre);
    for(int i = 0; i < 10; i++){
        UnloadTexture(Global.gameManager->numbers[i]);
    }
    std::string key;
    for(std::map<std::string, Texture2D>::iterator it = Global.gameManager->backgroundTextures.data.begin(); it != Global.gameManager->backgroundTextures.data.end(); ++it){
        key = it->first;
        std::cout << "Removed: " << it->first << "\n";
        UnloadTexture(Global.gameManager->backgroundTextures.data[key]);
    }

    Global.gameManager->backgroundTextures.data.clear();
    Global.gameManager->backgroundTextures.pos.clear();
    Global.gameManager->backgroundTextures.loaded.clear();
}

void LoadTextures(){
    Global.gameManager->sliderin = LoadTexture("resources/sliderin.png");
    Global.gameManager->sliderout = LoadTexture("resources/sliderout.png");
    Global.gameManager->loadDefaultSkin(Global.selectedPath); // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
    Global.gameManager->loadGameSkin(Global.selectedPath); // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
    if(!IsKeyDown(KEY_S)){
        Global.gameManager->loadBeatmapSkin(Global.selectedPath); // LOADING THE BEATMAP SKIN USING A SEPERATE FUNCTION
    }
    GenTextureMipmaps(&Global.gameManager->hit0);
    SetTextureFilter(Global.gameManager->hit0, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->hit50);
    SetTextureFilter(Global.gameManager->hit50, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->hit100);
    SetTextureFilter(Global.gameManager->hit100, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->hit300);
    SetTextureFilter(Global.gameManager->hit300, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->approachCircle);
    SetTextureFilter(Global.gameManager->approachCircle, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->selectCircle);
    SetTextureFilter(Global.gameManager->selectCircle, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->hitCircleOverlay);
    SetTextureFilter(Global.gameManager->hitCircleOverlay, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->hitCircle);
    SetTextureFilter(Global.gameManager->hitCircle, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&Global.gameManager->sliderb);
    SetTextureFilter(Global.gameManager->sliderb, TEXTURE_FILTER_TRILINEAR );


    SetTextureFilter(Global.gameManager->sliderin, TEXTURE_FILTER_POINT);
    SetTextureFilter(Global.gameManager->sliderout, TEXTURE_FILTER_POINT);

    GenTextureMipmaps(&Global.gameManager->sliderscorepoint);
    SetTextureFilter(Global.gameManager->sliderscorepoint, TEXTURE_FILTER_TRILINEAR );

    GenTextureMipmaps(&Global.gameManager->reverseArrow);
    SetTextureFilter(Global.gameManager->reverseArrow, TEXTURE_FILTER_TRILINEAR );
    for(int i = 0; i < 10; i++){
        GenTextureMipmaps(&Global.gameManager->numbers[i]);
        SetTextureFilter(Global.gameManager->numbers[i], TEXTURE_FILTER_TRILINEAR );  //OPENGL1.1 DOESNT SUPPORT THIS
    }

    Global.gameManager->backgroundTextures.data.clear();
	Global.gameManager->backgroundTextures.pos.clear();
	Global.gameManager->backgroundTextures.loaded.clear();

	std::vector<std::string> files;
	files.clear();
    
	Global.Path = Global.gameManager->lastPath + '/';
	files = ls(".png");
	std::vector<std::string> files2 = ls(".jpg");
	std::vector<std::string> files3 = ls(".jpeg");
	files.insert(files.end(), files2.begin(), files2.end());
	files.insert(files.end(), files3.begin(), files3.end());

	for(int i = 0; i < (int)Global.gameManager->gameFile.events.size(); i++){
		if(Global.gameManager->gameFile.events[i].eventType == 0){
			std::cout << "Time: " << Global.gameManager->gameFile.events[i].startTime << "ms - Filename: " << Global.gameManager->gameFile.events[i].filename << '.' << std::endl;
			if(Global.gameManager->gameFile.events[i].startTime == 0){
				Global.gameManager->gameFile.events[i].startTime -= 7000;
				std::cout << "Time changed to: " << Global.gameManager->gameFile.events[i].startTime << std::endl;
			}
		}
	}
	

	std::cout << "Found this many files: " << files.size() << std::endl;
	for(int i = 0; i < files.size(); i++){
		std::cout << files[i] << std::endl;
	}

	for(int i = 0; i < files.size(); i++){
		for(int j = 0; j < (int)Global.gameManager->gameFile.events.size(); j++){
			if(Global.gameManager->gameFile.events[j].eventType == 0){
				std::cout << "attempting to load a background\n";
				int t = Global.gameManager->gameFile.events[j].filename.size() - 1;
				while(Global.gameManager->gameFile.events[j].filename[t] == ' ' and t >= 0){
					Global.gameManager->gameFile.events[j].filename.pop_back();
					t--;
				}
				t = 0;
				while(Global.gameManager->gameFile.events[j].filename[t] == ' ' and Global.gameManager->gameFile.events[j].filename.size() > 0){
					Global.gameManager->gameFile.events[j].filename.erase(Global.gameManager->gameFile.events[j].filename.begin());
				}
				std::cout << "finding function returned: " << files[i].rfind(Global.gameManager->gameFile.events[j].filename, 0) << " for: " << Global.gameManager->gameFile.events[j].filename << " and " << files[i] << std::endl;
				if(files[i].rfind(Global.gameManager->gameFile.events[j].filename, 0) == 0){
					std::cout << "WHAT DA HEEEEEEEEEELLLLLLLLLLLLL" << std::endl;
					Image image = LoadImage((Global.Path + files[i]).c_str());
					Global.gameManager->backgroundTextures.data[Global.gameManager->gameFile.events[j].filename] = LoadTextureFromImage(image);
					UnloadImage(image); 
					Global.gameManager->backgroundTextures.pos[Global.gameManager->gameFile.events[j].filename] = {Global.gameManager->gameFile.events[j].xOffset, Global.gameManager->gameFile.events[j].yOffset};
					if(Global.gameManager->backgroundTextures.data[Global.gameManager->gameFile.events[j].filename].width != 0){
						Global.gameManager->backgroundTextures.loaded[Global.gameManager->gameFile.events[j].filename].value = true;
						std::cout << "Loaded: Background with filename: " << Global.gameManager->gameFile.events[j].filename << std::endl;
						GenTextureMipmaps(&Global.gameManager->backgroundTextures.data[Global.gameManager->gameFile.events[j].filename]);
						SetTextureFilter(Global.gameManager->backgroundTextures.data[Global.gameManager->gameFile.events[j].filename], TEXTURE_FILTER_TRILINEAR );
					}
				}
			}
		}
	}

    Global.GameTextures = 0;
}


int main() {
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
    Global.GameTextures = 0;

    //CORE.Window.handle;
    int wx, wy;
    GLFWwindow* window = (GLFWwindow*)GetWindowGL();
    std::cout << "set window\n";
    std::cout << window << "\n";
    glfwGetWindowSize(window, &wx, &wy);
    std::cout << "get window\n";
    std::cout << wx << " " << wy << "\n";
    glfwSetWindowTitle (window, "osus? osus.");
    std::cout << "set name\n";
    //glfwSetCursorPosCallback(window, NULL);
    std::cout << "set to null\n";
    glfwSetCursorPosCallback(window, cursorCall);
    std::cout << "set callback\n";

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
        PollInputEvents();
        //GameLoop();
        BeginDrawing();
        ClearBackground(Global.Background);
        locktite.lock();
        Global.CurrentState->render();
        if(Global.GameTextures == -1){
            UnloadTextures();
        }
        else if(Global.GameTextures == 1){
            LoadTextures();
        }
        locktite.unlock();
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse();
        /*DrawTextEx(Global.DefaultFont, TextFormat("FPS: %.3f",  1000.0/Timer), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("Rough MS per game loop: %.3f",  Timer/(double)num), {ScaleCordX(5), ScaleCordY(35)}, Scale(15), Scale(1), GREEN);*/
        rlDrawRenderBatchActive();
        SwapScreenBuffer();
        while(getTimer() - lastFrame < 1000.0/300.0 and getTimer() - lastFrame >= 0){
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
            //PollInputEvents();
            //GameLoop();
        }
        //std::cout << Mx << " " << My << std::endl;
    }
    geym.detach();

    UnloadTexture(Global.OsusLogo);
    UnloadFont(Global.DefaultFont);
    UnloadShader(Global.shdrOutline);
    CloseWindow();
}

