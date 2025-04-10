#include "hitobjectdefinition.hpp"

class Circle : public HitObject{
    public:
        Circle(HitObjectData data);
        void init() override; 
        void render() override;
        void update() override;
        void dead_update() override;
        void dead_render() override;
        void deinit() override; 
    private:

};