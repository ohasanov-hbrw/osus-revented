#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <array>

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

INIValue parse_INIValue(std::string line) {
    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
    size_t pos = line.find(":");
    std::string ls = line.substr(0, pos);

    //ya space yoksa ?!?
    std::string rs = line.substr(pos + 1);

    return {ls, rs};
}

//bunu direk sil raylibinkini kullancaz
struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a = 255;
};

struct INISettings {
    struct General {
        std::string name;
        std::string author;
        std::string version = "latest";
        int animationFramerate = -1;
        bool allowSliderBallTint = 0;
        bool comboBurstRandom = 0;
        bool cursorCentre = 1;
        bool cursorExpand = 1;
        bool cursorRotate = 1;
        bool cursorTrailRotate = 1;
        bool hitCircleOverlayAboveNumber = 1;
        bool layeredHitSounds = 1;
        bool sliderBallFlip = 1;
        bool spinnerFadePlayfield = 0;
        bool spinnerFrequencyModulate = 1;
        bool spinnerNoBlink = 0;
    } general;

    struct Colours {
        Color combo1 = {255, 192, 0};
        Color combo2 = {0, 202, 0};
        Color combo3 = {18, 124, 255};
        Color combo4 = {242, 24, 57};

        Color combo5;
        Color combo6;
        Color combo7;
        Color combo8;
        Color inputOverlayText = {0, 0, 0};
        Color menuGlow = {0, 78, 155};
        Color sliderBall = {2, 170, 255};
        Color sliderBorder = {255, 255, 255};

        //kg ömer
        Color sliderTrackOverride;
        Color songSelectActiveText = {0, 0, 0};
        Color songSelectInactiveText = {255, 255, 255};
        Color spinnerBackground = {100, 100, 100};
        Color starBreakAdditive = {255, 182, 193};
    } colours;

    struct Fonts {
        std::string hitCirclePrefix = "default";
        int hitCircleOverlap = -2;
        std::string scorePrefix = "score";
        int scoreOverlap = 0;
        std::string comboPrefix = "score";
        int comboOverlap = 0;
    } fonts;
};

const std::array<std::string, 4> HEADERS {"General", "Colours", "Fonts", "CatchTheBeat"};

template <class C, class K>
bool in(C const& c, K const& k) {
    return std::find(c.begin(), c.end(), k) != c.end();
}

Color parse_color(std::string rs) {
    rs.erase(std::remove_if(rs.begin(), rs.end(), isspace), rs.end());
    std::replace(rs.begin(), rs.end(), ',', ' ');
    Color color;

    std::stringstream iss(rs);
    std::string r, g, b;
    iss >> r >> g >> b;

    color.r = std::stoi(r); color.g = std::stoi(g); color.b = std::stoi(b);

    return color;
}

INISettings set_settings(const std::map<std::string, std::vector<INIValue>>& ini) {
    INISettings settings;

    for(auto const& [header, values] : ini) {
        if(header == "General") {
            INISettings::General general;

            for(auto const& [ls, rs] : values) {
                if(ls == "Name") general.name = rs;
                else if(ls == "Author") general.author = rs;
                else if(ls == "Version") general.version = rs;
                else if(ls == "AnimationFramerate") general.animationFramerate = std::stoi(rs);
                else if(ls == "AllowSliderBallTint") general.allowSliderBallTint = std::stoi(rs);
                else if(ls == "ComboBurstRandom") general.comboBurstRandom = std::stoi(rs);
                else if(ls == "CursorCentre") general.cursorCentre = std::stoi(rs);
                else if(ls == "CursorExpand") general.cursorExpand = std::stoi(rs);
                else if(ls == "CursorRotate") general.cursorRotate = std::stoi(rs);
                else if(ls == "CursorTrailRotate") general.cursorTrailRotate = std::stoi(rs);
                else if(ls == "HitCircleOverlayAboveNumber") general.hitCircleOverlayAboveNumber = std::stoi(rs);
                else if(ls == "LayeredHitSounds") general.layeredHitSounds = std::stoi(rs);
                else if(ls == "SliderBallFlip") general.sliderBallFlip = std::stoi(rs);
                else if(ls == "SpinnerFadePlayfield") general.spinnerFadePlayfield = std::stoi(rs);
                else if(ls == "SpinnerFrequencyModulate") general.spinnerFrequencyModulate = std::stoi(rs);
                else if(ls == "SpinnerNoBlink") general.spinnerNoBlink = std::stoi(rs);
            }

            settings.general = general;
        } else if(header == "Colours") {
            INISettings::Colours colours;
            
            for(auto const& [ls, rs] : values) {
                if(ls == "Combo1") colours.combo1 = parse_color(rs);
                else if(ls == "Combo2") colours.combo2 = parse_color(rs);
                else if(ls == "Combo3") colours.combo3 = parse_color(rs);
                else if(ls == "Combo4") colours.combo4 = parse_color(rs);
                else if(ls == "Combo5") colours.combo5 = parse_color(rs);
                else if(ls == "Combo6") colours.combo6 = parse_color(rs);
                else if(ls == "Combo7") colours.combo7 = parse_color(rs);
                else if(ls == "Combo8") colours.combo8 = parse_color(rs);
                else if(ls == "InputOverlayText") colours.inputOverlayText = parse_color(rs);
                else if(ls == "MenuGlow") colours.menuGlow = parse_color(rs);
                else if(ls == "SliderBall") colours.sliderBall = parse_color(rs);
                else if(ls == "SliderBorder") colours.sliderBorder = parse_color(rs);
                else if(ls == "SliderTrackOverride") colours.sliderTrackOverride = parse_color(rs);
                else if(ls == "SongSelectActiveText") colours.songSelectActiveText = parse_color(rs);
                else if(ls == "SongSelectInactiveText") colours.songSelectInactiveText = parse_color(rs);
                else if(ls == "SpinnerBackground") colours.spinnerBackground = parse_color(rs);
                else if(ls == "StarBreakAdditive") colours.starBreakAdditive = parse_color(rs);
            }

            settings.colours = colours;
        } else if(header == "Fonts") {
            INISettings::Fonts fonts;

            for(auto const& [ls, rs] : values) {
                if(ls == "HitCirclePrefix") fonts.hitCirclePrefix = rs;
                else if(ls == "HitCircleOverlap") fonts.hitCircleOverlap = std::stoi(rs);
                else if(ls == "ScorePrefix") fonts.scorePrefix = rs;
                else if(ls == "ScoreOverlap") fonts.scoreOverlap = std::stoi(rs);
                else if(ls == "ComboPrefix") fonts.comboPrefix = rs;
                else if(ls == "ComboOverlap") fonts.comboOverlap = std::stoi(rs);
            }

            settings.fonts = fonts;
        }
    }

    return settings;
}

int main() {
    std::ifstream ifs("resources/skin2/skin.ini");
    std::string line;
    std::map<std::string, std::vector<INIValue>> ini;

    std::streampos oldpos;

    while(std::getline(ifs, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        if(starts_with(line, "[") && ends_with(line, "]")) {
            std::string header = line.substr(1);
            header.pop_back();

            std::string subLine;
            while(std::getline(ifs, subLine)){
                if(starts_with(subLine, "//") || std::all_of(subLine.begin(), subLine.end(), isspace)) {
                    continue; 
                } else if(starts_with(subLine, "[")) {
                    ifs.seekg(oldpos);
                    break;
                }

                auto val = parse_INIValue(subLine);
                ini[header].push_back(val);
                oldpos = ifs.tellg();
            }
        }
    }

    INISettings settings = set_settings(ini);

    return 0;
}
