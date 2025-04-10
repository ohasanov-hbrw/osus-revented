#pragma once

#include "gui.hpp"
#include "utils.hpp"
#include "parser.hpp"
#include <globals.hpp>
#include <list>

class State {
public:
    State() = default;
    virtual ~State() = default;
    int initDone = 0;
    double initStartTime = 0;
    virtual void init() = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void unload() = 0;
    virtual void textureOps() = 0;
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
    TextBox name;
    Switch skin;
    Switch sound;
    TextBox usedskin;
    TextBox usedsound;
    std::string temp;
public:
    PlayMenu();
    std::string lastPos = "";
    int lastIndex = -3;
    void init() override;
    void render() override;
    void update() override;
    void unload() override;
    void textureOps() override;
};

class ResultsMenu : public State {
private:
    Button close;
    TextBox name;
    TextBox maxCombo;
    TextBox hit300;
    TextBox hit100;
    TextBox hit50;
    TextBox hit0;
    TextBox accuracy;
public:
    ResultsMenu();
    void init() override;
    void render() override;
    void update() override;
    void unload() override;
    void textureOps() override;
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
    void textureOps() override;
};

class MainMenu : public State {
private:
    Button play;
    Button wip;
    Button wip2;
    Button load;
    TestSlider volume;
    
public:
    MainMenu();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
    void textureOps() override;
};

class WipMenu2 : public State {
private:
    struct MenuItem{
        int location = 0;
        bool folder = true;
        int folderID = 0;
        int itemID = 0;
    };
    float lastMouse = 0.0f;
    int position;
    int minimumPosition = -240;
    int maximumPosition = -240;
    float graphicalPosition = -240;
    int addStuffAt = -1;
    int removeStuffAt = -1;
    int lastStuffAt = -1;
    bool canAddStuff = false;
    bool canRemoveStuff = false;
    std::vector<std::string> folderNames;
    std::vector<std::string> itemNames;
    float accel = 0.0f;
    std::list<MenuItem> locations;
    std::mutex scaryMulti;
    
public:
    WipMenu2();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
    void textureOps() override;
};

class Game : public State {
private:
    TestSlider volume;
public:
    Game();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
    void textureOps() override;
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
    Texture2D back;
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
    bool applyMouse = false;
    std::string Path = Global.BeatmapLocation  + "/";
    bool CanGoBack = false;
    bool renderMetadata = false;
    bool metadataAvailable = false;
    float animtime = 0.0f;
    std::vector<std::string> Metadata;
    std::vector<std::string> TempMeta;
    void init() override;
    void render() override;
    void update() override;
    void unload() override;
    void textureOps() override;
};

