#pragma once

#include "globals.hpp"
#include "raylib.h"
#include <string>

class GuiElement {
public:
    Vector2 position;
    Vector2 size;

    bool focused = false;
    bool focusbreak = false;
    bool clicked = false;
    bool action = false;

    GuiElement(Vector2 position, Vector2 size);
    
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

    void render() override;
    void update() override;
};

class TextBox : public GuiElement {
private:
    Color textcolor;
    std::string text;
    int textsize;
    bool editable;
public:
    TextBox(Vector2 position, Vector2 size, Color textcolor, int textsize, bool editable = true);
    
    void render() override;
    void update() override;
};

class SelectableList {
    
};

class ErrorDialogue {
    
};
