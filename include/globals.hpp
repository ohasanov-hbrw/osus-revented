#pragma once

#include "raylib.h"
#include <string.h>
#include <filesystem>
#include <memory>
#include "gamemanager.hpp"

#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif


class State;

struct Globals {
    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = { 15, 0, 30, 255 };
    //Color Background = {42,22,33,255};
    int skinNumberOverlap = 18;
    int FPS = 1000;
    int Width = 640;
    int Height = 480;
    float offset = 45.0f;

    struct timespec ts1 = timespec{0,0}, ts2 = timespec{0,0};
    bool paused = false;

    long int startTime2;
    double curTime = 0;
    double startTime = -1;
    double curTime2 = 0;
    double amogus;
    double amogus2;
    double amogus3;
    double amogus4;
    unsigned long long int startsTime = 0;
    unsigned long long int pausedFor = 0;
    Texture2D cursor;
    Texture2D cursorTrail;
    bool MouseInFocus = false;
    int MouseBlur = 20;
    Vector2 MousePosition;

    Font DefaultFont;

    std::string Path = std::filesystem::current_path().string();
    std::string BeatmapLocation = /*"C:/Users/renot/AppData/Local/osu!/Songs";*/ std::filesystem::current_path().string() + "/beatmaps";
    std::string GamePath = std::filesystem::current_path().string();
    std::string selectedPath = "";
    std::string CurrentLocation = std::filesystem::current_path().string();
    int MouseTrailSize = 150;

    float FrameTimeCounterWheel = 0.f;

    Texture2D OsusLogo;

    double volume = 0.8f;

    bool Key1P = false;
    bool Key1D = false;
    bool Key2P = false;
    bool Key2D = false;
    bool Key1R = false;
    bool Key2R = false;
    bool enableMouse = true;
    int Wheel = 0;

    double LastFrameTime;
    double FrameTime;

    Shader shdrOutline;

    std::shared_ptr<State> CurrentState;

    GameManager *gameManager = GameManager::getInstance();

    Globals() = default;

    float sliderTexSize = 2.0f;

    long long errorSum = 0;
    long long errorLast = 0;
    long long errorDiv = 0;

    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;




    double CurrentInterpolatedTime = 0;
    double LastOsuTime = 0;
    double currentOsuTime = 0;

    double avgSum = 0;
    double avgNum = 0;
    double avgTime = 0;
};

extern Globals Global;
