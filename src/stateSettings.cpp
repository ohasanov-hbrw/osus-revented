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
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <dirent.h>
#include <gamefile.hpp>
#include <iostream>
#include <memory>
#include <ostream>

#include "menu_shapes/shapes.hpp"

SettingsMenu::SettingsMenu() {}

static void updateSettingsInfo(TextBoxSmart &box, std::string &cached,
                               int selection,
                               const std::vector<std::string> &names) {
  const char *name = (selection >= 0 && selection < (int)names.size())
                         ? names[selection].c_str()
                         : "None";
  char buffer[512];
  std::snprintf(buffer, sizeof(buffer),
                "Settings\n\nSelected: %s\n\nScroll with the mouse wheel or "
                "by dragging the list.\nClick an option to bring it to the "
                "centre.\nClick the centred option to select it.",
                name);
  std::string text(buffer);
  if (text != cached) {
    cached = text;
    box.SetText(text);
  }
}

void SettingsMenu::init() {
  Global.NeedForBackgroundClear = true;
  Global.useAuto = false;
  Global.LastFrameTime = getGlobalTimer();

  float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
  float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
  float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
  float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

  menu.elements.push_back(std::make_unique<FancyScrollingList>());
  menu.elements[SCROLLER].get()->baseColor = {32, 24, 32, 128};
  menu.elements[SCROLLER].get()->textColor = WHITE;

  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[SAVE_BUTTON].get()->baseColor = {0xAA, 0x00, 0xAA, 192};
  menu.elements[SAVE_BUTTON].get()->positions =
      Create_Shape_Trapezoid(BOTTOM_RIGHT, 80, 40, 230, 10, 40., (-40.));

  auto *list =
      dynamic_cast<FancyScrollingList *>(menu.elements[SCROLLER].get());
  list->positions.push_back({rightMostX - 310, topMostY});
  list->positions.push_back({rightMostX, bottomMostY});
  list->mouseInputRect = {rightMostX - 310, topMostY, 310,
                          (bottomMostY - topMostY) - 55};
  list->objectNames = {"Skin", "Mouse", "Display", "Database"};

  auto *save =
      dynamic_cast<ClickableObject *>(menu.elements[SAVE_BUTTON].get());
  save->staticobject = false;
  save->text = "Save";
  save->internalBox.SetText(save->text);
  save->internalBox.SetFontSize(20);
  save->internalBox.SetSpacing(2);
  save->internalBox.SetWrapWords(false);
  save->textColor = WHITE;
  save->horizontalAlign = HAlign::Center;
  save->textRect = Rectangle{rightMostX - 220, bottomMostY - 10 - 40, 105, 40};
  save->internalBox.SetBox(save->textRect);

  menu.elements.push_back(std::make_unique<ClickableObject>());
  menu.elements[CHANGE_BUTTON].get()->baseColor = {0xAA, 0x00, 0xAA, 192};
  menu.elements[CHANGE_BUTTON].get()->positions =
      Create_Shape_Trapezoid(BOTTOM_RIGHT, 125, 40, 135, 10, 40., 0);
  auto *change =
      dynamic_cast<ClickableObject *>(menu.elements[CHANGE_BUTTON].get());
  change->staticobject = false;
  change->text = "Change";
  change->internalBox.SetText(change->text);
  change->internalBox.SetFontSize(20);
  change->internalBox.SetSpacing(2);
  change->internalBox.SetWrapWords(false);
  change->textColor = WHITE;
  change->horizontalAlign = HAlign::Center;
  change->textRect =
      Rectangle{rightMostX - 115, bottomMostY - 10 - 40, 105, 40};
  change->internalBox.SetBox(change->textRect);

  leftSideBox.SetFontSize(30.08f);
  leftSideBox.SetSpacing(2);
  leftSideBox.SetWrapWords(true);
  leftSideBox.SetBox((Rectangle){leftMostX + 20, topMostY + 40,
                                  (rightMostX - 330) - (leftMostX + 20),
                                  (bottomMostY - topMostY) - 80});

  topBar.baseColor = {48, 32, 48, 192};
  topBar.positions = Create_Shape_Rectangle_With_Trapezoid(
      MenuSizes.topCard.anchor, true, false, 0, MenuSizes.topCard.height, 0, 0,
      MenuSizes.topCard.edgeHeight, MenuSizes.topCard.edgeWidth,
      MenuSizes.topCard.edgeRampWidth);
  topBar.init();

  bottomCard.baseColor = {64, 48, 64, 192};
  bottomCard.positions = Create_Shape_Rectangle_With_Trapezoid(
      MenuSizes.bottomCard.anchor, true, false, 0, MenuSizes.bottomCard.height,
      0, 0, MenuSizes.bottomCard.edgeHeight, MenuSizes.bottomCard.edgeWidth,
      MenuSizes.bottomCard.edgeRampWidth);
  bottomCard.init();

  close = Button({leftMostX + 15, bottomMostY - 15}, {20, 20},
                 {0xAA, 0x00, 0xAA, 192}, "x", BLACK, 20);

  menu.init();
  updateSettingsInfo(leftSideBox, leftSideFormatted, list->currentSelection,
                     list->objectNames);
  initializationStage = STATE_INITIALIZED;
}

void SettingsMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;

  Global.NeedForBackgroundClear = true;

  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  topBar.render();
  menu.elements[SCROLLER]->render();
  bottomCard.render();
  menu.elements[CHANGE_BUTTON]->render();
  menu.elements[SAVE_BUTTON]->render();
  leftSideBox.Draw(HAlign::Left, VAlign::Top, WHITE);
  close.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);

  float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
  DrawRectangleLinesEx(ScaleRect({rightMostX - 317.5, 240 - 85 / 2.0, 340, 85}),
                       Scale(4), {0xAA, 0x00, 0xAA, 192});
}

void SettingsMenu::update() {
  if (initializationStage != STATE_INITIALIZED)
    return;

  float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
  float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
  float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
  float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  Global.enableMouse = true;

  close.update();

  auto *list =
      dynamic_cast<FancyScrollingList *>(menu.elements[SCROLLER].get());
  auto *save =
      dynamic_cast<ClickableObject *>(menu.elements[SAVE_BUTTON].get());
  auto *change =
      dynamic_cast<ClickableObject *>(menu.elements[CHANGE_BUTTON].get());

  if (Global.ScaleUpdated) {
    list->positions[0] = {rightMostX - 310, topMostY};
    list->positions[1] = {rightMostX, bottomMostY};
    list->mouseInputRect = {rightMostX - 310, topMostY, 310,
                            (bottomMostY - topMostY) - 55};

    save->positions =
        Create_Shape_Trapezoid(BOTTOM_RIGHT, 80, 40, 230, 10, 40., (-40.));
    save->boundary = save->GetTriangleStripBoundary(save->positions);
    save->textRect =
        Rectangle{rightMostX - 220, bottomMostY - 10 - 40, 105, 40};
    save->internalBox.SetBox(save->textRect);

    change->positions =
        Create_Shape_Trapezoid(BOTTOM_RIGHT, 125, 40, 135, 10, 40., 0);
    change->boundary = change->GetTriangleStripBoundary(change->positions);
    change->textRect =
        Rectangle{rightMostX - 115, bottomMostY - 10 - 40, 105, 40};
    change->internalBox.SetBox(change->textRect);

    leftSideBox.SetBox((Rectangle){leftMostX + 20, topMostY + 40,
                                    (rightMostX - 330) - (leftMostX + 20),
                                    (bottomMostY - topMostY) - 80});

    Update_Shape_Rectangle_With_Trapezoid(
        MenuSizes.topCard.anchor, true, false, 0, MenuSizes.topCard.height, 0,
        0, MenuSizes.topCard.edgeHeight, MenuSizes.topCard.edgeWidth,
        MenuSizes.topCard.edgeRampWidth, &topBar.positions);
    Update_Shape_Rectangle_With_Trapezoid(
        MenuSizes.bottomCard.anchor, true, false, 0,
        MenuSizes.bottomCard.height, 0, 0, MenuSizes.bottomCard.edgeHeight,
        MenuSizes.bottomCard.edgeWidth, MenuSizes.bottomCard.edgeRampWidth,
        &bottomCard.positions);
    close.position = {leftMostX + 15, bottomMostY - 15};
  }

  list->frameChange += Global.Wheel;
  menu.update();

  if (list->centerItemActivated) {
    list->centerItemActivated = false;
    int selection = list->currentSelection;
    if (selection >= 0 && selection < (int)list->objectNames.size()) {
      std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
                << "settings selection: " << list->objectNames[selection]
                << std::endl;
    }
  }

  updateSettingsInfo(leftSideBox, leftSideFormatted, list->currentSelection,
                     list->objectNames);

  bool leaveMenu = close.action;
  save->action = false;
  change->action = false;
  close.action = false;
  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if (leaveMenu) {
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

void SettingsMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
  topBar.deinit();
  bottomCard.deinit();
  menu.deinit();
  leftSideFormatted.clear();
}

// only this function can convert images to textures
void SettingsMenu::textureOps() {}
