#include "hitobjectdefinition.hpp"

class Slider : public HitObject{
    public:
        Slider(HitObjectData data);
        void init() override; 
        void render() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
        bool is_hit_at_first = false;
        bool is_hit_at_end = false;
        int demoPuan = 0;
        bool state = true;
        std::vector<Vector2> renderPoints; 
        std::vector<Vector2> edgePoints; 
        double position = 0;
        bool repeat = false;
        bool repeat2 = false;
        int calPos = 0;
        int curRepeat = 0;
        Vector2 extraPosition;
        bool earlyhit = false;
        bool playtick = false;
        int last = 0;
        int lastblack = 0;
        bool readyToDelete = false;

        std::vector<std::vector<bool>> renderedLocations = {std::vector<std::vector<bool>>(945, std::vector<bool>(785, false))};
        

        
        

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
        
        RenderTexture2D sliderTexture;

        Shader shdrOutline;

        
        
        float minX = static_cast<float>(INT_MAX), minY = static_cast<float>(INT_MAX), maxX = INT_MIN, maxY = INT_MIN;
};