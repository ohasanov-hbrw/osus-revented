#include "followpoint.hpp"
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <limits>
//This file includes functions for updating and drawing the followpoints

FollowPoint::FollowPoint(){

}

//Updating the state of a single followpoint
void FollowPoint::update(){
    GameManager* gm = GameManager::getInstance();
    //Initial expansion of the followpoints 
    if(gm->currentTime*1000.0f > startTime and gm->currentTime*1000.0f <= startTime2){
        shouldRender = true;
        startLoc = 0.0f;
        endLoc = (gm->currentTime*1000.0f - startTime) / (startTime2 - startTime);
    }
    //Drawing the full followpoints
    else if(gm->currentTime*1000.0f > startTime2 and gm->currentTime*1000.0f <= endTime){
        shouldRender = true;
        startLoc = 0.0f;
        endLoc = 1.0f;
    }
    //Compression of the followpoints
    else if(gm->currentTime*1000.0f > endTime and gm->currentTime*1000.0f <= endTime2){
        shouldRender = true;
        startLoc = 1.0f - (endTime2 - gm->currentTime*1000.0f) / (endTime2 - endTime);
        endLoc = 1.0f;
    }
    //Setting the points to be deletable
    else if(gm->currentTime*1000.0f > endTime2){
        shouldDelete = true;
        shouldRender = false;
    }
    //We dont need to render anything if its not the time for the followpoints
    if(gm->currentTime*1000.0f < startTime or gm->currentTime*1000.0f > endTime2){
        shouldRender = false;
    }   
}

void FollowPoint::render(){
    if(shouldRender){
        //The new way includes actual points, not just lines
        bool newWay = true;
        if(newWay){
            for(int i = 0; i < points.size(); i++){
                float range = 16.0f / distance;
                float opacity1 = 0.0f;
                
                //Calculating the opacity of the point, not optimal but whatever
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
                
                //Drawing a "circle" with 4 segments. Basically a diamond. The 3ds can't really handle more lol
                DrawCircleWithDepth((Vector2){ScaleCordX(points[i].x), ScaleCordY(points[i].y)}, Scale(3), 4, 0, {255,200,255,opacity});

            }
        }
        else{
            //Basic line
            Vector2 start = lerp({startX, startY}, {endX, endY}, startLoc);
            Vector2 end = lerp({startX, startY}, {endX, endY}, endLoc);
            DrawLineEx(ScaleCords(start), ScaleCords(end), Scale(3), {255,200,255,128});
        }
    }
}
