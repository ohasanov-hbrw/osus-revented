#include <math.h>
#include <vector>
#include <algorithm>
#include "raylib.h"
#include "globals.hpp"

Globals Global;

float Scale(float a){
    return a * Global.Scale;
}

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(Global.Width, Global.Height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(Global.FPS);
    Font DefaultFont = LoadFont("resources/Aller_Rg.ttf");
    SetTextureFilter(DefaultFont.texture, TEXTURE_FILTER_BILINEAR);
    HideCursor();

    while(!WindowShouldClose()){
        Global.Scale = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
        Global.ZeroPoint = {GetScreenWidth() / 2.0f - (Global.Scale * 320.0f), GetScreenHeight() / 2.0f - (Global.Scale * 240.0f)};
        Global.MouseInFocus = CheckCollisionPointRec(GetMousePosition(), (Rectangle){Global.ZeroPoint.x, Global.ZeroPoint.y, 640.0f * Global.Scale, 480.0f * Global.Scale});
        if(!Global.MouseInFocus)
            Global.MousePosition = {0,0};
        else
            Global.MousePosition = {(GetMouseX() - Global.ZeroPoint.x) / Global.Scale, (GetMouseY() - Global.ZeroPoint.y) / Global.Scale};
        BeginDrawing();
        ClearBackground({ 30, 0, 60, 255 });

        //DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, RED);
        //DrawRectangle(20 * Global.Scale + Global.ZeroPoint.x, 20 * Global.Scale + Global.ZeroPoint.y, 30 * Global.Scale, 30 * Global.Scale, BLUE);

        DrawCircle(GetMouseX(),GetMouseY(), 3.0f * Global.Scale, GREEN);

        DrawTextEx(DefaultFont, TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), {Scale(5), Scale(25)}, Scale(20), 1, LIGHTGRAY);
        DrawTextEx(DefaultFont, TextFormat("Scale: %.3f", Global.Scale), {Scale(5), Scale(45)}, Scale(20), 1, LIGHTGRAY);
        DrawTextEx(DefaultFont, TextFormat("MX: %.3f, MY: %.3f", Global.MousePosition.x, Global.MousePosition.y), {Scale(5), Scale(65)}, Scale(20), 1, LIGHTGRAY);
        DrawTextEx(DefaultFont, TextFormat("MouseFocus: %d", (int) Global.MouseInFocus), {Scale(5), Scale(85)}, Scale(20) , 1, LIGHTGRAY);
        DrawFPS(Scale(5), Scale(5));
        EndDrawing();
    }

    CloseWindow();
}

