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
            SetSoundVolume(gm->SoundFiles.data["combobreak"], 1.0f);
            PlaySound(gm->SoundFiles.data["combobreak"]);
        }
        gm->clickCombo = 0;
        gm->destroyHitObject(data.index);
    }
}

//renders the Circle
void Circle::render(){
    GameManager* gm = GameManager::getInstance();
    float approachScale = 3.5*(1-(gm->currentTime*1000.0f - data.time + gm->gameFile.preempt)/gm->gameFile.preempt)+1;
    if (approachScale <= 1)
        approachScale = 1;
    float clampedFade = (gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in;
    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade);
    

    DrawTextureCenter(gm->hitCircle, data.x, data.y, gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f) , renderColor);
    DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f), Fade(WHITE,clampedFade));
    DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, gm->circlesize/gm->hitCircleOverlay.width*(gm->hitCircleOverlay.width/128.0f) , Fade(WHITE,clampedFade));
    DrawTextureCenter(gm->approachCircle, data.x, data.y, approachScale*gm->circlesize/gm->approachCircle.width*(gm->approachCircle.width/128.0f) , renderColor);
}

//renders the "dead" Circle
void Circle::dead_render(){
    GameManager* gm = GameManager::getInstance();
    float clampedFade = (gm->gameFile.fade_in/1.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/1.0f);
    float clampedFade2 = (gm->gameFile.fade_in/2.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/2.0f);
    float scale = (gm->currentTime*1000.0f + gm->gameFile.fade_in/2.0f - data.time) / (gm->gameFile.fade_in/2.0f);
    scale = clip(scale,1,2);
    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade2);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade2);
    DrawTextureCenter(gm->hitCircle, data.x, data.y, clip(scale/1.5f,1,2)*gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f) , renderColor);
    //DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f), Fade(WHITE,clampedFade2));
    DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, clip(scale/1.5f,1,2)*gm->circlesize/gm->hitCircleOverlay.width*(gm->approachCircle.width/128.0f) , Fade(WHITE,clampedFade2));
    if(data.point != 0)
        DrawTextureCenter(gm->selectCircle, data.x, data.y, scale*gm->circlesize/gm->selectCircle.width*(gm->selectCircle.width/128.0f) , renderColor);
    if(data.point == 0)
        DrawTextureCenter(gm->hit0, data.x, data.y, (gm->circlesize/gm->hit0.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 1)
        DrawTextureCenter(gm->hit50, data.x, data.y, (gm->circlesize/gm->hit50.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 2)
        DrawTextureCenter(gm->hit100, data.x, data.y, (gm->circlesize/gm->hit100.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 3)
        DrawTextureCenter(gm->hit300, data.x, data.y, (gm->circlesize/gm->hit300.width)*0.7f , Fade(WHITE,clampedFade));
}

//just gives more time to render the "dead" Circle 
void Circle::dead_update(){
    GameManager* gm = GameManager::getInstance();
    //TODO: gives 400ms for the animation to play, MAKE IT DEPENDANT TO APPROACH RATE
    if (data.time+gm->gameFile.fade_in/1.0f < gm->currentTime*1000.0f){
        gm->destroyDeadHitObject(data.index);
    }
}
