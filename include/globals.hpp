#pragma once

#include "raylib.h"

struct Globals {
    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = BLACK;
    int FPS = 144;
    int Width = 640;
    int Height = 480;
    bool MouseInFocus = false;
    Vector2 MousePosition;
    Globals() = default;
};

extern Globals Global;
