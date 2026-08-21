#pragma once
#include <vector>
#include <gamefile.hpp>
#include <filesystem>

class Parser{
    public:
        Parser();
        ~Parser() = default;
        GameFile parse(std::string file);
        GameFile parseMetadata(std::string file);
        std::string parseBackground(std::string file);
    private:
        std::pair<std::string, std::string> parseKeyValue(std::string text, bool hasSpaceBefore, bool hasSpaceAfter);
        std::vector<std::string> parseSeperatedLists(std::string, char);
        std::string DecimalToBinaryString(int);
};


std::filesystem::path prepare_long_path(const std::string& input_path);