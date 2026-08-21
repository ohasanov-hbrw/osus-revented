#include "cachebuilder/metadataParser.hpp"
#include "utils.hpp"
#include <dirent.h>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

#include <algorithm>

#include "globals.hpp"

#include <functional>
#include <unordered_map>

namespace fs = std::filesystem;

std::map<int, std::vector<FileMetadata>> namesOfSets;
std::map<int, std::string> decidedNames;
std::map<int, int> numberOfMaps;

void buildFileMap(std::string path) {
  for (int i = 0; i < path.size(); i++) {
    if (path[i] == '\\') {
      path[i] = '/';
    }
  }

  if (!path.empty() && path.back() == '/') {
    path.pop_back();
  }
  std::vector<std::string> text;
  text.clear();

#ifdef _DIRENT_HAVE_D_TYPE
  struct dirent *de;

  DIR *dr = opendir(path.c_str());

  if (dr == NULL) { // opendir returns NULL if couldn't open directory {
    printf("Could not open current directory");
  } else {
    while ((de = readdir(dr)) != NULL) {
      std::string filename = de->d_name;
      std::string combinedPath = path + "/" + filename;
      if (filename[0] != '.') {
        if (de->d_type == DT_DIR) {
          buildFileMap(combinedPath);
        } else if (IsFileExtension(filename.c_str(), ".osu")) {
          text.push_back(filename);
          addFileToMap(combinedPath);
        }
      }
    }

    closedir(dr);
    free(de);
    std::sort(text.begin(), text.end());
  }
#endif
#ifndef _DIRENT_HAVE_D_TYPE
  for (const auto &entry : fs::directory_iterator(path)) {
    std::string filename = entry.path().filename().string();
    std::string filepath = entry.path().string();
    for (int i = 0; i < filepath.size(); i++) {
      if (filepath[i] == '\\') {
        filepath[i] = '/';
      }
    }
    fs::directory_entry isDirectory(entry.path());
    if (filename[0] != '.') {
      if (isDirectory.is_directory()) {
        buildFileMap(filepath);
      } else if (entry.path().extension() == ".osu") {
        text.push_back(filename);
        addFileToMap(filepath);
      }
    }
  }
  std::sort(text.begin(), text.end());
#endif
  return;
}

void addFileToMap(std::string path) {
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
      .setid = std::stoi(output[4]),
      .id = std::stoi(output[5]),
      .bgImage = bgImage,
      .coverFile = "" // will be filled later
  };
  namesOfSets[temp.setid].push_back(temp);
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

void clearFile(const std::string &filename) {
  FILE *file = fopen(filename.c_str(), "w");
  std::cout << "clearing: " << filename << std::endl;
  if (file == nullptr) {
    std::cerr << "Error: Could not open file " << filename << " for writing.\n";
    return;
  }

  fclose(file);
}

void writeBeatmapFile(int setid,
                      const std::vector<FileMetadata> &metadataList) {
  std::string dir_name = Global.DatabaseLocation + "/" + std::to_string(setid);
  if (!fs::exists(dir_name))
    fs::create_directory(dir_name);

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
    fprintf(file, "CoverFile:%s\n", file_info.coverFile.c_str()); // new line
    fclose(file);
  }
}

void writeBeatmapSetFile(const std::string &filename, int beatmap_set_id,
                         const std::string &title,
                         const std::map<std::string, int> &selection) {

  // Opening with "w" clears the file contents automatically before writing
  FILE *file = fopen(filename.c_str(), "a");

  if (file == nullptr) {
    std::cerr << "Error: Could not open file " << filename << " for writing.\n";
    return;
  }

  // 1. Write the Header ID and Title
  fprintf(file, "[%d]\n", beatmap_set_id);
  fprintf(file, "Title:%s\n", title.c_str());

  // 2. Build the Maps and IDs lists from your map
  std::string ids_list = "";

  for (int i = 0; i < namesOfSets[beatmap_set_id].size(); i++) {
    if (!ids_list.empty()) {
      ids_list += ",";
    }
    ids_list += std::to_string(namesOfSets[beatmap_set_id][i].id);
  }

  std::string artists_list = "";

  for (int i = 0; i < namesOfSets[beatmap_set_id].size(); i++) {
    if (artists_list.find(namesOfSets[beatmap_set_id][i].artist) ==
        std::string::npos) {
      if (!artists_list.empty()) {
        artists_list += ", ";
      }
      artists_list += namesOfSets[beatmap_set_id][i].artist;
    }
  }

  std::string creators_list = "";

  for (int i = 0; i < namesOfSets[beatmap_set_id].size(); i++) {
    if (creators_list.find(namesOfSets[beatmap_set_id][i].creator) ==
        std::string::npos) {
      if (!creators_list.empty()) {
        creators_list += ", ";
      }
      creators_list += namesOfSets[beatmap_set_id][i].creator;
    }
  }

  // 3. Write them to the file structure
  // fprintf(file, "Maps: %s\n", maps_list.c_str());
  fprintf(file, "Maps:%d\n", numberOfMaps[beatmap_set_id]);
  fprintf(file, "IDs:%s\n", ids_list.c_str());
  fprintf(file, "Artists:%s\n", artists_list.c_str());
  fprintf(file, "Creators:%s\n\n", creators_list.c_str());
  // Always close your file pointers!
  fclose(file);
}

void decideNamesForSets() {

  decidedNames.clear();
  numberOfMaps.clear();
  std::string dbFile = Global.DatabaseLocation + "/beatmapsets.db";
  processAllSetImages();
  clearFile(std::filesystem::path(dbFile).string());
  for (const auto &[setid, metadataList] : namesOfSets) {
    std::cout << "--- Map Set ID: " << setid << " ---" << std::endl;

    std::map<std::string, int> selection;
    // 2. Loop through the vector of FileMetadata objects for this specific set
    for (const auto &file : metadataList) {
      selection[file.title]++;
      numberOfMaps[setid]++;
    }
    std::string title = "error";
    if (!selection.empty()) {
      int max_value = -1;
      for (const auto &[key, value] : selection) {
        if (value > max_value) {
          max_value = value;
          title = key;
        }
      }
    }

    std::cout << setid << " - Title: " << title
              << " - Number of maps:  " << numberOfMaps[setid] << std::endl;
    writeBeatmapSetFile(std::filesystem::path(dbFile).string(), setid, title,
                        selection);
    writeBeatmapFile(setid, metadataList);
  }

  std::cout << "Decided Names" << std::endl;
}

void clearFileMap() {
  for (auto &pair : namesOfSets) {
    pair.second.clear();
  }
  namesOfSets.clear();
}

std::vector<SetFileMetadata> parseCachedSets(const std::string &db_path) {
  std::vector<SetFileMetadata> metadata_list;
  // Open the text file using standard C fopen
  FILE *file = fopen(db_path.c_str(), "r");
  if (file == nullptr) {
    std::cerr << "Error: Could not open database file: " << db_path << "\n";
    return metadata_list;
  }

  char line[1024];
  SetFileMetadata current_meta;
  bool processing_entry = false;
  while (fgets(line, sizeof(line), file) != nullptr) {
    std::string line_str(line);

    // Strip trailing newlines safely
    while (!line_str.empty() &&
           (line_str.back() == '\n' || line_str.back() == '\r')) {
      line_str.pop_back();
    }

    if (line_str.empty())
      continue;
    if (line_str.front() == '[' && line_str.find(']') != std::string::npos) {
      if (processing_entry) {
        metadata_list.push_back(current_meta);
      }
      current_meta = SetFileMetadata();
      processing_entry = true;
      size_t close_bracket = line_str.find(']');
      std::string id_str = line_str.substr(1, close_bracket - 1);
      current_meta.setid = std::stoi(id_str);
    }
    // 2. Parse out the Title
    else if (line_str.rfind("Title:", 0) == 0) {
      current_meta.title = line_str.substr(6);
    } else if (line_str.rfind("Maps:", 0) == 0) {
      std::string number_raw = line_str.substr(5);
      current_meta.number = std::stoi(number_raw);
    } else if (line_str.rfind("Artists:", 0) == 0) {
      current_meta.artists = line_str.substr(8);
    } else if (line_str.rfind("Creators:", 0) == 0) {
      current_meta.creators = line_str.substr(9);
    }
  }
  if (processing_entry) {
    metadata_list.push_back(current_meta);
  }
  fclose(file);
  return metadata_list;
}

std::vector<FileMetadata> parseCachedMaps(const std::string& db_path, int setid){
  
}

std::string extractBackgroundImage(const std::string &osuPath) {
  FILE *file = fopen(osuPath.c_str(), "r");
  if (!file) {
    std::string filename = osuPath;
    std::cout << "Couldn't open file for bgImage, retrying, maybe length? "
              << filename.size() << std::endl;
    for (int i = 0; i < filename.size(); i++) {
      if (filename[i] == '/') {
        filename[i] = '\\';
      }
    }
    filename = prepare_long_path(filename).string();
    std::cout << filename << std::endl;
    file = fopen(filename.c_str(), "r");
    if (file == nullptr) {
      return "";
    }
  }

  char line[1024];
  bool inEvents = false;
  std::string bgFile;

  while (fgets(line, sizeof(line), file)) {
    std::string lineStr(line);
    lineStr.erase(lineStr.find_last_not_of("\r\n") + 1); // trim newline

    if (lineStr == "[Events]") {
      inEvents = true;
      continue;
    }
    if (inEvents && lineStr.rfind("//", 0) == 0)
      continue; // skip comments
    if (inEvents && !lineStr.empty()) {
      // Look for a quoted string
      size_t first = lineStr.find('"');
      if (first != std::string::npos) {
        size_t second = lineStr.find('"', first + 1);
        if (second != std::string::npos) {
          bgFile = lineStr.substr(first + 1, second - first - 1);
          // Stop after finding the first background image
          break;
        }
      }
      // Stop if we hit another section header
      if (lineStr.front() == '[' && lineStr != "[Events]")
        break;
    }
    // Stop if we left the [Events] section
    if (inEvents && lineStr.front() == '[' && lineStr != "[Events]")
      break;
  }
  fclose(file);
  return bgFile;
}

void processAllSetImages() {
  for (auto &[setid, metadataList] : namesOfSets) {
    // Map: full path of original background -> processed cover filename
    std::unordered_map<std::string, std::string> bgToCover;
    // We'll generate unique filenames for each distinct background
    int coverIndex = 0;

    for (auto &file : metadataList) {
      if (file.bgImage.empty())
        continue;

      // Build the absolute path to the original image
      std::string beatmapDir = fs::path(file.path).parent_path().string();
      // Normalize slashes
      std::replace(beatmapDir.begin(), beatmapDir.end(), '\\', '/');
      std::string fullBgPath = beatmapDir + "/" + file.bgImage;
      // Normalize slashes
      // std::replace(fullBgPath.begin(), fullBgPath.end(), '\\', '/');

      if (!fs::exists(fullBgPath))
        continue;

      // Check if we already processed this exact image path
      auto it = bgToCover.find(fullBgPath);
      if (it != bgToCover.end()) {
        // Reuse the existing cover filename
        file.coverFile = it->second;
        continue;
      }
      std::cout << "Processing: " << fullBgPath << std::endl;
      // Load and process the image
      Image img = LoadImage(fullBgPath.c_str());
      if (img.data == nullptr) {
        std::cerr << "Failed to load image: " << fullBgPath << std::endl;
        continue;
      }
      float targetAspect = (float)COVER_WIDTH / COVER_HEIGHT;
      float imageAspect = (float)img.width / img.height;

      Rectangle cropRect;
      if (imageAspect > targetAspect) {
        // Image is wider – crop horizontal sides
        int cropWidth = (int)(img.height * targetAspect);
        cropRect = {(float)(img.width - cropWidth) / 2.0f, 0.0f,
                    (float)cropWidth, (float)img.height};
      } else {
        // Image is taller – crop vertical sides
        int cropHeight = (int)(img.width / targetAspect);
        cropRect = {0.0f, (float)(img.height - cropHeight) / 2.0f,
                    (float)img.width, (float)cropHeight};
      }
      ImageCrop(&img, cropRect);
      ImageResize(&img, COVER_WIDTH, COVER_HEIGHT);

      // Generate a unique filename for this cover
      // Use setid + an index (or a hash of the path to avoid collisions)
      std::string coverName = "cover_" + std::to_string(setid) + "_" +
                              std::to_string(coverIndex++) + ".bmp";

      // Save to the set directory
      std::string setDir =
          Global.DatabaseLocation + "/" + std::to_string(setid);
      if (!fs::exists(setDir))
        fs::create_directory(setDir);
      std::string outPath = setDir + "/" + coverName;
      ExportImage(img, outPath.c_str()); // fallback to BMP/PNG

      UnloadImage(&img);

      // Store mapping and assign to current beatmap
      bgToCover[fullBgPath] = coverName;
      file.coverFile = coverName;
    }
  }
}