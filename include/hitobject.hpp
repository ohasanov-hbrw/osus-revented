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
    double beatLength;
    float meter;
    float sampleSet;
    float sampleIndex;
    float volume;
    bool uninherited;
    int effects;
    double sliderSpeedOverride = 1;
    bool renderTicks = true;
};

struct HitObjectData{
    int x;
    int y;
    double time;
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
    std::string filename = "";
    bool startingACombo;
    int skipComboColours = 0;
    int comboNumber;
    bool useDefaultHitSound;
    std::vector<int> colour;
    timingSettings timing;
    bool touch = false;
    bool PlayAddition = false;
    bool PlayCustom = false;
    std::string CustomSound = "";
    std::string AdditionSound = "";
    std::string NormalSound = "";
    std::vector<std::string> EdgeNormalSound ;
    std::vector<std::string> EdgeAdditionSound;
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
    Music id;
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
        bool is_hit_at_end = false;
        int demoPuan = 0;
        bool state = true;
        std::vector<Vector2> renderPoints; 
        double position = 0;
        bool repeat = false;
        bool repeat2 = false;
        int calPos;
        int curRepeat = 0;
        Vector2 extraPosition;
        bool earlyhit = false;
        bool playtick = false;
        
    private:
        double sliderDuration;
        double currentDuration;
        double time;
        int ticks;
        std::vector<int> tickPositions;
        bool inSlider = false;
        std::vector<int> tickclicked;
        std::vector<int> reverseclicked;
        int ticknumber = 0;
        int reversenumber = 0;
        float timer;
        std::vector<Vector2> edgePoints; 
        RenderTexture2D sliderTexture;
        float minX = static_cast<float>(INT_MAX), minY = static_cast<float>(INT_MAX), maxX = INT_MIN, maxY = INT_MIN;
};

class Spinner : public HitObject{
    public:
        Spinner(HitObjectData data);
        void init() override; 
        void render() override;
        void render_combo() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
        float renderAngle = 0;
        float totalAngle = 0;
        float neededAngle = 0;
        float lastAngle = 0;
        int startRotation = 0;
        int rotation = 0;
        bool first = false;
        int extra = 1;
        bool hit = false;
        float addedAngle = 0;
    private:


};
