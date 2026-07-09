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

#include "globals.hpp"

namespace fs = std::filesystem;

std::map<int, std::vector<FileMetadata>> namesOfSets;
std::map<int, std::string> decidedNames;
std::map<int, int> numberOfMaps;

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

void clearFile(const std::string& filename){
    FILE* file = fopen(filename.c_str(), "w");
    std::cout << "clearing: " << filename << std::endl;
    if (file == nullptr) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    fclose(file);
}

void writeBeatmapFile(int setid, const std::vector<FileMetadata>& metadataList) {
    // 1. Create the directory name string
    std::string dir_name = Global.DatabaseLocation + "/" + std::to_string(setid);

    // Create the folder securely
    if (!fs::exists(dir_name)) {
        fs::create_directory(dir_name);
    }

    // 2. Loop through each item in your metadata vector
    for (size_t i = 0; i < metadataList.size(); ++i) {
        const auto& file_info = metadataList[i];

        std::string file_path = dir_name + "/" + std::to_string(file_info.id) + ".db";
        FILE* file = fopen(file_path.c_str(), "w");
        
        if (file == nullptr) {
            std::cerr << "Error: Could not create file via fopen: " << file_path << "\n";
            continue;
        }

        // 4. Write data using fprintf
        fprintf(file, "Path:%s\n", file_info.path.c_str());
        fprintf(file, "Title:%s\n", file_info.title.c_str());
        fprintf(file, "Artist:%s\n", file_info.artist.c_str());
        fprintf(file, "Creator:%s\n", file_info.creator.c_str());
        fprintf(file, "Version:%s\n", file_info.version.c_str());
        fprintf(file, "BeatmapID:%d\n", file_info.id);
        fprintf(file, "BeatmapSetID:%d\n", setid);

        // 5. Always close the file pointer
        fclose(file);
    }
}

void writeBeatmapSetFile(const std::string& filename, 
                       int beatmap_set_id, 
                       const std::string& title, 
                       const std::map<std::string, int>& selection) {
    
    // Opening with "w" clears the file contents automatically before writing
    FILE* file = fopen(filename.c_str(), "a");
    
    if (file == nullptr) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    // 1. Write the Header ID and Title
    fprintf(file, "[%d]\n", beatmap_set_id);
    fprintf(file, "Title:%s\n", title.c_str());

    // 2. Build the Maps and IDs lists from your map
    std::string ids_list = "";

    for(int i = 0; i < namesOfSets[beatmap_set_id].size(); i++){
        if (!ids_list.empty()) {
            ids_list += ",";
        }
        ids_list += std::to_string(namesOfSets[beatmap_set_id][i].id);
    }

    std::string artists_list = "";

    for(int i = 0; i < namesOfSets[beatmap_set_id].size(); i++){
        if(artists_list.find(namesOfSets[beatmap_set_id][i].artist) == std::string::npos){
            if (!artists_list.empty()) {
                artists_list += ", ";
            }
            artists_list += namesOfSets[beatmap_set_id][i].artist;
        }
    }

    std::string creators_list = "";

    for(int i = 0; i < namesOfSets[beatmap_set_id].size(); i++){
        if(creators_list.find(namesOfSets[beatmap_set_id][i].creator) == std::string::npos){
            if (!creators_list.empty()) {
                creators_list += ", ";
            }
            creators_list += namesOfSets[beatmap_set_id][i].creator;
        }
    }

    // 3. Write them to the file structure
    //fprintf(file, "Maps: %s\n", maps_list.c_str());
    fprintf(file, "Maps:%d\n", numberOfMaps[beatmap_set_id]);
    fprintf(file, "IDs:%s\n", ids_list.c_str());
    fprintf(file, "Artists:%s\n", artists_list.c_str());
    fprintf(file, "Creators:%s\n\n", creators_list.c_str());
    // Always close your file pointers!
    fclose(file);
}

void decideNamesForSets(){
    decidedNames.clear();
    numberOfMaps.clear();
    std::string dbFile = Global.DatabaseLocation + "/beatmapsets.db";
    clearFile(std::filesystem::path(dbFile).string());
    for (const auto& [setid, metadataList] : namesOfSets) {
        std::cout << "--- Map Set ID: " << setid << " ---" << std::endl;

        std::map<std::string, int> selection;
        // 2. Loop through the vector of FileMetadata objects for this specific set
        for (const auto& file : metadataList) {
            selection[file.title]++;
            numberOfMaps[setid]++;
        }
        std::string title = "error";
        if(!selection.empty()){
            int max_value = -1;
            for (const auto& [key, value] : selection) {
                if (value > max_value) {
                    max_value = value;
                    title = key;
                }
            }
        }

        std::cout << setid << " - Title: " << title << " - Number of maps:  " << numberOfMaps[setid] << std::endl; 
        writeBeatmapSetFile(std::filesystem::path(dbFile).string(), setid, title, selection);
        writeBeatmapFile(setid, metadataList);
    }
}

void clearFileMap(){
    for (auto& pair : namesOfSets) {
        pair.second.clear();
    }
    namesOfSets.clear();
}




std::vector<SetFileMetadata> parseCachedSets(const std::string& db_path) {
    std::vector<SetFileMetadata> metadata_list;
    // Open the text file using standard C fopen
    FILE* file = fopen(db_path.c_str(), "r");
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
        while (!line_str.empty() && (line_str.back() == '\n' || line_str.back() == '\r')) {
            line_str.pop_back();
        }

        if (line_str.empty()) continue;
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
        }
        else if (line_str.rfind("Maps:", 0) == 0) {
            std::string number_raw = line_str.substr(5);
            current_meta.number = std::stoi(number_raw);
        }
        else if (line_str.rfind("Artists:", 0) == 0) {
            current_meta.artists = line_str.substr(8);
        }
        else if (line_str.rfind("Creators:", 0) == 0) {
            current_meta.creators = line_str.substr(9);
        }
    }
    if (processing_entry) {
        metadata_list.push_back(current_meta);
    }
    fclose(file);
    return metadata_list;
}