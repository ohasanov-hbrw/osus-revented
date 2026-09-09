#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <atomic>

struct FileMetadata {
  std::string path;
  std::string title;
  std::string artist;
  std::string creator;
  std::string version;
  int setid;
  int id;
  std::string
      bgImage; // original background filename (relative to beatmap folder)
  std::string
      coverFile; // processed cover filename (e.g., "cover_12345_abc.jpg")
};

struct SetFileMetadata {
  std::string path;
  std::string title;
  std::string artists;
  std::string creators;
  int setid;
  int number;
  std::string bgImage;
};

void buildFileMap(const std::string& rootPath);
void listAllMaps();
void clearFileMap();
void addFileToMap(const std::string& path);
void decideNamesForSets();
std::vector<SetFileMetadata> parseCachedSets(const std::string &db_path);
std::vector<FileMetadata> parseCachedMaps(const std::string &db_path,
                                          int setid);
std::string extractBackgroundImage(const std::string &osuPath);
void writeBeatmapSetFile(const std::string &filename, int beatmap_set_id,
                         const std::string &title, int numMaps,
                         const std::vector<FileMetadata> &metadataListObj);
void writeBeatmapFile(int setid, const std::vector<FileMetadata> &metadataList);
void flushBatch();
void updateOrAppendSetInDb(const std::string &dbFile, int setid,
                           const std::string &title, int mapCount,
                           const std::vector<FileMetadata> &metadataList);
void processAllSetImages();
void normalizePath(std::string &path);
bool fileExists(const std::string &path);
extern std::atomic<int> numBeatmapsFound;

extern std::unordered_map<int, std::vector<FileMetadata>> namesOfSets;