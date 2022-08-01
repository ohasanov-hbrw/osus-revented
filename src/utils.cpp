#include <utils.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <globals.hpp>
#include <gamemanager.hpp>
#include <iostream>

void updateUpDown(){
    //Get the current state of the mouse wheel
    Global.Wheel = GetMouseWheelMove();
    if(IsKeyPressed(KEY_DOWN)){
        //If the down key is pressed, start a timer so that we can simulate a fast mouse wheel movement
        Global.Wheel = -1;
        Global.FrameTimeCounterWheel = -170;
    }
    if(IsKeyPressed(KEY_UP)){
        //The same deal as above but for the up key
        Global.Wheel = 1;
        Global.FrameTimeCounterWheel = -170;
    }
    Global.FrameTimeCounterWheel += GetFrameTime()*1000.0f;
    while(Global.FrameTimeCounterWheel > 50.0f){
        //If the keys are still down, trigger a wheel movement every 50 milliseconds
        Global.FrameTimeCounterWheel -= 50.0f;
        if(IsKeyDown(KEY_UP))
            Global.Wheel = 1;
        if(IsKeyDown(KEY_DOWN))
            Global.Wheel = -1;
    }
}

void GetScale(){
    //Get the scale and also get the current offset for the zero point of the game area
    Global.Scale = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
    Global.ZeroPoint = {GetScreenWidth() / 2.0f - (Global.Scale * 320.0f), GetScreenHeight() / 2.0f - (Global.Scale * 240.0f)};
}

void GetMouse(){
    //Get the mouse position and also check if it is in the game area
    Global.MouseInFocus = CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()});
    if(!Global.MouseInFocus){
        Global.MousePosition = {0,0};
        //if it isnt in the game area, show the cursor
        if(IsCursorHidden())
            ShowCursor();
    }
    else {
        Global.MousePosition = {(GetMouseX() - Global.ZeroPoint.x) / Global.Scale, (GetMouseY() - Global.ZeroPoint.y) / Global.Scale};
        //if it is in the game area, hide the cursor
        if(!IsCursorHidden())
            HideCursor();
    }
}

void GetKeys(){
    //Get all of the keys an store this data into some variables
    if(IsKeyPressed(KEY_Z) or IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        Global.Key1P = true;
    else
        Global.Key1P = false;
    if(IsKeyPressed(KEY_X) or IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        Global.Key2P = true;
    else
        Global.Key2P = false;
    if(IsKeyDown(KEY_Z) or IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        Global.Key1D = true;
    else
        Global.Key1D = false;
    if(IsKeyDown(KEY_X) or IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        Global.Key2D = true;
    else
        Global.Key2D = false;
    if(IsKeyReleased(KEY_Z) or IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        Global.Key1R = true;
    else
        Global.Key1R = false;
    if(IsKeyReleased(KEY_X) or IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
        Global.Key2R = true;
    else
        Global.Key2R = false;
}

float Scale(float a){
    //Basic scaling function
    return a * Global.Scale;
}

Rectangle ScaleRect(Rectangle a){
    //Scale the size and also move the rectangle depending on the offset
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y, a.width * Global.Scale, a.height * Global.Scale};
}

Vector2 ScaleCords(Vector2 a){
    //Scale so that the coordinates are correct for our window size and also move them based on the offset
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y};
}

float ScaleCordX(float a){
    //Do the same thing as the ScaleCords function but only for the x axis
    return a * Global.Scale + Global.ZeroPoint.x;
}

float ScaleCordY(float a){
    //Do the same thing as the ScaleCords function but only for the y axis
    return a * Global.Scale + Global.ZeroPoint.y;
}

Vector2 GetCenter(Rectangle a){
    //Get the center of a rectangle
    return {a.x + a.width / 2, a.y + a.height / 2};
}
Vector2 GetRaylibOrigin(Rectangle a){
    //Move the coordinates so that the center of the rectangle is where we want it to be for the rendering process
    return {a.x - a.width / 2, a.y - a.height / 2};
}

Rectangle GetRaylibOriginR(Rectangle a){
    //Do the same thing as the RaylibOrigin function but this time just take the whole rectangle
    return {a.x - a.width / 2, a.y - a.height / 2, a.width, a.height};
}

float clip( float n, float lower, float upper ){
    //I have no idea what this does but it works
    n = ( n > lower ) * n + !( n > lower ) * lower;
    return ( n < upper ) * n + !( n < upper ) * upper;
}

Vector2 lerp(Vector2 a, Vector2 b, float t){
    //Thank you geeksforgeeks
    return { .x = (1 - t) * a.x + t * b.x, .y = (1 - t) * a.y + t * b.y};
}



Vector2 vectorize(float i) {
    //Really dumb but raylib needs it
    return Vector2{i, i};
}

float distance(Vector2 &p0, Vector2 &p1){
    //Its a bit heavy of a process but we dont do this frequently anyway
    return std::sqrt(std::pow(std::abs(p0.x - p1.x),2) + std::pow(std::abs(p0.y - p1.y),2));
}

int Search(std::vector<float> arr, float x,int l,int r) {
    //Basic binary search implementation
    if (r >= l) {
        int mid = (l + r) / 2;
        if (arr[mid] == x || l==r)
            return mid;
        if (arr[mid] > x)
            return Search(arr, x,l, mid - 1);
        else 
            return Search(arr, x,mid + 1, r);
    }
    else
        return l;
}

void DrawTextureCenter(Texture2D tex, float x, float y, float s, Color color){
    //Make the necessary calculations inside the function
    DrawTextureEx(tex, ScaleCords(GetRaylibOrigin({x,y,tex.width*s,tex.height*s})), 0, Scale(s), color);
}
void DrawTextureSlider(Texture2D tex, float x, float y, Color color, float s){
    //Same thing as the DrawTextureCenter() function above
    DrawTextureEx(tex, ScaleCords({x-s/2.0f,y-s/2.0f}),0,Scale(1.0f), color);
}

void DrawTextureRotate(Texture2D tex, float x, float y, float s, float r, Color color){
    //Same thing as the DrawTextureCenter() function above
    DrawTexturePro(tex, Rectangle{0,0,tex.width,tex.height}, Rectangle{ScaleCordX(x),ScaleCordY(y),Scale(tex.width*s),Scale(tex.height*s)}, Vector2{Scale(tex.width*s/2.0f), Scale(tex.height*s/2.0f)}, r, color);
}

int nthDigit(int v, int n){
    //Find the nth digit of a number... Dumb but this is probably one of the best ways :D
    while ( n > 0 ) {
        v /= 10;
        -- n;
    }
    return "0123456789"[v % 10] - '0';
}

void DrawCNumbersCenter(int n, float x, float y, float s, Color color){
    //I will need to fix this function but currently it works good enough
    GameManager* gm = GameManager::getInstance();
    int digits = log10(n) + 1;
    int i = (digits - 1) * 18;
    for(int k = 0; k < digits; k++){
        DrawTextureCenter(gm->numbers[nthDigit(n, digits-k-1)], x - (float)i * s + k * 18 * s * 2, y, s, color);
    }
}

