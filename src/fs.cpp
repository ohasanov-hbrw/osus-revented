#include "fs.hpp"
#include "globals.hpp"

namespace fs = std::filesystem;

std::vector<std::string> ls(char* extension) {
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
            else if(entry.path().extension() == extension){
                text.push_back(filename);
            }
            
        }
    }
    sort(text.begin(), text.end());
    return text;
}

void create_dir(const std::string& path) {
    fs::create_directory(path);
}

std::string get_without_ext(const std::string& path) {
    fs::path path_p(Global.selectedPath);
    return path_p.stem();
}

int on_extract_entry(const char *filename, void *arg) {
    int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}
