#include "menuElements.hpp"
#include "utils.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <iostream>

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
        objects[i].get()->baseColor = baseColor;
    }
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
    objectOffsetFull += (int)trunc((objectOffset / objectDistance));
    objectOffset -= trunc((objectOffset / objectDistance)) * objectDistance;
    for(int i = 0; i < objects.size(); i++){
        objects[i].get()->positions[0] = positions[0] + (Vector2){0, objectDistance * (i - 2) + objectFreeSpace + objectOffset};
        objects[i].get()->positions[1] = positions[0] + (Vector2){positions[1].x - positions[0].x, objectDistance * (i - 1) - objectFreeSpace + objectOffset};
        if(objects[i].get()->positions[0].y + objects[i].get()->positions[1].y < positions[0].y){
            objects[i].get()->baseColor = Fade(baseColor, clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y + objects[i].get()->positions[1].y)) / (objectDistance * 1.5)), 0.0f, 1.0f));
        }
        else if(objects[i].get()->positions[0].y > positions[1].y){
            objects[i].get()->baseColor = Fade(baseColor, clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) / (objectDistance * 1.5)), 0.0f, 1.0f));
        }
        else{
            objects[i].get()->baseColor = baseColor;
        }

        double distanceFromCenter = (objects[i].get()->positions[0].y + objectDistance / 2) - (positions[0].y + ((positions[1].y - positions[0].y) / 2.0f));
        objects[i].get()->positions[0].x += (distanceFromCenter / 50.0) * (distanceFromCenter / 50.0);
        //if (i == 0) std::cout << "positions " << objects[i].get()->positions[0].y << " " << objects[i].get()->positions[1].y << " " << objects[i].get()->positions[0].x << std::endl;
    }
}

void FancyScrollingList::render() {
   for(int i = 0; i < objects.size(); i++){
        objects[i]->render();
    }
}