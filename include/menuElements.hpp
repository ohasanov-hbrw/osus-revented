#pragma once
#include <vector>
#include "raylib.h"

enum MenuObjectModes{
    CLICKABLEOBJECT_BUTTON = 0,
    CLICKABLEOBJECT_TOGGLE,
    CLICKABLEOBJECT_INACTIVE
};

class MenuElement {
public:
    
    bool drawOutline = false;
    
    Color outlineColor;
    Color outlineColorFocused;
    
    Color baseColor;
    Color baseColorFocused;
    Color baseColorFocusedActive;
    
    Color textColor;
    Color textColorActive;

    std::vector<Vector2> positions;
    
    MenuElement() = default;
    virtual ~MenuElement() = default;

    virtual void render();
    virtual void update();

    virtual void init();
    virtual void deinit();
};

class ClickableObject : public MenuElement {
private:
    
public:
    enum MenuObjectModes mode = CLICKABLEOBJECT_INACTIVE;

    ClickableObject() = default;
    ~ClickableObject() = default;
    void render() override;
    void update() override;

    void init() override;
    void deinit() override;
};