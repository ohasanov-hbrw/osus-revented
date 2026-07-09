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
};

struct SetFileMetadata{
    std::string path;
    std::string title;
    std::string artists;
    std::string creators;
    int setid;
    int number;
};

void buildFileMap(std::string path);
void listAllMaps();
void clearFileMap();
void addFileToMap(std::string path);
void decideNamesForSets();
std::vector<SetFileMetadata> parseCachedSets(const std::string& db_path);


