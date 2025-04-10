#include "gui.hpp"
#include "utils.hpp"
#include <iostream>
#include <cstring>
#include "globals.hpp"
//This file includes functions for updating and rendering the GUI elements

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

//A button consists of a Textbox, so it is important that we update the textbox as well
void Button::render() {
    Vector2 TextBoxSize = MeasureTextEx(&Global.DefaultFont, text, textsize, 2);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});
    if ((this->clicked or (Global.Key1D and this->focused)) and !this->focusbreak){
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        DrawTextEx(&Global.DefaultFont, text, Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    }
    else if (this->focused){
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
        DrawTextEx(&Global.DefaultFont, text, Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), textcolor);
    }
    else{
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(&Global.DefaultFont, text, Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), textcolor);
    }
}

//This function just checks if the button is "clicked" or not.
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
    Vector2 TextBoxSize = MeasureTextEx(&Global.DefaultFont, rendertext.c_str(), textsize, 2);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});\
    if (focused) {
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        if(renderpos > -1)
            DrawTextEx(&Global.DefaultFont, subtext.c_str(), Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), this->color);
        else
            DrawTextEx(&Global.DefaultFont, rendertext.c_str(), Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), this->color);
    }
    else if(hover){
        Color tempColor = this->color;
        tempColor.r = (this->color.r * 2 + textcolor.r) / 3;
        tempColor.g = (this->color.g * 2 + textcolor.g) / 3;
        tempColor.b = (this->color.b * 2 + textcolor.b) / 3;
        tempColor.a = (this->color.a * 2 + textcolor.a) / 3;
        DrawRectangleRec(ScaleRect(this->getRect()), tempColor);
        DrawTextEx(&Global.DefaultFont, rendertext.c_str(), Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), textcolor);
    }
    else {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(&Global.DefaultFont, rendertext.c_str(), Vector2{(int)ScaleCordX(TextBoxLocation.x), (int)ScaleCordY(TextBoxLocation.y)}, Scale(textsize),  Scale(2), textcolor);
    }
}
void TextBox::init() {
    rendertext = text;
    maxlength = rendertext.length();
    while(true){
        if(maxlength <= 0)
            break;
        Vector2 TextBoxSize = MeasureTextEx(&Global.DefaultFont, rendertext.substr(1,maxlength).c_str(), textsize, 2);
        //std::cout << TextBoxSize.x << " size?" << std::endl;
        if(TextBoxSize.x > (size.x * 0.95f))
            maxlength--;
        else
            break;
    }
    //std::cout << maxlength << std::endl;
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
            counter += (Global.FrameTime / 1000.0f)*1000.0f;
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
        if(mouseSelect and i == mouseSelectIndex){
            objects[i].hover = true;
        }
        if(selectedindex == i){
            objects[i].focused = true;
        }
        objects[i].render();
        if(selectedindex == i){
            objects[i].focused = false;
        }
        if(mouseSelect and i == mouseSelectIndex){
            objects[i].hover = false;
        }
    }

    if(hover)
         DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
}

void SelectableList::update() {
    action = false;
    renderindex2 = renderindex1 + std::min((int)size.y/objectsize, (int)objects.size());
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    mouseSelect = false;
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
        //std::cout << renderindex2 - renderindex1 << " out of max " << (int)size.y/objectsize << " and currently on " << renderindex1 + (int)((Global.MousePosition.y -  this->getRect().y) / objectsize) << std::endl;
        mouseSelectIndex = renderindex1 + (int)((Global.MousePosition.y -  this->getRect().y) / objectsize);
        mouseSelect = true;
        if(selectedindex == mouseSelectIndex and Global.Key1P){
            action = true;
        }
        if(Global.Key1P){
            selectedindex = mouseSelectIndex;
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
        //std::cout << "init of amogus nuumero " << i << std::endl;
        objects.push_back(TextBox({0,0}, {size.x, (float)objectsize}, color, text[i].c_str(), textcolor, textsize, maxlength));
        objects[objects.size()-1].init();
    }
    text.clear();
    bg = TextBox({position.x, position.y}, {size.x, size.y}, color, " ", BLACK, 0, 50);
    //std::cout << "init of amogus done\n";
}


Switch::Switch(Vector2 position, Vector2 size, Color color, Color altcolor, Color outcolor, Color switchcolor) 
    : GuiElement(position, size), color(color), altcolor(altcolor), switchcolor(switchcolor), outcolor(outcolor)
{  }


void Switch::render() {
    Rectangle Left = GetRaylibOriginR({position.x - size.x/4.0f, position.y, size.x/2.0f, size.y});
    Rectangle Right = GetRaylibOriginR({position.x + size.x/4.0f, position.y, size.x/2.0f, size.y});
    if(this->state){
        DrawRectangleRec(ScaleRect(Left), this->altcolor);
        DrawRectangleRec(ScaleRect(Right), this->switchcolor);
    }
    else{
        DrawRectangleRec(ScaleRect(Left), this->switchcolor);
        DrawRectangleRec(ScaleRect(Right), this->color);
    }
    if (this->focused){
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    }
    else{
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), this->outcolor);
        
    }
}

void Switch::update() {
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
        this->state = !this->state;
}

TestSlider::TestSlider(Vector2 position, Vector2 size, Color color, Color altcolor, Color outcolor, Color switchcolor) 
    : GuiElement(position, size), color(color), altcolor(altcolor), switchcolor(switchcolor), outcolor(outcolor)
{  }


void TestSlider::render() {
    double percentage = location / 100.0f;
    Rectangle Left = GetRaylibOriginR({position.x - (size.x * (1 - percentage)) / 2, position.y, size.x * percentage, size.y});
    Rectangle Right = GetRaylibOriginR({position.x + (size.x * (percentage)) / 2, position.y, size.x * (1.0f-percentage), size.y});
    Rectangle Dingus = GetRaylibOriginR({position.x - (size.x / 2) + (size.x * (percentage)), position.y, size.y / 2, size.y});
    if(Dingus.x < position.x - (size.x / 2))
        Dingus.x = position.x - (size.x / 2);
    if(Dingus.x > position.x + (size.x / 2) - size.y/2)
        Dingus.x = position.x + (size.x / 2) - size.y/2;
    DrawRectangleRec(ScaleRect(Left), this->altcolor);
    DrawRectangleRec(ScaleRect(Right), this->color);

    DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), this->outcolor);
    if(!active)
        DrawRectangleRec(ScaleRect(Dingus), this->switchcolor);
    else
        DrawRectangleRec(ScaleRect(Dingus), WHITE);
}

void TestSlider::update() {
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
    if(this->focused && Global.Wheel > 0){
        location += 1;
        if(location > 100.0)
            location = 100;
    }
    else if(this->focused && Global.Wheel < 0){
        location -= 1;
        if(location < 0.0)
            location = 0;
    }

    double percentage = location / 100.0f;
    Rectangle Dingus = GetRaylibOriginR({position.x - (size.x / 2) + (size.x * (percentage)), position.y, size.y / 2, size.y});
    if(Dingus.x < position.x - (size.x / 2))
        Dingus.x = position.x - (size.x / 2);
    if(Dingus.x > position.x + (size.x / 2) - size.y/2)
        Dingus.x = position.x + (size.x / 2) - size.y/2;
    if(Global.Key1P && CheckCollisionPointRec(Global.MousePosition, Dingus))
        active = true;
    if(!Global.Key1D)
        active = false;
    if(active && Global.Key1D){
        location = (Global.MousePosition.x - (position.x - (size.x / 2.0))) / size.x;
        location *= 100;
        if(location > 100.0)
            location = 100;
        else if(location < 0.0)
            location = 0;
    }
}