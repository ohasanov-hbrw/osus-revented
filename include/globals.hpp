#pragma once
#include "platformspesifics.hpp"
#include "raylib.h"
#include <filesystem>
#include <memory>
#include "gamemanager.hpp"
#include <queue>
#include "errorManagement.hpp"
#include <atomic>


// This is a weird quirk with how different GPUs handle depth differently
#define DEPTH_MULT -1

// Try to force opengl3.3
#define PLATFORM_DESKTOP
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

// Settings for an osu game
struct GameSettings {
    bool useDefaultSkin = false;
    bool useDefaultSounds = false;
};

// Basic input data struct
struct InputHandler {
    int mx = -1;
    int my = -1;
    int scr = 0;
    bool k1p = 0;
    bool k1d = 0;
    bool k1r = 0;
    bool k2p = 0;
    bool k2d = 0;
    bool k2r = 0;
};

// Sizes of textures so that they can be scaled correctly
class State;
struct TextureSizes{
    int hitCircle = 64;
    int comboNumber = 110;
    int hitCircleOverlay = 64;
    int approachCircle = 64;
    bool render300 = false;
};

// Global data struct
struct Globals {
    // Default background color
    Color Background = { 15, 0, 30, 255 };
    // Mouse Zero
    Vector2 ZeroPoint = {0.f, 0.f};
    // Mouse location to be got from a callback
    Vector2 CallbackMouse = {0, 0};
    // Current Mouse Position
    Vector2 MousePosition;
    // Destination of PC controlled Mouse cursor
    Vector2 AutoMousePosition = {0,0};
    // Start point of PC controlled Mouse cursor
    Vector2 AutoMousePositionStart = {320, 240};
    // Default font to be used in menus
    Font DefaultFont;
    // Sizes of textures
    TextureSizes textureSize;
    // osu game settings
    GameSettings settings;
    // State engine
    std::shared_ptr<State> CurrentState;
    // osu! game manager instance
    GameManager *gameManager = GameManager::getInstance();
    // Targetscreen options
    GPU_RENDER_TARGET * window;
    GPU_RENDER_TARGET * gpu_currentRenderTarget;
    // Textures for the cursor and its trail
    Texture2D cursor;
    Texture2D cursorTrail;
    // Main game logo
    Texture2D OsusLogo;
    // WIP Shaders for sliders
    Shader shdrOutline;
    Shader shdrTest;
    // Touchscreen options for 3DS
    INPUT_TOUCHSCREEN touch;
    INPUT_TOUCHSCREEN lastTouchPos;
    // Point which the PC controller Mouse cursor left its previous destination
    double AutoMouseStartTime;
    // Frametime statistics
    double LastFrameTime;
    double FrameTime;
    // Music and Hitsound volume
    double volume = 1.0f;
    double hitVolume = 1.0f;
    // Target FPS and TPS
    double FPS = 4.0f * 100.0f;
    // An "n" second timer before the song even begins
    double startTime = -5000;
    // Some extra leninency that I am sure gonna calculate based on feeling
    double extraJudgementTime = 16.0;
    // Time offset for the music, is ignored with 3DS
    double offsetTime = -70;
    // Smooth time interpolation
    double CurrentInterpolatedTime = 0;
    double LastOsuTime = 0;
    double currentOsuTime = 0;
    // Number of samples loaded at a time
    u64 totalNumOfSamples = 0;
    // Helper variables for click time error rate calculations
    long long errorSum = 0;
    long long errorLast = 0;
    long long errorDiv = 0;
    // Locations of critical system components
    #ifdef THREEDS_BUILD
        std::string Path = "sdmc:/3ds";//std::filesystem::current_path().string();
        std::string BeatmapLocation = "sdmc:/3ds/beatmaps";
        std::string GameBinaryPath = "sdmc:/3ds";//std::filesystem::current_path().string();
        std::string selectedPath = "sdmc:/3ds";
        std::string CurrentLocation = "sdmc:/3ds";
    #endif
    #ifndef THREEDS_BUILD
        std::string Path = std::filesystem::current_path().string();
        std::string BeatmapLocation = std::filesystem::current_path().string() + "/beatmaps";
        std::string GameBinaryPath = "."; //std::filesystem::current_path().string();
        std::string selectedPath = "";
        std::string CurrentLocation = std::filesystem::current_path().string();
    #endif
    // 3DS Spesific music state
    std::vector<Sound *> soundAtChannel;
    // Error management queue
    std::queue<ErrorMessage> errors;
    // System timer variables
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    // 3DS CONSOLE
    #ifdef THREEDS_BUILD
    PrintConsole * pc;
    #endif
    // Current state of the textures
    // 0 -> Textures are loaded
    // 1 -> Mark textures to be loaded
    // -1 -> Mark textures to be unloaded
    // 2 -> Starting to unload textures
    // 10 -> Some basic textures are unlaoded
    // 15 -> Hopefully all textures are unloaded
    std::atomic_int32_t GameTextures = -1;
    #ifdef THREEDS_BUILD
        int TPS = 240;
    #endif
    #ifndef THREEDS_BUILD
        int TPS = 1000;
    #endif
    // Default width and height
    int Width = 400;
    int Height = 240;
    // Maximum number of points in a slider
    int maxSliderSize = 10000;
    // Number of trailing objects behind the Mouse
    int MouseBlur = 20;
    // Offset between the number textures of a skin
    int skinNumberOverlap = 18;
    // Define some commonly used keys 
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
        int PAUSE_KEY = KEY_Y;
        int SKIP_KEY = KEY_A;
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
        int PAUSE_KEY = SDL_SCANCODE_ESCAPE;
        int SKIP_KEY = SDL_SCANCODE_RETURN;
    #endif
    // Slider texture scaling based on the default 640*480
    #ifdef THREEDS_BUILD
        int mipmaps = 1;
        float sliderTexSize = 0.5f;
    #endif
    #ifndef THREEDS_BUILD
        int mipmaps = 4;
        float sliderTexSize = 2.5f;
    #endif
    // When using polygonal rendering for the circles, how many sectors to use
    int circleSector = 24;
    // Number of lines in a document and number of them the parser parsed, this is a debug thing
    int numberLines = -1;
    int parsedLines = -1;
    // Mouse wheel speed
    int Wheel = 0;
    // Current game loading state:
    // 1 -> Hitobject precalculation
    // 2 -> Background music
    // 3 -> Combobreak sound
    // 4 -> Hitsound list
    // 5 -> Line parsing
    // 6 -> Sound load
    // 7 -> Load textures
    int loadingState = 0;
    // Slider Resolution Divider
    int sliderResDiv = 2;
    // Number of trailing objects behind the Mouse
    int MouseTrailSize = 150;
    // 3DS input bits
    u32 ds_kDown = 0;
    u32 ds_kHeld = 0;
    u32 ds_kUp = 0;
    // 3DS spesific memory debug
    u32 linearSpaceFree = 0;
    // Hard limits for the points on a slider, clamps to 4:3 quasi
    float sliderMinimumX = 0.0f; // -150
    float sliderMinimumY = 0.0f; // -150
    float sliderMaximumX = 640.0f; // 790
    float sliderMaximumY = 480.0f; // 630
    // Global scale and offset of rendering
    float Scale = 1.f;
    // A small timer to simulate key-repeat
    float FrameTimeCounterWheel = 0.f;
    // Having a background image means that I dont need to flush the background two times
    bool NeedForBackgroundClear = true;
    // Main loop killer
    bool stop = false;
    // Unique error id / counter
    int errorid = 0;
    // State of slider textures
    bool sliderTexNeedDeleting = false;
    // 3DS Spesific music state
    bool MusicLoaded = false;
    // 3DS Spesific audio engine management
    bool channelOccupied[24];
    // Multithread initialization helpers
    bool readyForRenderLoop = false;
    bool readyForGameLoop = false;
    // 3DS spesific input and output things
    bool useTopScreen = false;
    bool touchScreenTouchEnabled = true;
    // Helper in calculating clicks on 3DS
    bool lastTouch = false;
    // Music and Hitsound volume
    bool volumeChanged = true;
    // Keys that are important
    bool Key1P = false;
    bool Key1D = false;
    bool Key2P = false;
    bool Key2D = false;
    bool Key1R = false;
    bool Key2R = false;
    // See if the mouse is in focus or not
    bool MouseInFocus = false;
    // To use the PC controlled cursor or not
    bool useAuto = false;
    // Use mouse clicks as Key1 and Key2?
    bool enableMouse = true;
    // Legacy render doesnt use shaders, polygonal rendering is needed on pc for special effects
    #ifdef THREEDS_BUILD
        bool legacyRender = true;
        bool polygonalRendering = false;
    #endif
    #ifndef THREEDS_BUILD
        bool legacyRender = true;
        bool polygonalRendering = true;
    #endif
    // IDK honestly, this does something important
    Globals() = default;

    double TimeStepSize = 0; //in milliseconds
};

extern Globals Global;


void deleteGlobalVariables();