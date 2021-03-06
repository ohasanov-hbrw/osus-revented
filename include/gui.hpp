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

class Button : public GuiElement {
private:
    Color color;
    char* text;
    Color textcolor;
    int textsize;
public:
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
};

class ErrorDialogue {
    
};
