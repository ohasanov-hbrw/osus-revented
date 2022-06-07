#include <math.h>
#include <vector>
#include <algorithm>
#include "raylib.h"

float GLOBAL_SCALE = 1.0;
Vector2 ZERO_POINT = {0.0,0.0};

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 450, "osus-revented");
    SetWindowMinSize(320, 240);
    SetTargetFPS(42069);
    while(!WindowShouldClose()){
    	GLOBAL_SCALE = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
    	ZERO_POINT = {GetScreenWidth() / 2.0f - (GLOBAL_SCALE * 320.0f), GetScreenHeight() / 2.0f - (GLOBAL_SCALE * 240.0f)};
        BeginDrawing();
        ClearBackground({ 30, 0, 60, 255 });
        DrawRectangleV(ZERO_POINT, {640.0f * GLOBAL_SCALE, 480.0f * GLOBAL_SCALE}, RED);
        DrawText(TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), 5, 25, 20, LIGHTGRAY);
        DrawText(TextFormat("Scale: %f", GLOBAL_SCALE), 5, 45, 20, LIGHTGRAY);
        DrawFPS(5,5);
        EndDrawing();
    }

    CloseWindow();
}
