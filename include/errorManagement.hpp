#pragma once

#include <string>
#define ERR_INFO    0
#define ERR_FILEIO  1 << 0
#define ERR_SET     1 << 1
#define ERR_ZIP     1 << 3
#define ERR_RETRY   1 << 3
#define ERR_CRASH   1 << 4
#define ERR_CONFIRM 1 << 5

struct ErrorMessage {
    std::string message;
    int type;
    int id;
    int ans;
};