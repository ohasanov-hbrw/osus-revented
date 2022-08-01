#pragma once

#include "raylib.h"
#include <string.h>
#include <filesystem>
#include <memory>
#include "state.hpp"
#include "gamemanager.hpp"

struct Globals {
    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = { 30, 0, 60, 255 };
    int skinNumberOverlap = 18;
    int FPS = 480;
    int Width = 640;
    int Height = 480;
    float offset = 0.0f;

    Texture2D cursor;
    Texture2D cursorTrail;
    bool MouseInFocus = false;
    int MouseBlur = 20;
    Vector2 MousePosition;

    Font DefaultFont;

    std::string Path = std::filesystem::current_path().string();
    std::string BeatmapLocation = std::filesystem::current_path().string() + "/beatmaps";
    std::string GamePath = std::filesystem::current_path().string();
    std::string selectedPath = "";
    int MouseTrailSize = 150;

    float FrameTimeCounterWheel = 0.f;

    Texture2D OsusLogo;

    bool Key1P = false;
    bool Key1D = false;
    bool Key2P = false;
    bool Key2D = false;
    bool Key1R = false;
    bool Key2R = false;
    int Wheel = 0;

    std::shared_ptr<State> CurrentState = std::make_shared<MainMenu>();

    GameManager *gameManager = GameManager::getInstance();

    Globals() = default;
};

extern Globals Global;
