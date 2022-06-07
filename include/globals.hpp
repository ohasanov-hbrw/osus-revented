#pragma once

#include "raylib.h"

struct Globals {
    float scale = 1.f;
    Vector2 zero_point = {0.f, 0.f};
    Color background = BLACK;
    int FPS = 60;
    int width = 640;
    int height = 480;

    Globals() = default;
};

extern Globals global;
