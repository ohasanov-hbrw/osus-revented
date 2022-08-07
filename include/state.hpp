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
    TextBox path;
    TextBox description;
    std::string temp;
public:
    PlayMenu();
    std::string lastPos = "";
    void init() override;
    void render() override;
    void update() override;
};

class LoadMenu : public State {
private:
    SelectableList dir_list;
    Button select;
    Button back;
    Button close;
    TextBox bg;
    TextBox path;
    TextBox description;
public:
    LoadMenu();

    void init() override;
    void render() override;
    void update() override;
};

class MainMenu : public State {
private:
    Button play;
    Button load;
public:
    MainMenu();

    void init() override;
    void render() override;
    void update() override;
};

class Game : public State {
private:
public:
    Game();

    void init() override;
    void render() override;
    void update() override;
};
