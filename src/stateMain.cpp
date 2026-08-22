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

MainMenu::MainMenu() {
  play = Button({250, 420}, {120, 60}, {255, 135, 198, 255}, "Play", BLACK, 20);
  wip = Button({500, 340}, {220, 40}, {255, 135, 198, 0}, "  Load Database", BLACK, 20);
  wip2 = Button({500, 300}, {120, 40}, {255, 135, 198, 0}, "WIP2", BLACK, 20);
  load = Button({390, 420}, {120, 60}, {255, 135, 198, 255}, "Load", BLACK, 20);
  volume = TestSlider({510, 460}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
  popup =
      Popup({320, 240}, {300, 120}, GRAY, "Test Popup", WHITE, 20, 1 << 0, -1);
  logo = ImageObject({320, 200}, {400, 400}, WHITE, 1, 0, &Global.OsusLogo);
  animation = 0;
  animationStart = 0;
  animationDone = false;
}

void MainMenu::init() {
  // MutexLock(SWITCHING_STATE);
  Global.NeedForBackgroundClear = true;
  Global.LastFrameTime = getGlobalTimer();
  Global.FrameTime = 0.5;
  Global.useAuto = false;
  volume.location = Global.volume * 100.0f;
  setlocale(LC_ALL, "en_US.utf8");

  ErrorMessage tempMsg;
  tempMsg.id = Global.errorid;
  // Global.errorid++;
  tempMsg.message = "sugomatest\nsagop";
  tempMsg.type = ERR_FILEIO;
  // Global.errors.push(tempMsg);

  popup.block = !Global.errors.empty();

  if (animation == 0) {
    play =
        Button({250, 420}, {120, 60}, {255, 135, 198, 255}, "Play", BLACK, 20);
    wip = Button({500, 340}, {220, 40}, {255, 135, 198, 0}, "  Load Database", BLACK, 20);
    wip2 = Button({500, 300}, {120, 40}, {255, 135, 198, 0}, "WIP2", BLACK, 20);
    load =
        Button({390, 420}, {120, 60}, {255, 135, 198, 255}, "Load", BLACK, 20);
    volume = TestSlider({510, 460}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
    popup = Popup({320, 240}, {300, 120}, GRAY, tempMsg.message.c_str(), WHITE,
                  20, 1 << 0, -1);
  } else if (animation == 1) {
    play =
        Button({320, 240}, {120, 60}, {255, 135, 198, 255}, "Play", BLACK, 20);
    wip = Button({370, 290}, {220, 40}, {150, 80, 120, 255}, "  Load Database", BLACK, 20);
    wip2 = Button({540, 290}, {120, 40}, {100, 60, 80, 255}, "WIP2", BLACK, 20);
    load =
        Button({320, 240}, {120, 60}, {200, 100, 160, 255}, "Load", BLACK, 20);
    volume = TestSlider({320, 240}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
    popup = Popup({320, 240}, {300, 120}, GRAY, tempMsg.message.c_str(), WHITE,
                  20, 1 << 0, -1);
    logo = ImageObject({320, 200}, {400, 400}, WHITE, 1, 0, &Global.OsusLogo);
    popup.block = false;
    animationDone = false;
    animationMs = 0;//200
  } else if (animation == 2) {
    play = Button({0, 240}, {120, 60}, {255, 135, 198, 255}, "Play", BLACK, 20);
    wip = Button({0, 290}, {220, 40}, {150, 80, 120, 255}, "  Load Database", BLACK, 20);
    wip2 = Button({0, 290}, {120, 40}, {100, 60, 80, 255}, "WIP2", BLACK, 20);
    load = Button({0, 240}, {120, 60}, {200, 100, 160, 255}, "Load", BLACK, 20);
    volume = TestSlider({320, 240}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
    popup = Popup({320, 240}, {300, 120}, GRAY, tempMsg.message.c_str(), WHITE,
                  20, 1 << 0, -1);
    logo = ImageObject({0, 240}, {400, 400}, WHITE, 0, 0, &Global.OsusLogo);
    animationDone = false;
    animationMs = 0;//200
  } else if (animation == -1 || animation == -2) {
    play =
        Button({380, 240}, {120, 60}, {255, 135, 198, 255}, "Play", BLACK, 20);
    wip = Button({370, 290}, {220, 40}, {150, 80, 120, 255}, "  Load Database", BLACK, 20);
    wip2 = Button({540, 290}, {160, 40}, {100, 60, 80, 255}, "WIP2", BLACK, 20);
    load =
        Button({540, 240}, {160, 60}, {200, 100, 160, 255}, "Load", BLACK, 20);
    volume = TestSlider({320, 240}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
    popup = Popup({320, 240}, {300, 120}, GRAY, tempMsg.message.c_str(), WHITE,
                  20, 1 << 0, -1);
    logo = ImageObject({160, 240}, {300, 300}, WHITE, 0, 0, &Global.OsusLogo);
    animationDone = false;
    animationMs = 0; //200
  }
  animationStartTime = getGlobalTimer(); // disable animation
  initializationStage = STATE_INITIALIZED;
  // MutexUnlock(SWITCHING_STATE);
}

void toupper(std::string &s) {
  for (char &c : s)
    c = std::toupper(c);
}
bool strcasecmp2(std::string lhs, std::string rhs) {
  toupper(lhs);
  toupper(rhs);
  return lhs < rhs;
}

void MainMenu::update() {

  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);

  if (animation == 0) {
    animationDone = true;
  } else if (animation == 1) {
    double position = (getGlobalTimer() - animationStartTime) / animationMs;
    play.position = lerp({320, 240}, {380, 240}, position);
    play.textsize = 20 + 20 * position;
    load.position = lerp({320, 240}, {540, 240}, position);
    load.textsize = 20 + 20 * position;

    wip2.position = lerp({320, 290}, {440, 290}, position);

    logo.rotation = -360 * position;
    logo.position = lerp({320, 200}, {160, 240}, position);

    logo.size = lerp({400, 400}, {300, 300}, position);

    play.size = lerp({120, 60}, {160, 60}, position);
    load.size = lerp({120, 60}, {160, 60}, position);

    if (getGlobalTimer() - animationStartTime > animationMs) {
      animation = 0;
      animationDone = true;

      play.position = {380, 240};
      play.textsize = 40;
      load.position = {540, 240};
      load.textsize = 40;
      play.size = {160, 60};
      load.size = {160, 60};
      logo.rotation = 0;
      logo.size = {300, 300};
      logo.position = {160, 240};

      wip2.position = {540, 290};
    }
  } else if (animation == 2) {
    double position = (getGlobalTimer() - animationStartTime) / animationMs;
    play.position = lerp({0, 240}, {380, 240}, position);
    play.textsize = 20 + 20 * position;
    load.position = lerp({0, 240}, {540, 240}, position);
    load.textsize = 20 + 20 * position;

    wip.position = lerp({0, 290}, {370, 290}, position);
    wip2.position = lerp({0, 290}, {540, 290}, position);

    logo.position = lerp({0, 240}, {160, 240}, position);

    logo.size = lerp({400, 400}, {300, 300}, position);

    play.size = lerp({120, 60}, {160, 60}, position);
    load.size = lerp({120, 60}, {160, 60}, position);

    play.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    load.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip2.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));

    logo.opacity = position;

    play.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    load.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip2.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));

    if (getGlobalTimer() - animationStartTime > animationMs) {
      animation = 0;
      animationDone = true;

      play.position = {380, 240};
      play.textsize = 40;
      load.position = {540, 240};
      load.textsize = 40;
      play.size = {160, 60};
      load.size = {160, 60};
      logo.rotation = 0;
      logo.size = {300, 300};
      logo.position = {160, 240};

      wip2.position = {540, 290};
      wip.position = {370, 290};

      play.textcolor.a = 255;
      load.textcolor.a = 255;
      wip.textcolor.a = 255;
      wip2.textcolor.a = 255;

      logo.opacity = 1;

      play.color.a = 255;
      load.color.a = 255;
      wip.color.a = 255;
      wip2.color.a = 255;
    }
  } else if (animation == -1 || animation == -2) {
    play.focused = false;
    load.focused = false;
    wip.focused = false;
    wip2.focused = false;
    double position = 1 - ((getGlobalTimer() - animationStartTime) / animationMs);
    play.position = lerp({0, 240}, {380, 240}, position);
    play.textsize = 20 + 20 * position;
    load.position = lerp({0, 240}, {540, 240}, position);
    load.textsize = 20 + 20 * position;

    wip.position = lerp({0, 290}, {370, 290}, position);
    wip2.position = lerp({0, 290}, {540, 290}, position);

    logo.position = lerp({0, 240}, {160, 240}, position);

    logo.size = lerp({400, 400}, {300, 300}, position);

    play.size = lerp({120, 60}, {160, 60}, position);
    load.size = lerp({120, 60}, {160, 60}, position);

    play.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    load.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip2.textcolor.a = (unsigned char)((int)clip((position * 255.0), 0, 255));

    logo.opacity = position;

    play.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    load.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));
    wip2.color.a = (unsigned char)((int)clip((position * 255.0), 0, 255));

    if (getGlobalTimer() - animationStartTime > animationMs) {
      play.position = {0, 240};
      play.textsize = 20;
      load.position = {0, 240};
      load.textsize = 20;
      play.size = {120, 60};
      load.size = {120, 60};
      logo.rotation = 0;
      logo.size = {400, 400};
      logo.position = {0, 240};

      wip2.position = {0, 290};
      wip.position = {0, 290};

      play.textcolor.a = 0;
      load.textcolor.a = 0;
      wip.textcolor.a = 0;
      wip2.textcolor.a = 0;

      logo.opacity = 0;

      play.color.a = 0;
      load.color.a = 0;
      wip.color.a = 0;
      wip2.color.a = 0;

      if (animation == -1) {
        MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
        MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
        MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
        return;
      } else if (animation == -2) {
        MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
        MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new LoadMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
        MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
        return;
      }
      animation = 0;
      animationDone = true;
    }
  }

  if (!animationDone) {
    return;
  }

  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  Global.enableMouse = true;

  popup.update();

  if (popup.action) {
    if (popup.ans == 2) {
      Global.errors.pop();
    }
  }

  if (!popup.block) {
    play.update();
    wip.update();
    wip2.update();
    load.update();
  }

  if (Global.errors.empty()) {
    popup.block = false;
  }

  

  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if(wip.action){
    Global.doingTimeConsumingOp = true;
    buildFileMap(Global.BeatmapLocation);
    listAllMaps();
    #ifdef THREEDS_BUILD
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free Vram: " << _os_get_free_vram() / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_ALL: " << _os_get_free_ram(MEMREGION_ALL) / 1024 << "/" << _os_get_size_ram(MEMREGION_ALL) / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_APP: " << _os_get_free_ram(MEMREGION_APPLICATION) / 1024 << "/" << _os_get_size_ram(MEMREGION_APPLICATION) / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_SYS: " << _os_get_free_ram(MEMREGION_SYSTEM) / 1024 << "/" << _os_get_size_ram(MEMREGION_SYSTEM) / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_BSE: " << _os_get_free_ram(MEMREGION_BASE) / 1024<< "/" << _os_get_size_ram(MEMREGION_BASE) / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_LIN: " << _os_get_free_linear_ram() / 1024 << "/" << Global.linearSpaceFree / 1024 << "KB" << std::endl;
    #endif
    decideNamesForSets();
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Decided Names" << std::endl;
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Done rebuilding database" << std::endl;
    Global.doingTimeConsumingOp = 0;
  }

  // test.update();
  
  if (false && wip.action) {
    // Global.CurrentState->unload();
    // Global.CurrentState.reset(new WIPMenu());
    // Global.CurrentState->init();
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    std::string temp = Global.Path;
    Global.Path = Global.GameBinaryPath + "/database";

    struct dirent *de;
    DIR *dr = opendir(Global.Path.c_str());
    if (dr == NULL) { // opendir returns NULL if couldn't open directory {
      printf("Could not open current directory: database");
    } else {
      while ((de = readdir(dr)) != NULL) {
        std::remove((Global.Path + "/" + de->d_name).c_str());
      }
      closedir(dr);
    }

    Global.Path = temp;
    Parser parser = Parser();
    for (const auto &p : std::filesystem::recursive_directory_iterator(
             Global.BeatmapLocation)) {
      if (!std::filesystem::is_directory(p)) {
        if (p.path().extension().string() == ".osu") {
          // std::cout << p.path().string() << '\n';
          GameFile geym;
          geym = parser.parseMetadata(p.path().string());
          std::string GameTitle;
          std::string GameSetId;
          std::string GameVersion;
          std::string GameAuthor;

          if (geym.configMetadata.find("BeatmapSetID") ==
              geym.configMetadata.end()) {
            std::cout << "\e[1;38;5;220m[WARN] \e[38;5;236m"
                      << "didnt find setid bro, tryin to improvise\n";
            std::string doublecheck =
                p.path().parent_path().filename().string();
            int index = 0;
            bool works = false;
            bool startswithnumber = isdigit(doublecheck.at(0));
            int number = 0;

            while (true) {
              if (index >= doublecheck.length()) {
                break;
              }
              if (!isdigit(doublecheck.at(index))) {
                if (doublecheck[index] == ' ') {
                  if (startswithnumber) {
                    works = true;
                    break;
                  }
                }
              } else {
                number *= 10;
                number += int(doublecheck[index] - '0');
              }
              index++;
              // std::cout << "number: " << number << std::endl;
            }
            GameSetId = std::to_string(number);
          } else {
            GameSetId = geym.configMetadata["BeatmapSetID"];
          }

          if (geym.configMetadata.find("Title") == geym.configMetadata.end()) {
            std::cout << "\e[1;38;5;220m[WARN] \e[38;5;236m"
                      << "didnt title bro, setting as Unknown\n";
            GameTitle = "Unknown";
          } else {
            GameTitle = geym.configMetadata["Title"];
          }

          if (geym.configMetadata.find("Artist") == geym.configMetadata.end()) {
            std::cout << "\e[1;38;5;220m[WARN] \e[38;5;236m"
                      << "didnt find artist bro, setting as Unknown\n";
            GameAuthor = "Unknown";
          } else {
            GameAuthor = geym.configMetadata["Artist"];
          }

          std::string filename = Global.GameBinaryPath + "/database/" +
                                 GameTitle + " {" + GameSetId + "}.db";
          bool firstLine = !checkIfExists((filename).c_str());
          FILE *pFile;
          pFile = fopen((filename).c_str(), "a");
          if (pFile != NULL) {
            if (firstLine) {
              fprintf(pFile, "---[METADATA]---\n");
              fprintf(pFile, "%s", (GameTitle + "\n").c_str());
              fprintf(pFile, "%s", (GameAuthor + "\n").c_str());
              fprintf(pFile, "%s", (GameSetId + "\n").c_str());
              fprintf(pFile, "%s",
                      (p.path().parent_path().string() + "/" + "\n").c_str());
              fprintf(
                  pFile, "%s",
                  (parser.parseBackground(p.path().string()) + "\n").c_str());
              fprintf(pFile, "---[FILES]---\n");
            }
            fprintf(pFile, "%s", (p.path().string() + "\n").c_str());
            fclose(pFile);
          }
        }
      }
    }

    Global.Path = Global.GameBinaryPath + "/database";

    std::vector<std::string> files;
    files = ls(".db");
    std::sort(files.begin(), files.end(), strcasecmp2);
    std::string filename = Global.GameBinaryPath + "/database/mainFolder.db";
    FILE *pFile;
    pFile = fopen((filename).c_str(), "a");
    if (pFile != NULL) {
      for (int i = 0; i < files.size(); i++) {
        fprintf(pFile, "%s", (files[i] + "\n").c_str());
      }
      fclose(pFile);
    }
    Global.Path = temp;
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    return;
  } else if (play.action) {
    animationMs = 0; //200
    animationStartTime = getGlobalTimer();
    animation = -1;
    animationDone = false;
    return;
  } else if (load.action) {
    animationMs = 0; //200
    animationStartTime = getGlobalTimer();
    animation = -2;
    animationDone = false;
    return;
  } else if (wip2.action) {
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
    Global.CurrentState->unload();
    Global.CurrentState.reset(new WipMenu2());
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
    return;
  }

  if (IsKeyDown(Global.AUDIO_SETUP_KEY))
    volume.update();
  float lastVolume = Global.volume;
  Global.volume = volume.location / 100.0f;
  if (!AreSame(lastVolume, Global.volume)) {
    // std::cout << "Volume: " << Global.volume << std::endl;
    Global.volumeChanged = true;
  }
  // MutexUnlock(SWITCHING_STATE);
  // MutexUnlock(ACCESSING_OBJECTS);
}
void MainMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  // Global.mutex.lock();
  // MutexLock(SWITCHING_STATE);
  // MutexLock(ACCESSING_OBJECTS);
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);

  play.render();
  wip.render();
  wip2.render();

  load.render();

  if (IsKeyDown(Global.AUDIO_SETUP_KEY))
    volume.render();

  // DrawTextureCenter(&Global.OsusLogo, 320, 200, 400.0 /
  // (float)Global.OsusLogo.width, WHITE);
  logo.render();
  popup.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  // MutexUnlock(SWITCHING_STATE);
  // MutexUnlock(ACCESSING_OBJECTS);
  // test.render();
  // Global.mutex.unlock();
}
void MainMenu::unload() {
  initializationStage = STATE_UNINITIALIZED;
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(SWITCHING_STATE);
}
void MainMenu::textureOps() {
  // NEED FIXING>>> UNLOADING DOESNT STOP RENDEWRING
}
