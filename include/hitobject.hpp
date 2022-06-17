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
    float beatLength;
    float meter;
    float sampleSet;
    float sampleIndex;
    float volume;
    bool uninherited;
    int effects;
    float sliderSpeedOverride = 1;
};

struct HitObjectData{
    int x;
    int y;
    int time;
    int type;
    int hitSound;
    int point = 0;
    char curveType;
    std::vector<std::pair<int,int> > curvePoints;
    int slides;
    float length;
    std::vector<int> edgeSounds;
    std::vector<std::pair<int, int> > edgeSets;
    int endTime;
    int normalSet;
    int additionSet;
    int index;
    int volume;
    std::string filename;
    bool startingACombo;
    int skipComboColours = 0;
    int comboNumber;
    bool useDefaultHitSound;
    std::vector<int> colour;
    timingSettings timing;
};

class HitObject{
    public:
        
        HitObject(){};
        virtual ~HitObject() = default;
        virtual void init(){}
        virtual void render(){}
        virtual void render_combo(){}
        virtual void update(){}
        virtual void dead_update(){}
        virtual void dead_render(){}
        HitObjectData data;
        float x,y;
    private:
        
};

class Circle : public HitObject{
    public:
        Circle(HitObjectData data);
        void init() override; 
        void render() override;
        void render_combo() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
    private:

};

class Slider : public HitObject{
    public:
        Slider(HitObjectData data);
        void init() override; 
        void render() override;
        void render_combo() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
        bool is_hit_at_first = false;
        int demoPuan = 0;
        bool state = true;
        std::vector<Vector2> renderPoints; 
        float position = 0;
        bool repeat = false;
        bool repeat2 = false;
        int curRepeat = 0;
    private:
        int tickCount;
        float timer;
        std::vector<Vector2> edgePoints; 
        RenderTexture2D sliderTexture;
        float minX = static_cast<float>(INT_MAX), minY = static_cast<float>(INT_MAX), maxX = INT_MIN, maxY = INT_MIN;
};
