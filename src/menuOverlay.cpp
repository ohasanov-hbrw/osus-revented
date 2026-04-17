#include "menuOverlay.hpp"

MenuOverlay::MenuOverlay(){  }

void MenuOverlay::update() {
    for(int i = 0; i < elements.size(); i++)
        elements[i].update();
}

void MenuOverlay::render() {
    for(int i = 0; i < elements.size(); i++)
        elements[i].render();
}

void MenuOverlay::init() {
    for(int i = 0; i < elements.size(); i++)
        elements[i].init();
}

void MenuOverlay::deinit() {
    for(int i = 0; i < elements.size(); i++)
        elements[i].deinit();
    elements.clear();
    elements.shrink_to_fit();  
}