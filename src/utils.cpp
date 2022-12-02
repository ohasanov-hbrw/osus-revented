#include <utils.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <globals.hpp>
#include <gamemanager.hpp>
#include <iostream>
#include <ctype.h>
#include <parser.hpp>





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
    if(IsKeyPressed(KEY_Z) or (Global.enableMouse and IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        Global.Key1P = true;
    else
        Global.Key1P = false;
    if(IsKeyPressed(KEY_X) or (Global.enableMouse and IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
        Global.Key2P = true;
    else
        Global.Key2P = false;
    if(IsKeyDown(KEY_Z) or (Global.enableMouse and IsMouseButtonDown(MOUSE_BUTTON_LEFT)))
        Global.Key1D = true;
    else
        Global.Key1D = false;
    if(IsKeyDown(KEY_X) or (Global.enableMouse and IsMouseButtonDown(MOUSE_BUTTON_RIGHT)))
        Global.Key2D = true;
    else
        Global.Key2D = false;
    if(IsKeyReleased(KEY_Z) or (Global.enableMouse and IsMouseButtonReleased(MOUSE_BUTTON_LEFT)))
        Global.Key1R = true;
    else
        Global.Key1R = false;
    if(IsKeyReleased(KEY_X) or (Global.enableMouse and IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)))
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
    DrawTextureEx(tex, ScaleCords({x-s/2.0f,y-s/2.0f}),0,Scale(1.0f/Global.sliderTexSize), color);
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

std::string getSampleSetFromInt(int s) {
    if (s == 1) return "normal"; 
    else if (s == 2) return "soft"; 
    else if (s == 3) return "drum"; 
}

void DrawSpinnerMeter(Texture2D tex, float per){
    per = clip(per, 0.001f, 0.999f);
    float x = 0;
    float y = 0;
    float ratio = (float)tex.width / (float)tex.height;
    float defaultRatio = 640.0f / 480.0f;
    if(defaultRatio > ratio){
        x = 640.0f - 480.0f * ratio;
    }
    else{
        y = 480.0f - 640.0f / ratio;
    }
    //std::cout << tex.height*(1.0f-per) << " " << tex.height*per << " " << 480.0f*(1.0f-per) << " " << (480.0f-y)*per << std::endl;
    Rectangle source = {0,tex.height*(1.0f-per),tex.width, tex.height*per};
    DrawTexturePro(tex, Rectangle{0,0,tex.width, tex.height}, ScaleRect(Rectangle{0+x/2.0f,0+y/2.0f,640-x,480-y}), Vector2{0,0}, 0, BLACK);
    DrawTexturePro(tex, source, ScaleRect(Rectangle{0+x/2.0f,(480.0f-y)*(1.0f-per)+y/2.0f,640-x,(480.0f-y)*per}), Vector2{0,0}, 0, WHITE);
}
Vector2 getPointOnCircle(float x, float y, float radius, float angle){
    angle= (angle * M_PI) / 180;
    float xdiff = radius * cos(angle);
    float ydiff = radius * sin(angle);
    return Vector2{x + xdiff, y + ydiff};
}
void DrawTextureOnCircle(Texture2D tex, float x, float y, float rad, float s, float r, float ang, Color color){
    Vector2 pos = getPointOnCircle(x, y, rad, ang);
    DrawTextureRotate(tex, pos.x, pos.y, s, r, color);
}

void DrawTextCenter(const char *text, float x, float y, float s, Color color){
    Vector2 size = MeasureTextEx(Global.DefaultFont, text, s, 1);
    DrawTextPro(Global.DefaultFont, text, ScaleCords(Vector2{x - size.x / 2.0f, y - size.y / 2.0f}), Vector2{0,0}, 0, Scale(s), Scale(1), color);
}
void DrawTextLeft(const char *text, float x, float y, float s, Color color){
    Vector2 size = MeasureTextEx(Global.DefaultFont, text, s, 1);
    DrawTextPro(Global.DefaultFont, text, ScaleCords(Vector2{x, y - size.y / 2.0f}), Vector2{0,0}, 0, Scale(s), Scale(1), color);
}

float easeInOutCubic(float x){
    return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3) / 2.0f;
}

bool AreSame(double a, double b) {
    return std::fabs(a - b) < 0.0001f;
}

std::vector<std::string> ParseNameFolder(std::string folder){
    std::vector<std::string> output;
    if(folder[folder.size() - 1] == '/'){
        folder.pop_back();
    }
    int end = folder.size() - 1;
    int begin = folder.size() - 1;
    while(begin >= 0 and folder[begin] != '-')
        begin--;
    begin = std::max(0, begin);
    if(folder[begin] == '-'){
        output.push_back("");
        for(int i = begin + 2; i <= end; i++)
            output[output.size()-1].push_back(folder[i]);
        end = std::max(0, begin - 2);
        begin = std::max(0, begin - 2);
        while(begin >= 0 and !isdigit(folder[begin]))
            begin--;
        begin = std::max(0, begin);
        if(isdigit(folder[begin])){
            output.push_back("");
            for(int i = begin + 2; i <= end; i++)
                output[output.size()-1].push_back(folder[i]);
            end = std::max(0, begin);
            begin = std::max(0, begin);
            begin = 0;
            if(isdigit(folder[begin])){
                output.push_back("");
                for(int i = begin; i <= end; i++)
                    output[output.size()-1].push_back(folder[i]);
                end = std::max(0, begin - 2);
                begin = std::max(0, begin - 2);
            }
        }
        else if (begin == 0){
            output.push_back("");
            for(int i = begin; i <= end; i++)
                output[output.size()-1].push_back(folder[i]);
            end = std::max(0, begin);
            begin = std::max(0, begin);
        }
    }
    else{
        output.push_back(folder);
    }
    
    if(output.size() > 0)
        std::cout << output[0] << " - ";
    if(output.size() > 1)
        std::cout << output[1] << " - ";
    if(output.size() > 2)
        std::cout << output[2];
    if(output.size() == 0){
        std::cout << folder;
        output.push_back(folder);
    }
    
    std::cout << std::endl;
    return output;
}
std::vector<std::string> ParseNameFile(std::string file){
    std::vector<std::string> output;
    Parser parser = Parser();
    GameFile geym = parser.parseMetadata(file);
    output.push_back(geym.configMetadata["Title"]);
    output.push_back(geym.configMetadata["Artist"]);
    output.push_back(geym.configMetadata["Creator"]);
    output.push_back(geym.configMetadata["Version"]);
    output.push_back(geym.configMetadata["BeatmapSetID"]);
    for(int i = 0; i < output.size(); i++){
        std::cout << output[i] << " - ";
    }
    std::cout << std::endl;
    return output;
}

void initTimer(){
    Global.amogus = GetTime() * 1000.0f;
}

void pauseTimer(){
    
}

void resumeTimer(){
    
}


double getTimer(){
    return GetTime() * 1000.0f - Global.amogus;
}

void addOffsetTimer(unsigned long long int time){
    Global.pausedFor += time;
}

void updateTimer(){
    if(!Global.paused){
        
    }
}