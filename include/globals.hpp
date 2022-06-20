#pragma once

#include "raylib.h"

struct Globals {
    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = { 30, 0, 60, 255 };

    int FPS = 1440;
    int Width = 640;
    int Height = 480;

    bool MouseInFocus = false;
    int MouseBlur = 20;
    Vector2 MousePosition;

    Font DefaultFont;

    int MouseTrailSize = 150;

    bool Key1P = false;
    bool Key1D = false;
    bool Key2P = false;
    bool Key2D = false;
    bool Key1R = false;
    bool Key2R = false;
    int Wheel = 0;

    Globals() = default;
};

extern Globals Global;
