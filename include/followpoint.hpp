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
        float sx,sy;
        float ex,ey;
        float startTime,endTime;
    private:
        
};