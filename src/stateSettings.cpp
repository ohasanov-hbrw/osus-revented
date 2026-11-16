#include "fastrender.hpp"
#include "fs.hpp"
#include "gamemanager.hpp"
#include "globals.hpp"
#include "menuElements.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "settingsParser.hpp"
#include "state.hpp"
#include "time_util.hpp"
#include "utils.hpp"
#include "zip.h"
#include <algorithm>
#include <clocale>
#include <cmath>
#include <cstring>
#include <ctype.h>
#include <dirent.h>
#include <gamefile.hpp>
#include <iostream>
#include <memory>
#include <ostream>

#include "menu_shapes/shapes.hpp"

SettingsMenu::SettingsMenu() {
  
}

void SettingsMenu::init() {
  initializationStage = STATE_INITIALIZED;
}

void SettingsMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
}
void SettingsMenu::update() {

}

static int currentTexNumber = 0;
static int currentImgNumber = 0;

void SettingsMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
}

// only this function can convert images to textures
void SettingsMenu::textureOps() {
  
}
