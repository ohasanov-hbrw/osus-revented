#include <math.h>
#include <vector>
#include <algorithm>
#include "raylib.h"
#include "globals.hpp"

Globals Global;

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(Global.Width, Global.Height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(Global.FPS);
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

        DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, RED);
        DrawRectangle(20 * Global.Scale + Global.ZeroPoint.x, 20 * Global.Scale + Global.ZeroPoint.y, 30 * Global.Scale, 30 * Global.Scale, BLUE);

        DrawCircle(GetMouseX(),GetMouseY(), 3.0f * Global.Scale, GREEN);

        DrawText(TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), 5, 25, 20, LIGHTGRAY);
        DrawText(TextFormat("Scale: %.3f", Global.Scale), 5, 45, 20, LIGHTGRAY);
        DrawText(TextFormat("MX: %.3f, MY: %.3f", Global.MousePosition.x, Global.MousePosition.y), 5, 65, 20, LIGHTGRAY);
        DrawText(TextFormat("MouseFocus: %d", (int) Global.MouseInFocus), 5, 85, 20, LIGHTGRAY);
        DrawFPS(5,5);
        EndDrawing();
    }

    CloseWindow();
}
