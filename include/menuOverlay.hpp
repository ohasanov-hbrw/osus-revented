#pragma once
#include "menuElements.hpp"
#include <memory>

class MenuOverlay {
public:
    std::vector<std::unique_ptr<MenuElement>> elements;

    MenuOverlay() = default;
    ~MenuOverlay() = default;

    void init();
    void deinit();

    virtual void render();
    virtual void update();
};