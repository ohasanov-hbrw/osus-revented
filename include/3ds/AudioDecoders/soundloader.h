#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylibDefinitions.h"
#include <iostream>

Sound decodeSoundFromFile(const char *fileName);
int loadmp3(const char *fileName);