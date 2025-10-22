#pragma once

#include "raylib.h"
#include <string>
#include <vector>


class GuiElement {
public:
    Vector2 position;
    Vector2 size;

    bool focused = false;
    bool focusbreak = false;
    bool clicked = false;
    bool action = false;

    GuiElement(Vector2 position, Vector2 size);
    GuiElement() = default;
    
    virtual void render();
    virtual void update();

    inline Rectangle getRect();
};

class ImageObject : public GuiElement {
private:
    

public:
    Color color;
    float opacity;
    float rotation;

    Texture2D * tex;

    ImageObject(Vector2 position, Vector2 size, Color color, float opacity, float rotation, Texture2D * tex);
    ImageObject() = default;
    

    void render() override;
    void update() override;
};

class Button : public GuiElement {
private:
    
public:
    Color color;
    char* text;
    Color textcolor;
    int textsize;

    Button(Vector2 position, Vector2 size, Color color, char* text, Color textcolor, int textsize);
    Button() = default;

    void render() override;
    void update() override;
};

class TextBox : public GuiElement {
private:
    float counter = -100.f;
    int renderpos = -1;
public:
    TextBox(Vector2 position, Vector2 size, Color color, std::string text, Color textcolor, int textsize, int maxlength);
    TextBox() = default;

    Color color;
    Color textcolor;
    std::string text;
    std::string rendertext;
    std::string subtext;
    int textsize;
    int maxlength;
    bool hover = false;
    bool action = false;

    void init();
    void render() override;
    void update() override;
};

class SelectableList : public GuiElement {
private:
    
public:
    SelectableList(Vector2 position, Vector2 size, Color color, std::vector<std::string> text, Color textcolor, int textsize, int objectsize, int maxlength);
    SelectableList() = default;
    
    void init();
    void render() override;
    void update() override;
    Color color;
    Color textcolor;
    std::vector<std::string> text;
    int textsize;
    int objectsize;
    int maxlength;
    std::vector<TextBox> objects;
    TextBox bg;
    int selectedindex = 0;
    int renderindex1 = 0;
    int renderindex2 = 0;
    bool mouseSelect = false;
    int mouseSelectIndex = 0;
};

class Popup : public GuiElement {
private:
    
public:
    Popup(Vector2 position, Vector2 size, Color color, std::string text, Color textcolor, int textsize, int type, int id);
    Popup() = default;
    
    void init();
    void render() override;
    void update() override;
    Color color;
    Color textcolor;
    std::string text;
    int textsize;
    int type;
    TextBox bg;
    TextBox msg;
    TextBox top;
    Button b1;
    Button b2;
    Button b3;
    Button c;
    bool block = true;
    int ans = 0;
    bool action = false;
    int id;
};

class Switch : public GuiElement {
private:
    
public:
    Switch(Vector2 position, Vector2 size, Color color, Color altcolor, Color outcolor, Color switchcolor);
    Switch() = default;

    Color color;
    Color altcolor;
    Color switchcolor;
    Color outcolor;
    bool state = false;

    void render() override;
    void update() override;

};

class TestSlider : public GuiElement {
private:
    
public:
    TestSlider(Vector2 position, Vector2 size, Color color, Color altcolor, Color outcolor, Color switchcolor);
    TestSlider() = default;

    Color color;
    Color altcolor;
    Color switchcolor;
    Color outcolor;
    double location = 100;
    bool active = true;
    void render() override;
    void update() override;

};

class ErrorDialogue {
    
};
