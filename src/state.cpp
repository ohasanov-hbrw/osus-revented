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