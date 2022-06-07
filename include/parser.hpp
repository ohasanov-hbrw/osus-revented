#pragma once
#include <vector>
#include <stdio.h>
#include <GameFile.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <map>

class Parser{
    public:
        Parser();
        GameFile parse(std::string file);
    private:
        std::pair<std::string, std::string> parseKeyValue(std::string text, bool hasSpaceBefore, bool hasSpaceAfter);
        std::vector<std::string> parseSeperatedLists(std::string, char);
        std::string DecimalToBinaryString(int);
};
