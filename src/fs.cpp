#include "fs.hpp"
#include "globals.hpp"
#include <stdio.h> 
#include <dirent.h> 
#include <time_util.hpp>
//This file includes the functions for basic file operations
namespace fs = std::filesystem;

//Basic "ls" or "dir" function for listing files in a directory into a string vector.
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
        sort(text.begin(), text.end());
    #endif
    return text;
}

//Basically the "mkdir" command
void create_dir(const std::string& path) {
    fs::create_directory(path);
}

//Getting a file's name without the extension
std::string get_without_ext(const std::string& path) {
    fs::path path_p(Global.selectedPath);
    return path_p.stem().string();
}

//Something about the zip library I am using needed this...
int on_extract_entry(const char *filename, void *arg) {
    int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);
    return 0;
}



bool checkIfExists(const char *name){
    if (FILE *file = fopen(name, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}