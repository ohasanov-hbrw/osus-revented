#include "gui.hpp"
#include "utils.hpp"
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
        DrawTextEx(&Global.DefaultFont, text, Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    }
    else if (this->focused){
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
        DrawTextEx(&Global.DefaultFont, text, Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), textcolor);
    }
    else{
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(&Global.DefaultFont, text, Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), textcolor);
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


ImageObject::ImageObject(Vector2 position, Vector2 size, Color color, float opacity, float rotation, Texture2D * tex)
    : GuiElement(position, size), color(color), opacity(opacity), rotation(rotation), tex(tex)
{  }

//A button consists of a Textbox, so it is important that we update the textbox as well
void ImageObject::render() {
    DrawTextureRotate(tex, position.x, position.y, std::min(size.x / tex->width, size.y / tex->height), rotation, Fade(color, opacity));
}

//This function just checks if the button is "clicked" or not.
void ImageObject::update() {
    
}


TextBox::TextBox(Vector2 position, Vector2 size, Color color, std::string text, Color textcolor, int textsize, int maxlength)
    : GuiElement(position, size), color(color), textcolor(textcolor), text(text), textsize(textsize), maxlength(maxlength)
{
    init();
}

void TextBox::render() {
    Vector2 TextBoxSize = MeasureTextEx(&Global.DefaultFont, rendertext.c_str(), textsize, 2);
    Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});\
    if (focused) {
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        if(renderpos > -1)
            DrawTextEx(&Global.DefaultFont, subtext.c_str(), Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), this->color);
        else
            DrawTextEx(&Global.DefaultFont, rendertext.c_str(), Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), this->color);
    }
    else if(hover){
        Color tempColor = this->color;
        tempColor.r = (this->color.r * 2 + textcolor.r) / 3;
        tempColor.g = (this->color.g * 2 + textcolor.g) / 3;
        tempColor.b = (this->color.b * 2 + textcolor.b) / 3;
        tempColor.a = (this->color.a * 2 + textcolor.a) / 3;
        DrawRectangleRec(ScaleRect(this->getRect()), tempColor);
        DrawTextEx(&Global.DefaultFont, rendertext.c_str(), Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), textcolor);
    }
    else {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(&Global.DefaultFont, rendertext.c_str(), Vector2{static_cast<float>((int)ScaleCordX(TextBoxLocation.x)), static_cast<float>((int)ScaleCordY(TextBoxLocation.y))}, Scale(textsize),  Scale(2), textcolor);
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
    if((int)text.length() > maxlength){
        rendertext = text;
        while((int)rendertext.length() > 0 and (int)rendertext.length() > maxlength - 3){
            rendertext.pop_back();
        }
        rendertext.push_back('.');
        rendertext.push_back('.');
        rendertext.push_back('.');
    }
}

void TextBox::update() {
    if((int)text.size() > maxlength){
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
    : GuiElement(position, size), color(color), textcolor(textcolor), text(text), textsize(textsize), objectsize(objectsize), maxlength(maxlength)
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
    for(int i = 0; i < (int)text.size(); i++) {
        objects.push_back(TextBox({0,0}, {size.x, (float)objectsize}, color, text[i].c_str(), textcolor, textsize, maxlength));
        objects[objects.size()-1].init();
    }
    text.clear();
    bg = TextBox({position.x, position.y}, {size.x, size.y}, color, " ", BLACK, 0, 50);
}








Popup::Popup(Vector2 position, Vector2 size, Color color, std::string text, Color textcolor, int textsize, int type, int id)
    : GuiElement(position, size), color(color), textcolor(textcolor), text(text), textsize(textsize), type(type), id(id)
{
    init();
}

void Popup::render() {
    if(block){
        DrawRectangle(0,0,GetScreenWidth(), GetScreenHeight(), {0,0,0,200});
        bg.render();
        msg.render();
        top.render();
        b1.render();
        b2.render();
    }
}

void Popup::update() {
    action = false;
    if(block){
        b1.update();
        b2.update();
        if(b2.action){
            action = true;
            ans = 2;
        }
    }
}

void Popup::init() {
    //std::cout << "maybetext? " << text << std::endl;
    Vector2 size2 = MeasureTextEx(&Global.DefaultFont, "text.c_str()", textsize, 2);
    size2.x += textsize;
    size2.y += textsize;
    size.x = std::max(size2.x, size.x);
    size.y = std::max(size2.y, size.y);
    bg = TextBox({position.x, position.y}, {size.x, size.y}, color, " ", BLACK, 0, 50);
    msg = TextBox({position.x, position.y - 10}, {size.x, size.y}, {0,0,0,0} , text, WHITE, 15, 2000);
    std::string BorderText = "Border";
    if (type & ERR_FILEIO){
        BorderText = "Error while reading/writing files!";
    }
    top = TextBox({position.x, position.y - (size.y / 2.0f) + 10}, {size.x, 20}, {60,60,60,255} , BorderText.c_str(), WHITE, 10, 2000);

    b1 = Button({position.x - 40, position.y + (size.y / 2.0f) - 20}, {70,30}, {200,200,200,255}, "Cancel", BLACK, 15);
    b2 = Button({position.x + 40, position.y + (size.y / 2.0f) - 20}, {70,30}, {200,200,200,255}, "OK", BLACK, 15);

    if (type & ERR_RETRY){
        b2 = Button({position.x + 40, position.y + (size.y / 2.0f) - 20}, {70,30}, {200,200,200,255}, "Retry", BLACK, 15);
    }

    block = true;
    //Vector2 TextBoxLocation = GetRaylibOrigin({GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});
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
    Rectangle Left = GetRaylibOriginR({static_cast<float>(position.x - (size.x * (1 - percentage)) / 2), position.y, static_cast<float>(size.x * percentage), size.y});
    Rectangle Right = GetRaylibOriginR({static_cast<float>(position.x + (size.x * (percentage)) / 2), position.y, static_cast<float>(size.x * (1.0f-percentage)), size.y});
    Rectangle Dingus = GetRaylibOriginR({static_cast<float>(position.x - (size.x / 2) + (size.x * (percentage))), position.y, size.y / 2, size.y});
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
    Rectangle Dingus = GetRaylibOriginR({static_cast<float>(position.x - (size.x / 2) + (size.x * (percentage))), position.y, size.y / 2, size.y});
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