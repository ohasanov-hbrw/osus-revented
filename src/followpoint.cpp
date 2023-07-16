#include "followpoint.hpp"
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <limits>

FollowPoint::FollowPoint(){

}

void FollowPoint::update(){
    GameManager* gm = GameManager::getInstance();
    if(gm->currentTime*1000.0f > startTime and gm->currentTime*1000.0f <= startTime2){
        shouldRender = true;
        startLoc = 0.0f;
        endLoc = (gm->currentTime*1000.0f - startTime) / (startTime2 - startTime);
    }
    else if(gm->currentTime*1000.0f > startTime2 and gm->currentTime*1000.0f <= endTime){
        shouldRender = true;
        startLoc = 0.0f;
        endLoc = 1.0f;
    }
    else if(gm->currentTime*1000.0f > endTime and gm->currentTime*1000.0f <= endTime2){
        shouldRender = true;
        startLoc = 1.0f - (endTime2 - gm->currentTime*1000.0f) / (endTime2 - endTime);
        endLoc = 1.0f;
    }
    else if(gm->currentTime*1000.0f > endTime2){
        shouldDelete = true;
        shouldRender = false;
    }
}

void FollowPoint::render(){

    if(shouldRender){
        Vector2 start = lerp({startX, startY}, {endX, endY}, startLoc);
        Vector2 end = lerp({startX, startY}, {endX, endY}, endLoc);
        DrawLineEx(ScaleCords(start), ScaleCords(end), Scale(3), {255,200,255,128});

    }
}
