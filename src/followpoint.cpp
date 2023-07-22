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

    if(gm->currentTime*1000.0f < startTime or gm->currentTime*1000.0f > endTime2){
        shouldRender = false;
    }   
}

void FollowPoint::render(){

    if(shouldRender){
        bool newWay = true;
        if(newWay){
            for(int i = 0; i < points.size(); i++){
                float range = 16.0f / distance;
                float opacity1 = 0.0f;
                
                if (startLoc <= points[i].z - range and endLoc <= points[i].z + range){
                    opacity1 = (endLoc - (points[i].z - range)) / (range * 2);
                }
                if (startLoc > points[i].z - range and endLoc > points[i].z + range){
                    opacity1 = ((points[i].z + range) - startLoc) / (range * 2);
                }
                if (startLoc > points[i].z - range and endLoc <= points[i].z + range){
                    opacity1 = (endLoc - startLoc) / (range * 2);
                }
                if(startLoc <= points[i].z - range and endLoc > points[i].z + range){
                    opacity1 = 1.0f;
                }
                if (startLoc <= points[i].z - range and endLoc <= points[i].z - range){
                    opacity1 = 0.0f;
                }
                if (startLoc > points[i].z + range and endLoc > points[i].z + range){
                    opacity1 = 0.0f;
                }
                int opacity = (int)(128.0f * (opacity1));
                DrawCircle(ScaleCordX(points[i].x), ScaleCordY(points[i].y), Scale(3), {255,200,255,opacity});

            }
            //std::cout << std::endl;
        }
        else{
            Vector2 start = lerp({startX, startY}, {endX, endY}, startLoc);
            Vector2 end = lerp({startX, startY}, {endX, endY}, endLoc);
            DrawLineEx(ScaleCords(start), ScaleCords(end), Scale(3), {255,200,255,128});
        }
    }
}
