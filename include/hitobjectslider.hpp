#include "hitobjectdefinition.hpp"
#include "rlgl.h"
#include <climits>
class Slider : public HitObject{
    public:
        Slider(HitObjectData data);

        void init() override; 
        void render() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
        void deinit() override; 
        void unloadTextures();

        std::vector<Vector2> renderPoints; 
        std::vector<Vector2> edgePoints; 

        double position = 0;

        Vector2 extraPosition;
        Vector2 lastPosition;

        int demoPuan = 0;
        int calPos = 0;
        int curRepeat = 0;
        int last = 0;
        int lastblack = 0;

        float texSizeXoffset = 0.0f;
        float texSizeYoffset = 0.0f;

        bool is_hit_at_first = false;
        bool is_hit_at_end = false;
        bool state = true;
        bool repeat = false;
        bool repeat2 = false;
        bool earlyhit = false;
        bool playtick = false;
        bool readyToDelete = false;
        bool durationNull = false;

    private:
        RenderTexture2D sliderTexture; 
        Shader shdrOutline;

        double sliderDuration;
        double currentDuration;
        double time;
        
        std::vector<int> tickPositions;
        std::vector<int> tickclicked;
        std::vector<int> reverseclicked;

        unsigned int VAO;

        int sliderTextureReadyForMipmaps = 0;
        int ticks;
        int ticknumber = 0;
        int reversenumber = 0;

        float timer;
        float minX = static_cast<float>(INT_MAX), minY = static_cast<float>(INT_MAX), maxX = INT_MIN, maxY = INT_MIN;

        bool inSlider = false;
};