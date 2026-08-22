#include "fastrender.hpp"
#include "fs.hpp"
#include "gamemanager.hpp"
#include "globals.hpp"
#include "menuElements.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "settingsParser.hpp"
#include "state.hpp"
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
  close = Button({leftMostX + 15, bottomMostY - 15}, {20, 20}, {0xAA, 0x00, 0xAA, 192}, "x", BLACK, 20); //Button({70, 110}, {20, 20}, {0xAA, 0x00, 0xAA, 192}, "x", BLACK, 20); //{0xAA, 0x00, 0xAA, 192}
  skin = Switch({310, 350}, {40, 20}, RED, GREEN, {255, 135, 198, 255}, BLACK);
  sound = Switch({310, 370}, {40, 20}, RED, GREEN, {255, 135, 198, 255}, BLACK);
  usedskin = TextBox({180, 350}, {190, 20}, {240, 98, 161, 255},
                     "Use default skin", WHITE, 20, 50);
  usedsound = TextBox({180, 370}, {190, 20}, {240, 98, 161, 255},
                      "Use default sound", WHITE, 20, 50);
  skin.state = Global.settings.useDefaultSkin;
  sound.state = Global.settings.useDefaultSounds;
}

void PlayMenu::init() {
  lastSelection=0;
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
  menu.elements[0].get()->baseColor = {64, 48, 64, 192};
  menu.elements[0].get()->textColor = WHITE;
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[1].get()->baseColor = {64, 48, 64, 192};
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[2].get()->baseColor = {48, 32, 48, 192};
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[3].get()->baseColor = {0xAA, 0x00, 0xAA, 192};
  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[4].get()->baseColor = {0xAA, 0x00, 0xAA, 192}; // CC00AAFF

  // std::cout << menu.elements[0]->baseColor.r << " " <<
  // menu.elements[0]->baseColor.g << " " << menu.elements[0]->baseColor.b <<
  // std::endl;

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

  menu.elements[0].get()->positions.push_back({rightMostX - 310, topMostY});
  menu.elements[0].get()->positions.push_back({rightMostX, bottomMostY});

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
  leftSideBox.SetFontSize(30.05);
  leftSideBox.SetSpacing(1);
  leftSideBox.SetWrapWords(true);
  leftSideBox.SetBox((Rectangle){leftMostX + 20, topMostY + 40,
                                 (rightMostX - 330) - (leftMostX + 20),
                                 (bottomMostY - topMostY) - 80});
  // std::cout << menu.elements.size() << " " <<
  // menu.elements[0]->positions.size() << std::endl;
  initializationStage = STATE_INITIALIZED;
  // MutexUnlock(SWITCHING_STATE);
}
void PlayMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  // Global.mutex.lock();
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  //bg.render();
  //description.render();
  //back.render();
  //select.render();
  //dir_list.render();
  //close.render();
  //skin.render();
  //sound.render();
  //usedskin.render();
  //usedsound.render();
  //name.render();
  menu.render();

  int selection = dynamic_cast<FancyScrollingList *>(menu.elements[0].get())
                      ->currentSelection;
  // DrawTextEx(&Global.DefaultFont, TextFormat("Selection: %d Graphical: %.0f",
  // selection,
  // dynamic_cast<FancyScrollingList*>(menu.elements[0].get())->graphicalObjectOffset),
  // {static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(25))},
  // Scale(20.05), Scale(2), BLUE);
  leftSideBox.Draw(HAlign::Left, VAlign::Top, WHITE);
  close.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);

  // MutexUnlock(ACCESSING_OBJECTS);
  // MutexUnlock(SWITCHING_STATE);
  // Global.mutex.unlock();
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
  dir_list.update();
  select.update();
  back.update();
  close.update();
  skin.update();
  sound.update();
  /*if(lastIndex != dir_list.selectedindex){
      name.text = dir_list.objects[dir_list.selectedindex].text;
      name.update();
      lastIndex = dir_list.selectedindex;
  }*/
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

    menu.elements[0].get()->positions[0] = {rightMostX - 310, topMostY};
    menu.elements[0].get()->positions[1] = {rightMostX, bottomMostY};

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
  }

  dynamic_cast<FancyScrollingList *>(menu.elements[0].get())->frameChange +=
      (Global.Wheel);

  // dynamic_cast<FancyScrollingList*>(menu.elements[4].get())->updateTextBox =
  // Global.Wheel != 0 ? true : false;
  // dynamic_cast<FancyScrollingList*>(menu.elements[4].get())->updateTexts =
  // Global.Wheel != 0 ? true : false;
  menu.update();

  auto *fancyList = dynamic_cast<FancyScrollingList *>(menu.elements[0].get());

  if (dynamic_cast<ClickableObject *>(menu.elements[3].get())->action) {
    dynamic_cast<ClickableObject *>(menu.elements[3].get())->action = false;
    if (!inBeatmapView) {
      // Switch from set view to beatmap view
      int selection = fancyList->currentSelection;
      lastSelection = selection;
      if (!beatmapSets.empty() && selection >= 0 &&
          selection < (int)beatmapSets.size()) {
        int setid = beatmapSets[selection].setid;
        currentBeatmaps.clear();
        currentBeatmaps = parseCachedMaps(Global.DatabaseLocation, setid);
        // Repopulate the fancy list
        //std::cout << "stareting clear" << std::endl;
        fancyList->objectNames.clear();
        //std::cout << "stareting population" << std::endl;
        for (int i = 0; i < currentBeatmaps.size(); i++) {
          std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << currentBeatmaps[i].title + " [" +
                           currentBeatmaps[i].version + "]"
                    << std::endl;
          fancyList->objectNames.push_back(currentBeatmaps[i].version);
        }

        fancyList->currentSelection = 0; // reset highlight
        //std::cout << "added names\n";
        fancyList->reinit();
        //std::cout << "init fancylist\n";
        inBeatmapView = true;
        auto *selectBtn = dynamic_cast<ClickableObject *>(menu.elements[3].get());
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
    dynamic_cast<ClickableObject *>(menu.elements[4].get())->action = false;
    if (inBeatmapView) {
      inBeatmapView = false;
      currentBeatmaps.clear();
      fancyList->objectNames.clear();
      for (const auto &set : beatmapSets) {
        fancyList->objectNames.push_back(set.title);
      }
      fancyList->currentSelection = lastSelection;
      fancyList->reinit();
      auto *selectBtn = dynamic_cast<ClickableObject *>(menu.elements[3].get());
      selectBtn->text = "Select";
      selectBtn->internalBox.SetText("Select");
    }
  }

  int selection = dynamic_cast<FancyScrollingList *>(menu.elements[0].get())
                      ->currentSelection;
  if (!beatmapSets.empty()) {
    selection %= beatmapSets.size();

    if (!inBeatmapView) {
      std::string newString = TextFormat(
          "[cCC00AAFF]Title: [r]%s\n[cCC00AAFF]Maps: "
          "[r]%d\n[cCC00AAFF]Artists: "
          "[r]%s\n[cCC00AAFF]Creators: [r]%s\n[cCC00AA99]SetID: [cFFFFFF99]%d",
          beatmapSets[selection].title.c_str(), beatmapSets[selection].number,
          beatmapSets[selection].artists.c_str(),
          beatmapSets[selection].creators.c_str(),
          beatmapSets[selection].setid);
      if (newString != leftSideFormatted) {
        leftSideFormatted = newString;
        leftSideBox.SetText(leftSideFormatted);
      }
    } else {
      std::string newString = TextFormat(
          "[cCC00AAFF]Version: [r]%s\n[cCC00AAFF]Artist: "
          "[r]%s\n[cCC00AAFF]Creator: "
          "[r]%s\n[cCC00AAFF]Title: [r]%s\n[cCC00AA99]SetID: [cFFFFFF99]%d",
          currentBeatmaps[selection].version.c_str(),
          currentBeatmaps[selection].artist.c_str(),
          currentBeatmaps[selection].creator.c_str(),
          currentBeatmaps[selection].title.c_str(),
          currentBeatmaps[selection].setid);
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

  //if (select.action or dir_list.action) {
  //  if (dir_list.objects.size() > 0 and
  //      dir_list.objects[dir_list.selectedindex].text.size() > 0) {
  //    if (dir_list.objects[dir_list.selectedindex]
  //            .text[dir_list.objects[dir_list.selectedindex].text.size() - 1] ==
  //        '/') {
  //      MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //      dir_list.objects[dir_list.selectedindex].text.pop_back();
  //      if (Global.Path.size() == 1)
  //        Global.Path.pop_back();
  //      Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
  //      lastPos = dir_list.objects[dir_list.selectedindex].text;
  //      auto dir = ls(".osu");
  //      dir_list =
  //          SelectableList(dir_list.position, dir_list.size, dir_list.color,
  //                         dir, dir_list.textcolor, dir_list.textsize,
  //                         dir_list.objectsize, dir_list.maxlength);
  //      dir_list.init();
  //      lastIndex = -3;
  //      MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //    } else {
  //      MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
  //      MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
  //      MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //      Global.selectedPath =
  //          Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
  //      Global.CurrentLocation = "beatmaps/" + lastPos + "/";
  //      Global.CurrentState->unload();
  //      Global.CurrentState.reset(new Game());
  //      Global.CurrentState->init();
  //      MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //      MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
  //      MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  //    }
  //  }
  //} else if (back.action) {
  //  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //  Global.Path = Global.BeatmapLocation;
  //  auto dir = ls(".osu");
  //  dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color,
  //                            dir, dir_list.textcolor, dir_list.textsize,
  //                            dir_list.objectsize, dir_list.maxlength);
  //  dir_list.init();
  //  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  //}
  //// MutexUnlock(ACCESSING_OBJECTS);
  //// MutexUnlock(SWITCHING_STATE);
}
void PlayMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
  menu.deinit();
  beatmapSets.clear();
  leftSideFormatted.clear();
  inBeatmapView = false;
  currentBeatmaps.clear();
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(SWITCHING_STATE);
}
void PlayMenu::textureOps() {}
