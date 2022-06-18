#include <utils.hpp>
#include <math.h>
#include <vector>
#include <algorithm>
#include <globals.hpp>

void GetScale(){
    Global.Scale = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
    Global.ZeroPoint = {GetScreenWidth() / 2.0f - (Global.Scale * 320.0f), GetScreenHeight() / 2.0f - (Global.Scale * 240.0f)};
}

void GetMouse(){
    Global.MouseInFocus = CheckCollisionPointRec(GetMousePosition(), (Rectangle){Global.ZeroPoint.x, Global.ZeroPoint.y, 640.0f * Global.Scale, 480.0f * Global.Scale});
    if(!Global.MouseInFocus){
        Global.MousePosition = {-10,-10};
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