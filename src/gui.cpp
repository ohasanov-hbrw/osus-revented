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
    : GuiElement(position, size), color(color), text(text), textcolor(textcolor), textsize(textsize)
{  }


void Button::render() {
    Vector2 TextBoxSize = MeasureTextEx(Global.DefaultFont, text, textsize, 1);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});
    //melih buradaki variablelar calismio placeholder koydum
    if ((this->clicked or (Global.Key1D and this->focused)) and !this->focusbreak){
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    }
    else if (this->focused){
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), textcolor);
    }
    else{
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), textcolor);
    }
}

void Button::update() {
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    bool click = Global.MouseInFocus and Global.Key1P;

    if (hover and click) {
        this->focused = true;
        this->clicked = true;
        this->focusbreak = false;
    }
    else if (hover) {
        this->focused = true;
        this->clicked = false;
    }
    else {
        this->focused = false;
        this->clicked = false;
        this->focusbreak = true;
    }

    if(hover and !focusbreak and Global.Key1R)
        action = true;
    else
        action = false;

    if(action == true)
        std::cout << "amongus" << std::endl;
}

TextBox::TextBox(Vector2 position, Vector2 size, Color color, char* text, Color textcolor, int textsize)
    : GuiElement(position, size), color(color), text(text), textcolor(textcolor), textsize(textsize)
{  }

void TextBox::render() {
    Vector2 TextBoxSize = MeasureTextEx(Global.DefaultFont, text, textsize, 1);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});

    DrawRectangleRec(ScaleRect(this->getRect()), BLUE);
    if (focused) {
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), this->color);
    }
    else {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), textcolor);
    }
}

void TextBox::update() {
    
}
