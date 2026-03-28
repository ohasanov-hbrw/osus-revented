#include "fs.hpp"
#include "globals.hpp"
#include <iostream>
#include <stdio.h> 
#include <dirent.h> 
#include <time_util.hpp>
#include <string>
#ifndef _DIRENT_HAVE_D_TYPE
    #include <algorithm>
#endif
//This file includes the functions for basic file operations
namespace fs = std::filesystem;

// Basic "ls" or "dir" function for listing files in a directory into a string vector.
std::vector<std::string> ls(char* extension) {
    for (int i = 0; i < Global.Path.size(); i++) {
        if (Global.Path[i] == '\\') {
            Global.Path[i] = '/';
        }
    }
    std::vector<std::string> text;
    text.clear();

    #ifdef _DIRENT_HAVE_D_TYPE
        struct dirent *de;

        DIR *dr = opendir(Global.Path.c_str()); 
    
        if (dr == NULL){ // opendir returns NULL if couldn't open directory { 
            printf("Could not open current directory" ); 
        }
        else{
            while ((de = readdir(dr)) != NULL) {
                std::string filename = de->d_name;
                if(filename[0] != '.'){
                    if(de->d_type == DT_DIR){
                        filename.push_back('/');
                        text.push_back(filename);
                    }
                    else if(IsFileExtension(filename.c_str(), extension)){
                        text.push_back(filename);
                    }
                }
            }
    
            closedir(dr);
            free(de);
            std::sort(text.begin(), text.end());
        }
    #endif
    #ifndef _DIRENT_HAVE_D_TYPE
        for (const auto & entry : fs::directory_iterator(Global.Path)){
            std::string filename = entry.path().filename().string();
            fs::directory_entry isDirectory(entry.path());
            if(filename[0] != '.'){
                if(isDirectory.is_directory()){
                    filename.push_back('/');
                    text.push_back(filename);
                }
                else if(entry.path().extension() == extension){
                    text.push_back(filename);
                }
                
            }
        }
        std::sort(text.begin(), text.end());
    #endif
    return text;
}

std::vector<std::string> ls(char* extension, std::string_view path) {
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
                if(filename[0] != '.'){
                    if(de->d_type == DT_DIR){
                        filename.push_back('/');
                        text.push_back(filename);
                    }
                    else if(IsFileExtension(filename.c_str(), extension)){
                        text.push_back(filename);
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
            fs::directory_entry isDirectory(entry.path());
            if(filename[0] != '.'){
                if(isDirectory.is_directory()){
                    filename.push_back('/');
                    text.push_back(filename);
                }
                else if(entry.path().extension() == extension){
                    text.push_back(filename);
                }
                
            }
        }
        std::sort(text.begin(), text.end());
    #endif
    return text;
}

// Basically the "mkdir" command
void create_dir(const std::string& path) {
    fs::create_directory(path);
}

// Check if a path exists
int check_dir(const std::string &path){
    fs::path path_p(path);
    return fs::exists(path_p);
}

// Convert path to a correct notation
std::string correct_path_notation(const std::string &path){
    fs::path path_p(path);
    return path_p.string();
}

// Debug, print string as a directory, both local and absolute
void print_dir(const std::string &path){
    fs::path path_p(path);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << path_p << " or " << fs::absolute(path_p) << std::endl;
    return;
}

// Getting a file's name without the extension
std::string get_without_ext(const std::string& path) {
    fs::path path_p(path);
    return path_p.stem().string();
}

// Get filename
std::string get_filename(const std::string& path) {
    fs::path path_p(path);
    return path_p.filename().string();
}


// Something about the zip library I am using needed this...
int on_extract_entry(const char *filename, void *arg) {
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;40m" << "Extracted file " << filename << std::endl;
    return 0;
}

// Check if a file exists or not
bool checkIfExists(const char *name){
    if (FILE *file = fopen(name, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}