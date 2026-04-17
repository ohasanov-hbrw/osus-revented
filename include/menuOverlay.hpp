

#include <deque>
#include "menuElements.hpp"

class MenuOverlay {
public:
    std::deque<MenuElement> elements;

    MenuOverlay();

    void init();
    void deinit();

    virtual void render();
    virtual void update();
};