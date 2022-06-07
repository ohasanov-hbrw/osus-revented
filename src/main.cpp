#include <math.h>
#include <vector>
#include <algorithm>
#include "raylib.h"
#include "globals.hpp"

Globals global;

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(global.width, global.height, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(global.FPS);

    while(!WindowShouldClose()){
        global.scale = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
        global.zero_point = {GetScreenWidth() / 2.0f - (global.scale * 320.0f), GetScreenHeight() / 2.0f - (global.scale * 240.0f)};
        BeginDrawing();
        ClearBackground({ 30, 0, 60, 255 });
        DrawRectangleV(global.zero_point, {640.0f * global.scale, 480.0f * global.scale}, RED);
        DrawText(TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), 5, 25, 20, LIGHTGRAY);
        DrawText(TextFormat("Scale: %.3f", global.scale), 5, 45, 20, LIGHTGRAY);
        DrawFPS(5,5);
        EndDrawing();
    }

    CloseWindow();
}
