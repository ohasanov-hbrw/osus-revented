#include "hitobjectdefinition.hpp"

class Spinner : public HitObject{
    public:
        Spinner(HitObjectData data);
        void init() override; 
        void render() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
        void deinit() override; 
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