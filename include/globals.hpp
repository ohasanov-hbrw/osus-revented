#pragma once
#include "platformspesifics.hpp"
#include "raylib.h"
#include <string.h>
#include <filesystem>
#include <memory>
#include "gamemanager.hpp"
#include <thread>
#include <functional>
#include <mutex>
#include <cstdint>

#define DEPTH_MULT -1


#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

struct GameSettings {
    bool useDefaultSkin = false;
    bool useDefaultSounds = false;
};

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
struct TextureSizes{
    int hitCircle = 64;
    int comboNumber = 110;
    int hitCircleOverlay = 64;
    int approachCircle = 64;
    bool render300 = false;
};

struct Globals {
    #ifdef THREEDS_BUILD
        int AUDIO_SETUP_KEY = KEY_SELECT;
        int GO_BACK_KEY = KEY_B;
        int UP_KEY = KEY_DUP;
        int DOWN_KEY = KEY_DDOWN;
        int LEFT_KEY = KEY_DLEFT;
        int RIGHT_KEY = KEY_DRIGHT;
        int AUTO_KEY = KEY_X;
        int P1_KEY = KEY_L;
        int P2_KEY = KEY_R;
    #endif
    #ifndef THREEDS_BUILD
        int AUDIO_SETUP_KEY = SDL_SCANCODE_LALT;
        int GO_BACK_KEY = SDL_SCANCODE_BACKSPACE;
        int UP_KEY = SDL_SCANCODE_UP;
        int DOWN_KEY = SDL_SCANCODE_DOWN;
        int LEFT_KEY = SDL_SCANCODE_LEFT;
        int RIGHT_KEY = SDL_SCANCODE_RIGHT;
        int AUTO_KEY = SDL_SCANCODE_LEFT;
        int P1_KEY = SDL_SCANCODE_Z;
        int P2_KEY = SDL_SCANCODE_X;
    #endif


    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = { 15, 0, 30, 255 };


    Vector2 CallbackMouse = {0, 0};

    int GameTextures = 0;

    //Color Background = {42,22,33,255};
    int skinNumberOverlap = 18;
    double FPS = 4.0f * 100.0f;
    
    #ifdef THREEDS_BUILD
        int TPS = 240;
    #endif
    #ifndef THREEDS_BUILD
        int TPS = 1000;
    #endif

    int Width = 400;
    int Height = 240;
    float offset = 45.0f;

    struct timespec ts1 = timespec{0,0}, ts2 = timespec{0,0};
    bool paused = false;

    int maxSliderSize = 10000;


    long int startTime2;
    double curTime = 0;
    double startTime = -1;
    double curTime2 = 0;
    
    double extraJudgementTime = 16.0;

    double offsetTime = -50;

    unsigned long long int startsTime = 0;
    unsigned long long int pausedFor = 0;
    Texture2D cursor;
    Texture2D cursorTrail;

    bool MouseInFocus = false;
    int MouseBlur = 20;

    Vector2 MousePosition;
    Vector2 AutoMousePosition = {0,0};
    Vector2 AutoMousePositionStart = {-1, -1};
    double AutoMouseStartTime;
    bool useAuto = false;

    //float audioSecondsElapsed = 0.0f;
    //bool audioPlaying = false;
    Font DefaultFont;

    #ifdef THREEDS_BUILD
        std::string Path = "sdmc:/3ds";//std::filesystem::current_path().string();
        std::string BeatmapLocation = "sdmc:/3ds/beatmaps";
        std::string GamePath = "sdmc:/3ds";//std::filesystem::current_path().string();
        std::string selectedPath = "sdmc:/3ds";
        std::string CurrentLocation = "sdmc:/3ds";
    #endif
    #ifndef THREEDS_BUILD
        std::string Path = std::filesystem::current_path().string();
        std::string BeatmapLocation = std::filesystem::current_path().string() + "/beatmaps";
        std::string GamePath = std::filesystem::current_path().string();
        std::string selectedPath = "";
        std::string CurrentLocation = std::filesystem::current_path().string();
    #endif

    int MouseTrailSize = 150;

    float FrameTimeCounterWheel = 0.f;

    Texture2D OsusLogo;

    double volume = 1.0f;
    double hitVolume = 1.0f;
    bool volumeChanged = true;

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
    Shader shdrTest;
    std::shared_ptr<State> CurrentState;

    GameManager *gameManager = GameManager::getInstance();

    Globals() = default;

    float sliderTexSize = 0.50f;
    float sliderMinimumX = 0.0f; // -150
    float sliderMinimumY = 0.0f; // -150
    float sliderMaximumX = 640.0f; // 790
    float sliderMaximumY = 480.0f; // 630
    int circleSector = 16;
    bool legacyRender = false;

    long long errorSum = 0;
    long long errorLast = 0;
    long long errorDiv = 0;

    std::chrono::time_point<std::chrono::steady_clock> start, end;


    bool NeedForBackgroundClear = true;

    double CurrentInterpolatedTime = 0;
    double LastOsuTime = 0;
    double currentOsuTime = 0;

    double avgSum = 0;
    double avgNum = 0;
    double avgTime = 0;

    int numberLines = -1;
    int parsedLines = -1;

    int loadingState = 0;

    bool quit = false;

    TextureSizes textureSize;

    InputHandler Input;
    bool renderFrame;
    std::mutex mutex;
    std::mutex mutex2;

    MULTITHREAD_MUTEX lightlock;


    GameSettings settings;

    bool useTopScreen = false;
    bool touchScreenTouchEnabled = true;

    INPUT_TOUCHSCREEN touch;
    INPUT_TOUCHSCREEN lastTouchPos;

    bool lastTouch = false;

    GPU_RENDER_TARGET * window;
    GPU_RENDER_TARGET * gpu_currentRenderTarget;
    
    u32 ds_kDown = 0;
    u32 ds_kHeld = 0;
    u32 ds_kUp = 0;

    u64 totalNumOfSamples = 0;
    
    bool sliderTexNeedDeleting = false;
    u32 linearSpaceFree = 0;

    bool MusicLoaded = false;

    bool channelOccupied[24];
    std::vector<Sound *> soundAtChannel;

    bool stop = false;

    bool polygonalRendering = true;

    


};

extern Globals Global;
