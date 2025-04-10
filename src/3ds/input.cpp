#include "input.h"
#include "globals.hpp"

bool IsKeyDown(int index){
    return (Global.ds_kHeld & index);
}

bool IsKeyPressed(int index){
    return (Global.ds_kDown & index);
}

bool IsKeyReleased(int index){
    return (Global.ds_kUp & index);
}

void PollInputEvents(){
    hidScanInput();
    Global.lastTouch = (Global.lastTouchPos.px != 0 && Global.lastTouchPos.py != 0);
    hidTouchRead(&Global.lastTouchPos);
    if(Global.lastTouchPos.px != 0 && Global.lastTouchPos.py != 0){
        Global.touch.px = Global.lastTouchPos.px;
        Global.touch.py = Global.lastTouchPos.py;
    }
    
	Global.ds_kDown = hidKeysDown();
	Global.ds_kHeld = hidKeysHeld();
    Global.ds_kUp = hidKeysUp();
    
}

int GetMouseWheelMove(){
    return 0;
}

Vector2 GetMousePosition(){
    if(Global.useTopScreen){
        return (Vector2){(Global.touch.px * 400) / 320, Global.touch.py};
    }
    return (Vector2){Global.touch.px, Global.touch.py};
}

int GetMouseX(){
    if(Global.useTopScreen){
        return (Global.touch.px * 400) / 320;
    }
    return Global.touch.px;
}

int GetMouseY(){
    return Global.touch.py;
}

//WHERE WE ARE GOING, WE NEED NO MICE!!! (ignore the top.)
bool IsMouseButtonDown(int index){
    if(index == SDL_BUTTON_RIGHT)
        return false;
    return Global.lastTouchPos.px != 0 && Global.lastTouchPos.py != 0;
}
bool IsMouseButtonPressed(int index){
    if(index == SDL_BUTTON_RIGHT)
        return false;
    if(Global.lastTouch == false && Global.lastTouchPos.px != 0 && Global.lastTouchPos.py != 0){
        return true;
    }
    return false;
}
bool IsMouseButtonReleased(int index){
    if(index == SDL_BUTTON_RIGHT)
        return false;
    if(Global.lastTouch == true && Global.lastTouchPos.px == 0 && Global.lastTouchPos.py == 0){
        return true;
    }
    return false;
}