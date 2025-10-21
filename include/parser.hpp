#pragma once
#include <vector>
#include <gamefile.hpp>

class Parser{
    public:
        Parser();
        GameFile parse(std::string file);
        GameFile parseMetadata(std::string file);
        std::string parseBackground(std::string file);
    private:
        std::pair<std::string, std::string> parseKeyValue(std::string text, bool hasSpaceBefore, bool hasSpaceAfter);
        std::vector<std::string> parseSeperatedLists(std::string, char);
        std::string DecimalToBinaryString(int);
};
