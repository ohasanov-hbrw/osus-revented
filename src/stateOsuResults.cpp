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
#include "cachebuilder/metadataParser.hpp"


void saveScore(int setid, int beatmapid, int score, int maxCombo,
               int hit300, int hit100, int hit50, int hit0,
               float accuracy, int rank) {
  if (!dirExists(Global.ScoreLocation)) {
    createDir(Global.ScoreLocation);
  }

  std::string folderPath = Global.ScoreLocation + "/" + 
                           std::to_string(setid) + "-" + 
                           std::to_string(beatmapid) + "-score";
  normalizePath(folderPath);

  if (!dirExists(folderPath)) {
    createDir(folderPath);
  }

  // Scan directory for existing .db files
  std::vector<int> indices;
  DIR *dir = opendir(folderPath.c_str());
  if (dir) {
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
      std::string name = entry->d_name;
      if (name.size() > 3 && name.compare(name.size() - 3, 3, ".db") == 0) {
        try {
          indices.push_back(std::stoi(name.substr(0, name.size() - 3)));
        } catch (...) {}
      }
    }
    closedir(dir);
  }

  std::sort(indices.begin(), indices.end());

  // Delete oldest files if max limit of 50 is reached
  while (indices.size() >= 50) {
    int oldest = indices.front();
    std::string deletePath = folderPath + "/" + std::to_string(oldest) + ".db";
    normalizePath(deletePath);
    std::remove(deletePath.c_str());
    indices.erase(indices.begin());
  }

  // Calculate next index
  int nextIndex = indices.empty() ? 0 : indices.back() + 1;

  // Save new play file
  std::string filePath = folderPath + "/" + std::to_string(nextIndex) + ".db";
  normalizePath(filePath);

  FILE *f = fopen(filePath.c_str(), "w");
  if (!f) return;

  fprintf(f, "Score:%d\nMaxCombo:%d\n300s:%d\n100s:%d\n50s:%d\n0s:%d\nAccuracy:%.2f\nRank:%d\n",
          score, maxCombo, hit300, hit100, hit50, hit0, accuracy, rank);
  fclose(f);
}

ResultsMenu::ResultsMenu() {
  name = TextBox({320, 40}, {520, 60}, {0, 0, 0, 0}, "Results!", WHITE, 40, 50);
  close = Button({480, 440}, {160, 30}, {255, 135, 198, 255}, "Back to menu",
                 BLACK, 20);
  maxCombo = TextBox({320, 80}, {520, 60}, {0, 0, 0, 0},
                     "Maximum Combo: Not available", WHITE, 20, 50);
  hit300 = TextBox({320, 120}, {520, 60}, {0, 0, 0, 0},
                   "300s hit: Not available", WHITE, 20, 50);
  hit100 = TextBox({320, 150}, {520, 60}, {0, 0, 0, 0},
                   "100s hit: Not available", WHITE, 20, 50);
  hit50 = TextBox({320, 180}, {520, 60}, {0, 0, 0, 0}, "50s hit: Not available",
                  WHITE, 20, 50);
  hit0 = TextBox({320, 210}, {520, 60}, {0, 0, 0, 0}, "Misses: Not available",
                 WHITE, 20, 50);
  accuracy = TextBox({320, 240}, {520, 60}, {0, 0, 0, 0},
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
  float accuracyFloat =
      ((300.0f * Global.gameManager->hit300s +
        10.0f * Global.gameManager->hit100s +
        50.0f * Global.gameManager->hit50s) /
       (300.0f * (Global.gameManager->hit300s + Global.gameManager->hit100s +
                  Global.gameManager->hit50s + Global.gameManager->hit0s))) *
      100.0f;
  // accuracy = std::ceil(accuracy * 100.0) / 100.0;
  char buffer[16];
  std::snprintf(buffer, sizeof(buffer), "%.2f", accuracyFloat);
  std::string str(buffer);

  accuracy.text = "Accuracy: " + str + "%";
  accuracy.init();

  accuracies.clear();
  accuracies.shrink_to_fit();
  GameManager *gm = GameManager::getInstance();
  std::vector<float> local;
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m"
            << "gm->objectPoints.size(): " << gm->objectPoints.size() << "\n";
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
  if (local.size() > maxPoints) {

    accuracies.reserve(maxPoints);

    // Bucket size
    int bucketSize = (local.size() - 2) / (maxPoints - 2);
    if (bucketSize < 1)
      bucketSize = 1;

    // Always include first and last point
    accuracies.push_back(local[0]);

    for (int i = 1; i < maxPoints - 1; i++) {
      int start = (i - 1) * bucketSize + 1;
      int end = i * bucketSize;
      if (end > local.size() - 2)
        end = local.size() - 2;

      // Find the point in this bucket that forms the largest triangle with the
      // previous point and the next bucket's average
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
        float area = std::abs((accuracies.back() - avgY) * (j - avgX) -
                              (accuracies.back() - local[j]) * (start - avgX)) /
                     2.0f;
        if (area > maxArea) {
          maxArea = area;
          selected = j;
        }
      }
      accuracies.push_back(local[selected]);
    }
    accuracies.push_back(local.back());
  } else {
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

  int hit300 = Global.gameManager->hit300s;
  int hit100 = Global.gameManager->hit100s;
  int hit50 = Global.gameManager->hit50s;
  int hit0 = Global.gameManager->hit0s;

  int totalHits = hit300 + hit100 + hit50 + hit0;

  rank = RANK_D;

  if (totalHits > 0) {
    float ratio300 = (float)hit300 / totalHits;
    float ratio50 = (float)hit50 / totalHits;
    bool noMisses = (hit0 == 0);

    if (ratio300 == 1.0f) {
      rank = RANK_SS;
    } else if (ratio300 > 0.90f && ratio50 < 0.01f && noMisses) {
      rank = RANK_S;
    } else if ((ratio300 > 0.80f && noMisses) || ratio300 > 0.90f) {
      rank = RANK_A;
    } else if ((ratio300 > 0.70f && noMisses) || ratio300 > 0.80f) {
      rank = RANK_B;
    } else if (ratio300 > 0.60f) {
      rank = RANK_C;
    } else {
      rank = RANK_D;
    }
  }


  saveScore(
    Global.scoreSetId,
    Global.scoreBeatmapId,
    Global.scoreScore,    
    Global.gameManager->maxCombo,
    hit300,
    hit100,
    hit50,
    hit0,
    accuracyFloat,
    static_cast<int>(rank)
  );

  textureOpsState.store(TEX_LOAD);

  initializationStage = STATE_INITIALIZED;
}
void ResultsMenu::render() {
  if (initializationStage != STATE_INITIALIZED)
    return;
  MutexLock(ACCESSING_OBJECTS, RENDERTHREAD_ID);
  if (textureOpsState.load() == TEX_LOADED)
    DrawTextureEx(&symbol, ScaleCords({420, 280}), 0,
                  Scale(64.0 / symbol.width), WHITE);
  close.render();
  name.render();
  maxCombo.render();
  hit300.render();
  hit100.render();
  hit50.render();
  hit0.render();
  accuracy.render();
  drawAccuracyGraph(ScaleRect({320 - 100, 280, 200, 100}));

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
  bool switchingstateLocked =
      __mutex_threads_locks[SWITCHING_STATE][UPDATETHREAD_ID];
  bool renderLocked = __mutex_threads_locks[RENDER_BLOCK][UPDATETHREAD_ID];
  bool accessLocked = __mutex_threads_locks[ACCESSING_OBJECTS][UPDATETHREAD_ID];
  if (switchingstateLocked)
    MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
  if (renderLocked)
    MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
  if (accessLocked)
    MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  textureOpsState.store(TEX_FREE);
  while (textureOpsState.load() != TEX_FREED) {
    std::cout << "waiting for textureops\n";
    SleepInMs(10);
  }
  if (accessLocked)
    MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
  if (renderLocked)
    MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
  if (switchingstateLocked)
    MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
  // MutexLock(SWITCHING_STATE);
  // MutexUnlock(SWITCHING_STATE);
}
void ResultsMenu::textureOps() {
  int state = textureOpsState.load();
  switch (state) {
  case TEX_LOAD: {
    std::string rankFilename;
    switch (rank) {
    case RANK_SS:
      rankFilename = "ranking-X.png";
      break;
    case RANK_S:
      rankFilename = "ranking-S.png";
      break;
    case RANK_A:
      rankFilename = "ranking-A.png";
      break;
    case RANK_B:
      rankFilename = "ranking-B.png";
      break;
    case RANK_C:
      rankFilename = "ranking-C.png";
      break;
    case RANK_D:
      rankFilename = "ranking-D.png";
      break;
    default:
      rankFilename = "ranking-D.png";
      break;
    }
    std::string customSkinPath =
        Global.GameBinaryPath + "/resources/skin/" + rankFilename;
    symbol = LoadTexture(customSkinPath.c_str());
    if (!IsTextureReady(&symbol)) {
      UnloadTexture(&symbol);
      std::string defaultSkinPath =
          Global.GameBinaryPath + "/resources/default_skin/" + rankFilename;
      symbol = LoadTexture(defaultSkinPath.c_str());
    }
    std::cout << "loaded " << rankFilename << std::endl;
    textureOpsState.store(TEX_LOADED);
    break;
  }
  case TEX_FREE:
    if (IsTextureReady(&symbol))
      UnloadTexture(&symbol);
    textureOpsState.store(TEX_FREED);
    break;
  default:
    break;
  }
}

void ResultsMenu::drawAccuracyGraph(Rectangle area) {
  if (accuracies.size() < 2) {
    DrawTextEx(&Global.DefaultFont, "Not enough data", {area.x, area.y},
               Scale(20), Scale(0.5f), GRAY);
    return;
  }

  // Scale padding dynamically based on height (prevents overflow on small
  // rectangles)
  float paddingX = Scale(15.0f);
  float paddingY = Scale(12.0f);

  // Reserve margin on the left for y-axis text (0%, 25%, etc.)
  float textMarginLeft = Scale(30.0f);

  float graphX = area.x + paddingX + textMarginLeft;
  float graphY = area.y + paddingY;
  float graphWidth = area.width - (2 * paddingX) - textMarginLeft;
  float graphHeight = area.height - (2 * paddingY);

  // Draw background and outline
  DrawRectangleRec(area, DARKGRAY);
  DrawRectangleLinesEx(area, Scale(1.0f), LIGHTGRAY);

  // Draw grid lines and y-axis labels
  float fontSize = Scale(12.0f);
  float lineThickness = Scale(1.5f);

  for (int i = 0; i <= 4; i++) {
    float y = graphY + graphHeight - (i * graphHeight / 4.0f);

    // Grid line
    DrawLineEx({graphX, y}, {graphX + graphWidth, y}, lineThickness,
               Fade(LIGHTGRAY, 0.3f));

    // Label text aligned properly to left margin
    const char *text = TextFormat("%d%%", i * 25);
    Vector2 textSize =
        MeasureTextEx(&Global.DefaultFont, text, fontSize, Scale(0.5f));

    // Vertically center text on the grid line
    Vector2 textPos = {graphX - textSize.x - Scale(6.0f),
                       y - (textSize.y / 2.0f)};
    DrawTextEx(&Global.DefaultFont, text, textPos, fontSize, Scale(0.5f), GRAY);
  }

  // Build points array for smooth line rendering
  int n = static_cast<int>(accuracies.size());
  std::vector<Vector2> points(n);
  float stepX = graphWidth / (n - 1);

  for (int i = 0; i < n; i++) {
    // Clamp accuracy between 0 and 100 just in case
    float acc = ::std::clamp(accuracies[i], 0.0f, 100.0f);

    points[i] = {graphX + i * stepX,
                 graphY + graphHeight - (acc / 100.0f) * graphHeight};
  }

  // Draw smooth line curve with round joint caps
  float curveThickness = Scale(2.5f);
  float capRadius = curveThickness / 2.0f;

  for (int i = 0; i < n - 1; i++) {
    DrawLineEx(points[i], points[i + 1], curveThickness, GREEN);
  }

  // Add round caps on points to eliminate jagged edges at segment connections
  for (int i = 0; i < n; i++) {
    DrawCircleV(points[i], capRadius, GREEN);
  }
}