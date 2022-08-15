#pragma once

#include "gui.hpp"
#include "utils.hpp"
#include "parser.hpp"

class State {
public:
    State() = default;
    virtual ~State() = default;

    virtual void init() = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void unload() = 0;
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
    void unload() override;
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
    void unload() override;
};

class MainMenu : public State {
private:
    Button play;
    Button wip;
    Button load;
public:
    MainMenu();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};

class Game : public State {
private:
public:
    Game();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};

class WIPMenu : public State {
private:
public:
    WIPMenu();
    Parser parser = Parser();
    GameFile gameFile; // = parser.parse(filename);
    std::vector<std::pair<std::string, std::string>> subObjects;
    Texture2D logo;
    Texture2D menu;
    float angle = 0;
    float posangle = 0;
    int adjust = 0;
    float accel = 0;
    float lastMouse = 0;
    float mouseMovement = 0;
    float absMouseMovement = 0;
    bool moving = false;
    int selectedIndex = -1;
    int selectedAngleIndex = -1;
    std::vector<std::string> dir;
    std::vector<std::string> subDir;
    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};