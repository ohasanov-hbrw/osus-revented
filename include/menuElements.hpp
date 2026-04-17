#include <vector>
#include "raylib.h"

enum MenuObjectModes{
    CLICKABLEOBJECT_BUTTON = 0,
    CLICKABLEOBJECT_TOGGLE,
    CLICKABLEOBJECT_INACTIVE
};

class MenuElement {
public:
    std::vector<int> positions;
    bool drawOutline = false;
    
    Color outlineColor;
    Color outlineColorFocused;
    
    Color baseColor;
    Color baseColorFocused;
    Color baseColorFocusedActive;
    
    Color textColor;
    Color textColorActive;


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

    void render() override;
    void update() override;
};