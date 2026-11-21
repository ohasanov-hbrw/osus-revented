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
        float totalRotation = 0;
        float neededAngle = 0;
        float minSpinsPerSecond = 0;
        int completedSpins = 0;
        int scoredSpins = 0;
        float spinAccum = 0;
        float spinAtCompletion = 0;
        float currentSpinMax = 0;

        float lastAngle = 0;
        bool tracking = false;

        int extra = 1;

    private:
        void reportDelta(float delta);
};
