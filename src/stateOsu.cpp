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

#include "cachebuilder/metadataParser.hpp" // for namesOfSets



Game::Game() {
  volume = TestSlider({510, 460}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
}

void Game::init() {
  // MutexLock(SWITCHING_STATE);
  Global.NeedForBackgroundClear = true;
  Global.useAuto = false;
  initializationStage = STATE_UNINITIALIZED;
  Global.LastFrameTime = getGlobalTimer();
  // std::cout << Global.selectedPath << std::endl;

  Global.GameTextures = TEXTUREOPS_UNLOADED; // 0???
  Global.numberLines = 0;
  Global.parsedLines = 0;
  Global.loadingState = LOADINGSTATE_DEFAULT;
  initializationStage = STATE_LOADING_GAME;
  initStartTime = getTimer();
  // Global.mutex.unlock();
  // LightLock_Unlock(&Global.lightlock);
  // MutexUnlock(SWITCHING_STATE);
  // While loading the game chaos can happen, no problem

  // MutexUnlock(SWITCHING_STATE);
  MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
  MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  parseSettings();
  Global.doingTimeConsumingOp = true;
  Global.gameManager->loadGame(Global.selectedPath);
  // MutexLock(ACCESSING_OBJECTS);
  // MutexUnlock(ACCESSING_OBJECTS);
  // MutexLock(SWITCHING_STATE);
  // MutexLock(SWITCHING_STATE);
  Global.gameManager->timingSettingsForHitObject.clear();
  // Global.mutex.lock();
  // LightLock_Lock(&Global.lightlock);
  Global.startTime = -5000.0f;
  Global.errorSum = 0;
  Global.errorLast = 0;
  Global.errorDiv = 0;

  volume.location = Global.volume * 100.0f;
  Global.doingTimeConsumingOp = false;
  MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
  MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
  // MutexLock(SWITCHING_STATE);
}
void Game::update() {

  if (IsKeyDown(Global.AUDIO_SETUP_KEY))
    volume.update();
  float lastVolume = Global.volume;
  Global.volume = volume.location / 100.0f;
  if (!AreSame(lastVolume, Global.volume)) {
    // std::cout << "Volume: " << Global.volume << std::endl;
    Global.volumeChanged = true;
  }

  if (initializationStage == STATE_INITIALIZED) {
    // Global.enableMouse = false;

    // MutexLock(SWITCHING_STATE);

    if (IsKeyPressed(Global.GO_BACK_KEY) ||
        !(!WindowShouldClose() and _os_should_program_run())) {
      Global.CurrentState->initializationStage = STATE_FORCED_EXIT;
    }
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    Global.gameManager->run();
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  } else {
    if (initializationStage == STATE_UNINITIALIZED or
        Global.GameTextures == TEXTUREOPS_LOADED) {
      initializationStage = STATE_COUNTDOWN;
    }
    if (initializationStage == STATE_COUNTDOWN and
        getTimer() - initStartTime > 0.0f) {
      std::cout << "\e[1;38;5;236m[INFO] \e[38;5;40m" << "init done in "
                << getTimer() - initStartTime << " msecs\n";
      initializationStage = STATE_INITIALIZED;
    }
  }
}
void Game::render() {

  if (initializationStage == STATE_INITIALIZED) {
    // Global.enableMouse = false;
    MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
    Global.gameManager->render();
    // Global.mutex.lock();
    if (IsMusicStreamPlaying(&Global.gameManager->backgroundMusic)) {
      DrawTextEx(
          &Global.DefaultFont,
          TextFormat("Playing: %.3f/%.3f", (Global.currentOsuTime / 1000.0),
                     GetMusicTimeLength(&Global.gameManager->backgroundMusic)),
          {static_cast<float>((int)Scale(5)),
           static_cast<float>((int)Scale(25))},
          Scale(20.05), Scale(2), WHITE);
      // DrawTextEx(Global.DefaultFont, TextFormat("Timer: %.3f ms",
      // getTimer()), {ScaleCordX(5), ScaleCordY(55)}, Scale(10) , Scale(1),
      // WHITE); DrawTextEx(Global.DefaultFont, TextFormat("Last Error: %.3f
      // ms", Global.errorLast/1000.0f), {ScaleCordX(5), ScaleCordY(65)},
      // Scale(10) , Scale(1), WHITE);
    } else {
      DrawTextEx(
          &Global.DefaultFont,
          TextFormat("Paused: %.3f/%.3f",
                     GetMusicTimePlayed(&Global.gameManager->backgroundMusic) *
                         1000000.0f,
                     GetMusicTimeLength(&Global.gameManager->backgroundMusic)),
          {static_cast<float>((int)Scale(5)),
           static_cast<float>((int)Scale(25))},
          Scale(20.05), Scale(2), WHITE);
      if (Global.errorDiv != 0)
        DrawTextEx(&Global.DefaultFont,
                   TextFormat("Error Avg: %ld ms",
                              (Global.errorSum / Global.errorDiv) / 1000),
                   {static_cast<float>((int)Scale(5)),
                    static_cast<float>((int)Scale(40))},
                   Scale(20.05), Scale(2), WHITE);
    }
    if (GetMusicTimeLength(&Global.gameManager->backgroundMusic) != 0) {
      DrawLineEx(
          {0, GetScreenHeight() - Scale(2)},
          {static_cast<float>(
               GetScreenWidth() *
               ((Global.currentOsuTime / 1000.0) /
                GetMusicTimeLength(&Global.gameManager->backgroundMusic))),
           GetScreenHeight() - Scale(2)},
          Scale(3), Fade(WHITE, 0.8));
    }
    MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);

    // Global.mutex.unlock();
  } else if (initializationStage == STATE_COUNTDOWN) {
    std::string message;
    if (getTimer() - initStartTime < 2000.0f)
      message = "Loaded Game!";
    else if (getTimer() - initStartTime < 2500.0f)
      message = "3...";
    else if (getTimer() - initStartTime < 3000.0f)
      message = "2...";
    else if (getTimer() - initStartTime < 3500.0f)
      message = "1...";
    else if (getTimer() - initStartTime < 4000.0f)
      message = "GO!";
    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                  (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                          (unsigned char)(255 * (int)Global.Key1D), 100});
    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                  (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                          (unsigned char)(255 * (int)Global.Key2D), 100});
    // Global.mutex.lock();
    DrawTextEx(
        &Global.DefaultFont, message.c_str(),
        {static_cast<float>((int)ScaleCordX(320 - message.size() * 7.5f)),
         static_cast<float>((int)ScaleCordY(220))},
        Scale(20.05), Scale(2), WHITE);
    // Global.mutex.unlock();
  } else if (initializationStage == STATE_LOADING_GAME) {
    // Global.mutex.lock();
    std::string message;
    message = "Loading Game...";

    if (Global.loadingState == LOADINGSTATE_PRECALC_HITOBJECT) {
      // std::cout << "Precalculating HitObjects" << std::endl;
      message = "Precalculating HitObjects";
    } else if (Global.loadingState == LOADINGSTATE_LOADING_BACKGROUND_MUSIC) {
      // std::cout << "Loading Background Music" << std::endl;
      message = "Loading Background Music";
    } else if (Global.loadingState == LOADINGSTATE_LOADING_COMBOBREAK) {
      // std::cout << "Loading ComboBreak Sound" << std::endl;
      message = "Loading ComboBreak Sound";
    } else if (Global.loadingState == LOADINGSTATE_LISTING_HITSOUNDS) {
      // std::cout << "Loading Hit Sounds" << std::endl;
      message = "Loading Hitsounds";
    } else if (Global.loadingState == LOADINGSTATE_PARSING_LINES) {
      message = "Parsing line " + std::to_string(Global.parsedLines) + " of " +
                std::to_string(Global.numberLines);
    } else if (Global.loadingState == LOADINGSTATE_LOADING_SOUNDS) {
      message = "Parsing Sounds";
    } else if (Global.loadingState == LOADINGSTATE_LOADING_TEXTURES) {
      message = "Loading Textures";
    }
    DrawTextEx(
        &Global.DefaultFont, message.c_str(),
        {static_cast<float>((int)ScaleCordX(320 - message.size() * 7.5f)),
         static_cast<float>((int)ScaleCordY(220))},
        Scale(20.05), Scale(2), WHITE);
    // Global.mutex.unlock();
  }
  if (IsKeyDown(Global.AUDIO_SETUP_KEY))
    volume.render();
}
void Game::unload() {
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  Global.doingTimeConsumingOp = true;
  Global.gameManager->unloadGame();
  Global.doingTimeConsumingOp = false;
  Global.NeedForBackgroundClear = true;
  // MutexUnlock(ACCESSING_OBJECTS);
  // MutexUnlock(SWITCHING_STATE);
}
void Game::textureOps() {
  // std::cout << "Trying to acquire Lock for accessing objects\n";

  // std::cout << "Got Permission!\n";

  if (Global.sliderTexNeedDeleting) {
    MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
    Global.gameManager->unloadSliderTextures();
    MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  }

  if (Global.GameTextures == TEXTUREOPS_START_LOADING) {
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
              << "trying to get lock for object access for game texture load"
              << std::endl;
    MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
              << "got lock for object access for game texture load"
              << std::endl;
    Global.gameManager->loadGameTextures();
    MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
              << "loaded textures and unlocked access lock" << std::endl;
  }

  if (Global.GameTextures == TEXTUREOPS_START_UNLOADING) {
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
              << "trying to get lock for object access for game texture unload"
              << std::endl;
    MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
              << "got lock for object access for game texture unload"
              << std::endl;
    Global.gameManager->unloadGameTextures();
    MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
              << "unloaded textures and unlocked access lock" << std::endl;
  }
}