#include <math.h>
#include <vector>
#include <algorithm>
#include <utils.hpp>
#include "raylib.h"
#include "globals.hpp"
#include "fastrender.hpp"
#include <gui.hpp>


std::vector<std::string> text = {"aaa", "bbb", "ccc", "ddd", "eee" , "fff", "ggg", "hhh", "iii", "jjj", "kkk", "lll"};
std::vector<std::string> text2 = {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "bbb"};

SelectableList DemoList ({320, 170}, {520, 60}, PURPLE, text, BLACK, 10, 15, 50);
SelectableList DemoList2 ({320, 300}, {520, 120}, PURPLE, text2, BLACK, 10, 15, 50);
Button DemoButton ({520,420}, {120,40}, PURPLE, "Select", BLACK, 15);
TextBox DemoTextbox ({360,420}, {120,40}, PURPLE, "N/A", BLACK, 15, 10);
TextBox DemoTextbox2 ({200,420}, {120,40}, PURPLE, "N/A", BLACK, 15, 10);
TextBox DemoTextbox3 ({320,240}, {540,420}, BLUE, "", BLUE, 15, 10);
TextBox DemoTextbox4 ({320,100}, {540,40}, BLUE, "Selectable List Demo", WHITE, 15, 50);

Globals Global;

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(Global.Width, Global.Height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(Global.FPS);
    Global.DefaultFont = LoadFont("resources/telegrama_render.otf");
    GenTextureMipmaps(&Global.DefaultFont.texture);
    SetTextureFilter(Global.DefaultFont.texture, TEXTURE_FILTER_TRILINEAR );
    HideCursor();
    initMouseTrail();

    DemoList.init();
    DemoTextbox.init();
    DemoTextbox2.init();
    DemoTextbox4.init();
    DemoList2.init();

    while(!WindowShouldClose()){
        GetScale();
        GetMouse();
        GetKeys();

        updateMouseTrail();
        updateUpDown();
        DemoButton.update();
        DemoList.update();
        DemoList2.update();

        if(DemoButton.action){
            DemoTextbox.text = DemoList.objects[DemoList.selectedindex].text;
            DemoTextbox2.text = DemoList2.objects[DemoList2.selectedindex].text;
            DemoTextbox.init();
            DemoTextbox2.init();
        }

        BeginDrawing();

        ClearBackground(Global.Background);

        DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, BLACK);
        //DrawRectangle(20 * Global.Scale + Global.ZeroPoint.x, 20 * Global.Scale + Global.ZeroPoint.y, 30 * Global.Scale, 30 * Global.Scale, BLUE);
        
        DemoTextbox3.render();
        DemoButton.render();
        DemoTextbox.render();
        DemoTextbox2.render();
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

