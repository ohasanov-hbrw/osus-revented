#include "raylib.h"
#include "test_header.hpp"

int main() {
    InitWindow(800, 450, "raylib [core] example - basic window");
    Something::AA();
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
}
