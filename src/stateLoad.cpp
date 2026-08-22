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


LoadMenu::LoadMenu() {
  description =
      TextBox({320, 140}, {520, 40}, {240, 98, 161, 255},
              "Select a .OSZ Beatmap file\n    to extract!", WHITE, 20, 50);
  bg = TextBox({320, 240}, {530, 290}, {240, 98, 161, 255}, "",
               {240, 98, 161, 255}, 20, 10);
  back = Button({395, 360}, {120, 40}, {255, 135, 198, 255}, "Back", BLACK, 20);
  select =
      Button({520, 360}, {120, 40}, {255, 135, 198, 255}, "Select", BLACK, 20);
  close = Button({70, 110}, {20, 20}, {255, 135, 198, 255}, "x", BLACK, 20);
  auto dir = ls(".osz");
  dir_list = SelectableList({320, 260}, {520, 150}, {255, 135, 198, 255}, dir,
                            BLACK, 20, 20, 60);
  path = TextBox({195, 360}, {270, 40}, {240, 98, 161, 255}, Global.Path, WHITE,
                 20, 40);
}

void LoadMenu::init() {
  // MutexLock(SWITCHING_STATE);
  Global.NeedForBackgroundClear = true;
  Global.useAuto = false;
  Global.LastFrameTime = getGlobalTimer();
  Global.FrameTime = 0.5;
  initializationStage = STATE_INITIALIZED;
  // MutexUnlock(SWITCHING_STATE);
}
void LoadMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  // Global.mutex.lock();
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  bg.render();
  description.render();
  back.render();
  path.render();
  select.render();
  dir_list.render();
  close.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  // MutexUnlock(SWITCHING_STATE);
  // MutexUnlock(ACCESSING_OBJECTS);
  // Global.mutex.unlock();
}
void LoadMenu::update() {
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  Global.enableMouse = true;
  dir_list.update();
  select.update();
  back.update();
  close.update();
  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if (close.action) {
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

  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  if (select.action or dir_list.action) {
    if (dir_list.objects.size() > 0 and
        dir_list.objects[dir_list.selectedindex].text.size() > 0) {
      if (dir_list.objects[dir_list.selectedindex]
              .text[dir_list.objects[dir_list.selectedindex].text.size() - 1] ==
          '/') {
        dir_list.objects[dir_list.selectedindex].text.pop_back();
        if (Global.Path.size() == 1)
          Global.Path.pop_back();
        Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
        auto dir = ls(".osz");
        dir_list =
            SelectableList(dir_list.position, dir_list.size, dir_list.color,
                           dir, dir_list.textcolor, dir_list.textsize,
                           dir_list.objectsize, dir_list.maxlength);
        dir_list.init();
        path = TextBox(path.position, path.size, path.color, Global.Path,
                       path.textcolor, path.textsize, path.maxlength);
      } else {
        Global.selectedPath =
            Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
        std::string base_file = get_without_ext(Global.selectedPath);
        std::string final_path = Global.GameBinaryPath + "/beatmaps/" +
                                 base_file; //  may be a bit cringe when it
                                            //  comes to unzipping stuff...
        final_path = correct_path_notation(final_path);
        if (check_dir(final_path)) {
          std::cout << "\e[1;38;5;219m[ZIP] \e[38;5;220m"
                    << "the path already exists?" << std::endl;
        } else {
          std::cout << "\e[1;38;5;219m[ZIP] \e[38;5;219m"
                    << "trying to unzip new beatmap" << std::endl;
          create_dir(final_path);
          int arg = 2;
          // std::cout << Global.selectedPath.c_str() << std::endl;
          print_dir(final_path);
          int res = zip_extract(Global.selectedPath.c_str(), final_path.c_str(),
                                on_extract_entry, NULL);
          std::cout << "\e[1;38;5;219m[ZIP] \e[38;5;219m"
                    << "ZIP errorcode:" << res << std::endl;
        }
      }
    }
  } else if (back.action) {
    while (Global.Path.size() > 0) {
      if (Global.Path[Global.Path.size() - 1] == '/' ||
          Global.Path[Global.Path.size() - 1] == ':') {
        if (Global.Path.size() > 1 && Global.Path.size() != 2)
          Global.Path.pop_back();
        break;
      }
      Global.Path.pop_back();
    }
    auto dir = ls(".osz");
    dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color,
                              dir, dir_list.textcolor, dir_list.textsize,
                              dir_list.objectsize, dir_list.maxlength);
    dir_list.init();
    path = TextBox(path.position, path.size, path.color, Global.Path,
                   path.textcolor, path.textsize, path.maxlength);
  }
  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  // MutexUnlock(SWITCHING_STATE);
  // MutexUnlock(ACCESSING_OBJECTS);
}
void LoadMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(SWITCHING_STATE);
}
void LoadMenu::textureOps() {}
