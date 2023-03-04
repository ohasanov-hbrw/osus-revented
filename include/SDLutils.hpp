#pragma once

#include "SDL.h"
#include "globals.hpp"
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <chrono>



void initSDL();
void quitSDL();

void SDLpollEvent();
void SDLGetMouse();