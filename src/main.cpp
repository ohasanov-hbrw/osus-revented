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


//hello from arch!
Globals Global;

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
    SetAudioStreamBufferSizeDefault(16);
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
    
    HideCursor();
    initMouseTrail();

    while(!WindowShouldClose()){
        GetScale();
        GetMouse();
        GetKeys();

        updateMouseTrail();
        updateUpDown();
        Global.CurrentState->update();

        BeginDrawing();

        ClearBackground(Global.Background);

        //DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, BLACK);
        
        Global.CurrentState->render();
        
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});

        renderMouse();

        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        //DrawTextEx(Global.DefaultFont, TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), {ScaleCordX(5), ScaleCordY(25)}, Scale(15), Scale(1), WHITE);
        //DrawTextEx(Global.DefaultFont, TextFormat("Scale: %.3f", Global.Scale), {ScaleCordX(5), ScaleCordY(40)}, Scale(15), Scale(1), WHITE);
        //DrawTextEx(Global.DefaultFont, TextFormat("MX: %.3f, MY: %.3f", Global.MousePosition.x, Global.MousePosition.y), {ScaleCordX(5), ScaleCordY(55)}, Scale(15), Scale(1), WHITE);
        //DrawTextEx(Global.DefaultFont, TextFormat("MouseWheel: %.3f", GetMouseWheelMove()), {ScaleCordX(5), ScaleCordY(70)}, Scale(15) , Scale(1), WHITE);


        EndDrawing();
    }

    UnloadTexture(Global.OsusLogo);
    UnloadFont(Global.DefaultFont);
    UnloadShader(Global.shdrOutline);
    CloseWindow();
}

