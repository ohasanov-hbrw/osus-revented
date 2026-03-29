#pragma once
#include <vector>
#include <raylib.h>


class FollowPoint{
    public:
        FollowPoint();

        virtual ~FollowPoint() = default;
        void render();
        void update();

        std::vector<Vector3> points;

        float startTime;
        float startTime2;
        float endTime;
        float endTime2;
        float startX,startY,endX,endY;
        float distance = 0;
        float startLoc = 0.0f;
        float endLoc = 0.0f;
        double angle;
        
        Color color;
        
        bool shouldRender = false;
        bool shouldDelete = false;
    private:
        
};