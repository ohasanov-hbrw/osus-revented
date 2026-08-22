#include "state.hpp"
#include "fastrender.hpp"
#include "fs.hpp"
#include "gamemanager.hpp"
#include "globals.hpp"
#include "menuElements.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "settingsParser.hpp"
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

StartMenu::StartMenu() {
  description = TextBox({320, 440}, {520, 40}, {240, 98, 161, 0},
                        "Click the circles!", WHITE, 50, 50);
  popup =
      Popup({320, 240}, {300, 120}, GRAY, "Test Popup", WHITE, 20, 1 << 0, -1);
  logo = ImageObject({320, 200}, {400, 400}, WHITE, 1, 0, &Global.OsusLogo);
}

void StartMenu::init() {
  // MutexLock(SWITCHING_STATE);
  Global.NeedForBackgroundClear = true;
  Global.LastFrameTime = getGlobalTimer();
  Global.FrameTime = 0.5;
  Global.useAuto = false;
  action = false;

  setlocale(LC_ALL, "en_US.utf8");
  popup.block = !Global.errors.empty();
  popup.update();
  animation = 0;
  // MutexUnlock(SWITCHING_STATE);
  initializationStage = STATE_INITIALIZED;
}

void StartMenu::update() {

  if (animation == 1) {
    double position = (getGlobalTimer() - animationStartTime) / animationMs;
    description.textcolor.a =
        (unsigned char)((int)clip(255 - (position * 255.0), 0, 255));
    if (getGlobalTimer() - animationStartTime > animationMs) {
      animation = -1;
      animationDone = true;
    }
    if (!animationDone)
      return;
    else {
      MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
      MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
      Global.CurrentState->unload();
      Global.CurrentState.reset(new MainMenu());
      ((MainMenu *)(Global.CurrentState.get()))->animation = 1;
      Global.CurrentState->init();
      MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
      MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
      return;
    }
  }

  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  Global.enableMouse = true;

  popup.update();

  if (popup.action) {
    if (popup.ans == 2) {
      Global.errors.pop();
    }
  }

  if (!popup.block) {
    bool hover =
        CheckCollisionPointCircle(Global.MousePosition, {320, 200}, 175);
    bool click = Global.MouseInFocus and Global.Key1P;

    if (hover and click) {
      focused = true;
      clicked = true;
      focusbreak = false;
    } else if (hover) {
      focused = true;
      clicked = false;
    } else {
      focused = false;
      clicked = false;
      focusbreak = true;
    }

    if (hover and !focusbreak and Global.Key1R)
      action = true;
    else
      action = false;
  }

  if (Global.errors.empty()) {
    popup.block = false;
  }

  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if (action) {
    animationStartTime = getGlobalTimer();
    animation = 1;
    animationDone = false;
    animationMs = 0; // 200
    return;
  }
}
void StartMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  // Global.mutex.lock();
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  // DrawTextureCenter(&Global.OsusLogo, 320, 200, 400.0 /
  // (float)Global.OsusLogo.width, WHITE);
  logo.render();
  description.render();
  popup.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  // MutexUnlock(SWITCHING_STATE);
  // MutexUnlock(ACCESSING_OBJECTS);
  // test.render();
  // Global.mutex.unlock();
}
void StartMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(SWITCHING_STATE);
}
void StartMenu::textureOps() {
  // NEED FIXING>>> UNLOADING DOESNT STOP RENDEWRING
}

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

WIPMenu::WIPMenu() {}

void WIPMenu::init() {
  // index = 0;
  Global.NeedForBackgroundClear = true;
  applyMouse = false;
  std::string temp = Global.Path;
  Global.Path = Path;
  dir.clear();
  dir = ls(".osu");
  // std::cout << "lsdone" << std::endl;
  std::sort(dir.begin(), dir.end());
  CanGoBack = false;
  if (Path != Global.BeatmapLocation + "/") {
    dir.insert(dir.begin(), "Back");
    CanGoBack = true;
  }
  Global.Path = temp;
  logo = LoadTexture("resources/osus.png");
  menu = LoadTexture("resources/menu.png");
  back = LoadTexture("resources/metadata.png");
  SetTextureFilter(&logo, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(&back, TEXTURE_FILTER_BILINEAR);
  float time = 0.2f;
  while (time <= 1.0f) {
    time += Global.FrameTime / 1000.0f;
    // float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
    float weirdSmooth = easeInOutCubic(time);
    angle = -250 + 255 * weirdSmooth;
    GetScale();
    GetMouse();
    GetKeys();
    updateMouseTrail();
    updateUpDown();
    _gpu_start_drawing(Global.window);
    ClearBackground(Global.Background);
    int index = 0;
    float tempangle = angle;
    if (tempangle < 0)
      tempangle -= 10;
    if (tempangle > 0)
      tempangle += 10;
    index = (tempangle) / 20;
    for (int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++) {
      float tempAngle = angle - i * 20.0f;
      while (tempAngle > 0.0f)
        tempAngle -= 360.0f;
      while (tempAngle < 0.0f)
        tempAngle += 360.0f;
      if (tempAngle < 0)
        tempAngle -= 10;
      if (tempAngle > 0)
        tempAngle += 10;
      int tempindex = (int)(tempAngle / 20) % 18;
      if (tempindex > 9)
        tempindex = 18 - tempindex;
      else
        tempindex = -tempindex;
      index = (tempangle) / 20;
      index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();

      float tempAngle2 = angle - i * 20.0f;

      int offset = 0;
      Vector2 textpos =
          getPointOnCircle(610, 220, 300 * weirdSmooth, tempAngle2 - 180.0f);
      int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
      Color temp = WHITE;
      if (dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
        temp = {255, 135, 198, 255};
      DrawTextureOnCircle(&menu, 800, 240, 300 * weirdSmooth, 0.4f, 0,
                          tempAngle2 - 180.0f, temp);
      DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
      DrawTextLeft(
          (std::to_string(
               (tempindex + index + dir.size() + offset) % dir.size() + 1) +
           " out of " + std::to_string(dir.size()))
              .c_str(),
          textpos.x, textpos.y + 15 * weirdSmooth, 7, WHITE);
    }
    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                  (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                          (unsigned char)(255 * (int)Global.Key1D), 100});
    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                  (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                          (unsigned char)(255 * (int)Global.Key2D), 100});
    renderMouse();
    DrawTextEx(
        &Global.DefaultFont, TextFormat("FPS: %d", GetFPS()),
        {static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(5))},
        Scale(20.05), Scale(2), GREEN);
    _gpu_end_drawing();
  }
  applyMouse = true;
  // SetTextureFilter(menu, TEXTURE_FILTER_BILINEAR );
  initializationStage = STATE_INITIALIZED;
}
void WIPMenu::render() {
  // Global.mutex.lock();
  int index = 0;
  float tempangle = angle;
  if (tempangle < 0)
    tempangle -= 10;
  if (tempangle > 0)
    tempangle += 10;
  index = (tempangle) / 20;
  for (int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++) {
    float tempAngle = angle - i * 20.0f;
    while (tempAngle > 0.0f)
      tempAngle -= 360.0f;
    while (tempAngle < 0.0f)
      tempAngle += 360.0f;
    if (tempAngle < 0)
      tempAngle -= 10;
    if (tempAngle > 0)
      tempAngle += 10;
    int tempindex = (int)(tempAngle / 20) % 18;
    if (tempindex > 9)
      tempindex = 18 - tempindex;
    else
      tempindex = -tempindex;
    index = (tempangle) / 20;
    index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();

    float tempAngle2 = angle - i * 20.0f;

    int offset = 0;
    Vector2 textpos = getPointOnCircle(610, 220, 300, tempAngle2 - 180.0f);
    int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
    Color temp = WHITE;
    if (dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
      temp = {255, 135, 198, 255};
    DrawTextureOnCircle(&menu, 800, 240, 300, 0.4f, 0, tempAngle2 - 180.0f,
                        temp);
    DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
    DrawTextLeft(
        (std::to_string((tempindex + index + dir.size() + offset) % dir.size() +
                        1) +
         " out of " + std::to_string(dir.size()))
            .c_str(),
        textpos.x, textpos.y + 15, 7, WHITE);
  }

  DrawTextureCenter(
      &back, 145, 240, 0.45f,
      Color{255, 255, 255,
            static_cast<unsigned char>(255 * easeInOutCubic(animtime))});
  if (TempMeta.size() == 5) {
    DrawTextLeft(
        (TempMeta[0]).c_str(), 25, 55, 9,
        Color{255, 255, 255,
              static_cast<unsigned char>(255 * easeInOutCubic(animtime))});
    DrawTextLeft(
        (TempMeta[1]).c_str(), 25, 70, 9,
        Color{255, 255, 255,
              static_cast<unsigned char>(255 * easeInOutCubic(animtime))});
    DrawTextLeft(
        (TempMeta[2]).c_str(), 25, 85, 9,
        Color{255, 255, 255,
              static_cast<unsigned char>(255 * easeInOutCubic(animtime))});
    DrawTextLeft(
        (TempMeta[3]).c_str(), 25, 100, 9,
        Color{255, 255, 255,
              static_cast<unsigned char>(255 * easeInOutCubic(animtime))});
    DrawTextLeft(
        (TempMeta[4]).c_str(), 25, 115, 9,
        Color{255, 255, 255,
              static_cast<unsigned char>(255 * easeInOutCubic(animtime))});
  }

  DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);
  // Global.mutex.unlock();
}
void WIPMenu::update() {
  float clampaccel = 0;
  if (applyMouse) {
    accel +=
        (float)(Global.FrameTime / 1000.0f) * (float)(100.0f * -Global.Wheel);
    if (accel > 60.0f)
      accel = 60.0f;
    if (accel < -60.0f)
      accel = -60.0f;
    if (accel < 0.01f and accel > -0.01f)
      accel = 0.0f;
  }
  float floatangle = ((int)posangle) % 20;
  if (floatangle >= 20.0f)
    floatangle -= 20.0f;
  if (floatangle >= 10.0f)
    clampaccel = (float)(Global.FrameTime / 1000.0f) *
                 (float)(2.5f * (20.0f - floatangle));
  else
    clampaccel =
        -(float)(Global.FrameTime / 1000.0f) * (float)(2.5f * (floatangle));

  posangle = angle;
  while (posangle < 0.0f)
    posangle += 360.0f;

  int index = 0;
  float tempangle = angle;
  if (tempangle < 0)
    tempangle -= 10;
  if (tempangle > 0)
    tempangle += 10;
  index = (tempangle) / 20;
  index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
  index = index % dir.size();

  if (Global.Key1D and Global.MouseInFocus and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{320, -2000, 320, 6000})) {
    if (Global.MouseInFocus)
      mouseMovement += Global.MousePosition.y - lastMouse;
    if (Global.MouseInFocus)
      absMouseMovement += std::abs(Global.MousePosition.y - lastMouse);
    if (absMouseMovement > 0.5f) {
      if (applyMouse)
        angle += (Global.MousePosition.y - lastMouse) / -5.0f;
      moving = true;
    }
  }
  if (!moving and Global.Key1R and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{305, 198, 210, 81})) {
    if (selectedIndex == index) {
      selectedIndex = -1;
      subObjects.clear();
    } else
      selectedIndex = index;
    index = 0;
    float tempangle = angle;
    if (tempangle < 0)
      tempangle -= 10;
    if (tempangle > 0)
      tempangle += 10;
    index = (tempangle) / 20;
    selectedAngleIndex = index;
    if (selectedIndex != -1) {
      Global.Key1R = false;
      int size = dir[selectedIndex].size();
      if (dir[selectedIndex].size() > 0 and
          dir[selectedIndex][dir[selectedIndex].size() - 1] == '/') {
        Path += dir[selectedIndex];
        ParseNameFolder(dir[selectedIndex]);
        applyMouse = false;
        float time = 0.2f;
        while (time <= 1.0f) {
          time += (Global.FrameTime / 1000.0f) * 1.0f;
          // float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
          float weirdSmooth = easeInOutCubic(1.0f - time);
          angle = -250 + 250 * weirdSmooth;
          GetScale();
          GetMouse();
          GetKeys();
          updateMouseTrail();
          updateUpDown();
          _gpu_start_drawing(Global.window);
          ClearBackground(Global.Background);
          int index = 0;
          float tempangle = angle;
          if (tempangle < 0)
            tempangle -= 10;
          if (tempangle > 0)
            tempangle += 10;
          index = (tempangle) / 20;
          for (int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++) {
            float tempAngle = angle - i * 20.0f;
            while (tempAngle > 0.0f)
              tempAngle -= 360.0f;
            while (tempAngle < 0.0f)
              tempAngle += 360.0f;
            if (tempAngle < 0)
              tempAngle -= 10;
            if (tempAngle > 0)
              tempAngle += 10;
            int tempindex = (int)(tempAngle / 20) % 18;
            if (tempindex > 9)
              tempindex = 18 - tempindex;
            else
              tempindex = -tempindex;
            index = (tempangle) / 20;
            index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();

            float tempAngle2 = angle - i * 20.0f;

            int offset = 0;
            Vector2 textpos = getPointOnCircle(610, 220, 300 * weirdSmooth,
                                               tempAngle2 - 180.0f);

            int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
            Color temp = WHITE;
            if (dir[dirNum] != "Back" and
                dir[dirNum][dir[dirNum].size() - 1] != '/')
              temp = {255, 135, 198, 255};
            DrawTextureOnCircle(&menu, 800, 240, 300 * weirdSmooth, 0.4f, 0,
                                tempAngle2 - 180.0f, temp);
            DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
            DrawTextLeft(
                (std::to_string((tempindex + index + dir.size() + offset) %
                                    dir.size() +
                                1) +
                 " out of " + std::to_string(dir.size()))
                    .c_str(),
                textpos.x, textpos.y + 15 * weirdSmooth, 7, WHITE);
          }
          DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

          DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                        (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                                (unsigned char)(255 * (int)Global.Key1D), 100});
          DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                        (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                                (unsigned char)(255 * (int)Global.Key2D), 100});
          renderMouse();
          DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d", GetFPS()),
                     {static_cast<float>((int)Scale(5)),
                      static_cast<float>((int)Scale(5))},
                     Scale(20.05), Scale(2), GREEN);
          _gpu_end_drawing();
        }
        applyMouse = true;
        init();
        index = 0;
      } else if (dir[selectedIndex] == "Back" and selectedIndex == 0 and
                 CanGoBack) {
        Path.pop_back();
        while (Path[Path.size() - 1] != '/') {
          Path.pop_back();
        }
        if (Path.size() <= (Global.BeatmapLocation + "/").size()) {
          Path = Global.BeatmapLocation + "/";
        }
        selectedIndex = -1;
        // std::cout << Path << std::endl;
        applyMouse = false;
        float time = 0.2f;
        while (time <= 1.0f) {
          time += (Global.FrameTime / 1000.0f) * 1.0f;
          // float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
          float weirdSmooth = easeInOutCubic(1.0f - time);
          angle = -250 + 250 * weirdSmooth;
          GetScale();
          GetMouse();
          GetKeys();
          updateMouseTrail();
          updateUpDown();
          _gpu_start_drawing(Global.window);
          ClearBackground(Global.Background);
          int index = 0;
          float tempangle = angle;
          if (tempangle < 0)
            tempangle -= 10;
          if (tempangle > 0)
            tempangle += 10;
          index = (tempangle) / 20;
          for (int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++) {
            float tempAngle = angle - i * 20.0f;
            while (tempAngle > 0.0f)
              tempAngle -= 360.0f;
            while (tempAngle < 0.0f)
              tempAngle += 360.0f;
            if (tempAngle < 0)
              tempAngle -= 10;
            if (tempAngle > 0)
              tempAngle += 10;
            int tempindex = (int)(tempAngle / 20) % 18;
            if (tempindex > 9)
              tempindex = 18 - tempindex;
            else
              tempindex = -tempindex;
            index = (tempangle) / 20;
            index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();

            float tempAngle2 = angle - i * 20.0f;

            int offset = 0;
            Vector2 textpos = getPointOnCircle(610, 220, 300 * weirdSmooth,
                                               tempAngle2 - 180.0f);
            int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
            Color temp = WHITE;
            if (dir[dirNum] != "Back" and
                dir[dirNum][dir[dirNum].size() - 1] != '/')
              temp = {255, 135, 198, 255};
            DrawTextureOnCircle(&menu, 800, 240, 300 * weirdSmooth, 0.4f, 0,
                                tempAngle2 - 180.0f, temp);
            DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
            DrawTextLeft(
                (std::to_string((tempindex + index + dir.size() + offset) %
                                    dir.size() +
                                1) +
                 " out of " + std::to_string(dir.size()))
                    .c_str(),
                textpos.x, textpos.y + 15 * weirdSmooth, 7, WHITE);
          }
          DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

          DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                        (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                                (unsigned char)(255 * (int)Global.Key1D), 100});
          DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                        (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                                (unsigned char)(255 * (int)Global.Key2D), 100});
          renderMouse();
          DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d", GetFPS()),
                     {static_cast<float>((int)Scale(5)),
                      static_cast<float>((int)Scale(5))},
                     Scale(20.05), Scale(2), GREEN);
          _gpu_end_drawing();
        }
        applyMouse = true;
        init();
        index = 0;
      } else if (size >= 4 and dir[selectedIndex][size - 1] == 'u' and
                 dir[selectedIndex][size - 2] == 's' and
                 dir[selectedIndex][size - 3] == 'o' and
                 dir[selectedIndex][size - 4] == '.') {
        ParseNameFile(Path + dir[selectedIndex]);
      }
      selectedIndex = -1;
    }
  }
  if (Global.Key1R and Global.MouseInFocus and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{320, -2000, 320, 6000})) {
    moving = false;
    if (applyMouse)
      accel += (Global.MousePosition.y - lastMouse) / -10.0f;
    mouseMovement = 0;
    absMouseMovement = 0;
  }
  if (Global.MouseInFocus and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{320, -2000, 320, 6000}))
    lastMouse = Global.MousePosition.y;
  // std::cout << accel << std::endl;
  if (applyMouse)
    accel += ((-accel) / 2.0f) * ((float)(Global.FrameTime / 1000.0f) * 8.0f);
  angle += accel;
  if (!moving)
    angle += clampaccel;

  if (AreSame(accel, 0.0f) and AreSame(clampaccel, 0.0f)) {
    if (dir[index] != "Back" and dir[index][dir[index].size() - 1] != '/') {
      renderMetadata = true;
      if (Metadata.size() == 0) {
        TempMeta.clear();
        int size = dir[index].size();
        std::vector<std::string> output;
        if (size >= 4 and dir[index][size - 1] == 'u' and
            dir[index][size - 2] == 's' and dir[index][size - 3] == 'o' and
            dir[index][size - 4] == '.') {
          output = ParseNameFile(Path + dir[index]);
        }
        if (output.size() >= 5) {
          Metadata.push_back("Title: " + output[0]);
          Metadata.push_back("Artist: " + output[1]);
          Metadata.push_back("Creator: " + output[2]);
          Metadata.push_back("Ver. " + output[3]);
          Metadata.push_back("ID: " + output[4]);
          TempMeta = Metadata;
        }
      }
      animtime += (Global.FrameTime / 1000.0f) * 2.0f;
      if (animtime > 1.0f)
        animtime = 1.0f;
    }
  } else {
    renderMetadata = false;
    if (Metadata.size() > 0)
      Metadata.clear();
    animtime -= (Global.FrameTime / 1000.0f) * 2.0f;
    if (animtime < 0.0f)
      animtime = 0.0f;
  }

  if (IsKeyPressed(Global.GO_BACK_KEY) and CanGoBack) {
    Path.pop_back();
    while (Path[Path.size() - 1] != '/') {
      Path.pop_back();
    }
    if (Path.size() <= (Global.BeatmapLocation + "/").size()) {
      Path = Global.BeatmapLocation + "/";
    }
    selectedIndex = -1;
    // std::cout << Path << std::endl;
    applyMouse = false;
    float time = 0.2f;
    while (time <= 1.0f) {
      time += (Global.FrameTime / 1000.0f) * 1.0f;
      // float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
      float weirdSmooth = easeInOutCubic(1.0f - time);
      angle = -250 + 250 * weirdSmooth;
      GetScale();
      GetMouse();
      GetKeys();
      updateMouseTrail();
      updateUpDown();
      _gpu_start_drawing(Global.window);
      ClearBackground(Global.Background);
      int index = 0;
      float tempangle = angle;
      if (tempangle < 0)
        tempangle -= 10;
      if (tempangle > 0)
        tempangle += 10;
      index = (tempangle) / 20;
      for (int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++) {
        float tempAngle = angle - i * 20.0f;
        while (tempAngle > 0.0f)
          tempAngle -= 360.0f;
        while (tempAngle < 0.0f)
          tempAngle += 360.0f;
        if (tempAngle < 0)
          tempAngle -= 10;
        if (tempAngle > 0)
          tempAngle += 10;
        int tempindex = (int)(tempAngle / 20) % 18;
        if (tempindex > 9)
          tempindex = 18 - tempindex;
        else
          tempindex = -tempindex;
        index = (tempangle) / 20;
        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();

        float tempAngle2 = angle - i * 20.0f;

        int offset = 0;
        Vector2 textpos =
            getPointOnCircle(610, 220, 300 * weirdSmooth, tempAngle2 - 180.0f);
        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
        Color temp = WHITE;
        if (dir[dirNum] != "Back" and
            dir[dirNum][dir[dirNum].size() - 1] != '/')
          temp = {255, 135, 198, 255};
        DrawTextureOnCircle(&menu, 800, 240, 300 * weirdSmooth, 0.4f, 0,
                            tempAngle2 - 180.0f, temp);
        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
        DrawTextLeft(
            (std::to_string(
                 (tempindex + index + dir.size() + offset) % dir.size() + 1) +
             " out of " + std::to_string(dir.size()))
                .c_str(),
            textpos.x, textpos.y + 15 * weirdSmooth, 7, WHITE);
      }
      DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

      DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                    (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                            (unsigned char)(255 * (int)Global.Key1D), 100});
      DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                    (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                            (unsigned char)(255 * (int)Global.Key2D), 100});
      renderMouse();
      DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d", GetFPS()),
                 {static_cast<float>((int)Scale(5)),
                  static_cast<float>((int)Scale(5))},
                 Scale(20.05), Scale(2), GREEN);
      _gpu_end_drawing();
    }
    applyMouse = true;
    init();
  }
  if (IsKeyPressed(Global.GO_BACK_KEY) and !CanGoBack) {
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    Global.CurrentState->unload();
    Global.CurrentState.reset(new MainMenu());
    ((MainMenu *)(Global.CurrentState.get()))->animation = 2;
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  }
}
void WIPMenu::unload() {
  // MutexLock(SWITCHING_STATE);
  dir.clear();
  subObjects.clear();
  UnloadTexture(&logo);
  UnloadTexture(&back);
  UnloadTexture(&menu);
  // MutexUnlock(SWITCHING_STATE);
}
void WIPMenu::textureOps() {}

ResultsMenu::ResultsMenu() {
  name = TextBox({320, 40}, {520, 60}, {0, 0, 0, 0}, "Results!", WHITE, 40, 50);
  close = Button({480, 440}, {160, 30}, {255, 135, 198, 255}, "Back to menu",
                 BLACK, 20);
  maxCombo = TextBox({320, 80}, {520, 60}, {0, 0, 0, 0},
                     "Maximum Combo: Not available", WHITE, 20, 50);
  hit300 = TextBox({320, 120}, {520, 60}, {0, 0, 0, 0},
                   "300s hit: Not available", WHITE, 20, 50);
  hit100 = TextBox({320, 160}, {520, 60}, {0, 0, 0, 0},
                   "100s hit: Not available", WHITE, 20, 50);
  hit50 = TextBox({320, 200}, {520, 60}, {0, 0, 0, 0}, "50s hit: Not available",
                  WHITE, 20, 50);
  hit0 = TextBox({320, 240}, {520, 60}, {0, 0, 0, 0}, "Misses: Not available",
                 WHITE, 20, 50);
  accuracy = TextBox({320, 280}, {520, 60}, {0, 0, 0, 0},
                     "Accuracy: Not available", WHITE, 20, 50);
}

void ResultsMenu::init() {
  // std::cout << "loading the scores" << std::endl;;
  // std::cout << "Maximum Combo: " +
  // std::to_string(Global.gameManager->maxCombo) << std::endl;
  maxCombo.text =
      "Maximum Combo: " + std::to_string(Global.gameManager->maxCombo);
  maxCombo.init();
  hit300.text = "300s hit: " + std::to_string(Global.gameManager->hit300s);
  hit300.init();
  hit100.text = "100s hit: " + std::to_string(Global.gameManager->hit100s);
  hit100.init();
  hit50.text = "50s hit: " + std::to_string(Global.gameManager->hit50s);
  hit50.init();
  hit0.text = "0s hit: " + std::to_string(Global.gameManager->hit0s);
  hit0.init();
  float accuracy =
      ((300.0f * Global.gameManager->hit300s +
        10.0f * Global.gameManager->hit100s +
        50.0f * Global.gameManager->hit50s) /
       (300.0f * (Global.gameManager->hit300s + Global.gameManager->hit100s +
                  Global.gameManager->hit50s + Global.gameManager->hit0s))) *
      100.0f;
  // accuracy = std::ceil(accuracy * 100.0) / 100.0;
  hit0.text = "Accuracy: " + std::to_string(accuracy) + "%";
  hit0.init();

  accuracies.clear();
  accuracies.shrink_to_fit();
  GameManager *gm = GameManager::getInstance();
  std::vector<float> local;
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "gm->objectPoints.size(): " << gm->objectPoints.size() << "\n";
  for (size_t i = 0; i < gm->objectPoints.size(); i++) {
    int start = std::max(0, (int)i - 10 + 1);
    int sum300 = 0, sum100 = 0, sum50 = 0, sum0 = 0;
    for (int j = start; j <= i; ++j) {
      if (gm->objectPoints[j] == OSU_300)
        sum300++;
      else if (gm->objectPoints[j] == OSU_100)
        sum100++;
      else if (gm->objectPoints[j] == OSU_50)
        sum50++;
      else
        sum0++;
    }
    int total = sum300 + sum100 + sum50 + sum0;
    if (total == 0) {
      local.push_back(0);
      continue;
    }
    float weighted = 300.0f * sum300 + 100.0f * sum100 + 50.0f * sum50;
    float maxPossible = 300.0f * total;
    local.push_back((weighted / maxPossible) * 100.0f);
  }

  const int maxPoints = 240;
  if (local.size() > maxPoints){

    accuracies.reserve(maxPoints);

    // Bucket size
    int bucketSize = (local.size() - 2) / (maxPoints - 2);
    if (bucketSize < 1) bucketSize = 1;

    // Always include first and last point
    accuracies.push_back(local[0]);

    for (int i = 1; i < maxPoints - 1; i++) {
        int start = (i - 1) * bucketSize + 1;
        int end = i * bucketSize;
        if (end > local.size() - 2) end = local.size() - 2;

        // Find the point in this bucket that forms the largest triangle with the previous point and the next bucket's average
        float avgX = (start + end) / 2.0f;
        float avgY = 0.0f;
        for (int j = start; j <= end; j++) {
            avgY += local[j];
        }
        avgY /= (end - start + 1);

        // Find point with largest area
        float maxArea = -1.0f;
        int selected = start;
        for (int j = start; j <= end; j++) {
            // Area of triangle (prev, current, next_bucket_avg)
            float area = std::abs((accuracies.back() - avgY) * (j - avgX) - (accuracies.back() - local[j]) * (start - avgX)) / 2.0f;
            if (area > maxArea) {
                maxArea = area;
                selected = j;
            }
        }
        accuracies.push_back(local[selected]);
    }
    accuracies.push_back(local.back());
  }
  else{
    accuracies.reserve(local.size());
    for (int i = 0; i < local.size(); i++) {
      accuracies.push_back(local[i]);
    }
  }

  local.clear();
  local.shrink_to_fit();

  Global.NeedForBackgroundClear = true;
  Global.useAuto = false;
  Global.LastFrameTime = getTimer();
  initializationStage = STATE_INITIALIZED;
}
void ResultsMenu::render() {
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  close.render();
  name.render();
  maxCombo.render();
  hit300.render();
  hit100.render();
  hit50.render();
  hit0.render();
  drawAccuracyGraph(ScaleRect({320-100, 280, 200, 100}));
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  // Global.mutex.unlock();
}
void ResultsMenu::update() {
  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  close.update();
  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  if (close.action) {
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    Global.CurrentState->unload();
    Global.CurrentState.reset(new PlayMenu());
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
    return;
  }
}
void ResultsMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
  GameManager *gm = GameManager::getInstance();
  gm->objectPoints.clear();
  gm->objectPoints.shrink_to_fit();
  accuracies.clear();
  accuracies.shrink_to_fit();
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(SWITCHING_STATE);
}
void ResultsMenu::textureOps() {}

void ResultsMenu::drawAccuracyGraph(Rectangle area) {
    if (accuracies.size() < 2) {
        DrawTextEx(&Global.DefaultFont,"Not enough data", {area.x, area.y}, Scale(20), Scale(0.5), GRAY);
        return;
    }

    // Graph padding
    float padding = 10.0f;
    float graphWidth = area.width - 2 * padding;
    float graphHeight = area.height - 2 * padding;

    // Draw background and axes
    DrawRectangleRec(area, DARKGRAY);
    DrawRectangleLinesEx(area, 1, LIGHTGRAY);

    // Find min/max (y: 0..100)
    float minAcc = 0.0f, maxAcc = 100.0f;

    // Draw grid lines at 25%, 50%, 75%, 100%
    for (int i = 0; i <= 4; i++) {
        float y = area.y + padding + graphHeight - (i * graphHeight / 4.0f);
        DrawLineEx({area.x + padding, y}, {area.x + padding + graphWidth, y}, Scale(1.5), Fade(LIGHTGRAY, 0.5f));
        DrawTextEx(&Global.DefaultFont, TextFormat("%d%%", i*25), {area.x + 2, y - 8}, Scale(12), Scale(0.5), GRAY);
    }

    // Draw the curve
    int n = accuracies.size();
    float stepX = graphWidth / (n - 1);
    for (int i = 0; i < n - 1; i++) {
        float x1 = area.x + padding + i * stepX;
        float y1 = area.y + padding + graphHeight - (accuracies[i] - minAcc) / (maxAcc - minAcc) * graphHeight;
        float x2 = area.x + padding + (i+1) * stepX;
        float y2 = area.y + padding + graphHeight - (accuracies[i+1] - minAcc) / (maxAcc - minAcc) * graphHeight;
        DrawLineEx({x1, y1}, {x2, y2}, Scale(2.5f), GREEN);
    }

    // Draw a horizontal line at 100% (if you want)
    //float y100 = area.y + padding;  // top of graph (since 100% is max)
    //DrawLineEx({area.x + padding, y100}, {area.x + padding + graphWidth}, y100, Fade(GREEN, 0.3f));
}

WipMenu2::WipMenu2() {}

void WipMenu2::init() {
  // is unstable
  locations.clear();
  locations = std::list<MenuItem>();

  folderNames.clear();
  folderNames = std::vector<std::string>();

  itemNames.clear();
  itemNames = std::vector<std::string>();
  position = minimumPosition;
  // const std::lock_guard<std::mutex> lock(scaryMulti);
  for (int i = 0; i < 16; i++) {
    MenuItem tempItem;
    tempItem.location = i * 40;
    tempItem.folder = true;
    tempItem.folderID = i;
    locations.push_back(tempItem);
    folderNames.push_back("Folder: " + std::to_string(i));
  }
  for (int i = 0; i < 6; i++) {
    itemNames.push_back("Item: " + std::to_string(i));
  }
  maximumPosition = minimumPosition + locations.back().location;
  // std::cout << "initilized the wip2 menu" << std::endl;
  removeStuffAt = -1;
  addStuffAt = -1;
  lastStuffAt = -1;
  canAddStuff = true;
  canRemoveStuff = false;
  initializationStage = STATE_INITIALIZED;
}

void WipMenu2::update() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  if (Global.Key2P && addStuffAt == -1 && removeStuffAt == -1) {
    removeStuffAt = lastStuffAt;
    addStuffAt = (lastStuffAt + 1) % 16;
  }

  bool addClamping = true;
  accel +=
      (float)(Global.FrameTime / 1000.0f) * (float)(120.0f * -Global.Wheel);
  if (accel > 120.0f)
    accel = 120.0f;
  if (accel < -120.0f)
    accel = -120.0f;
  if (accel < 0.01f and accel > -0.01f) {
    accel = 0.0f;
  }
  if (!(accel < 0.05f and accel > -0.05f)) {
    addClamping = false;
  }

  float clampAccel = 0;
  float clampingPosition = graphicalPosition;
  if (addClamping) {
    while (clampingPosition <= 0.0f)
      clampingPosition += 40.0f;
    while (clampingPosition >= 40.0f)
      clampingPosition -= 40.0f;
    if (clampingPosition >= 20.0f) {
      clampAccel = (float)(Global.FrameTime / 1000.0f) *
                   (float)(10.0f * (40.0f - clampingPosition));
      if (clampingPosition + clampAccel >= 40.0f) {
        clampAccel = 40.0f - clampingPosition;
      }
    } else {
      clampAccel = -(float)(Global.FrameTime / 1000.0f) *
                   (float)(10.0f * (clampingPosition));
      if (clampingPosition - clampAccel <= 0.0f) {
        clampAccel = -clampingPosition;
      }
    }
  }
  if (Global.Key1P) {
    lastMouse = Global.MousePosition.y;
  }
  if (Global.Key1D and Global.MouseInFocus and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{320, -2000, 320, 6000})) {
    accel = (Global.MousePosition.y - lastMouse) / -1.0f;
    addClamping = false;
  }
  if (Global.Key1R and Global.MouseInFocus and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{320, -2000, 320, 6000})) {
    accel += (Global.MousePosition.y - lastMouse) / -1.0f;
  }

  if (Global.MouseInFocus and
      CheckCollisionPointRec(Global.MousePosition,
                             Rectangle{320, -2000, 320, 6000}))
    lastMouse = Global.MousePosition.y;

  graphicalPosition += accel;
  if (addClamping)
    graphicalPosition += clampAccel;

  if (graphicalPosition < minimumPosition) {
    graphicalPosition = minimumPosition;
    accel = 0;
    clampAccel = 0;
  }
  if (graphicalPosition > maximumPosition) {
    graphicalPosition = maximumPosition;
    accel = 0;
    clampAccel = 0;
  }

  position = graphicalPosition;
  if (position < minimumPosition) {
    position = minimumPosition;
  }
  if (position > maximumPosition) {
    position = maximumPosition;
  }

  accel += ((-accel) / 2.0f) * ((float)(Global.FrameTime / 1000.0f) * 8.0f);
  if (removeStuffAt >= 0 and canRemoveStuff == true) {
    auto beginIt = locations.begin();
    int i = 0;
    while (true) {
      if (beginIt == locations.end())
        break;
      if (i >= removeStuffAt)
        break;
      beginIt++;
      i++;
    }
    // std::cout << "among us: " << i << std::endl;
    int removedItems = 0;
    while (true) {
      auto locationIt = beginIt;
      if (beginIt != locations.end()) {
        locationIt++;
      } else {
        break;
      }
      if (locationIt == locations.end()) {
        break;
      }
      if ((*locationIt).folder == true) {
        break;
      }
      removedItems += 40;
      locations.erase(locationIt);
    }
    while (true) {
      if (beginIt == locations.end())
        break;
      if (i > removeStuffAt) {
        (*beginIt).location -= removedItems;
      }
      beginIt++;
      i++;
    }
    maximumPosition = minimumPosition + locations.back().location;
    if (graphicalPosition > maximumPosition) {
      graphicalPosition = maximumPosition;
    }
    position = graphicalPosition;
    if (position < minimumPosition) {
      position = minimumPosition;
    }
    if (position > maximumPosition) {
      position = maximumPosition;
    }
    canAddStuff = true;
    canRemoveStuff = false;
    removeStuffAt = -1;
  }
  if (addStuffAt >= 0 and canAddStuff == true) {
    auto beginIt = locations.begin();
    int i = 0;
    while (true) {
      if (beginIt == locations.end())
        break;
      if (i >= addStuffAt)
        break;
      beginIt++;
      i++;
    }
    // std::cout << "among us: " << i << std::endl;
    int locationToAdd = (*beginIt).location;

    // std::cout << "locationToAdd: " << locationToAdd << std::endl;
    int addedItems = 0;

    int addNumber = (rand() % 6) + 1;
    locationToAdd += 40 * addNumber;
    for (int j = 0; j < addNumber; j++) {
      auto locationIt = beginIt;
      if (beginIt != locations.end()) {
        locationIt++;
      }
      MenuItem tempItem;
      tempItem.location = locationToAdd;
      addedItems += 40;
      locationToAdd -= 40;
      tempItem.folder = false;
      tempItem.folderID = (*beginIt).folderID;
      tempItem.itemID = j;
      locations.insert(locationIt, tempItem);
    }
    int changedI = i + addNumber;
    while (true) {
      if (beginIt == locations.end())
        break;
      if (i > changedI) {
        (*beginIt).location += addedItems;
      }
      beginIt++;
      i++;
    }
    maximumPosition = minimumPosition + locations.back().location;
    if (graphicalPosition > maximumPosition) {
      graphicalPosition = maximumPosition;
    }
    position = graphicalPosition;
    if (position < minimumPosition) {
      position = minimumPosition;
    }
    if (position > maximumPosition) {
      position = maximumPosition;
    }
    canAddStuff = false;
    canRemoveStuff = true;
    lastStuffAt = addStuffAt;
    addStuffAt = -1;
  }

  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if (IsKeyPressed(Global.GO_BACK_KEY)) {
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    Global.CurrentState->unload();
    Global.CurrentState.reset(new MainMenu());
    ((MainMenu *)(Global.CurrentState.get()))->animation = 2;
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
    return;
  }
}
void WipMenu2::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  Rectangle rect;
  rect.x = 322;
  rect.y = 240 - 18;
  rect.width = 2;
  rect.height = 36;
  DrawRectangleRec(ScaleRect(rect), {200, 120, 200, 128});

  rect.x = 5;
  rect.y = 5;
  rect.width = 315;
  rect.height = 470;

  DrawRectangleRec(ScaleRect(rect), {70, 30, 70, 170});
  for (MenuItem n : locations) {
    Rectangle r;
    if (n.folder) {
      r.x = 325;
      r.y = (n.location - 18) - graphicalPosition;
      r.width = 315;
      r.height = 36;
    } else {
      r.x = 330;
      r.y = (n.location - 18) - graphicalPosition;
      r.width = 310;
      r.height = 36;
    }
    if (r.y + r.width < -10 or r.y > 490)
      continue;
    if (n.folder) {
      DrawRectangleRec(ScaleRect(r), {255, 255, 255, 255});
      std::string name = "unknown";
      if (n.folderID >= 0 && n.folderID < folderNames.size()) {
        name = folderNames[n.folderID];
      }
      // DrawTextLeft(name.c_str(), r.x + 10, r.y + 18, 20.05, BLACK);
      DrawTextEx(&Global.DefaultFont, name.c_str(),
                 {static_cast<float>((int)ScaleCordX(r.x + 10)),
                  static_cast<float>((int)ScaleCordY(r.y + 18 - 10))},
                 Scale(20.05), Scale(2), BLACK);
    } else {
      DrawRectangleRec(ScaleRect(r), {200, 150, 200, 255});
      std::string name = "unknown";
      if (n.itemID >= 0 && n.itemID < itemNames.size()) {
        name = itemNames[n.itemID];
      }
      // DrawTextLeft(name.c_str(), r.x + 10, r.y + 18, 20.05, BLACK);
      DrawTextEx(&Global.DefaultFont, name.c_str(),
                 {(float)((int)ScaleCordX(r.x + 10)),
                  (float)((int)ScaleCordY(r.y + 18 - 10))},
                 Scale(20.05), Scale(2), BLACK);
    }
  }
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
}
void WipMenu2::unload() {
  initializationStage = STATE_UNINITIALIZED;
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(ACCESSING_OBJECTS);
  // MutexLock(ACCESSING_OBJECTS);
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "locking render\n";
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "starting menu unload\n";
  locations.clear();
  locations = std::list<MenuItem>();

  folderNames.clear();
  folderNames = std::vector<std::string>();

  itemNames.clear();
  itemNames = std::vector<std::string>();
  // MutexUnlock(ACCESSING_OBJECTS);
  // MutexUnlock(SWITCHING_STATE);
}
void WipMenu2::textureOps() {}