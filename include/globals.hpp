#pragma once

#include "raylib.h"
#include <string.h>
#include <filesystem>
#include <memory>
#include "gamemanager.hpp"
#include <thread>
#include <functional>
#include "SDL2/SDL.h"
#include <mutex>

#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif


struct InputHandler {
    int mx = -1;
    int my = -1;
    bool k1p = 0;
    bool k1d = 0;
    bool k1r = 0;
    bool k2p = 0;
    bool k2d = 0;
    bool k2r = 0;
    int scr = 0;
};

class State;

struct Globals {
    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = { 15, 0, 30, 255 };


    Vector2 CallbackMouse = {0, 0};

    int GameTextures = 0;

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

    double volume = 1.0f;

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

    int numberLines = -1;
    int parsedLines = -1;

    int loadingState = 0;

    int glfwWindowSizeX = 640;
    int glfwWindowSizeY = 480;
    int glfwWindowPosX = 0;
    int glfwWindowPosY = 0;

    SDL_Window* win;
    bool quit = false;
    uint8_t amogs = 0;
    /*SDL_Cursor* amog = SDL_CreateCursor(&amogs,
                             &amogs,
                             1, 1, 1,
                             1);*/
    InputHandler Input;
    bool renderFrame;
    std::mutex mutex;
};

extern Globals Global;
