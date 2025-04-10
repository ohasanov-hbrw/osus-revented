#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <raylib.h>
#include <climits>


class FollowPoint{
    public:
        FollowPoint();
        virtual ~FollowPoint() = default;
        void render();
        void update();
        float startTime;
        float startTime2;
        float endTime;
        float endTime2;
        float startX,startY,endX,endY;
        bool shouldRender = false;
        bool shouldDelete = false;
        float distance = 0;
        float startLoc = 0.0f;
        float endLoc = 0.0f;
        std::vector<Vector3> points;
        float angle;
    private:
        
};