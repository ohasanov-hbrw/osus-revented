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

PlayMenu *PlayMenu::instance = nullptr;

enum PLAYMENU_ELEMENTS {
  SCROLLER = 0,
  BOTTOM_CARD = 1,
  TOP_CARD = 2,
  SELECT_BUTTON = 3,
  BACK_BUTTON = 4
};

PlayMenu::PlayMenu() {
  float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
  float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
  float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
  float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

  name = TextBox({320, 440}, {520, 40}, {0, 0, 0, 0}, "BETA VERSION!", WHITE,
                 20, 50);
  description = TextBox({320, 140}, {520, 40}, {240, 98, 161, 255},
                        "Select a Beatmap to play!", WHITE, 20, 50);
  bg = TextBox({320, 240}, {530, 290}, {240, 98, 161, 255}, "",
               {240, 98, 161, 255}, 20, 10);
  back = Button({395, 360}, {120, 40}, {255, 135, 198, 255}, "Back", WHITE, 20);
  select =
      Button({520, 360}, {120, 40}, {255, 135, 198, 255}, "Select", BLACK, 20);
  close = Button({leftMostX + 15, bottomMostY - 15}, {20, 20},
                 {0xAA, 0x00, 0xAA, 192}, "x", BLACK,
                 20); // Button({70, 110}, {20, 20}, {0xAA, 0x00, 0xAA, 192},
                      // "x", BLACK, 20); //{0xAA, 0x00, 0xAA, 192}
  options = Button({leftMostX + 75, bottomMostY - 15}, {90, 20},
                   {0xAA, 0x00, 0xAA, 192}, "Options", BLACK,
                   20); // Button({70, 110}, {20, 20}, {0xAA, 0x00, 0xAA, 192},
                        // "x", BLACK, 20); //{0xAA, 0x00, 0xAA, 192}
  skin = Switch({leftMostX + 220, bottomMostY - 40}, {40, 20}, RED, GREEN,
                {32, 24, 32, 192}, BLACK);
  sound = Switch({leftMostX + 220, bottomMostY - 60}, {40, 20}, RED, GREEN,
                 {32, 24, 32, 192}, BLACK);
  usedskin = TextBox({leftMostX + 100, bottomMostY - 40}, {200, 20},
                     {32, 24, 32, 192}, "Default skin", WHITE, 20, 50);
  usedsound = TextBox({leftMostX + 100, bottomMostY - 60}, {200, 20},
                      {32, 24, 32, 192}, "Default sound", WHITE, 20, 50);

  skin.position = {leftMostX + 220, bottomMostY - 40};
  sound.position = {leftMostX + 220, bottomMostY - 60};
  usedskin.position = {leftMostX + 100, bottomMostY - 40};
  usedsound.position = {leftMostX + 100, bottomMostY - 60};
  options.position = {leftMostX + 75, bottomMostY - 15};
  skin.state = Global.settings.useDefaultSkin;
  sound.state = Global.settings.useDefaultSounds;
  showOptions = false;
}

void PlayMenu::init() {

  lastSelection = 0;
  // MutexLock(SWITCHING_STATE);
  // std::cout << "loading the playmenu/n";
  Global.NeedForBackgroundClear = true;
  Global.useAuto = false;
  Global.LastFrameTime = getGlobalTimer();
  temp = Global.Path;
  Global.Path = Global.BeatmapLocation;
  std::vector<std::string> dir = ls(".osu");

  dir_list = SelectableList({320, 250}, {520, 160}, {255, 135, 198, 255}, dir,
                            BLACK, 20, 20, 65);

  menu.elements.push_back(std::make_unique<FancyScrollingList>());
  menu.elements[SCROLLER].get()->baseColor = {32, 24, 32, 128};
  menu.elements[SCROLLER].get()->textColor = WHITE;
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[1].get()->baseColor = {64, 48, 64, 192};
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[2].get()->baseColor = {48, 32, 48, 192};
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[3].get()->baseColor = {0xAA, 0x00, 0xAA, 192};
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[4].get()->baseColor = {0xAA, 0x00, 0xAA, 192}; // CC00AAFF

  float spacingWidth = 160;
  float spacingWidthTop = 240;
  float spacingHeight = 40;
  float spacingAngleWidth = 20;
  float height = 10;

  float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
  float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
  float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
  float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

  menu.elements[1].get()->positions = Create_Shape_Rectangle_With_Trapezoid(
      MenuSizes.bottomCard.anchor, true, false, 0, MenuSizes.bottomCard.height,
      0, 0, MenuSizes.bottomCard.edgeHeight, MenuSizes.bottomCard.edgeWidth,
      MenuSizes.bottomCard.edgeRampWidth);
  menu.elements[2].get()->positions = Create_Shape_Rectangle_With_Trapezoid(
      MenuSizes.topCard.anchor, true, false, 0, MenuSizes.topCard.height, 0, 0,
      MenuSizes.topCard.edgeHeight, MenuSizes.topCard.edgeWidth,
      MenuSizes.topCard.edgeRampWidth);
  menu.elements[3].get()->positions =
      Create_Shape_Trapezoid(BOTTOM_RIGHT, 125, 40, 135, 10, 40., 0);
  menu.elements[4].get()->positions =
      Create_Shape_Trapezoid(BOTTOM_RIGHT, 80, 40, 230, 10, 40., (-40.));

  menu.elements[3].get()->text = "Select";
  menu.elements[4].get()->text = "Back";
  dynamic_cast<ClickableObject *>(menu.elements[3].get())->staticobject = false;
  dynamic_cast<ClickableObject *>(menu.elements[4].get())->staticobject = false;

  menu.elements[4].get()->internalBox.SetText(menu.elements[4].get()->text);
  menu.elements[4].get()->internalBox.SetFontSize(20.05);
  menu.elements[4].get()->internalBox.SetSpacing(2);
  menu.elements[4].get()->internalBox.SetWrapWords(false);
  menu.elements[4].get()->textColor = WHITE;
  menu.elements[4].get()->horizontalAlign = HAlign::Center;

  menu.elements[3].get()->internalBox.SetText(menu.elements[3].get()->text);
  menu.elements[3].get()->internalBox.SetFontSize(20.05);
  menu.elements[3].get()->internalBox.SetSpacing(2);
  menu.elements[3].get()->internalBox.SetWrapWords(false);
  menu.elements[3].get()->textColor = WHITE;
  menu.elements[3].get()->horizontalAlign = HAlign::Center;

  menu.elements[3].get()->textRect =
      Rectangle{rightMostX - 115, bottomMostY - 10 - 40, 105, 40};
  menu.elements[3].get()->internalBox.SetBox(menu.elements[3].get()->textRect);

  menu.elements[4].get()->textRect =
      Rectangle{rightMostX - 220, bottomMostY - 10 - 40, 105, 40};
  menu.elements[4].get()->internalBox.SetBox(menu.elements[4].get()->textRect);

  menu.elements[SCROLLER].get()->positions.push_back(
      {rightMostX - 310, topMostY});
  menu.elements[SCROLLER].get()->positions.push_back({rightMostX, bottomMostY});

  inBeatmapView = false;
  currentBeatmaps.clear();
  beatmapSets.clear();
  beatmapSets = parseCachedSets(Global.DatabaseLocation + "/beatmapsets.db");
  for (int i = 0; i < beatmapSets.size(); i++) {
    dynamic_cast<FancyScrollingList *>(menu.elements[0].get())
        ->objectNames.push_back(beatmapSets[i].title);
  }
  menu.init();
  leftSideBox.SetText(leftSideFormatted);
  leftSideBox.SetFontSize(30.08f);
  leftSideBox.SetSpacing(2);
  leftSideBox.SetWrapWords(true);
  leftSideBox.SetBox((Rectangle){leftMostX + 20, topMostY + 40,
                                 (rightMostX - 330) - (leftMostX + 20),
                                 (bottomMostY - topMostY) - 80});
  loadLoaderThread();
  initializationStage = STATE_INITIALIZED;
}

void PlayMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  // Global.mutex.lock();
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
  float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
  float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
  float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  // bg.render();
  // description.render();
  // back.render();
  // select.render();
  // dir_list.render();
  // close.render();

  // name.render();

  float screenW = rightMostX - leftMostX;
  float screenH = bottomMostY - topMostY;

  auto *fancyList =
      dynamic_cast<FancyScrollingList *>(menu.elements[SCROLLER].get());

  int selectionCurrent = fancyList ? fancyList->currentSelection : 0;
  if (!inBeatmapView && !beatmapSets.empty() && selectionCurrent >= 0 &&
      selectionCurrent < (int)beatmapSets.size()) {
    int selectedSid = beatmapSets[selectionCurrent % beatmapSets.size()].setid;
    for (auto &slot : icons) {
      if (slot.setid.load() == selectedSid &&
          slot.state.load() == TEX_STATE_READY) {
        float screenAspect = screenW / screenH;
        float texW = (float)slot.texture.width;
        float texH = (float)slot.texture.height;
        float texAspect = texW / texH;
        Rectangle srcRect;
        if (texAspect > screenAspect) {
          float cropW = texH * screenAspect;
          float cropX = (texW - cropW) / 2.0f;
          srcRect = {cropX, 0.0f, cropW, texH};
        } else {
          float cropH = texW / screenAspect;
          float cropY = (texH - cropH) / 2.0f;
          srcRect = {0.0f, cropY, texW, cropH};
        }
        Rectangle destRect = ScaleRect({leftMostX, topMostY, screenW, screenH});
        DrawTexturePro(&slot.texture, srcRect, destRect, {0.0f, 0.0f}, 0.0f,
                       Color{64, 64, 64, 128});
        break;
      }
    }
  } else if (inBeatmapView) {
    int selection = fancyList->currentSelection;
    if (!currentBeatmaps.empty() && selection >= 0 &&
        selection < (int)currentBeatmaps.size()) {
      int sid = currentBeatmaps[selection].setid;
      for (auto &slot : icons) {
        if (slot.setid.load() == sid && slot.state.load() == TEX_STATE_READY) {
          float screenAspect = screenW / screenH;
          float texW = (float)slot.texture.width;
          float texH = (float)slot.texture.height;
          float texAspect = texW / texH;
          Rectangle srcRect;
          if (texAspect > screenAspect) {
            float cropW = texH * screenAspect;
            float cropX = (texW - cropW) / 2.0f;
            srcRect = {cropX, 0.0f, cropW, texH};
          } else {
            float cropH = texW / screenAspect;
            float cropY = (texH - cropH) / 2.0f;
            srcRect = {0.0f, cropY, texW, cropH};
          }
          Rectangle destRect =
              ScaleRect({leftMostX, topMostY, screenW, screenH});
          DrawTexturePro(&slot.texture, srcRect, destRect, {0.0f, 0.0f}, 0.0f,
                         Color{64, 64, 64, 128});
          break;
        }
      }
    }
  }

  if (!inBeatmapView && fancyList && !beatmapSets.empty()) {
    for (size_t i = 0; i < fancyList->objects.size(); ++i) {
      int itemIdx = (int)i - (int)fancyList->objects.size() / 2 -
                    fancyList->graphicalObjectOffsetFull;

      if (itemIdx < 0 || itemIdx >= (int)beatmapSets.size())
        continue;

      const auto &obj = fancyList->objects[i];

      // Skip off-screen items
      if (obj->positions[1].y <= fancyList->positions[0].y ||
          obj->positions[0].y >= fancyList->positions[1].y)
        continue;

      int sid = beatmapSets[itemIdx].setid;

      for (auto &slot : icons) {
        if (slot.setid.load() == sid && slot.state.load() == TEX_STATE_READY) {
          float cardX = obj->positions[0].x;
          float cardY = obj->positions[0].y;
          float cardW = obj->positions[1].x - obj->positions[0].x;
          float cardH = obj->positions[1].y - obj->positions[0].y;
          Rectangle destRect = ScaleRect({cardX, cardY, cardW, cardH});
          float cardAspect = cardW / cardH;
          float texAspect =
              (float)slot.texture.width / (float)slot.texture.height;

          Rectangle srcRect;
          if (texAspect > cardAspect) {
            float srcW = slot.texture.height * cardAspect;
            float srcX = (slot.texture.width - srcW) / 2.0f;
            srcRect = {srcX, 0.0f, srcW, (float)slot.texture.height};
          } else {
            float srcH = slot.texture.width / cardAspect;
            float srcY = (slot.texture.height - srcH) / 2.0f;
            srcRect = {0.0f, srcY, (float)slot.texture.width, srcH};
          }
          Color tint = WHITE;
          tint.a = obj->baseColor.a;

          DrawTexturePro(&slot.texture, srcRect, destRect, {0.0f, 0.0f}, 0.0f,
                         tint);
          break;
        }
      }
    }
  }

  menu.render();

  int selection = dynamic_cast<FancyScrollingList *>(menu.elements[0].get())
                      ->currentSelection;
  leftSideBox.Draw(HAlign::Left, VAlign::Top, WHITE);
  if (showOptions) {
    skin.render();
    sound.render();
    usedskin.render();
    usedsound.render();
  }

  options.render();
  close.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);

  DrawRectangleLinesEx(ScaleRect({rightMostX - 317.5, 240 - 85 / 2.0, 340, 85}),
                       Scale(4), {0xAA, 0x00, 0xAA, 192});
}
void PlayMenu::update() {
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  float spacingWidth = 160;
  float spacingWidthTop = 240;
  float spacingHeight = 40;
  float spacingAngleWidth = 20;
  float height = 20;

  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  Global.enableMouse = true;
  // dir_list.update();
  // select.update();
  // back.update();
  close.update();
  if (showOptions) {
    skin.update();
    sound.update();
  }
  options.update();
  if (options.action)
    showOptions = !showOptions;

  if (skin.state != Global.settings.useDefaultSkin)
    Global.settings.useDefaultSkin = skin.state;

  if (sound.state != Global.settings.useDefaultSounds)
    Global.settings.useDefaultSounds = sound.state;

  if (Global.ScaleUpdated) {
    Update_Shape_Rectangle_With_Trapezoid(
        MenuSizes.bottomCard.anchor, true, false, 0,
        MenuSizes.bottomCard.height, 0, 0, MenuSizes.bottomCard.edgeHeight,
        MenuSizes.bottomCard.edgeWidth, MenuSizes.bottomCard.edgeRampWidth,
        &(menu.elements[1].get()->positions));
    Update_Shape_Rectangle_With_Trapezoid(
        MenuSizes.topCard.anchor, true, false, 0, MenuSizes.topCard.height, 0,
        0, MenuSizes.topCard.edgeHeight, MenuSizes.topCard.edgeWidth,
        MenuSizes.topCard.edgeRampWidth, &(menu.elements[2].get()->positions));
    Update_Shape_Trapezoid(BOTTOM_RIGHT, 125, 40, 135, 10, 40., 0,
                           &(menu.elements[3].get()->positions));
    Update_Shape_Trapezoid(BOTTOM_RIGHT, 80, 40, 230, 10, 40., (-40.),
                           &(menu.elements[4].get()->positions));
    dynamic_cast<ClickableObject *>(menu.elements[3].get())->boundary =
        dynamic_cast<ClickableObject *>(menu.elements[3].get())
            ->GetTriangleStripBoundary(menu.elements[3].get()->positions);
    dynamic_cast<ClickableObject *>(menu.elements[4].get())->boundary =
        dynamic_cast<ClickableObject *>(menu.elements[4].get())
            ->GetTriangleStripBoundary(menu.elements[4].get()->positions);

    float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
    float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
    float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
    float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

    menu.elements[SCROLLER].get()->positions[0] = {rightMostX - 310, topMostY};
    menu.elements[SCROLLER].get()->positions[1] = {rightMostX, bottomMostY};

    leftSideBox.SetBox((Rectangle){leftMostX + 20, topMostY + 40,
                                   (rightMostX - 330) - (leftMostX + 20),
                                   (bottomMostY - topMostY) - 80});
    menu.elements[3].get()->textRect =
        Rectangle{rightMostX - 115, bottomMostY - 10 - 40, 105, 40};
    menu.elements[3].get()->internalBox.SetBox(
        menu.elements[3].get()->textRect);
    menu.elements[4].get()->textRect =
        Rectangle{rightMostX - 220, bottomMostY - 10 - 40, 105, 40};
    menu.elements[4].get()->internalBox.SetBox(
        menu.elements[4].get()->textRect);
    close.position = {leftMostX + 15, bottomMostY - 15};
    skin.position = {leftMostX + 220, bottomMostY - 40};
    sound.position = {leftMostX + 220, bottomMostY - 60};
    usedskin.position = {leftMostX + 100, bottomMostY - 40};
    usedsound.position = {leftMostX + 100, bottomMostY - 60};
    options.position = {leftMostX + 75, bottomMostY - 15};
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    unloadLoaderThread(false);
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  }

  dynamic_cast<FancyScrollingList *>(menu.elements[0].get())->frameChange +=
      (Global.Wheel);
  menu.update();

  if (icons.size() == 0) {
    std::cout << "trying to reload loaderthread" << std::endl;
    loadLoaderThread();
  }

  auto *fancyList = dynamic_cast<FancyScrollingList *>(menu.elements[0].get());

  if (dynamic_cast<ClickableObject *>(menu.elements[3].get())->action) {

    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    unloadLoaderThread(false);
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

    dynamic_cast<ClickableObject *>(menu.elements[3].get())->action = false;
    if (!inBeatmapView) {
      // Switch from set view to beatmap view
      int selection = fancyList->currentSelection;
      lastSelection = fancyList->objectOffsetFull;
      if (!beatmapSets.empty() && selection >= 0 &&
          selection < (int)beatmapSets.size()) {
        int setid = beatmapSets[selection].setid;
        currentBeatmaps.clear();
        currentBeatmaps = parseCachedMaps(Global.DatabaseLocation, setid);
        // Repopulate the fancy list
        // std::cout << "stareting clear" << std::endl;
        fancyList->objectNames.clear();
        // std::cout << "stareting population" << std::endl;
        for (int i = 0; i < currentBeatmaps.size(); i++) {
          std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
                    << currentBeatmaps[i].title + " [" +
                           currentBeatmaps[i].version + "]"
                    << std::endl;
          fancyList->objectNames.push_back(currentBeatmaps[i].version);
        }

        fancyList->reinit();
        fancyList->updateTexts = true;
        fancyList->updateTextBox = true;
        fancyList->update();
        inBeatmapView = true;
        auto *selectBtn =
            dynamic_cast<ClickableObject *>(menu.elements[3].get());
        selectBtn->text = "Play";
        selectBtn->internalBox.SetText("Play");
      }
    } else {
      int selection = fancyList->currentSelection;
      if (!currentBeatmaps.empty() && selection >= 0 &&
          selection < (int)currentBeatmaps.size()) {
        Global.selectedPath = currentBeatmaps[selection].path;
        MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

        MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
        MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
        MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
        Global.CurrentLocation =
            "beatmaps/" + std::to_string(currentBeatmaps[selection].setid) +
            "/";
        Global.CurrentState->unload();
        Global.CurrentState.reset(new Game());
        Global.CurrentState->init();
        MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
        MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
        MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
        return; // after state change
      }
    }
  }
  if (dynamic_cast<ClickableObject *>(menu.elements[4].get())->action) {
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    unloadLoaderThread(false);
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

    dynamic_cast<ClickableObject *>(menu.elements[4].get())->action = false;
    if (inBeatmapView) {
      inBeatmapView = false;
      currentBeatmaps.clear();
      fancyList->objectNames.clear();
      for (const auto &set : beatmapSets) {
        fancyList->objectNames.push_back(set.title);
      }
      // fancyList->currentSelection = lastSelection;
      fancyList->reinit();
      fancyList->objectOffsetFull = lastSelection;
      fancyList->graphicalObjectOffsetFull = lastSelection;
      // fancyList->graphicalObjectOffsetFull = 0;
      // fancyList->currentSelection = -fancyList->objectOffsetFull;
      fancyList->graphicalObjectOffset = 0;
      fancyList->updateTexts = true;
      fancyList->updateTextBox = true;
      fancyList->update();
      auto *selectBtn = dynamic_cast<ClickableObject *>(menu.elements[3].get());
      selectBtn->text = "Select";
      selectBtn->internalBox.SetText("Select");
    }
  }

  int selection = dynamic_cast<FancyScrollingList *>(menu.elements[0].get())
                      ->currentSelection;
  if (!beatmapSets.empty()) {
    if (!inBeatmapView) {
      selection %= beatmapSets.size();
      std::string newString = TextFormat(
          "[cCC00AAFF]Title: [r]%s\n\n[cCC00AAFF]Maps: "
          "[r]%d\n[cCC00AAFF]Artists: "
          "[r]%s\n[cCC00AAFF]Creators: [r]%s\n[cCC00AA99]SetID: "
          "[cFFFFFF99]%d" //,
          ,
          beatmapSets[selection].title.c_str(), beatmapSets[selection].number,
          beatmapSets[selection].artists.c_str(),
          beatmapSets[selection].creators.c_str(),
          beatmapSets[selection].setid);
      if (newString != leftSideFormatted) {
        leftSideFormatted = newString;
        leftSideBox.SetText(leftSideFormatted);
      }
    } else {
      selection %= currentBeatmaps.size();
      std::string newString = TextFormat(
          "[cCC00AAFF]Version: [r]%s\n\n[cCC00AAFF]Creator: "
          "[r]%s\n[cCC00AAFF]Title: "
          "[r]%s\n[cCC00AAFF]Artist: [r]%s\n[cCC00AA99]ID: [cFFFFFF99]%d",
          currentBeatmaps[selection].version.c_str(),
          currentBeatmaps[selection].creator.c_str(),
          currentBeatmaps[selection].title.c_str(),
          currentBeatmaps[selection].artist.c_str(),
          currentBeatmaps[selection].id);
      if (newString != leftSideFormatted) {
        leftSideFormatted = newString;
        leftSideBox.SetText(leftSideFormatted);
      }
    }
  }

  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if (close.action) {
    Global.Path = temp;
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    Global.CurrentState->unload();
    Global.CurrentState.reset(new MainMenu());
    ((MainMenu *)(Global.CurrentState.get()))->animation = 2; // 2
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
    return;
  }

  if (inBeatmapView) {
    int selection = fancyList->currentSelection;
    if (!currentBeatmaps.empty() && selection >= 0 &&
        selection < (int)currentBeatmaps.size()) {
      int sid = currentBeatmaps[selection].setid;
      bool alreadyAssigned = false;
      for (const auto &slot : icons) {
        if (slot.setid.load() == sid && slot.state.load() != TEX_STATE_FREE) {
          alreadyAssigned = true;
          break;
        }
      }
      if (!alreadyAssigned) {
        for (auto &slot : icons) {
          if (slot.state.load() == TEX_STATE_FREE) {
            slot.setid.store(sid);
            slot.state.store(TEX_STATE_NEEDS_DISK_LOAD);
            break;
          }
        }
      }
    }
    for (auto &slot : icons) {
      if (slot.state.load() == TEX_STATE_DISK_LOADED ||
          slot.state.load() == TEX_STATE_NEEDS_UNLOAD)
        textureOpsDone.store(false);
    }
    return;
  }
  if (!fancyList || beatmapSets.empty())
    return;

  int totalSets = beatmapSets.size();
  int numSlots = fancyList->objects.size();

  int minVisibleIdx = -numSlots / 2 - fancyList->graphicalObjectOffsetFull - 1;
  int maxVisibleIdx = numSlots / 2 - fancyList->graphicalObjectOffsetFull + 1;

  std::unordered_set<int> visibleSetIds;
  for (int itemIdx = maxVisibleIdx; itemIdx >= minVisibleIdx; itemIdx--) {
    if (itemIdx >= 0 && itemIdx < totalSets) {
      visibleSetIds.insert(beatmapSets[itemIdx].setid);
    }
  }

  // 1. Mark out-of-view textures for GPU unload
  for (auto &slot : icons) {
    int sid = slot.setid.load();
    int st = slot.state.load();

    if (st != TEX_STATE_FREE && sid != -1) {
      if (visibleSetIds.find(sid) == visibleSetIds.end()) {
        if (st == TEX_STATE_READY) {
          slot.state.store(TEX_STATE_NEEDS_UNLOAD);
        } else if (st == TEX_STATE_NEEDS_DISK_LOAD) {
          slot.state.store(TEX_STATE_FREE);
          slot.setid.store(-1);
        }
      }
    }
  }

  // 2. Assign missing visible setIDs to free slots
  for (int sid : visibleSetIds) {
    bool alreadyAssigned = false;
    for (const auto &slot : icons) {
      if (slot.setid.load() == sid && slot.state.load() != TEX_STATE_FREE) {
        alreadyAssigned = true;
        break;
      }
    }

    if (!alreadyAssigned) {
      for (auto &slot : icons) {
        if (slot.state.load() == TEX_STATE_FREE) {
          slot.setid.store(sid);
          slot.state.store(TEX_STATE_NEEDS_DISK_LOAD);
          break;
        }
      }
    }
  }

  for (auto &slot : icons) {
    if (slot.state.load() == TEX_STATE_DISK_LOADED ||
        slot.state.load() == TEX_STATE_NEEDS_UNLOAD)
      textureOpsDone.store(false);
  }

  // Signal render thread that ops are pending

  // if (select.action or dir_list.action) {
  //   if (dir_list.objects.size() > 0 and
  //       dir_list.objects[dir_list.selectedindex].text.size() > 0) {
  //     if (dir_list.objects[dir_list.selectedindex]
  //             .text[dir_list.objects[dir_list.selectedindex].text.size() - 1]
  //             ==
  //         '/') {
  //       MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //       dir_list.objects[dir_list.selectedindex].text.pop_back();
  //       if (Global.Path.size() == 1)
  //         Global.Path.pop_back();
  //       Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
  //       lastPos = dir_list.objects[dir_list.selectedindex].text;
  //       auto dir = ls(".osu");
  //       dir_list =
  //           SelectableList(dir_list.position, dir_list.size, dir_list.color,
  //                          dir, dir_list.textcolor, dir_list.textsize,
  //                          dir_list.objectsize, dir_list.maxlength);
  //       dir_list.init();
  //       lastIndex = -3;
  //       MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //     } else {
  //       MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
  //       MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
  //       MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //       Global.selectedPath =
  //           Global.Path + '/' +
  //           dir_list.objects[dir_list.selectedindex].text;
  //       Global.CurrentLocation = "beatmaps/" + lastPos + "/";
  //       Global.CurrentState->unload();
  //       Global.CurrentState.reset(new Game());
  //       Global.CurrentState->init();
  //       MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //       MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
  //       MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  //     }
  //   }
  // } else if (back.action) {
  //   MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //   Global.Path = Global.BeatmapLocation;
  //   auto dir = ls(".osu");
  //   dir_list = SelectableList(dir_list.position, dir_list.size,
  //   dir_list.color,
  //                             dir, dir_list.textcolor, dir_list.textsize,
  //                             dir_list.objectsize, dir_list.maxlength);
  //   dir_list.init();
  //   MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  // }
  //// MutexUnlock(ACCESSING_OBJECTS);
  //// MutexUnlock(SWITCHING_STATE);
}

static int currentTexNumber = 0;
static int currentImgNumber = 0;

void PlayMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;

  unloadLoaderThread(true);

  instance = nullptr;

  menu.deinit();
  beatmapSets.clear();
  leftSideFormatted.clear();
  inBeatmapView = false;
  currentBeatmaps.clear();
}

// only this function can convert images to textures
void PlayMenu::textureOps() {
  if (textureOpsDone)
    return;

  for (auto &slot : icons) {
    int st = slot.state.load();

    // Upload decoded CPU image to GPU VRAM
    if (st == TEX_STATE_DISK_LOADED) {
      currentTexNumber++;
      std::cout << currentTexNumber << " " << slot.setid.load()
                << " loaded a tex\n";

      slot.texture = LoadTextureFromImage(&slot.image);
      currentImgNumber--;
      std::cout << "image: " << currentImgNumber << std::endl;
      UnloadImage(&slot.image);
      SetTextureFilter(&slot.texture, TEXTURE_FILTER_BILINEAR);
      slot.state.store(TEX_STATE_READY);
    }
    // Free VRAM when scrolled out of view
    else if (st == TEX_STATE_NEEDS_UNLOAD) {
      currentTexNumber--;
      std::cout << currentTexNumber << " " << slot.setid.load()
                << " unloaded a tex\n";
      slot.state.store(TEX_STATE_FREE);
      UnloadTexture(&slot.texture);
      slot.setid.store(-1);
    }
  }

  textureOpsDone.store(true);
}

void PlayMenu::workerThreadEntryPoint(void *arg) {
  if (instance) {
    instance->workerThreadImpl();
  }
}

void PlayMenu::workerThreadImpl() {
  while (!loaderShouldStop.load()) {
    bool worked = false;

    for (auto &slot : icons) {
      if (slot.state.load() == TEX_STATE_NEEDS_DISK_LOAD) {
        int sid = slot.setid.load();
        std::string path = Global.DatabaseLocation + "/" + std::to_string(sid) + "/cover_" +
                           std::to_string(sid) + "_0.bmp";

        if (FileExists(path.c_str())) {
          slot.image = LoadImage(path.c_str());
          currentImgNumber++;
          std::cout << "image: " << currentImgNumber << std::endl;
          slot.state.store(TEX_STATE_DISK_LOADED);

        } else {
          slot.state.store(TEX_STATE_FREE);
          slot.setid.store(-1);
        }
        worked = true;
        break;
      }
    }

    if (worked) {
      textureOpsDone.store(false);
    }

    if (!worked) {
      SleepInMs(16);
    }
  }
}

void PlayMenu::unloadLoaderThread(bool lockedMutexes) {
  if (!loaderLoaded)
    return;

  loaderShouldStop.store(true);
  _multithread_join_thread(&backgroundLoader);
  _multithread_free_thread(&backgroundLoader);
  for (auto &slot : icons) {
    if (slot.state.load() == TEX_STATE_READY) {
      slot.state.store(TEX_STATE_NEEDS_UNLOAD);
    }
    if (slot.state.load() == TEX_STATE_DISK_LOADED) {
      slot.state.store(TEX_STATE_FREE);
      currentImgNumber--;
      std::cout << "image: " << currentImgNumber << std::endl;
      UnloadImage(&slot.image);
    }
  }
  if (lockedMutexes) {
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  }
  textureOpsDone.store(false);
  while (!textureOpsDone.load()) {
    std::cout << "waiting for textureops\n";
    SleepInMs(lockedMutexes ? 50 : 1);
  }
  if (lockedMutexes) {
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    icons.clear();
  } else {
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    icons.clear();
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  }
  loaderLoaded = false;
}

void PlayMenu::loadLoaderThread() {
  if (loaderLoaded)
    return;
  instance = this;
  loaderShouldStop.store(false);
  textureOpsDone.store(true);
  auto *fancyList =
      dynamic_cast<FancyScrollingList *>(menu.elements[SCROLLER].get());
  int numSlots = fancyList ? fancyList->objects.size() : 12;
  std::cout << "allocated place for " << numSlots << " objects" << std::endl;
  icons.resize(numSlots + 2);
  for (auto &slot : icons) {
    slot.state.store(TEX_STATE_FREE);
    slot.setid.store(-1);
  }
  loaderLoaded = true;
  backgroundLoader =
      _multithread_thread_create(&PlayMenu::workerThreadEntryPoint);
}