#include "cachebuilder/metadataParser.hpp"
#include "globals.hpp"
#include "time_util.hpp"
#include "utils.hpp"

#include <algorithm>
#include <dirent.h>
#include <functional>
#include <iostream>
#include <stack>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

// Global storage
std::unordered_map<int, std::vector<FileMetadata>> namesOfSets;
std::unordered_map<int, int> numberOfMaps;
std::atomic<int> numBeatmapsFound;
static size_t mapsInCurrentBatch = 0;

// Helper functions
bool dirExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

void createDir(const std::string &path) {
#if defined(THREEDS_BUILD) || defined(__linux__)
  mkdir(path.c_str(), 0755);
#else
  mkdir(path.c_str());
#endif
}

bool fileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

void normalizePath(std::string &path) {
  for (char &c : path) {
    if (c == '\\')
      c = '/';
  }
}

struct DirState {
  std::string path;
  std::vector<std::string> entries;
  size_t index;
};

// Configurable threshold for RAM management
constexpr size_t BATCH_SET_LIMIT = 5;
constexpr size_t BATCH_MAP_LIMIT = 50;

// Helper: Updates an entry in beatmapsets.db or appends it if new
void updateOrAppendSetInDb(const std::string &dbFile, int setid,
                           const std::string &title, int mapCount,
                           const std::vector<FileMetadata> &metadataList) {
  std::vector<SetFileMetadata> cachedSets = parseCachedSets(dbFile);

  bool setExists = false;
  for (const auto &s : cachedSets) {
    if (s.setid == setid) {
      setExists = true;
      break;
    }
  }

  if (!setExists) {
    // O(1) Fast append for brand new sets
    writeBeatmapSetFile(dbFile, setid, title, mapCount, metadataList);
  } else {
    // Rewrite beatmapsets.db updating only this modified set
    FILE *f = fopen(dbFile.c_str(), "w");
    if (f)
      fclose(f);

    for (const auto &s : cachedSets) {
      if (s.setid == setid) {
        writeBeatmapSetFile(dbFile, setid, title, mapCount, metadataList);
      } else {
        std::vector<FileMetadata> existingMaps =
            parseCachedMaps(Global.DatabaseLocation, s.setid);
        writeBeatmapSetFile(dbFile, s.setid, s.title, s.number, existingMaps);
      }
    }
  }
}

void flushBatch() {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mFlushing current batch"
            << std::endl;
  if (namesOfSets.empty())
    return;

  // Ensure database root folder exists
  if (!dirExists(Global.DatabaseLocation)) {
    createDir(Global.DatabaseLocation);
  }

  processAllSetImages();

  std::string dbFile = Global.DatabaseLocation + "/beatmapsets.db";
  normalizePath(dbFile);

  while (!namesOfSets.empty()) {
    auto node = namesOfSets.extract(namesOfSets.begin());
    int setid = node.key();
    std::vector<FileMetadata> &newMaps = node.mapped();

    std::string setDir = Global.DatabaseLocation + "/" + std::to_string(setid);
    normalizePath(setDir);

    std::cout << "\e[1;35m[DATABASE] \e[38;5;236mWriting " << setid
              << std::endl;

    std::vector<FileMetadata> combinedList = std::move(newMaps);

    // Merge with on-disk maps if set folder already exists
    if (dirExists(setDir)) {
      std::vector<FileMetadata> existingMaps =
          parseCachedMaps(Global.DatabaseLocation, setid);
      for (auto &existing : existingMaps) {
        bool exists = false;
        for (const auto &m : combinedList) {
          if (m.id == existing.id) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          combinedList.push_back(std::move(existing));
        }
      }
    }

    // Determine most common title
    std::unordered_map<std::string, int> selection;
    for (const auto &file : combinedList) {
      selection[file.title]++;
    }
    std::string title = "error";
    int max_value = -1;
    for (const auto &[key, value] : selection) {
      if (value > max_value) {
        max_value = value;
        title = key;
      }
    }

    // Write map files and update beatmapsets.db
    writeBeatmapFile(setid, combinedList);
    updateOrAppendSetInDb(dbFile, setid, title,
                          static_cast<int>(combinedList.size()), combinedList);
  }

  namesOfSets.clear();
  mapsInCurrentBatch = 0;
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mFlushed" << std::endl;
}

void decideNamesForSets() { flushBatch(); }

void buildFileMap(const std::string &rootPath) {
  std::string root = rootPath;
  normalizePath(root);
  if (!root.empty() && root.back() == '/')
    root.pop_back();

  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mStarting search at " << root
            << std::endl;
  numBeatmapsFound = 0;

  auto readEntries =
      [](const std::string &dirPath) -> std::vector<std::string> {
    std::vector<std::string> result;
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
      return result;

    struct dirent *de;
    while ((de = readdir(dir)) != nullptr) {
      if (de->d_name[0] != '.')
        result.emplace_back(de->d_name);
    }
    closedir(dir);
    return result;
  };

  std::stack<DirState> stack;
  stack.push({root, readEntries(root), 0});

  while (!stack.empty()) {
    DirState &state = stack.top();

    if (state.index < state.entries.size()) {
      std::string entryName = std::move(state.entries[state.index]);
      state.index++;

      std::string fullPath = state.path + "/" + entryName;
      normalizePath(fullPath);

      struct stat st;
      if (stat(fullPath.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
          DirState newState;
          newState.path = fullPath;
          newState.entries = readEntries(fullPath);
          newState.index = 0;
          stack.push(std::move(newState));
          continue;
        } else if (S_ISREG(st.st_mode) &&
                   IsFileExtension(entryName.c_str(), ".osu")) {
          numBeatmapsFound = numBeatmapsFound + 1;
          // std::cout << "\e[1;35m[DATABASE] \e[38;5;236mFound " <<
          // numBeatmapsFound << " files"
          //   << std::endl;
          addFileToMap(fullPath);

          // Flush RAM to disk every 50 unique sets
          if (mapsInCurrentBatch >= BATCH_MAP_LIMIT ||
              namesOfSets.size() >= BATCH_SET_LIMIT) {
            flushBatch();
          }
        }
      }
    } else {
      stack.pop();
    }
  }

  // Flush remaining sets
  flushBatch();
}

void addFileToMap(const std::string &path) {
  std::vector<std::string> output = ParseNameFile(path);
  if (output.size() < 6)
    return;

  FileMetadata temp{
      .path = path,
      .title = std::move(output[0]),
      .artist = std::move(output[1]),
      .creator = std::move(output[2]),
      .version = std::move(output[3]),
      .setid = static_cast<int>(std::strtol(output[4].c_str(), nullptr, 10)),
      .id = static_cast<int>(std::strtol(output[5].c_str(), nullptr, 10)),
      .bgImage = extractBackgroundImage(path),
      .coverFile = " "};

  namesOfSets[temp.setid].push_back(std::move(temp));
  mapsInCurrentBatch++;
}

void writeBeatmapFile(int setid,
                      const std::vector<FileMetadata> &metadataList) {
  std::string dir_name = Global.DatabaseLocation + "/" + std::to_string(setid);
  normalizePath(dir_name);
  if (!dirExists(dir_name))
    createDir(dir_name);

  for (const auto &file_info : metadataList) {
    std::string file_path =
        dir_name + "/" + std::to_string(file_info.id) + ".db";
    FILE *file = fopen(file_path.c_str(), "w");
    if (!file)
      continue;

    fprintf(file,
            "Path:%s\nTitle:%s\nArtist:%s\nCreator:%s\nVersion:%s\nBeatmapID:%"
            "d\nBeatmapSetID:%d\nCoverFile:%s\n",
            file_info.path.c_str(), file_info.title.c_str(),
            file_info.artist.c_str(), file_info.creator.c_str(),
            file_info.version.c_str(), file_info.id, setid,
            file_info.coverFile.c_str());
    fclose(file);
  }
}

void writeBeatmapSetFile(const std::string &filename, int beatmap_set_id,
                         const std::string &title, int numMaps,
                         const std::vector<FileMetadata> &metadataListObj) {
  FILE *file = fopen(filename.c_str(), "a");
  if (!file)
    return;

  std::string ids_list, artists_list, creators_list;
  std::unordered_set<std::string> unique_artists, unique_creators;

  for (size_t i = 0; i < metadataListObj.size(); ++i) {
    if (i > 0)
      ids_list += ",";
    ids_list += std::to_string(metadataListObj[i].id);

    if (unique_artists.insert(metadataListObj[i].artist).second) {
      if (!artists_list.empty())
        artists_list += ", ";
      artists_list += metadataListObj[i].artist;
    }
    if (unique_creators.insert(metadataListObj[i].creator).second) {
      if (!creators_list.empty())
        creators_list += ", ";
      creators_list += metadataListObj[i].creator;
    }
  }

  fprintf(file, "[%d]\nTitle:%s\nMaps:%d\nIDs:%s\nArtists:%s\nCreators:%s\n\n",
          beatmap_set_id, title.c_str(), numMaps, ids_list.c_str(),
          artists_list.c_str(), creators_list.c_str());

  fclose(file);
}

void clearFileMap() {
  namesOfSets.clear();
  numberOfMaps.clear();
}

void listAllMaps() {}

std::vector<SetFileMetadata> parseCachedSets(const std::string &db_path) {
  std::vector<SetFileMetadata> metadata_list;
  FILE *file = fopen(db_path.c_str(), "r");
  if (!file)
    return metadata_list;

  char line[1024];
  SetFileMetadata current_meta;
  bool processing_entry = false;

  while (fgets(line, sizeof(line), file)) {
    std::string line_str(line);
    while (!line_str.empty() &&
           (line_str.back() == '\n' || line_str.back() == '\r'))
      line_str.pop_back();
    if (line_str.empty())
      continue;

    if (line_str.front() == '[' && line_str.find(']') != std::string::npos) {
      if (processing_entry)
        metadata_list.push_back(current_meta);
      current_meta = SetFileMetadata();
      processing_entry = true;
      size_t close = line_str.find(']');
      current_meta.setid = std::stoi(line_str.substr(1, close - 1));
    } else if (line_str.rfind("Title:", 0) == 0) {
      current_meta.title = line_str.substr(6);
    } else if (line_str.rfind("Maps:", 0) == 0) {
      current_meta.number = std::stoi(line_str.substr(5));
    } else if (line_str.rfind("Artists:", 0) == 0) {
      current_meta.artists = line_str.substr(8);
    } else if (line_str.rfind("Creators:", 0) == 0) {
      current_meta.creators = line_str.substr(9);
    }
  }
  if (processing_entry)
    metadata_list.push_back(current_meta);
  fclose(file);
  return metadata_list;
}

std::vector<FileMetadata> parseCachedMaps(const std::string &db_path,
                                          int setid) {
  std::vector<FileMetadata> result;
  std::string setDir = db_path + "/" + std::to_string(setid);
  normalizePath(setDir);

  DIR *dir = opendir(setDir.c_str());
  if (!dir)
    return result;

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name[0] == '.' || name.size() < 3 ||
        name.compare(name.size() - 3, 3, ".db") != 0)
      continue;

    std::string filePath = setDir + "/" + name;
    normalizePath(filePath);

    struct stat st;
    if (stat(filePath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
      continue;

    FILE *file = fopen(filePath.c_str(), "r");
    if (!file)
      continue;

    FileMetadata meta{.setid = setid, .id = 0};
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
      std::string lineStr(line);
      lineStr.erase(lineStr.find_last_not_of("\r\n") + 1);
      size_t colon = lineStr.find(':');
      if (colon == std::string::npos)
        continue;

      std::string key = lineStr.substr(0, colon);
      std::string value = lineStr.substr(colon + 1);

      if (key == "Path")
        meta.path = value;
      else if (key == "Title")
        meta.title = value;
      else if (key == "Artist")
        meta.artist = value;
      else if (key == "Creator")
        meta.creator = value;
      else if (key == "Version")
        meta.version = value;
      else if (key == "BeatmapID")
        meta.id = std::stoi(value);
      else if (key == "CoverFile")
        meta.coverFile = value;
    }
    fclose(file);

    if (meta.id != 0)
      result.push_back(std::move(meta));
  }
  closedir(dir);

  std::sort(
      result.begin(), result.end(),
      [](const FileMetadata &a, const FileMetadata &b) { return a.id < b.id; });
  return result;
}

std::string extractBackgroundImage(const std::string &osuPath) {
  FILE *file = fopen(osuPath.c_str(), "r");
  if (!file)
    return "";

  char line[1024];
  bool inEvents = false;
  std::string bgFile;

  while (fgets(line, sizeof(line), file)) {
    std::string lineStr(line);
    lineStr.erase(lineStr.find_last_not_of("\r\n") + 1);

    if (lineStr == "[Events]") {
      inEvents = true;
      continue;
    }
    if (inEvents && lineStr.rfind("//", 0) == 0)
      continue;

    if (inEvents && !lineStr.empty()) {
      if (lineStr.front() == '[' && lineStr != "[Events]")
        break;

      size_t first = lineStr.find('"');
      if (first != std::string::npos) {
        size_t second = lineStr.find('"', first + 1);
        if (second != std::string::npos) {
          std::string candidate = lineStr.substr(first + 1, second - first - 1);

          // Convert to lowercase for checking
          std::string lower = candidate;
          for (char &c : lower)
            c = std::tolower((unsigned char)c);

          bool isVideo = lower.rfind(".mp4") != std::string::npos ||
                         lower.rfind(".avi") != std::string::npos ||
                         lower.rfind(".flv") != std::string::npos ||
                         lower.rfind(".mkv") != std::string::npos ||
                         lower.rfind(".mov") != std::string::npos ||
                         lower.rfind(".wmv") != std::string::npos ||
                         lower.rfind(".m4v") != std::string::npos ||
                         lower.rfind(".MP4") != std::string::npos ||
                         lower.rfind(".AVI") != std::string::npos ||
                         lower.rfind(".FLV") != std::string::npos ||
                         lower.rfind(".MKV") != std::string::npos ||
                         lower.rfind(".MOV") != std::string::npos ||
                         lower.rfind(".WMV") != std::string::npos ||
                         lower.rfind(".M4V") != std::string::npos;

          if (!isVideo) {
            bgFile = candidate;
            break;
          }
        }
      }
    }
  }
  fclose(file);
  return bgFile;
}

void processAllSetImages() {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mProcessing images in sets "
            << namesOfSets.size() << std::endl;
  for (auto &[setid, metadataList] : namesOfSets) {
    std::unordered_map<std::string, std::string> bgToCover;
    int coverIndex = 0;

    for (auto &file : metadataList) {
      if (file.bgImage.empty()) {
        std::cout << setid << " empty image?" << std::endl;
        continue;
      }

      std::string beatmapDir = file.path;
      size_t lastSlash = beatmapDir.find_last_of("/\\");
      if (lastSlash != std::string::npos)
        beatmapDir = beatmapDir.substr(0, lastSlash);
      //std::cout << beatmapDir << std::endl;

      normalizePath(beatmapDir);

      std::string fullBgPath = beatmapDir + "/" + file.bgImage;
      normalizePath(fullBgPath);

      if (!fileExists(fullBgPath)) {
        std::string upperBg = file.bgImage;
        for (char &c : upperBg)
          c = std::toupper((unsigned char)c);

        std::string testPath = beatmapDir + "/" + upperBg;
        normalizePath(testPath);

        if (fileExists(testPath)) {
          fullBgPath = testPath;
        } else {
          size_t dotPos = file.bgImage.find_last_of('.');
          if (dotPos != std::string::npos) {
            std::string extUpperBg = file.bgImage;
            for (size_t i = dotPos; i < extUpperBg.length(); ++i) {
              extUpperBg[i] = std::toupper((unsigned char)extUpperBg[i]);
            }
            testPath = beatmapDir + "/" + extUpperBg;
            normalizePath(testPath);

            if (fileExists(testPath)) {
              fullBgPath = testPath;
            }
          }
        }

        // Final check after fallbacks
        if (!fileExists(fullBgPath)) {
          continue;
        }
      }

      auto it = bgToCover.find(fullBgPath);
      if (it != bgToCover.end()) {
        file.coverFile = it->second;
        continue;
      }

      std::cout << "loading " << fullBgPath << " " << fullBgPath.size()
                << std::endl;
      Image img = LoadImage(fullBgPath.c_str());
      if (img.data == nullptr) {
        std::cout << "failed" << std::endl;
        continue;
      }

      float targetAspect = (float)COVER_WIDTH / COVER_HEIGHT;
      float imageAspect = (float)img.width / img.height;
      Rectangle cropRect;
      if (imageAspect > targetAspect) {
        int cropWidth = (int)(img.height * targetAspect);
        cropRect = {(float)(img.width - cropWidth) / 2.0f, 0.0f,
                    (float)cropWidth, (float)img.height};
      } else {
        int cropHeight = (int)(img.width / targetAspect);
        cropRect = {0.0f, (float)(img.height - cropHeight) / 2.0f,
                    (float)img.width, (float)cropHeight};
      }
      ImageCrop(&img, cropRect);
      ImageResize(&img, COVER_WIDTH, COVER_HEIGHT);

      std::string coverName = "cover_" + std::to_string(setid) + "_" +
                              std::to_string(coverIndex++) + ".bmp";
      std::string setDir =
          Global.DatabaseLocation + "/" + std::to_string(setid);
      normalizePath(setDir);
      std::cout << "\e[1;35m[DATABASE] \e[38;5;236mProcessing " << coverName
                << std::endl;
      if (!dirExists(setDir))
        createDir(setDir);

      std::string outPath = setDir + "/" + coverName;
      ExportImage(img, outPath.c_str());
      UnloadImage(&img);

      bgToCover[fullBgPath] = coverName;
      file.coverFile = coverName;
    }
  }
}

// Appends a single new .osu map to disk without requiring a full rebuild
bool appendSingleBeatmap(const std::string &osuPath) {
  std::vector<std::string> output = ParseNameFile(osuPath);
  if (output.size() < 6)
    return false;

  FileMetadata meta{
      .path = osuPath,
      .title = std::move(output[0]),
      .artist = std::move(output[1]),
      .creator = std::move(output[2]),
      .version = std::move(output[3]),
      .setid = static_cast<int>(std::strtol(output[4].c_str(), nullptr, 10)),
      .id = static_cast<int>(std::strtol(output[5].c_str(), nullptr, 10)),
      .bgImage = extractBackgroundImage(osuPath),
      .coverFile = " "};

  // Write individual map file (<id>.db)
  writeBeatmapFile(meta.setid, {meta});

  // Update beatmapsets.db entry
  std::string dbFile = Global.DatabaseLocation + "/beatmapsets.db";
  normalizePath(dbFile);

  std::vector<FileMetadata> existingMaps =
      parseCachedMaps(Global.DatabaseLocation, meta.setid);

  // Check if map ID is already present
  bool found = false;
  for (const auto &m : existingMaps) {
    if (m.id == meta.id) {
      found = true;
      break;
    }
  }
  if (!found)
    existingMaps.push_back(meta);

  // Re-read existing beatmapsets.db into memory to modify/append entry
  std::vector<SetFileMetadata> cachedSets = parseCachedSets(dbFile);
  bool setExists = false;
  for (auto &set : cachedSets) {
    if (set.setid == meta.setid) {
      setExists = true;
      break;
    }
  }

  if (!setExists) {
    // Simple append if set is brand new
    writeBeatmapSetFile(dbFile, meta.setid, meta.title,
                        static_cast<int>(existingMaps.size()), existingMaps);
  } else {
    // Rewrite entire beatmapsets.db with updated map set metadata
    FILE *f = fopen(dbFile.c_str(), "w");
    if (!f)
      return false;
    fclose(f);

    for (const auto &set : cachedSets) {
      if (set.setid == meta.setid) {
        writeBeatmapSetFile(dbFile, meta.setid, meta.title,
                            static_cast<int>(existingMaps.size()),
                            existingMaps);
      } else {
        std::vector<FileMetadata> setMaps =
            parseCachedMaps(Global.DatabaseLocation, set.setid);
        writeBeatmapSetFile(dbFile, set.setid, set.title, set.number, setMaps);
      }
    }
  }

  return true;
}