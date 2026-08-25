#include "cachebuilder/metadataParser.hpp"
#include "time_util.hpp"
#include "utils.hpp"
#include <algorithm>
#include <dirent.h>
#include <functional>
#include <iostream>
#include <map>
#include <stack>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "globals.hpp"

// Global storage (unchanged)
std::map<int, std::vector<FileMetadata>> namesOfSets;
std::map<int, std::string> decidedNames;
std::map<int, int> numberOfMaps;

// Helper: check if a directory exists
static bool dirExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

// Helper: create a directory (ignores errors if it already exists)
static void createDir(const std::string &path) {
#ifdef THREEDS_BUILD
  mkdir(path.c_str(), 0755);
#else
#ifdef __linux__
  mkdir(path.c_str(), 0755);
  #else
  mkdir(path.c_str());
  #endif
#endif
}

// Helper: check if a file exists
static bool fileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

// Helper: normalize slashes (replace '\' with '/')
static void normalizePath(std::string &path) {
  for (char &c : path) {
    if (c == '\\')
      c = '/';
  }
}

struct DirState {
    std::string path;
    std::vector<std::string> entries;  // names of files/subdirs (excluding hidden)
    size_t index;
};

void buildFileMap(std::string root) {
    normalizePath(root);
    if (!root.empty() && root.back() == '/')
        root.pop_back();

    std::cout << "\e[1;35m[DATABASE] \e[38;5;236mStarting search at " << root << std::endl;
    //SleepInMs(30);

    // --- Helper to read directory entries (skip hidden) ---
    auto readEntries = [](const std::string& dirPath) -> std::vector<std::string> {
        std::vector<std::string> result;
        DIR* dir = opendir(dirPath.c_str());
        if (!dir) return result;
        struct dirent* de;
        while ((de = readdir(dir)) != nullptr) {
            std::string name = de->d_name;
            if (name[0] != '.')   // skip hidden and "."/".."
                result.push_back(name);
        }
        closedir(dir);
        return result;
    };

    // --- Initialise stack with root ---
    std::stack<DirState> stack;
    stack.push({root, readEntries(root), 0});

    std::cout << "\e[1;35m[DATABASE] \e[38;5;236mPushed root to stack" << std::endl;
    //SleepInMs(30);
    while (!stack.empty()) {
        DirState& state = stack.top();

        if (state.index < state.entries.size()) {
            std::string entryName = state.entries[state.index];
            std::string fullPath = state.path + "/" + entryName;
            normalizePath(fullPath);

            // Determine type using stat()
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    // --- It's a directory: push new frame after incrementing index ---
                    state.index++;  // remember to skip this directory when we return
                    DirState newState;
                    newState.path = fullPath;
                    newState.entries = readEntries(fullPath);
                    newState.index = 0;
                    stack.push(std::move(newState));
                    std::cout << "\e[1;35m[DATABASE] \e[38;5;236mPushed " << fullPath << " to stack, size:" << stack.size() << std::endl;
                    //SleepInMs(30);
                    continue;  // process the new directory now
                }
                else if (S_ISREG(st.st_mode) && IsFileExtension(entryName.c_str(), ".osu")) {
                    // --- Beatmap file ---
                    std::cout << "\e[1;35m[DATABASE] \e[38;5;236mAdding " << fullPath << " to map" << std::endl;
                    //SleepInMs(30);
                    addFileToMap(fullPath);
                }
                // else: other file types are ignored
            }
            // If stat failed, we skip this entry (e.g., broken symlink)
            state.index++;   // move to next entry
        } else {
            // --- Finished all entries in current directory ---
            stack.pop();
        }
    }
}

// ------------------------------------------------------------------
// addFileToMap – unchanged, but uses normalizePath already done
void addFileToMap(std::string path) {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236maddFileToMap " << path << std::endl;
  //SleepInMs(30);
  std::vector<std::string> output = ParseNameFile(path);
  if (output.empty() || output.size() < 6)
    return;

  std::string bgImage = extractBackgroundImage(path);

  FileMetadata temp = {
      .path = path,
      .title = output[0],
      .artist = output[1],
      .creator = output[2],
      .version = output[3],
      .setid = std::strtol(output[4].c_str(), nullptr, 10),
      .id = std::strtol(output[5].c_str(), nullptr, 10),
      .bgImage = bgImage,
      .coverFile = " " // will be filled later
  };
  namesOfSets[temp.setid].push_back(temp);
}

// ------------------------------------------------------------------
// writeBeatmapFile – uses POSIX mkdir and fopen
void writeBeatmapFile(int setid,
                      const std::vector<FileMetadata> &metadataList) {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mwriteBeatmapFile " << setid << std::endl;
  //SleepInMs(30);
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

    fprintf(file, "Path:%s\n", file_info.path.c_str());
    fprintf(file, "Title:%s\n", file_info.title.c_str());
    fprintf(file, "Artist:%s\n", file_info.artist.c_str());
    fprintf(file, "Creator:%s\n", file_info.creator.c_str());
    fprintf(file, "Version:%s\n", file_info.version.c_str());
    fprintf(file, "BeatmapID:%d\n", file_info.id);
    fprintf(file, "BeatmapSetID:%d\n", setid);
    fprintf(file, "CoverFile:%s\n", file_info.coverFile.c_str());
    fclose(file);
  }
}

void listAllMaps() {
  // 1. Loop through each key-value pair in the map
  // 'setid' is the key (int), 'metadataList' is the value (std::vector)
  /*for (const auto& [setid, metadataList] : namesOfSets) {
      std::cout << "--- Map Set ID: " << setid << " ---" << std::endl;

      // 2. Loop through the vector of FileMetadata objects for this specific
  set for (const auto& file : metadataList) { std::cout << "Path:    " <<
  file.path    << "\n"
                    << "Title:   " << file.title   << "\n"
                    << "Artist:  " << file.artist  << "\n"
                    << "Creator: " << file.creator << "\n"
                    << "Version: " << file.version << "\n"
                    << "Beatmap ID: " << file.id   << "\n"
                    << "----------------------------------" << std::endl;
      }
  }*/
}

// ------------------------------------------------------------------
// writeBeatmapSetFile – appends to beatmapsets.db
void writeBeatmapSetFile(const std::string &filename, int beatmap_set_id,
                         const std::string &title,
                         const std::map<std::string, int> &selection) {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mwriteBeatmapSetFile " << filename << std::endl;
  //SleepInMs(30);
  FILE *file = fopen(filename.c_str(), "a");
  if (!file) {
    std::cerr << "Error: Could not open " << filename << " for appending.\n";
    return;
  }

  auto it = namesOfSets.find(beatmap_set_id);
  if (it == namesOfSets.end()){
    fclose(file);
    return;
  }
  const auto& metadataListObj = it->second;


  fprintf(file, "[%d]\n", beatmap_set_id);
  fprintf(file, "Title:%s\n", title.c_str());


  

  // Build IDs list
  std::string ids_list;
  for (size_t i = 0; i < metadataListObj.size(); ++i) {
    if (!ids_list.empty())
      ids_list += ",";
    ids_list += std::to_string(metadataListObj[i].id);
  }

  // Build Artists list (unique)
  std::string artists_list;
  for (const auto &file : metadataListObj) {
    if (artists_list.find(file.artist) == std::string::npos) {
      if (!artists_list.empty())
        artists_list += ", ";
      artists_list += file.artist;
    }
  }

  // Build Creators list (unique)
  std::string creators_list;
  for (const auto &file : metadataListObj) {
    if (creators_list.find(file.creator) == std::string::npos) {
      if (!creators_list.empty())
        creators_list += ", ";
      creators_list += file.creator;
    }
  }

  auto nit = numberOfMaps.find(beatmap_set_id);
  if (nit == numberOfMaps.end()){
    fclose(file);
    return;
  }
  const auto& numObj = nit->second;

  fprintf(file, "Maps:%d\n", numObj);
  fprintf(file, "IDs:%s\n", ids_list.c_str());
  fprintf(file, "Artists:%s\n", artists_list.c_str());
  fprintf(file, "Creators:%s\n\n", creators_list.c_str());

  fclose(file);
}

// ------------------------------------------------------------------
// decideNamesForSets – now clears each set after writing to save memory
void decideNamesForSets() {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mdecideNamesForSets " << std::endl;
  //SleepInMs(30);
  decidedNames.clear();
  numberOfMaps.clear();
  std::string dbFile = Global.DatabaseLocation + "/beatmapsets.db";
  normalizePath(dbFile);

  // Process all cover images (on 3DS this just sets coverFile to " ")
  processAllSetImages();

  // Clear the beatmapsets.db file
  FILE *f = fopen(dbFile.c_str(), "w");
  if (f)
    fclose(f);

  for (auto &[setid, metadataList] : namesOfSets) {
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m--- Map Set ID: " << setid
              << " ---" << std::endl;

    std::map<std::string, int> selection;
    numberOfMaps[setid] = 0;
    for (const auto &file : metadataList) {
      selection[file.title]++;
      numberOfMaps[setid]++;
    }

    std::string title = "error";
    int max_value = -1;
    for (const auto &[key, value] : selection) {
      if (value > max_value) {
        max_value = value;
        title = key;
      }
    }

    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << setid
              << " - Title: " << title
              << " - Number of maps: " << numberOfMaps[setid] << std::endl;

    writeBeatmapSetFile(dbFile, setid, title, selection);
    writeBeatmapFile(setid, metadataList);

    // Free this set's metadata to reduce memory
    metadataList.clear();
  }

  // Optionally clear the main map too
  namesOfSets.clear();
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236mDecided Names" << std::endl;
}

// ------------------------------------------------------------------
// clearFileMap – already okay

void clearFileMap() {
  for (auto &pair : namesOfSets) {
    pair.second.clear();
  }
  namesOfSets.clear();
  for (auto &pair : decidedNames) {
    pair.second.clear();
  }
  decidedNames.clear();
  numberOfMaps.clear();
}

// ------------------------------------------------------------------
// parseCachedSets – unchanged (uses fopen)
std::vector<SetFileMetadata> parseCachedSets(const std::string &db_path) {
  std::vector<SetFileMetadata> metadata_list;
  FILE *file = fopen(db_path.c_str(), "r");
  if (!file) {
    std::cerr << "Error: Could not open database file: " << db_path << "\n";
    return metadata_list;
  }

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

// ------------------------------------------------------------------
// parseCachedMaps – rewritten with POSIX directory iteration
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
    if (name[0] == '.')
      continue;
    std::string filePath = setDir + "/" + name;
    normalizePath(filePath);

    // Check extension .db
    if (name.size() < 3 || name.compare(name.size() - 3, 3, ".db") != 0)
      continue;

    // Ensure it's a regular file
    struct stat st;
    if (stat(filePath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
      continue;

    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236mopening: " << filePath
              << std::endl;
    FILE *file = fopen(filePath.c_str(), "r");
    if (!file)
      continue;

    FileMetadata meta;
    meta.setid = setid;
    meta.id = 0;
    meta.bgImage = "";
    meta.coverFile = "";

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
      std::string lineStr(line);
      lineStr.erase(lineStr.find_last_not_of("\r\n") + 1);
      if (lineStr.empty())
        continue;

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
      result.push_back(meta);
  }
  closedir(dir);

  std::sort(
      result.begin(), result.end(),
      [](const FileMetadata &a, const FileMetadata &b) { return a.id < b.id; });
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236mparsed " << result.size()
            << " maps\n";
  return result;
}

// ------------------------------------------------------------------
// extractBackgroundImage – unchanged (only uses FILE*)
std::string extractBackgroundImage(const std::string &osuPath) {
#ifdef THREEDS_BUILD
  return "";
#else
  FILE *file = fopen(osuPath.c_str(), "r");
  if (!file) {
    // Retry with backslashes (Windows fallback)
    std::string altPath = osuPath;
    for (char &c : altPath)
      if (c == '/')
        c = '\\';
    altPath = prepare_long_path(altPath).string();
    file = fopen(altPath.c_str(), "r");
    if (!file)
      return "";
  }

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
      size_t first = lineStr.find('"');
      if (first != std::string::npos) {
        size_t second = lineStr.find('"', first + 1);
        if (second != std::string::npos) {
          bgFile = lineStr.substr(first + 1, second - first - 1);
          break;
        }
      }
      if (lineStr.front() == '[' && lineStr != "[Events]")
        break;
    }
    if (inEvents && lineStr.front() == '[' && lineStr != "[Events]")
      break;
  }
  fclose(file);
  return bgFile;
#endif
}

// ------------------------------------------------------------------
// processAllSetImages – fixed for 3DS and uses POSIX file checks
void processAllSetImages() {
#ifdef THREEDS_BUILD
  // No image processing on 3DS – just set coverFile to " "
  for (auto &[setid, metadataList] : namesOfSets) {
    for (auto &file : metadataList) {
      file.coverFile = " ";
    }
  }
  return;
#else
  for (auto &[setid, metadataList] : namesOfSets) {
    std::unordered_map<std::string, std::string> bgToCover;
    int coverIndex = 0;

    for (auto &file : metadataList) {
      if (file.bgImage.empty())
        continue;

      std::string beatmapDir = file.path;
      size_t lastSlash = beatmapDir.find_last_of("/\\");
      if (lastSlash != std::string::npos)
        beatmapDir = beatmapDir.substr(0, lastSlash);
      normalizePath(beatmapDir);

      std::string fullBgPath = beatmapDir + "/" + file.bgImage;
      normalizePath(fullBgPath);

      if (!fileExists(fullBgPath))
        continue;

      auto it = bgToCover.find(fullBgPath);
      if (it != bgToCover.end()) {
        file.coverFile = it->second;
        continue;
      }

      std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236mProcessing: " << fullBgPath
                << std::endl;

      // Load and process image (raylib)
      Image img = LoadImage(fullBgPath.c_str());
      if (img.data == nullptr) {
        std::cerr << "Failed to load image: " << fullBgPath << std::endl;
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
      if (!dirExists(setDir))
        createDir(setDir);
      std::string outPath = setDir + "/" + coverName;
      ExportImage(img, outPath.c_str());
      UnloadImage(&img);

      bgToCover[fullBgPath] = coverName;
      file.coverFile = coverName;
    }
  }
#endif
}