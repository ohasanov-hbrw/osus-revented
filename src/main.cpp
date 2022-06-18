#include <math.h>
#include <vector>
#include <algorithm>
#include <utils.hpp>
#include "raylib.h"
#include "globals.hpp"
#include "fastrender.hpp"
#include <gui.hpp>


Button DemoButton ({320,240}, {69,31}, RED, "Hello World", BLUE, 10);

Globals Global;

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(Global.Width, Global.Height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(Global.FPS);
    Global.DefaultFont = LoadFont("resources/fragile_bombers.ttf");
    SetTextureFilter(Global.DefaultFont.texture, TEXTURE_FILTER_TRILINEAR );
    HideCursor();

    while(!WindowShouldClose()){
        GetScale();
        GetMouse();
        GetKeys();

        updateMouseTrail();
        DemoButton.update();

        BeginDrawing();

        ClearBackground(Global.Background);

        DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, BLACK);
        //DrawRectangle(20 * Global.Scale + Global.ZeroPoint.x, 20 * Global.Scale + Global.ZeroPoint.y, 30 * Global.Scale, 30 * Global.Scale, BLUE);
        
        DemoButton.render();
        
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, 255 * (int)Global.Key1P , 255 * (int)Global.Key1D, 255});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, 255 * (int)Global.Key2P , 255 * (int)Global.Key2D, 255});

        renderMouse();

        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(10)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), {ScaleCordX(5), ScaleCordY(25)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("Scale: %.3f", Global.Scale), {ScaleCordX(5), ScaleCordY(40)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("MX: %.3f, MY: %.3f", Global.MousePosition.x, Global.MousePosition.y), {ScaleCordX(5), ScaleCordY(55)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("MouseFocus: %d", (int) Global.MouseInFocus), {ScaleCordX(5), ScaleCordY(70)}, Scale(15) , Scale(1), WHITE);


        EndDrawing();
    }

    CloseWindow();
}

