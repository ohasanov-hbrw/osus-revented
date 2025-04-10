#pragma once
#include <vector>
#include <string>

struct Event
{
    int eventType;
    int startTime;

    std::string filename;
    int xOffset;
    int yOffset;

    int endTime;
};
