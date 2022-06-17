#pragma once

#include "globals.hpp"
#include "raylib.h"

class GuiElement {
public:
    Vector2 position;
    Vector2 size;
    enum class MouseEvent {
        Hover,
        Press,
        None,
    } event;

    GuiElement(Vector2 position, Vector2 size);
    
    virtual void render();
    virtual void update();

    inline Rectangle getRect();
};

class Button : public GuiElement {
private:
    Color color;
public:
    Button(Vector2 position, Vector2 size, Color color);

    void render() override;
    void update() override;
};

class TextBox {
    
};

class SelectableList {
    
};

class ErrorDialogue {
    
};
