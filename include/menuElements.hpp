#pragma once
#include <vector>
#include "raylib.h"
#include <string>
#include <memory>

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
    std::string text;
    ClickableObject() = default;
    ~ClickableObject() = default;
    void render() override;
    void update() override;

    void init() override;
    void deinit() override;
};

class FancyScrollingList : public MenuElement {
private:
    
public:
    enum MenuObjectModes mode = CLICKABLEOBJECT_INACTIVE;
    double objectDistance = 40.0f;
    double objectFreeSpace = 2.0f;
    int objectOffsetFull = 0;
    double objectOffset = 0;
    const int hardCodedOffset = -2;
    int numberOfObjects = 0;
    std::vector<std::string> objectNames;
    std::vector<std::unique_ptr<ClickableObject>> objects;
    FancyScrollingList() = default;
    ~FancyScrollingList() = default;
    void render() override;
    void update() override;

    void init() override;
    void deinit() override;
};