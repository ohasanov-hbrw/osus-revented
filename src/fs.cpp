#include "fs.hpp"
#include "globals.hpp"

namespace fs = std::filesystem;

std::vector<std::string> ls() {
    std::vector<std::string> text;
    text.clear();
    for (const auto & entry : fs::directory_iterator(Global.Path)){
        std::string filename = entry.path().filename();
        fs::directory_entry isDirectory(entry.path());
        if(filename[0] != '.'){
            if(isDirectory.is_directory()){
                filename.push_back('/');
                text.push_back(filename);
            }
            else if(entry.path().extension() == ".osu"){
                text.push_back(filename);
            }
            
        }
    }
    sort(text.begin(), text.end());
    return text;
}
