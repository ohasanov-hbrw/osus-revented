#include <hitobject.hpp>
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <limits>

//creates a circle
Circle::Circle(HitObjectData data){
    this->data = data;
    init();
}

//initilizes a circle
void Circle::init(){
    //std::cout << data.hitSound << std::endl;
    data.ex = data.x;
    data.ey = data.y;
}

//the main code that runs for every circle on screen, the collision and point manager is in the GamerManager
void Circle::update(){
    GameManager* gm = GameManager::getInstance();
    //the circle is not clickable after some time so we check that
    if(gm->currentTime*1000.0f > data.time + gm->gameFile.p50Final){
        //this is needed for dead_update to work, maybe there is a smarter way to do that
        data.time = gm->currentTime*1000.0f;
        data.point = 0;
        //resets the combo
        if(gm->clickCombo > 30){
            SetSoundVolume(&gm->SoundFilesAll.data["combobreak"], 1.0f);
            PlaySound(&gm->SoundFilesAll.data["combobreak"]);
        }
        gm->maxCombo = std::max(gm->maxCombo, gm->clickCombo);
        gm->clickCombo = 0;
        //gm->destroyHitObject(data.index);
        data.destruct = true;
    }
}

//renders the Circle
void Circle::render(){
    GameManager* gm = GameManager::getInstance();
    float approachScale = 3.5f*easeInOutCubic((1-(gm->currentTime*1000.0f - data.time + gm->gameFile.preempt)/gm->gameFile.preempt))+1.0f;
    if (approachScale <= 1.0f)
        approachScale = 1.0f;
    float clampedFade = (gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in;

    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade);
    
    //td::cout << "renderingCircle\n";
    approachScale *= 1.2f;
    DrawTextureCenter(&gm->hitCircle, data.x, data.y, gm->circlesize/(float)gm->hitCircle.width, renderColor);
    DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/(float)Global.textureSize.comboNumber, Fade(WHITE,clampedFade));
    DrawTextureCenter(&gm->hitCircleOverlay, data.x, data.y, gm->circlesize/(float)gm->hitCircleOverlay.width , Fade(WHITE,clampedFade));
    DrawTextureCenter(&gm->approachCircle, data.x, data.y, approachScale*(gm->circlesize/(float)gm->approachCircle.width) , renderColor);//renderColor);

}

//renders the "dead" Circle
void Circle::dead_render(){
    GameManager* gm = GameManager::getInstance();
    float clampedFade = (gm->gameFile.fade_in/1.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/1.0f);
    float clampedFade2 = (gm->gameFile.fade_in/4.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/4.0f);

    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    clampedFade2 = clip(clampedFade2, 0.0f, 1.0f);
    clampedFade2 = easeInOutCubic(clampedFade2);

    float scale = (gm->currentTime*1000.0f + gm->gameFile.fade_in/2.0f - data.time) / (gm->gameFile.fade_in/2.0f);
    scale = clip(scale,1,2);
    scale -= 1.0;
    scale = easeOutQuad(scale);
    scale = scale * 0.4f;
    scale += 1.0f;
    
    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade2);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade2);
    DrawTextureCenter(&gm->hitCircle, data.x, data.y, clip(scale,1,2)*gm->circlesize/(float)gm->hitCircle.width , renderColor);
    DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/(float)Global.textureSize.comboNumber, Fade(WHITE,clampedFade2));
    DrawTextureCenter(&gm->hitCircleOverlay, data.x, data.y, clip(scale,1,2)*gm->circlesize/(float)gm->hitCircleOverlay.width , Fade(WHITE,clampedFade2));
    /*if(data.point != 0)
        DrawTextureCenter(gm->selectCircle, data.x, data.y, scale*gm->circlesize/gm->selectCircle.width*(gm->selectCircle.width/128.0f) , renderColor);*/
    if(data.point == 0)
        DrawTextureCenter(&gm->hit0, data.x, data.y, (gm->circlesize/gm->hit0.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 1)
        DrawTextureCenter(&gm->hit50, data.x, data.y, (gm->circlesize/gm->hit50.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 2)
        DrawTextureCenter(&gm->hit100, data.x, data.y, (gm->circlesize/gm->hit100.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 3 && Global.textureSize.render300)
        DrawTextureCenter(&gm->hit300, data.x, data.y, (gm->circlesize/gm->hit300.width)*0.7f , Fade(WHITE,clampedFade));
}

//just gives more time to render the "dead" Circle 
void Circle::dead_update(){
    GameManager* gm = GameManager::getInstance();
    //TODO: gives 400ms for the animation to play, MAKE IT DEPENDANT TO APPROACH RATE
    if (data.time+gm->gameFile.fade_in/1.0f < gm->currentTime*1000.0f){
        //gm->destroyDeadHitObject(data.index);
        data.expired = true;
    }
}

void Circle::deinit(){
    std::vector<std::pair<short,short> > curvePoints;
    std::vector<int> edgeSounds;
    std::vector<std::pair<short, short> > edgeSets;
    std::string filename;
    std::vector<short> colour;
    std::vector<float> lengths;

    data.curvePoints.clear();
    data.edgeSounds.clear();
    data.edgeSets.clear();
    data.filename = "";
    data.colour.clear();
    data.lengths.clear();

    data.curvePoints = curvePoints;
    data.edgeSounds = edgeSounds;
    data.edgeSets = edgeSets;
    data.filename = filename;
    data.colour = colour;
    data.lengths = lengths;
}