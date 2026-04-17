#include "menuElements.hpp"
#include "utils.hpp"
#include <vector>

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
    positions.clear();
    positions.shrink_to_fit();
}

void ClickableObject::deinit() {
    positions.clear();
    positions.shrink_to_fit(); 
}