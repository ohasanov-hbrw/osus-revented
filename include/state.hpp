#pragma once

#include "gui.hpp"

class State {
public:
    State() = default;
    virtual ~State() = default;

    virtual void init() = 0;
    virtual void render() = 0;
    virtual void update() = 0;
};

class PlayMenu : public State {
private:
    SelectableList dir_list;
    Button select;
    Button back;
    Button close;
    TextBox bg;
    TextBox description;
public:
    PlayMenu();

    void init() override;
    void render() override;
    void update() override;
};
