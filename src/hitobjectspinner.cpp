#include <hitobject.hpp>
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <limits>

//creates a circle
Spinner::Spinner(HitObjectData data){
    this->data = data;
    init();
}

//initilizes a circle
void Spinner::init(){
    GameManager* gm = GameManager::getInstance();
    neededAngle = (((gm->spinsPerSecond) * (data.endTime - data.time) * 360) / 1000.0f ) / 3.0f;
    data.ex = data.x;
    data.ey = data.y;
}

//the main code that runs for every circle on screen, the collision and point manager is in the GamerManager
void Spinner::update(){
    GameManager* gm = GameManager::getInstance();
    //the circle is not clickable after some time so we check that
    float angle = atan2(240 - Global.MousePosition.y, 320 - Global.MousePosition.x) * 180.0f / PI;

    if(data.touch and gm->currentTime*1000.0f > data.time){
        if(Global.Key1P or Global.Key2P){
            first = true;
            lastAngle = angle;
            hit = true;
        }
        if((Global.Key1D or Global.Key2D) and hit){
            if(angle != lastAngle){
                if(angle > lastAngle){
                    if(angle > 120 && lastAngle < -120)
                        rotation = -1;
                    else
                        rotation = 1;
                }
                else{
                    if(angle < -120 && lastAngle > 120)
                        rotation = 1;
                    else
                        rotation = -1;
                }
                if(first && std::abs(angle - lastAngle) > 7){
                    startRotation = rotation;
                    first = false;
                }
                if(startRotation == rotation && !first){
                    addedAngle = std::abs(angle - lastAngle);
                    if((angle > 120 and lastAngle < -120)) {
                        addedAngle = std::abs(180 - angle) + std::abs(-180 - lastAngle);
                    }
                    if((angle < -120 and lastAngle > 120)){
                        addedAngle = std::abs(-180 - angle) + std::abs(180 - lastAngle);
                    }
                    /*if(addedAngle > ((477.0f * 360.0f) / (60.0f * 1000.0f)) * ((float)GetFrameTime() * 1000.0f)){
                        addedAngle = ((477.0f * 360.0f) / (60.0f * 1000.0f)) * ((float)GetFrameTime() * 1000.0f);
                    }*/
                    totalAngle += addedAngle;
                    renderAngle += rotation * addedAngle;
                    
                }
                if(!first)
                    lastAngle = angle;
            }
            //std::cout << std::abs(angle - lastAngle) << " " << startRotation << " " << rotation << "\n";
        }
        else{
            first = false;
            lastAngle = 0;
            rotation = 0;
            startRotation = -2;
        }
    }
    if(totalAngle > neededAngle + 360 * extra){
        extra++;
        gm->score += 1000;
    }
    if(gm->currentTime*1000.0f > data.endTime){
        data.time = gm->currentTime*1000.0f;
        data.point = 0;
        if(neededAngle < 360){
            data.point = 3;
            gm->score += 300 + (300 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
            gm->clickCombo++;
            gm->hit300s++;
        }
        else{
            if(totalAngle > neededAngle){
                data.point = 3;
                gm->score += 300 + (300 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
                gm->hit300s++;
            }
            else if(totalAngle >= neededAngle - 360){
                data.point = 2;
                gm->score += 100 + (100 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
                gm->hit100s++;
            }
            else if(totalAngle >= neededAngle / 4.0f){
                gm->score += 50 + (50 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
                data.point = 1;
                gm->hit50s++;
            }
            else{
                data.point = 0;
                gm->maxCombo = std::max(gm->maxCombo, gm->clickCombo);
                gm->clickCombo = 0;
                gm->hit0s++;
            }
        }
        //gm->destroyHitObject(data.index);
        data.destruct = true;
    }
}

//renders the Spinner
void Spinner::render(){
    GameManager* gm = GameManager::getInstance();
    float clampedFade = (gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in;
    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    Color spinnerColor = WHITE;
    if(!(data.touch and gm->currentTime*1000.0f > data.time)){
        spinnerColor = BLACK;
        clampedFade /= 2.0f;
    }
    if(gm->renderSpinnerBack){
        DrawSpinnerBack(&gm->spinnerBack, Fade(WHITE, clampedFade / 2.0f));
    }
    if(gm->renderSpinnerMetre)
        DrawSpinnerMeter(&gm->spinnerMetre,totalAngle/neededAngle);
    
    DrawTextureRotate(&gm->spinnerBottom, data.x, data.y, (310.0f/gm->spinnerBottom.width), 0, Fade(spinnerColor, clampedFade));
    DrawTextureRotate(&gm->spinnerTop, data.x, data.y, (310.0f/gm->spinnerTop.width), renderAngle, Fade(spinnerColor, clampedFade));
    
    if(gm->renderSpinnerCircle)
        DrawTextureRotate(&gm->spinnerCircle, data.x, data.y, (310.0f/gm->spinnerCircle.width), renderAngle, Fade(spinnerColor, clampedFade));
    if(extra > 1){
        DrawCNumbersCenter((extra - 1) * 1000, 320, 400, 1, spinnerColor);
    }
    
}

//renders the "dead" Circle
void Spinner::dead_render(){
    GameManager* gm = GameManager::getInstance();
    float clampedFade = (gm->gameFile.fade_in/1.5f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/1.5f);
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
void Spinner::dead_update(){
    GameManager* gm = GameManager::getInstance();
    //TODO: gives 400ms for the animation to play, MAKE IT DEPENDANT TO APPROACH RATE
    if (data.time+gm->gameFile.fade_in/1.5f < gm->currentTime*1000.0f){
        //gm->destroyDeadHitObject(data.index);
        data.expired = true;
    }
}

void Spinner::deinit(){
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