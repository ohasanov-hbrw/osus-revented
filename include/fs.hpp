#pragma once

#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>



std::vector<std::string> ls(char*);

std::string get_without_ext(const std::string& path);
void create_dir(const std::string& path);
int on_extract_entry(const char *filename, void *arg);
bool checkIfExists(const char *name);