#include "cachebuilder/metadataParser.hpp"
#include "utils.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdio.h> 
#include <filesystem>
#include <dirent.h> 
#ifndef _DIRENT_HAVE_D_TYPE
    #include <algorithm>
#endif

namespace fs = std::filesystem;

std::map<int, std::vector<FileMetadata>> namesOfSets;

void buildFileMap(std::string path){
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
    
        if (dr == NULL){ // opendir returns NULL if couldn't open directory { 
            printf("Could not open current directory" ); 
        }
        else{
            while ((de = readdir(dr)) != NULL) {
                std::string filename = de->d_name;
                std::string combinedPath = path + "/" + filename;
                if(filename[0] != '.'){
                    if(de->d_type == DT_DIR){
                        buildFileMap(combinedPath);
                    }
                    else if(IsFileExtension(filename.c_str(), ".osu")){
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
        for (const auto & entry : fs::directory_iterator(path)){
            std::string filename = entry.path().filename().string();
            std::string filepath = entry.path().string();
            for (int i = 0; i < filepath.size(); i++) {
                if (filepath[i] == '\\') {
                    filepath[i] = '/';
                }
            }
            fs::directory_entry isDirectory(entry.path());
            if(filename[0] != '.'){
                if(isDirectory.is_directory()){
                    buildFileMap(filepath);
                }
                else if(entry.path().extension() == ".osu"){
                    text.push_back(filename);
                    addFileToMap(filepath);
                }
                
            }
        }
        std::sort(text.begin(), text.end());
    #endif
    return;
}

void addFileToMap(std::string path){
    std::vector<std::string> output;
    output = ParseNameFile(path);
    //std::cout << path << std::endl;
    if(output.empty())
        return;
    for(int i = 0; i < output.size(); i++){
        if(output[i].empty()) return;
    }
    //std::cout << "Title:" << output[0] << " Artist:" << output[1] << " Creator:" << output[2] << " Version:" << output[3] << " SetID:" << output[4] << " ID:" << output[5] << std::endl;
    FileMetadata temp = { .path = path, .title = output[0], .artist = output[1], .creator = output[2], .version = output[3], .setid = std::stoi(output[4]), .id = std::stoi(output[5])};
    namesOfSets[temp.setid].push_back(temp);
}

void listAllMaps() {
    // 1. Loop through each key-value pair in the map
    // 'setid' is the key (int), 'metadataList' is the value (std::vector)
    for (const auto& [setid, metadataList] : namesOfSets) {
        std::cout << "--- Map Set ID: " << setid << " ---" << std::endl;

        // 2. Loop through the vector of FileMetadata objects for this specific set
        for (const auto& file : metadataList) {
            std::cout << "Path:    " << file.path    << "\n"
                      << "Title:   " << file.title   << "\n"
                      << "Artist:  " << file.artist  << "\n"
                      << "Creator: " << file.creator << "\n"
                      << "Version: " << file.version << "\n"
                      << "Beatmap ID: " << file.id   << "\n"
                      << "----------------------------------" << std::endl;
        }
    }
}

void clearFileMap(){
    for (auto& pair : namesOfSets) {
        pair.second.clear();
    }
    namesOfSets.clear();
}


