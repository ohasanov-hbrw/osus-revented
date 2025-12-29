#pragma once
#include <vector>
#include <string>
#include <raylib.h>

struct catmullRomSegment
{
    Vector2 a;
    Vector2 b;
    Vector2 c;
    Vector2 d;
};

struct timingSettings{
    int time;
    int effects;

    float beatLength;
    float meter;
    float sampleSet;
    float sampleIndex;
    float volume;
    float sliderSpeedOverride = 1;

    bool renderTicks = true;
    bool uninherited;
};

struct HitObjectData{
    std::vector<std::pair<short, short>> curvePoints;
    std::vector<int> edgeSounds;
    std::vector<std::pair<short, short>> edgeSets;
    std::vector<short> colour;
    std::vector<float> lengths;
    std::string filename = "";

    timingSettings timing;
    Vector2 extraPos;

    int ex;
    int ey;
    int endTime;
    int hindex;
    int index;
    int skipComboColours = 0;
    int comboNumber;

    float time;
    float length;
    float totalLength;

    short x;
    short y;
    short type;
    short hitSound;
    short point = 0;
    short slides;
    short normalSet;
    short additionSet;
    short volume;

    char curveType;

    bool startingACombo;
    bool useDefaultHitSound;
    bool touch = false;
    bool PlayAddition = false;
    bool PlayCustom = false;
    bool textureReady = false;
    bool textureLoaded = false;
    bool expired = false;
    bool destruct = false;
};

class HitObject{
    public:
        HitObject(){};
        virtual ~HitObject() = default;
        virtual void init(){}
        virtual void render(){}
        virtual void update(){}
        virtual void dead_update(){}
        virtual void dead_render(){}
        virtual void deinit(){}
        HitObjectData data;
        //float x,y;
        //Music id;
    private:
        
};