#include "menuElements.hpp"
#include "utils.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include "raylib.h"
#include "globals.hpp"


void MenuElement::init() {
    
}

void MenuElement::deinit() {
   
}

void MenuElement::update() {
    
}

void MenuElement::render() {
   
}

void ClickableObject::render() {
    if(positions.size() < 2);
    else if(positions.size() < 3){
        DrawRectangleV(ScaleCords(positions[0]), ScaleCords(positions[1]) - ScaleCords(positions[0]), baseColor);
        
    }
    else{
        std::vector<Vector2> temp = positions;
        for(int i = 0; i < temp.size(); i++){
            temp[i] = ScaleCords(temp[i]);
        }
        DrawTriangleStrip(temp.data(), positions.size(), baseColor);
        
    }
    if(text != ""){
        internalBox.Draw(horizontalAlign, verticalAlign, textColor);
    }
}

void ClickableObject::update() {
    
}

void ClickableObject::init() {

}

void ClickableObject::deinit() {
    positions.clear();
    positions.shrink_to_fit(); 
}

bool initFunctionRan = false;

void FancyScrollingList::init() {
    numberOfObjects = (int)((positions[1].y - positions[0].y) / objectDistance) + 5;
    std::cout << "nubmer of objects " << numberOfObjects << std::endl;
    objects.reserve(numberOfObjects);
    for(int i = 0; i < numberOfObjects; i++){
        objects.push_back(std::make_unique<ClickableObject>());
        objects[i].get()->positions.push_back(positions[0] + (Vector2){0, objectFreeSpace});
        objects[i].get()->positions.push_back(positions[0] + (Vector2){positions[1].x - positions[0].x, objectDistance - objectFreeSpace});
        objects[i].get()->text = objectNames.size() > 0 ? objectNames[i % objectNames.size()]: "Error, object: " + std::to_string(i);
        objects[i].get()->internalBox.SetText(objects[i].get()->text);
        objects[i].get()->internalBox.SetFontSize(textSize);
        objects[i].get()->internalBox.SetSpacing(1);
        objects[i].get()->internalBox.SetWrapWords(false);
        objects[i].get()->textRect = Rectangle{objects[i].get()->positions[0].x + textSpacing, objects[i].get()->positions[0].y + textSpacing, (objects[i].get()->positions[1].x - objects[i].get()->positions[0].x) - 2.0f*textSpacing, objects[i].get()->positions[1].y - objects[i].get()->positions[0].y - 2.0f*textSpacing};
        objects[i].get()->internalBox.SetBox(objects[i].get()->textRect);
        objects[i].get()->baseColor = baseColor;
        objects[i].get()->textColor = textColor;
    }
    updateTextBox = true;
    objectOffsetFull = 0;
    graphicalObjectOffsetFull = 0;
    currentSelection = -objectOffsetFull;
    objectOffset = 0;
    updateTexts = true;
    if(!Global.ScaleUpdated){
        initFunctionRan = true;
        update();
        initFunctionRan = false;
        std::cout << "calling initial update for list\n";
    }
    updateTexts = false;
}

void FancyScrollingList::deinit() {
    for(int i = 0; i < objects.size(); i++){
        objects[i]->deinit();
        objects[i].reset();
    }
    objects.clear();
    objects.shrink_to_fit();  
    positions.clear();
    positions.shrink_to_fit(); 
}

void FancyScrollingList::update() {
    objectOffsetFull += frameChange;
    
    if(objectOffsetFull > 0){
        objectOffsetFull = 0;
    }

    if(objectOffsetFull < 0 - std::max(1, (int)objectNames.size()) + 1){ // + 1 critical!
        objectOffsetFull = 0 - std::max(1, (int)objectNames.size()) + 1;
    }
    frameChange = 0;
    if(Global.ScaleUpdated){
        for(int i = 0; i < objects.size(); i++){
            objects[i]->deinit();
            objects[i].reset();
        }
        objects.clear();
        objects.shrink_to_fit();  
        init();
        initFunctionRan = true;
        updateTextBox = true;
        updateTexts = true;
    }
    
    objectOffsetFull += (int)trunc((objectOffset / objectDistance));


    
    //bool updateTexts = false;
    if((int)trunc((objectOffset / objectDistance)) != 0){
        updateTexts = true;
    }

    objectOffset -= trunc((objectOffset / objectDistance)) * objectDistance;

    double distance = (objectOffsetFull - graphicalObjectOffsetFull) * objectDistance + (objectOffset - graphicalObjectOffset);
    updateTextBox = true;
    if(!(std::abs(distance) < 0.5 && !initFunctionRan)){
        // Define the distance at which full speed is reached
        double maxDistance = objectDistance * 8.0; // Increased to handle fast scrolls better
        
        // Normalize distance between -1.0 and 1.0
        double t = std::max(-1.0, std::min(1.0, distance / maxDistance));
        double absT = std::abs(t);

        // Quadratic Ease In Out (faster response than Cubic)
        double easeFactor = (absT < 0.5) ? (2.0 * absT * absT) : (1.0 - std::pow(-2.0 * absT + 2.0, 2.0) / 2.0);

        double sign = (distance > 0) ? 1.0 : -1.0;
        double baseSpeed = 250.0f; // Increased base speed to eliminate lag
        
        // Calculate step, ensuring a minimum speed so it doesn't crawl at the end
        double minSpeed = 300; 
        double step = easeFactor * baseSpeed * objectDistance;
        
        if (step < minSpeed) {
            step = minSpeed; // Prevents the infinite slow crawl at the end
        }
        step*=(Global.FrameTime / 1000.f);
        graphicalObjectOffset += step * sign;
    }

    if(std::abs(distance) < 5 && !initFunctionRan)
        updateTextBox = false;



    if(initFunctionRan) initFunctionRan = false;
    int change = (int)trunc((graphicalObjectOffset / (0.5*objectDistance)));
    graphicalObjectOffsetFull += change;

    if(change != 0){
        updateTexts = true;
        updateTextBox = true;
    }

    graphicalObjectOffset -= change * objectDistance;

    float centeringOffset = objectDistance / 2.;

    currentSelection = -objectOffsetFull;

    for(int i = 0; i < objects.size(); i++){
        objects[i].get()->positions[0] = positions[0] - (Vector2){0, (positions[0].y - positions[1].y) / 2} + (Vector2){0, objectDistance * (float)(i - 1 - (int)objects.size() / 2) + objectFreeSpace + graphicalObjectOffset} + (Vector2){0, centeringOffset};
        objects[i].get()->positions[1] = positions[0] - (Vector2){0, (positions[0].y - positions[1].y) / 2} + (Vector2){positions[1].x - positions[0].x, objectDistance * (float)(i - 0 - (int)objects.size() / 2) - objectFreeSpace + graphicalObjectOffset} + (Vector2){0, centeringOffset};
        if(objects[i].get()->positions[0].y + objectFreeSpace < positions[0].y){
            objects[i].get()->baseColor = Fade(baseColor, clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y + objectFreeSpace)) / (objectDistance * 1.5)), 0.0f, 1.0f));
            objects[i].get()->textColor = Fade(textColor, clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y + objectFreeSpace)) / (objectDistance * 1.5)), 0.0f, 1.0f));
        }
        else if(objects[i].get()->positions[0].y > positions[1].y){
            objects[i].get()->baseColor = Fade(baseColor, clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) / (objectDistance * 1.5)), 0.0f, 1.0f));
            objects[i].get()->textColor = Fade(textColor, clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) / (objectDistance * 1.5)), 0.0f, 1.0f));
        }
        else{
            objects[i].get()->baseColor = baseColor;
            objects[i].get()->textColor = textColor;
        }

        double distanceFromCenter = (objects[i].get()->positions[0].y + objectDistance / 2) - (positions[0].y + ((positions[1].y - positions[0].y) / 2.0f));
        objects[i].get()->positions[0].x += (distanceFromCenter / 50.0) * (distanceFromCenter / 50.0);
        if(updateTexts){
            objects[i].get()->text = objectNames.size() > 0 ? objectNames[std::max(0, i - graphicalObjectOffsetFull) % objectNames.size()]: "Error, object: " + std::to_string(i - (int)objects.size() / 2 - graphicalObjectOffsetFull);
            objects[i].get()->internalBox.SetText(objects[i].get()->text);
        }
        if(updateTextBox){
            objects[i].get()->textRect = Rectangle{objects[i].get()->positions[0].x + textSpacing, objects[i].get()->positions[0].y + textSpacing, (objects[i].get()->positions[1].x - objects[i].get()->positions[0].x) - 2.0f*textSpacing, objects[i].get()->positions[1].y - objects[i].get()->positions[0].y - 2.0f*textSpacing};
            objects[i].get()->internalBox.SetBox(objects[i].get()->textRect);
        }
    }
    if(updateTexts)
        updateTexts = false;
    if(updateTextBox)
        updateTextBox = false;
}

void FancyScrollingList::render() {
    //DrawTextEx(&Global.DefaultFont, TextFormat("OffsetFull: %d Graphical: %.0f", graphicalObjectOffsetFull, graphicalObjectOffset), {static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(400))}, Scale(20.05), Scale(2), BLUE);
    for(int i = 0; i < objects.size(); i++){
//if (i - objectOffsetFull + hardCodedOffset >= 0)
            if(i - (int)objects.size() / 2 - graphicalObjectOffsetFull >= 0 && i - (int)objects.size() / 2 - graphicalObjectOffsetFull < std::max(1, (int)objectNames.size()) && objects[i]->positions[1].y > positions[0].y && objects[i]->positions[0].y < positions[1].y)
                objects[i]->render();
    }
}