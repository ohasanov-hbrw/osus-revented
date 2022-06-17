#include "gui.hpp"

GuiElement::GuiElement(Vector2 position, Vector2 size) : position(position), size(size), event(MouseEvent::None)
{  }

void GuiElement::update() {
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    bool click = Global.MouseInFocus and IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    if (hover and click) {
        event = MouseEvent::Press;
    }
    else if (hover) {
        event = MouseEvent::Hover;
    }
    else {
        event = MouseEvent::None;
    }
}

void GuiElement::render() {

}

Rectangle GuiElement::getRect() {
    return Rectangle{.x = position.x, .y = position.y, .width = size.x, .height = size.y};
}

Button::Button(Vector2 position, Vector2 size, Color color) 
    : GuiElement(position, size), color(color)
{  }


void Button::render() {
}

void Button::update() {

}
