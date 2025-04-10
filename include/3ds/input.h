#pragma once
#include "raylibDefinitions.h"
#include <cstdint>
#include "3ds.h"

typedef uint32_t u32;

static char ds_keysNames[32][32] = {
    "KEY_A", "KEY_B", "KEY_SELECT", "KEY_START",
    "KEY_DRIGHT", "KEY_DLEFT", "KEY_DUP", "KEY_DDOWN",
    "KEY_R", "KEY_L", "KEY_X", "KEY_Y",
    "", "", "KEY_ZL", "KEY_ZR",
    "", "", "", "",
    "KEY_TOUCH", "", "", "",
    "KEY_CSTICK_RIGHT", "KEY_CSTICK_LEFT", "KEY_CSTICK_UP", "KEY_CSTICK_DOWN",
    "KEY_CPAD_RIGHT", "KEY_CPAD_LEFT", "KEY_CPAD_UP", "KEY_CPAD_DOWN"
};



bool IsKeyDown(int index);
bool IsKeyPressed(int index);
bool IsKeyReleased(int index);

void PollInputEvents();

Vector2 GetMousePosition();
int GetMouseX();
int GetMouseY();

bool IsMouseButtonDown(int index);
bool IsMouseButtonPressed(int index);
bool IsMouseButtonReleased(int index);

int GetMouseWheelMove();
