#pragma once

#include <string>
#include <vector>
#include <map>

void buildFileMap(std::string path);
void listAllMaps();
void clearFileMap();
void addFileToMap(std::string path);


struct FileMetadata{
    std::string path;
    std::string title;
    std::string artist;
    std::string creator;
    std::string version;
    int setid;
    int id;
};