#pragma once
#include <vector>
#include <map>
#include "hitobject.hpp"
#include "gamefile.hpp"
#include "timingpoint.hpp"
#include "event.hpp"
#include <string.h>

struct GameFile{
    std::map<std::string, std::string> configGeneral;
    std::map<std::string, std::string> configEditor;
    std::map<std::string, std::string> configMetadata;
    std::map<std::string, std::string> configDifficulty;
    std::map<std::string, std::string> configColours;
    std::vector<std::vector<int>> comboColours;
    std::vector<HitObjectData> hitObjects;
    std::vector<TimingPoint> timingPoints;
    std::vector<Event> events;
    float p300 = 79.5f;
    float p300Change = 6.0f;
    float p300Final;
    float p100 = 139.5f;
    float p100Change = 8.0f;
    float p100Final;
    float p50 = 199.5f;
    float p50Change = 10.0f;
    float p50Final;
    int preempt;
    int fade_in;
};