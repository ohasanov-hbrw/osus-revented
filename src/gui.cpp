#include "gui.hpp"
#include "utils.hpp"
#include <iostream>
#include <cstring>
#include "globals.hpp"

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
}

TextBox::TextBox(Vector2 position, Vector2 size, Color color, std::string text, Color textcolor, int textsize, int maxlength)
    : GuiElement(position, size), color(color), text(text), textcolor(textcolor), textsize(textsize), maxlength(maxlength)
{
    init();
}

void TextBox::render() {
    Vector2 TextBoxSize = MeasureTextEx(Global.DefaultFont, rendertext.c_str(), textsize, 1);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});\
    DrawRectangleRec(ScaleRect(this->getRect()), BLUE);
    if (focused) {
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        if(renderpos > -1)
            DrawTextEx(Global.DefaultFont, subtext.c_str(), ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), this->color);
        else
            DrawTextEx(Global.DefaultFont, rendertext.c_str(), ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), this->color);
    }
    else {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(Global.DefaultFont, rendertext.c_str(), ScaleCords(TextBoxLocation), Scale(textsize),  Scale(1), textcolor);
    }
}
void TextBox::init() {
    rendertext = text;
    if(text.length() > maxlength){
        rendertext = text;
        while(rendertext.length() > 0 and rendertext.length() > maxlength - 3){
            rendertext.pop_back();
        }
        rendertext.push_back('.');
        rendertext.push_back('.');
        rendertext.push_back('.');
    }
}

void TextBox::update() {
    if(text.size() > maxlength){
        if(focused){
            if(counter < 0.f)
                counter = 0.f;
            counter += GetFrameTime()*1000.0f;
            while(counter > 1000.0f){
                renderpos++;
                if(renderpos > (int)text.size()){
                    renderpos = 0;
                }
                if(renderpos + maxlength <= (int)text.size()){
                    subtext = text.substr(renderpos,maxlength);
                }
                else{
                    subtext = text.substr(renderpos, text.size() - renderpos) + "|" + text.substr(0,maxlength - (text.size() - renderpos) - 1);
                }
                counter -= 100.0f;
            }
        }
        else{
            counter = -100.0f;
            renderpos = -1;
        }
    }
}

SelectableList::SelectableList(Vector2 position, Vector2 size, Color color, std::vector<std::string> text, Color textcolor, int textsize, int objectsize, int maxlength)
    : GuiElement(position, size), color(color), text(text), textcolor(textcolor), textsize(textsize), objectsize(objectsize), maxlength(maxlength)
{
    init();
}

void SelectableList::render() {
    bg.render();
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    for(int i = renderindex1; i < renderindex2; i++){
        if(selectedindex == i){
            objects[i].focused = true;
            //objects[i].color = textcolor;
            //objects[i].textcolor = color;
        }
        objects[i].render();
        if(selectedindex == i){
            objects[i].focused = false;
            //objects[i].color = color;
            //objects[i].textcolor = textcolor;
        }
    }

    if(hover)
         DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
}

void SelectableList::update() {
    renderindex2 = renderindex1 + std::min((int)size.y/objectsize, (int)objects.size());
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    if (hover){   
        selectedindex -= Global.Wheel;
        if(selectedindex >= (int)objects.size()){
            selectedindex = 0;
        }
        if(selectedindex < 0){
            selectedindex = objects.size() - 1;
        }
        if(selectedindex >= renderindex2){
            renderindex1 = selectedindex - size.y/objectsize + 1;
            renderindex2 = renderindex1 + std::min((int)size.y/objectsize, (int)objects.size());
        }
        if(selectedindex < renderindex1){
            renderindex1 = selectedindex;
            renderindex2 = renderindex1 + std::min((int)size.y/objectsize, (int)objects.size());
        }
    }
    for(int i = renderindex1; i < renderindex2; i++){
        objects[i].position = {position.x, position.y - size.y / 2.0f + (i-renderindex1)*objectsize + objectsize/2.0f };
    }
    for(int i = renderindex1; i < renderindex2; i++){
        if(selectedindex == i)
            objects[i].focused = true;
        objects[i].update();
        if(selectedindex == i)
            objects[i].focused = false;
    }
}

void SelectableList::init() {
    for(int i = 0; i < text.size(); i++) {
        objects.push_back(TextBox({0,0}, {size.x, (float)objectsize}, color, text[i], textcolor, textsize, maxlength));
        objects[objects.size()-1].init();
    }
    text.clear();
    bg = TextBox({position.x, position.y}, {size.x, size.y}, color, " ", BLACK, 0, 50);
}
