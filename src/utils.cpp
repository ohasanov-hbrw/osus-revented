#include <utils.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <globals.hpp>
#include <gamemanager.hpp>
#include <iostream>

void updateUpDown(){
    Global.Wheel = GetMouseWheelMove();
    if(IsKeyPressed(KEY_DOWN)){
        Global.Wheel = -1;
        Global.FrameTimeCounterWheel = -170;
    }
    if(IsKeyPressed(KEY_UP)){
        Global.Wheel = 1;
        Global.FrameTimeCounterWheel = -170;
    }
    Global.FrameTimeCounterWheel += GetFrameTime()*1000.0f;
    while(Global.FrameTimeCounterWheel > 50.0f){
        Global.FrameTimeCounterWheel -= 50.0f;
        if(IsKeyDown(KEY_UP))
            Global.Wheel = 1;
        if(IsKeyDown(KEY_DOWN))
            Global.Wheel = -1;
    }
}

void GetScale(){
    Global.Scale = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
    Global.ZeroPoint = {GetScreenWidth() / 2.0f - (Global.Scale * 320.0f), GetScreenHeight() / 2.0f - (Global.Scale * 240.0f)};
}

void GetMouse(){
    Global.MouseInFocus = CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()});
    if(!Global.MouseInFocus){
        Global.MousePosition = {0,0};
        if(IsCursorHidden())
            ShowCursor();
    }
    else {
        Global.MousePosition = {(GetMouseX() - Global.ZeroPoint.x) / Global.Scale, (GetMouseY() - Global.ZeroPoint.y) / Global.Scale};
        if(!IsCursorHidden())
            HideCursor();
    }
}

void GetKeys(){
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
    return a * Global.Scale;
}

Rectangle ScaleRect(Rectangle a){
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y, a.width * Global.Scale, a.height * Global.Scale};
}

Vector2 ScaleCords(Vector2 a){
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y};
}

float ScaleCordX(float a){
    return a * Global.Scale + Global.ZeroPoint.x;
}

float ScaleCordY(float a){
    return a * Global.Scale + Global.ZeroPoint.y;
}

Vector2 GetCenter(Rectangle a){
    return {a.x + a.width / 2, a.y + a.height / 2};
}
Vector2 GetRaylibOrigin(Rectangle a){
    return {a.x - a.width / 2, a.y - a.height / 2};
}

Rectangle GetRaylibOriginR(Rectangle a){
    return {a.x - a.width / 2, a.y - a.height / 2, a.width, a.height};
}

float clip( float n, float lower, float upper ){
    n = ( n > lower ) * n + !( n > lower ) * lower;
    return ( n < upper ) * n + !( n < upper ) * upper;
}

Vector2 lerp(Vector2 a, Vector2 b, float t){
    return { .x = (1 - t) * a.x + t * b.x, .y = (1 - t) * a.y + t * b.y};
}



Vector2 vectorize(float i) {
    return Vector2{i, i};
}

float distance(Vector2 &p0, Vector2 &p1){
    return std::sqrt(std::pow(std::abs(p0.x - p1.x),2) + std::pow(std::abs(p0.y - p1.y),2));
}

int Search(std::vector<float> arr, float x,int l,int r) {
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
    DrawTextureEx(tex, ScaleCords(GetRaylibOrigin({x,y,tex.width*s,tex.height*s})), 0, Scale(s), color);
}
void DrawTextureSlider(Texture2D tex, float x, float y, Color color, float s){
    DrawTextureEx(tex, ScaleCords({x-s/2.0f,y-s/2.0f}),0,Scale(1.0f), color);
}
//DrawTexturePro(gm->reverseArrow, Rectangle{0,0,gm->reverseArrow.width,gm->reverseArrow.height}, Rectangle{ScaleCordX(renderPoints[index].x),ScaleCordY(renderPoints[index].y),Scale(gm->reverseArrow.width*0.5f),Scale(gm->reverseArrow.height*0.5f)}, Vector2{Scale(gm->reverseArrow.width*0.5f/2.0f), Scale(gm->reverseArrow.height*0.5f/2.0f)}, angle, Fade(WHITE, clampedFade));

void DrawTextureRotate(Texture2D tex, float x, float y, float s, float r, Color color){
    DrawTexturePro(tex, Rectangle{0,0,tex.width,tex.height}, Rectangle{ScaleCordX(x),ScaleCordY(y),Scale(tex.width*s),Scale(tex.height*s)}, Vector2{Scale(tex.width*s/2.0f), Scale(tex.height*s/2.0f)}, r, color);
}

int nthDigit(int v, int n){
    while ( n > 0 ) {
        v /= 10;
        -- n;
    }
    return "0123456789"[v % 10] - '0';
}

void DrawCNumbersCenter(int n, float x, float y, float s, Color color){
    GameManager* gm = GameManager::getInstance();
    int digits = log10(n) + 1;
    //std::cout << gm->numbers[1].width << std::endl;
    int i = (digits - 1) * 9;
    for(int k = 0; k < digits; k++){
        DrawTextureCenter(gm->numbers[nthDigit(n, digits-k-1)], x - i + k * 18, y, s, color);
    }
}
