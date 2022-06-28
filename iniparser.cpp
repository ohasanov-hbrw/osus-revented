#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

bool starts_with(const std::string& str, const std::string& thing) {
    return str.rfind(thing, 0) == 0;
}

bool ends_with(std::string str, const std::string &thing) {
    if(str.size() >= thing.size() && str.compare(str.size() - thing.size(), thing.size(), thing) == 0)
        return true;
    else
        return false;
}

std::string read_file(const std::string& path) {
    std::ifstream ifs(path);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    return content;
}

struct INIValue {
    std::string ls;
    std::string rs;
};

INIValue parse_INIValue(const std::string& line) {
    size_t pos = line.find(":");
    std::string ls = line.substr(0, pos);

    //ya space yoksa ?!?
    std::string rs = line.substr(pos + 2);

    return {ls, rs};
}

int main() {
    std::ifstream ifs("resources/skin/skin.ini");
    std::string line;
    std::map<std::string, std::vector<INIValue>> ini;
    while(std::getline(ifs, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        if(starts_with(line, "[") && ends_with(line, "]")) {
            std::vector<INIValue> values;
            std::string header = line.substr(1);
            header.pop_back();

            std::string subLine;
            while(std::getline(ifs, subLine)){
                if(starts_with(subLine, "//") || std::all_of(subLine.begin(), subLine.end(), isspace)) {
                    continue; 
                } else if(starts_with(subLine, "[")) {
                    break;
                }

                auto val = parse_INIValue(subLine);
                values.push_back(val);
                std::cout << header << " -> " << val.ls << ":" << val.rs << std::endl;
            }
            ini[header] = values;
        }
    }

    return 0;
}
