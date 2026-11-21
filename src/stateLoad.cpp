#include "cachebuilder/metadataParser.hpp"
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
#include <filesystem>
#include <gamefile.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <system_error>

#include "menu_shapes/shapes.hpp"

namespace {
Color LoadPanelColor() {
  const Color b = Global.Background;
  return {(unsigned char)std::min<int>(255, b.r + 24),
          (unsigned char)std::min<int>(255, b.g + 20),
          (unsigned char)std::min<int>(255, b.b + 24), 235};
}

const Color LOAD_ACCENT = {170, 120, 200, 255};
const Color LOAD_ROW = {72, 52, 92, 220};
const Color LOAD_SUCCESS = {120, 220, 140, 255};
const Color LOAD_WARN = {240, 200, 110, 255};
const Color LOAD_ERROR = {240, 120, 120, 255};

struct ExtractStats {
  int osuFiles = 0;
};

int countExtractedEntry(const char *filename, void *arg) {
  if (filename) {
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;40m"
              << "Extracted file " << filename << std::endl;
  }
  auto *stats = static_cast<ExtractStats *>(arg);
  if (stats && filename) {
    std::string f(filename);
    std::transform(f.begin(), f.end(), f.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    if (f.size() >= 4 && f.compare(f.size() - 4, 4, ".osu") == 0)
      stats->osuFiles++;
  }
  return 0;
}

std::string shortName(const std::string &name, size_t maxChars = 24) {
  return name.size() <= maxChars ? name : name.substr(0, maxChars - 3) + "...";
}
} // namespace

LoadMenu::LoadMenu() {
  const Color panel = LoadPanelColor();

  title = TextBox({320, 48}, {560, 34}, {0, 0, 0, 0}, "Load Beatmap (.osz)",
                  LOAD_ACCENT, MenuFontSize(22), 64);
  description = TextBox({320, 84}, {560, 28}, {0, 0, 0, 0},
                        "Select a .OSZ file, then press Select to extract it.",
                        WHITE, MenuFontSize(14), 120);
  bg = TextBox({320, 240}, {600, 440}, panel, "", WHITE, MenuFontSize(20), 10);
  // Height is an exact multiple of the row height so there is no dead space.
  dir_list = SelectableList({320, 250}, {560, 250}, LOAD_ROW, ls(".osz"), WHITE,
                            MenuFontSize(16), 25, 60);
  dir_list.scrollWithoutHover = true;
  back = Button({250, 408}, {120, 40}, LOAD_ACCENT, "Back", BLACK,
                MenuFontSize(20));
  select = Button({390, 408}, {120, 40}, LOAD_ACCENT, "Select", BLACK,
                  MenuFontSize(20));
  close =
      Button({595, 45}, {22, 22}, LOAD_ACCENT, "x", BLACK, MenuFontSize(16));
  path = TextBox({320, 444}, {560, 28}, panel, Global.Path, WHITE,
                 MenuFontSize(14), 80);

  statusMessage.clear();
  statusColor = LOAD_SUCCESS;
  statusUntil = 0.0;
}

void LoadMenu::rebuildDirList() {
  dir_list.objects.clear();
  dir_list.text = ls(".osz");
  dir_list.selectedindex = 0;
  dir_list.renderindex1 = 0;
  dir_list.renderindex2 = 0;
  dir_list.mouseSelectIndex = 0;
  dir_list.scrollWithoutHover = true;
  dir_list.init();
  path = TextBox(path.position, path.size, path.color, Global.Path,
                 path.textcolor, path.textsize, path.maxlength);
}

void LoadMenu::init() {
  Global.NeedForBackgroundClear = true;
  Global.useAuto = false;
  Global.LastFrameTime = getGlobalTimer();
  Global.FrameTime = 0.5;
  initializationStage = STATE_INITIALIZED;
}

void LoadMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  bg.render();
  title.render();
  description.render();

  if (!statusMessage.empty() && getGlobalTimer() < statusUntil) {
    float fontSize = MenuFontSize(20);
    Vector2 textSize = MeasureTextEx(&Global.DefaultFont,
                                           statusMessage.c_str(), fontSize, 2);
    DrawTextEx(&Global.DefaultFont, statusMessage.c_str(),
               ScaleCords({320.0f - textSize.x / 2.0f,
                           108.0f - textSize.y / 2.0f}),
               Scale(fontSize), Scale(2), statusColor);
  }

  dir_list.render();
  back.render();
  select.render();
  close.render();
  path.render();
  MutexUnlock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
}

void LoadMenu::update() {
  auto report = [&](const std::string &msg, Color col) {
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
    statusMessage = msg;
    statusColor = col;
    statusUntil = getGlobalTimer() + 4000.0;
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  };

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
    ((MainMenu *)(Global.CurrentState.get()))->animation = 2;
    Global.CurrentState->init();
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
    return;
  }

  enum class Pending { None, Enter, Up, Extract };
  Pending pending = Pending::None;
  bool emptySelection = false;
  std::string entryText;
  std::string selectedPath;
  std::string finalPath;

  MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  if (select.action) {
    if (dir_list.selectedindex >= 0 &&
        dir_list.selectedindex < (int)dir_list.objects.size()) {
      entryText = dir_list.objects[dir_list.selectedindex].text;
      if (!entryText.empty()) {
        if (entryText.back() == '/') {
          pending = Pending::Enter;
        } else {
          pending = Pending::Extract;
          selectedPath = Global.Path + '/' + entryText;
          std::string base = get_without_ext(selectedPath);
          finalPath = correct_path_notation(Global.GameBinaryPath +
                                            "/beatmaps/" + base);
        }
      } else {
        emptySelection = true;
      }
    } else {
      emptySelection = true;
    }
  } else if (back.action) {
    pending = Pending::Up;
  }

  if (pending == Pending::Enter) {
    entryText.pop_back();
    if (Global.Path.size() == 1)
      Global.Path.pop_back();
    Global.Path += '/' + entryText;
    rebuildDirList();
  } else if (pending == Pending::Up) {
    while (Global.Path.size() > 0) {
      if (Global.Path[Global.Path.size() - 1] == '/' ||
          Global.Path[Global.Path.size() - 1] == ':') {
        if (Global.Path.size() > 1 && Global.Path.size() != 2)
          Global.Path.pop_back();
        break;
      }
      Global.Path.pop_back();
    }
    rebuildDirList();
  }
  MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);

  if (pending == Pending::None) {
    if (emptySelection)
      report("Select a .osz file first", LOAD_ERROR);
    return;
  }
  if (pending != Pending::Extract)
    return;

  std::string base = get_without_ext(selectedPath);
  bool already = check_dir(finalPath) != 0;
  ExtractStats stats;
  int zipRes = 0;

  Global.doingTimeConsumingOp = true;

  if (!already) {
    create_dir(finalPath);
    zipRes = zip_extract(selectedPath.c_str(), finalPath.c_str(),
                         countExtractedEntry, &stats);
  }

  if (!already && zipRes != 0) {
    std::error_code ec;
    std::filesystem::remove_all(finalPath, ec);
    report("Extraction failed (code " + std::to_string(zipRes) + ")",
           LOAD_ERROR);
  } else {
    buildFileMap(finalPath);
    decideNamesForSets();

    if (already) {
      report("Already extracted: " + shortName(base), LOAD_WARN);
    } else {
      report("Imported " + shortName(base) + " (" +
                 std::to_string(stats.osuFiles) + " maps)",
             LOAD_SUCCESS);
    }
  }

  Global.doingTimeConsumingOp = false;
}

void LoadMenu::unload() { initializationStage = STATE_UNINITIALIZED; }
void LoadMenu::textureOps() {}
