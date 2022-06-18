#include "gui.hpp"
#include "utils.hpp"
#include <iostream>

GuiElement::GuiElement(Vector2 position, Vector2 size) : position(position), size(size)
{  }

void GuiElement::update() {
    
}

void GuiElement::render() {
   
}

Rectangle GuiElement::getRect() {
    return {position.x, position.y, size.x, size.y};
}

Button::Button(Vector2 position, Vector2 size, Color color) 
    : GuiElement(position, size), color(color)
{  }


void Button::render() {
    if (this->clicked or Global.Key1D){
        DrawRectangleRec(ScaleRect(this->getRect()), BLACK);
    }
    else{
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
    }
    if (this->focused){
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    }
}

void Button::update() {
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    bool click = Global.MouseInFocus and Global.Key1P;

    if (hover and click) {
        this->focused = true;
        this->clicked = true;
    }
    else if (hover) {
        this->focused = true;
        this->clicked = false;
    }
    else {
        this->focused = false;
        this->clicked = false;
    }
}
