#include <math.h>
#include <vector>
#include <algorithm>
#include <utils.hpp>
#include "raylib.h"
#include "globals.hpp"
#include <gui.hpp>


Button DemoButton ({320,240}, {69,31}, RED, "Hello World", BLACK, 10);

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

    std::vector<Vector2> MouseTrail(Global.MouseTrailSize, {-10,-10});
    float FrameTimeCounter = 0;
    int index = 0;
    
    while(!WindowShouldClose()){
        
        FrameTimeCounter += GetFrameTime()*1000.0f;

        while(FrameTimeCounter > 1.0f){
            FrameTimeCounter -= 1.0f;
            index++;
            index = index % Global.MouseTrailSize;
            MouseTrail[index] = Global.MousePosition;
        }

        GetScale();
        GetMouse();
        GetKeys();

        DemoButton.update();

        BeginDrawing();

        ClearBackground(Global.Background);

        DrawRectangleV(Global.ZeroPoint, {640.0f * Global.Scale, 480.0f * Global.Scale}, BLACK);
        //DrawRectangle(20 * Global.Scale + Global.ZeroPoint.x, 20 * Global.Scale + Global.ZeroPoint.y, 30 * Global.Scale, 30 * Global.Scale, BLUE);
        
        DemoButton.render();
        
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, 255 * (int)Global.Key1P , 255 * (int)Global.Key1D, 255});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, 255 * (int)Global.Key2P , 255 * (int)Global.Key2D, 255});

        Vector2 LastPosition = {-10,-10};
        if(Global.MouseInFocus){
            DrawCircleV(ScaleCords(Global.MousePosition), Scale(3.0f), GREEN);
            for(int i = Global.MouseTrailSize - 1; i >= 0; i--){
                if(MouseTrail[(index+i) % Global.MouseTrailSize].x != LastPosition.x or MouseTrail[(index+i) % Global.MouseTrailSize].y != LastPosition.y ){ 
                    LastPosition = MouseTrail[(index+i) % Global.MouseTrailSize];
                    Color MouseTrailer = {0,255,0,255 - i*(255/Global.MouseTrailSize)*255};
                    DrawCircleV(ScaleCords(MouseTrail[(index+i) % Global.MouseTrailSize]), Scale(3.0f), MouseTrailer);
                }
            }
        }

        DrawTextEx(Global.DefaultFont, TextFormat("Width: %d, Height: %d", GetScreenWidth(), GetScreenHeight()), {ScaleCordX(5), ScaleCordY(25)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("Scale: %.3f", Global.Scale), {ScaleCordX(5), ScaleCordY(40)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("MX: %.3f, MY: %.3f", Global.MousePosition.x, Global.MousePosition.y), {ScaleCordX(5), ScaleCordY(55)}, Scale(15), Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("MouseFocus: %d", (int) Global.MouseInFocus), {ScaleCordX(5), ScaleCordY(70)}, Scale(15) , Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("FrameTime: %f", GetFrameTime()*1000.0f), {ScaleCordX(5), ScaleCordY(85)}, Scale(15) , Scale(1), WHITE);
        DrawTextEx(Global.DefaultFont, TextFormat("Index: %d", index), {ScaleCordX(5), ScaleCordY(100)}, Scale(15) , Scale(1), WHITE);

        DrawFPS(Scale(5), Scale(5));

        EndDrawing();
    }

    CloseWindow();
}

