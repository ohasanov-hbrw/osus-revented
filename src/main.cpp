#include <math.h>
#include <vector>
#include <algorithm>
#include <utils.hpp>
#include "raylib.h"
#include "globals.hpp"
#include <iostream>
#include <filesystem>
#include "fastrender.hpp"
#include <gui.hpp>

namespace fs = std::filesystem;

std::vector<std::string> text;
std::vector<std::string> text2 = {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "bbb"};
std::string path = fs::current_path();

Globals Global;

void ls(){
    text.clear();
    for (const auto & entry : fs::directory_iterator(path)){
        std::string filename = entry.path().filename();
        fs::directory_entry isDirectory(entry.path());
        if(filename[0] != '.'){
            if(isDirectory.is_directory()){
                filename.push_back('/');
                text.push_back(filename);
            }
            else if(entry.path().extension() == ".osu"){
                text.push_back(filename);
            }
            
        }
    }
    sort(text.begin(), text.end());
}

int main() {
    ls();

    SelectableList DemoList2 ({320, 300}, {520, 120}, PURPLE, text, BLACK, 10, 15, 50);
    SelectableList DemoList ({320, 170}, {520, 60}, PURPLE, text2, BLACK, 10, 15, 50);
    Button DemoButton ({520,420}, {120,40}, PURPLE, "Select", BLACK, 15);
    Button DemoButton2 ({360,420}, {120,40}, PURPLE, "Back", BLACK, 15);
    TextBox DemoTextbox3 ({320,240}, {540,420}, BLUE, "", BLUE, 15, 10);
    TextBox DemoTextbox4 ({320,100}, {540,40}, BLUE, "Selectable List Demo", WHITE, 15, 50);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(Global.Width, Global.Height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(Global.FPS);
    Global.DefaultFont = LoadFont("resources/telegrama_render.otf");
    GenTextureMipmaps(&Global.DefaultFont.texture);
    SetTextureFilter(Global.DefaultFont.texture, TEXTURE_FILTER_TRILINEAR );
    HideCursor();
    initMouseTrail();

    while(!WindowShouldClose()){
        GetScale();
        GetMouse();
        GetKeys();

        updateMouseTrail();
        updateUpDown();
        DemoButton.update();
        DemoButton2.update();
        DemoList.update();
        DemoList2.update();

        if(DemoButton.action){
            if(DemoList2.objects.size() > 0 and DemoList2.objects[DemoList2.selectedindex].text.size() > 0){
                
                if(DemoList2.objects[DemoList2.selectedindex].text[DemoList2.objects[DemoList2.selectedindex].text.size()-1] == '/'){
                    DemoList2.objects[DemoList2.selectedindex].text.pop_back();
                    path = path + '/' + DemoList2.objects[DemoList2.selectedindex].text;
                    ls();
                    DemoList2 = SelectableList({320, 300}, {520, 120}, PURPLE, text, BLACK, 10, 15, 50);
                    DemoList2.init();
                }
                else{
                    std::string temp = DemoList2.objects[DemoList2.selectedindex].text;
                    std::cout << path + temp << std::endl;
                }
            }
        }
        if(DemoButton2.action){
            while(true){
                if(path[path.size()-1] == '/'){
                    if(path.size() > 1)
                        path.pop_back();
                    break;
                }
                path.pop_back();
            }
            ls();
            DemoList2 = SelectableList({320, 300}, {520, 120}, PURPLE, text, BLACK, 10, 15, 50);
            DemoList2.init();
        }

        BeginDrawing();

        ClearBackground(Global.Background);

        DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, BLACK);
        
        DemoTextbox3.render();
        DemoButton.render();
        DemoButton2.render();
        DemoTextbox4.render();
        DemoList.render();
        DemoList2.render();
        
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, 255 * (int)Global.Key1P , 255 * (int)Global.Key1D, 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, 255 * (int)Global.Key2P , 255 * (int)Global.Key2D, 100});

        renderMouse();

        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(10)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), {ScaleCordX(5), ScaleCordY(25)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("Scale: %.3f", Global.Scale), {ScaleCordX(5), ScaleCordY(40)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("MX: %.3f, MY: %.3f", Global.MousePosition.x, Global.MousePosition.y), {ScaleCordX(5), ScaleCordY(55)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("MouseWheel: %.3f", GetMouseWheelMove()), {ScaleCordX(5), ScaleCordY(70)}, Scale(15) , Scale(1), WHITE);


        EndDrawing();
    }

    CloseWindow();
}

