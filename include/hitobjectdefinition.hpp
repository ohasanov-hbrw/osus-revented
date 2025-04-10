#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <raylib.h>
#include <climits>

struct catmullRomSegment
{
    Vector2 a;
    Vector2 b;
    Vector2 c;
    Vector2 d;
};

struct timingSettings{
    int time;
    float beatLength;
    float meter;
    float sampleSet;
    float sampleIndex;
    float volume;
    bool uninherited;
    int effects;
    float sliderSpeedOverride = 1;
    bool renderTicks = true;
};

struct HitObjectData{
    short x;
    short y;
    int ex;
    int ey;
    float time;
    short type;
    short hitSound;
    short point = 0;
    char curveType;
    std::vector<std::pair<short,short> > curvePoints;
    short slides;
    float length;
    std::vector<int> edgeSounds;
    std::vector<std::pair<short, short> > edgeSets;
    int endTime;
    short normalSet;
    short additionSet;
    int hindex;
    int index;
    short volume;
    std::string filename = "";
    bool startingACombo;
    int skipComboColours = 0;
    int comboNumber;
    bool useDefaultHitSound;
    std::vector<short> colour;
    timingSettings timing;
    bool touch = false;
    bool PlayAddition = false;
    bool PlayCustom = false;
    std::vector<float> lengths;
    bool textureReady = false;
    bool textureLoaded = false;
    float totalLength;
    Vector2 extraPos;
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