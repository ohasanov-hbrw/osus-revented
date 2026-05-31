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
        internalBox.Draw(HAlign::Left, VAlign::Middle, textColor);
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


void FancyScrollingList::init() {
    numberOfObjects = (int)((positions[1].y - positions[0].y) / objectDistance) + 5;
    std::cout << "nubmer of objects " << numberOfObjects << std::endl;
    objects.reserve(numberOfObjects);
    for(int i = 0; i < numberOfObjects; i++){
        objects.push_back(std::make_unique<ClickableObject>());
        objects[i].get()->positions.push_back(positions[0] + (Vector2){0, objectFreeSpace});
        objects[i].get()->positions.push_back(positions[0] + (Vector2){positions[1].x - positions[0].x, objectDistance - objectFreeSpace});
        objects[i].get()->text = "Object: " + std::to_string(i);
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
    objectOffset = 0;
    if(!Global.ScaleUpdated)
        update();
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

    if(Global.ScaleUpdated){
        for(int i = 0; i < objects.size(); i++){
            objects[i]->deinit();
            objects[i].reset();
        }
        objects.clear();
        objects.shrink_to_fit();  
        init();
    }

    objectOffsetFull += (int)trunc((objectOffset / objectDistance));


    
    bool updateTexts = false;
    if((int)trunc((objectOffset / objectDistance)) != 0){
        updateTexts = true;
    }

    objectOffset -= trunc((objectOffset / objectDistance)) * objectDistance;
    for(int i = 0; i < objects.size(); i++){
        objects[i].get()->positions[0] = positions[0] + (Vector2){0, objectDistance * (i - 2) + objectFreeSpace + objectOffset};
        objects[i].get()->positions[1] = positions[0] + (Vector2){positions[1].x - positions[0].x, objectDistance * (i - 1) - objectFreeSpace + objectOffset};
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
            objects[i].get()->text = "Object: " + std::to_string(i - objectOffsetFull + hardCodedOffset) + "textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test textbox test ";
            objects[i].get()->internalBox.SetText(objects[i].get()->text);
        }
        if(updateTextBox){
            objects[i].get()->textRect = Rectangle{objects[i].get()->positions[0].x + textSpacing, objects[i].get()->positions[0].y + textSpacing, (objects[i].get()->positions[1].x - objects[i].get()->positions[0].x) - 2.0f*textSpacing, objects[i].get()->positions[1].y - objects[i].get()->positions[0].y - 2.0f*textSpacing};
            objects[i].get()->internalBox.SetBox(objects[i].get()->textRect);
        }
    }
}

void FancyScrollingList::render() {
   for(int i = 0; i < objects.size(); i++){
//if (i - objectOffsetFull + hardCodedOffset >= 0)
            objects[i]->render();
    }
}