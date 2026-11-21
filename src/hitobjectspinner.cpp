#include <hitobject.hpp>
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"

//creates a circle
Spinner::Spinner(HitObjectData data){
    this->data = data;
    init();
}

//initilizes a circle
void Spinner::init(){
    GameManager* gm = GameManager::getInstance();
    minSpinsPerSecond = gm->spinsPerSecond;
    float duration_s = std::max(0.0f, (data.endTime - data.time) / 1000.0f);
    float spinsRequired = std::floor((duration_s * minSpinsPerSecond + 0.5f) * 2.0f) / 2.0f;
    neededAngle = spinsRequired * 360.0f;
    data.ex = data.x;
    data.ey = data.y;
}

void Spinner::reportDelta(float delta) {
    if (delta == 0.0f)
        return;
    spinAccum += delta;
    float current = spinAccum - spinAtCompletion;
    currentSpinMax = std::max(currentSpinMax, std::abs(current));
    while (currentSpinMax >= 360.0f) {
        int direction = current >= 0.0f ? 1 : -1;
        completedSpins++;
        spinAtCompletion += direction * 360.0f;
        current = spinAccum - spinAtCompletion;
        currentSpinMax = std::abs(current);
    }
    totalRotation = 360.0f * completedSpins + currentSpinMax;
    renderAngle = spinAccum;
}

//the main code that runs for every circle on screen, the collision and point manager is in the GamerManager
void Spinner::update(){
    GameManager* gm = GameManager::getInstance();
    float now = gm->currentTime*1000.0f;

    if(data.touch and now > data.time and now <= data.endTime){
        float angle = atan2(static_cast<float>(data.y) - Global.MousePosition.y,
                            static_cast<float>(data.x) - Global.MousePosition.x) * 180.0f / PI;
        if(Global.Key1D or Global.Key2D){
            float delta = angle - lastAngle;
            while(delta > 180.0f) delta -= 360.0f;
            while(delta < -180.0f) delta += 360.0f;
            // velocity bound: 477 RPM max; clamp per-frame delta so a single
            // cursor jump cannot instantly complete the spinner.
            float maxDelta = (477.0f / 60.0f) * 360.0f * (Global.FrameTime / 1000.0f) * 1.05f;
            maxDelta = clip(maxDelta, 1.0f, 180.0f);
            delta = clip(delta, -maxDelta, maxDelta);
            reportDelta(delta);
            lastAngle = angle;
        } else {
            lastAngle = angle;
        }
    }
    while (scoredSpins < completedSpins) {
        scoredSpins++;
        if (360.0f * scoredSpins <= neededAngle) {
            gm->score += 100;
        } else {
            gm->score += 1000;
            extra++;
        }
    }

    if(now > data.endTime){
        data.time = now;
        data.point = 0;
        // Very short spinners (less than one full spin required) auto complete
        if(neededAngle < 360.0f or totalRotation >= neededAngle){
            data.point = 3;
            gm->score += 300 + (300 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
            gm->clickCombo++;
            gm->hit300s++;
            gm->objectPoints.push_back(OSU_300);
        }
        else if(totalRotation >= neededAngle - 360.0f){
            data.point = 2;
            gm->score += 100 + (100 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
            gm->clickCombo++;
            gm->hit100s++;
            gm->objectPoints.push_back(OSU_100);
        }
        else if(totalRotation >= neededAngle / 4.0f){
            gm->score += 50 + (50 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
            gm->clickCombo++;
            data.point = 1;
            gm->hit50s++;
            gm->objectPoints.push_back(OSU_50);
        }
        else{
            data.point = 0;
            gm->maxCombo = std::max(gm->maxCombo, gm->clickCombo);
            gm->clickCombo = 0;
            gm->hit0s++;
            gm->objectPoints.push_back(OSU_0);
        }
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
    if(gm->renderSpinnerMetre){
        float progress = neededAngle > 0.0f ? totalRotation / neededAngle : 1.0f;
        DrawSpinnerMeter(&gm->spinnerMetre, progress);
    }

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