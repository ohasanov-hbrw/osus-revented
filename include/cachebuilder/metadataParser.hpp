#pragma once

#include <string>
#include <vector>
#include <map>



struct FileMetadata{
    std::string path;
    std::string title;
    std::string artist;
    std::string creator;
    std::string version;
    int setid;
    int id;
    std::string bgImage;      // original background filename (relative to beatmap folder)
    std::string coverFile;    // processed cover filename (e.g., "cover_12345_abc.jpg")
};

struct SetFileMetadata{
    std::string path;
    std::string title;
    std::string artists;
    std::string creators;
    int setid;
    int number;
    std::string bgImage;
};

void buildFileMap(std::string path);
void listAllMaps();
void clearFileMap();
void addFileToMap(std::string path);
void decideNamesForSets();
std::vector<SetFileMetadata> parseCachedSets(const std::string& db_path);
std::vector<FileMetadata> parseCachedMaps(const std::string& db_path, int setid);
std::string extractBackgroundImage(const std::string& osuPath);

void processAllSetImages();

extern std::map<int, std::vector<FileMetadata>> namesOfSets;