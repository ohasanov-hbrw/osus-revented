#pragma once

#include <vector>
#include <string>
#include <algorithm>


std::vector<std::string> ls(char*);
std::vector<std::string> ls(char*, std::string_view path);

std::string get_without_ext(std::string_view path);
void create_dir(std::string_view path);
int on_extract_entry(const char *filename, void *arg);
bool checkIfExists(const char *name);
int check_dir(std::string_view path);
void print_dir(std::string_view path);
std::string correct_path_notation(std::string_view path);
std::string get_filename(std::string_view path);