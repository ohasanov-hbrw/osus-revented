#pragma once

#ifdef THREEDS_BUILD

#include <string>
#include <vector>
#include <streambuf>
#include <iostream>
#include "platformspesifics.hpp"

class OnScreenConsoleBuf : public std::streambuf {
public:
    OnScreenConsoleBuf();
protected:
    int overflow(int c) override;
    int sync() override;
    std::streamsize xsputn(const char *s, std::streamsize n) override;
private:
    std::string current;
    void flushLineLocked();
};

class OnScreenConsole {
public:
    OnScreenConsole();
    void init();
    void shutdown();
    void addLine(const std::string &line);
    void draw();
    void setMaxLines(int n);
    int getMaxLines() const;
    void updateMetrics();
private:
    void addLineLocked(const std::string &line);
    std::vector<std::string> lines;
    int maxLines = 0;
    MULTITHREAD_MUTEX mtx;
    int logStartY = 0;
    int logLineHeight = 10;
    int hudHeight = 0;
    friend class OnScreenConsoleBuf;
};

extern OnScreenConsole g_onScreenConsole;

void consoleui_init();
void consoleui_shutdown();
void consoleui_draw();
void consoleui_capture_install();
void consoleui_capture_restore();

#endif
