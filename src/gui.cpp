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
    return GetRaylibOriginR({position.x, position.y, size.x, size.y});
}

Button::Button(Vector2 position, Vector2 size, Color color, char* text, Color textcolor, int textsize) 
    : GuiElement(position, size), color(color)
{  }


void Button::render() {
    Vector2 TextBoxSize = MeasureTextEx(Global.DefaultFont, "text", 20, 1);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});
    //melih buradaki variablelar calismio placeholder koydum
    if (this->clicked or (Global.Key1D and this->focused)){
        DrawRectangleRec(ScaleRect(this->getRect()), BLACK);
        DrawTextEx(Global.DefaultFont, "text", ScaleCords(TextBoxLocation), Scale(20),  Scale(1), WHITE);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    }
    else if (this->focused){
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
        DrawTextEx(Global.DefaultFont, "text", ScaleCords(TextBoxLocation), Scale(20),  Scale(1), BLACK);
    }
    else{
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(Global.DefaultFont, "text", ScaleCords(TextBoxLocation), Scale(20),  Scale(1), BLACK);
    }
    

    std::cout << TextBoxLocation.x << " " << TextBoxLocation.y << " " << "text" << std::endl;
    std::cout << TextBoxSize.x << " " << TextBoxSize.y << " " << "size" << std::endl;
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
