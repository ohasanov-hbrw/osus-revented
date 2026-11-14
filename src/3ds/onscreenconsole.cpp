#include "onscreenconsole.hpp"

#ifdef THREEDS_BUILD

#include "globals.hpp"
#include "platformspesifics.hpp"
#include "raylib.h"
#include "text.h"
#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <malloc.h>

OnScreenConsole g_onScreenConsole;

static Color ColorFrom256(int n) {
    if (n < 0) n = 0;
    if (n < 16) {
        static const Color basic[16] = {
            {0,0,0,255},{128,0,0,255},{0,128,0,255},{128,128,0,255},
            {0,0,128,255},{128,0,128,255},{0,128,128,255},{192,192,192,255},
            {128,128,128,255},{255,0,0,255},{0,255,0,255},{255,255,0,255},
            {0,0,255,255},{255,0,255,255},{0,255,255,255},{255,255,255,255}
        };
        return basic[n];
    } else if (n < 232) {
        int idx = n - 16;
        int b = idx % 6; idx /= 6;
        int g = idx % 6; idx /= 6;
        int r = idx % 6;
        int vals[6] = {0,95,135,175,215,255};
        return (Color){(unsigned char)vals[r],(unsigned char)vals[g],(unsigned char)vals[b],255};
    } else {
        int g = 8 + (n - 232) * 10;
        if (g > 255) g = 255;
        return (Color){(unsigned char)g,(unsigned char)g,(unsigned char)g,255};
    }
}

static void DrawAnsiText(Font *f, const char *text, float x, float y, float fontSize, float spacing, Color defaultColor) {
    Color cur = defaultColor;
    std::string seg;
    float cx = x;
    for (size_t i = 0; text[i] != '\0'; ) {
        unsigned char c = (unsigned char)text[i];
        if (c == 0x1b || (c == '\\' && text[i+1] == '0' && text[i+2] == '3' && text[i+3] == '3')) {
            if (!seg.empty()) {
                DrawTextEx(f, seg.c_str(), (Vector2){cx, y}, fontSize, spacing, cur);
                Vector2 m = MeasureTextEx(f, seg.c_str(), fontSize, spacing);
                cx += m.x;
                seg.clear();
            }
            size_t j = i;
            if (c == 0x1b) j = i + 1;
            else j = i + 4;
            if (text[j] == '[') {
                j++;
                std::vector<int> params;
                int val = 0;
                bool has = false;
                while (text[j] != '\0') {
                    if (text[j] >= '0' && text[j] <= '9') {
                        val = val * 10 + (text[j] - '0');
                        has = true;
                        j++;
                    } else if (text[j] == ';') {
                        params.push_back(has ? val : 0);
                        val = 0; has = false;
                        j++;
                    } else {
                        if (has || (!params.empty())) params.push_back(has ? val : 0);
                        char cmd = text[j];
                        j++;
                        if (cmd == 'm') {
                            if (params.empty()) params.push_back(0);
                            for (size_t p = 0; p < params.size(); p++) {
                                int code = params[p];
                                if (code == 0) {
                                    cur = defaultColor;
                                } else if (code >= 30 && code <= 37) {
                                    static const Color c8[8]={{0,0,0,255},{170,0,0,255},{0,170,0,255},{170,85,0,255},{0,0,170,255},{170,0,170,255},{0,170,170,255},{170,170,170,255}};
                                    cur = c8[code-30];
                                } else if (code >= 90 && code <= 97) {
                                    static const Color c8b[8]={{85,85,85,255},{255,85,85,255},{85,255,85,255},{255,255,85,255},{85,85,255,255},{255,85,255,255},{85,255,255,255},{255,255,255,255}};
                                    cur = c8b[code-90];
                                } else if (code == 39) {
                                    cur = defaultColor;
                                } else if (code == 38 && p + 2 < params.size() && params[p+1] == 5) {
                                    cur = ColorFrom256(params[p+2]);
                                    p += 2;
                                }
                            }
                        }
                        break;
                    }
                }
                i = j;
                continue;
            } else {
                i = j;
                continue;
            }
        }
        seg.push_back((char)c);
        i++;
    }
    if (!seg.empty()) {
        DrawTextEx(f, seg.c_str(), (Vector2){cx, y}, fontSize, spacing, cur);
    }
}

OnScreenConsoleBuf::OnScreenConsoleBuf() {}

int OnScreenConsoleBuf::overflow(int c) {
    if (c == traits_type::eof()) return traits_type::not_eof(c);
    char ch = static_cast<char>(c);
    if (ch == '\n') {
        flushLine();
    } else if (ch == '\r') {
    } else {
        current.push_back(ch);
    }
    return c;
}

int OnScreenConsoleBuf::sync() {
    if (!current.empty()) {
        flushLine();
    }
    return 0;
}

void OnScreenConsoleBuf::flushLine() {
    g_onScreenConsole.addLine(current);
    current.clear();
}

OnScreenConsole::OnScreenConsole() {
    _multithread_mutex_init(&mtx);
}

void OnScreenConsole::setMaxLines(int n) {
    _multithread_mutex_lock(&mtx);
    maxLines = n;
    if (maxLines < 1) maxLines = 1;
    while ((int)lines.size() > maxLines) {
        lines.erase(lines.begin());
    }
    _multithread_mutex_unlock(&mtx);
}

int OnScreenConsole::getMaxLines() const {
    return maxLines;
}

void OnScreenConsole::init() {
    updateMetrics();
}

void OnScreenConsole::updateMetrics() {
    hudHeight = 10 * 8 + 4;
    logStartY = hudHeight;
    logLineHeight = 10;
    int screenH = 240;
    int available = screenH - logStartY - 2;
    int n = available / logLineHeight;
    if (n < 1) n = 1;
    setMaxLines(n);
}

void OnScreenConsole::shutdown() {
    _multithread_mutex_lock(&mtx);
    lines.clear();
    _multithread_mutex_unlock(&mtx);
}

void OnScreenConsole::addLine(const std::string &line) {
    _multithread_mutex_lock(&mtx);
    if (maxLines <= 0) maxLines = 1;
    lines.push_back(line);
    while ((int)lines.size() > maxLines) {
        lines.erase(lines.begin());
    }
    _multithread_mutex_unlock(&mtx);
}

static Color PressureColor(double ratio) {
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;
    if (ratio > 0.5) return (Color){40, 200, 80, 255};
    if (ratio > 0.25) return (Color){230, 200, 40, 255};
    if (ratio > 0.1) return (Color){230, 120, 40, 255};
    return (Color){230, 50, 50, 255};
}

static void DrawMemBar(Font *f, int x, int y, int w, const char *label, long long freeKb, long long totalKb, float sw) {
    double ratio = 0.0;
    if (totalKb > 0) ratio = (double)freeKb / (double)totalKb;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;
    Color col = PressureColor(ratio);
    char buf[64];
    if (totalKb > 0)
        snprintf(buf, sizeof(buf), "%s FREE %lld/%lldKB", label, freeKb, totalKb);
    else
        snprintf(buf, sizeof(buf), "%s FREE %lldKB", label, freeKb);
    DrawTextEx(f, buf, (Vector2){(float)x, (float)y}, 10, 1, (Color){220,220,220,255});
    int barH = 6;
    int barY = y + 2;
    int barX = x + 150;
    int barW = (int)sw - barX - 4;
    if (barW > 40) {
        DrawRectangle(barX, barY, barW, barH, (Color){50,50,60,255});
        int fillW = (int)(barW * ratio);
        if (fillW < 0) fillW = 0;
        if (fillW > barW) fillW = barW;
        DrawRectangle(barX, barY, fillW, barH, col);
        DrawRectangleLinesEx((Rectangle){(float)barX, (float)barY, (float)barW, (float)barH}, 1, (Color){90,90,100,255});
    }
}

extern "C" u32 __ctru_heap_size;


void OnScreenConsole::draw() {
    updateMetrics();
    float sw = Global.useTopScreen ? 320.0f : 400.0f;
    float sh = 240.0f;
    DrawRectangle(0, 0, (int)sw, (int)sh, (Color){16, 16, 24, 255});

    struct mallinfo mi = mallinfo();
    long long unclaimedApp = osGetMemRegionFree(MEMREGION_APPLICATION);
    long long linFree = _os_get_free_linear_ram() / 1024;
    long long linTotal = (long long)Global.linearSpaceFree / 1024;
    long long vramFree = _os_get_free_vram() / 1024;
    // 3. C/C++ Heap (actual live allocations vs. total heap capacity)
    long long heapTotal = (long long)__ctru_heap_size / 1024;
    long long heapUsed  = mi.uordblks / 1024;
    long long heapFree  = heapTotal - heapUsed;

    // 4. Mapped Heap Pages (sbrk high-water mark vs. total heap capacity)
    long long pagesTotal = heapTotal;
    long long pagesUsed  = mi.arena / 1024;
    long long pagesFree  = pagesTotal - pagesUsed;

    long long allFree = _os_get_free_ram(MEMREGION_ALL) / 1024;
    long long allTotal = _os_get_size_ram(MEMREGION_ALL) / 1024;

    double budget = 1000.0 / (double)(Global.TPS > 0 ? Global.TPS : 1);
    double avgSleep = Global.avgSleepMs;
    double util = 0.0;
    if (budget > 0.0) {
        util = (budget - avgSleep) / budget * 100.0;
    }
    if (util < 0.0) util = 0.0;
    if (util > 100.0) util = 100.0;

    int x = 2;
    int y = 2;
    int lh = 10;
    Font *f = &Global.DefaultFont;

    DrawMemBar(f, x, y, (int)sw, "LIN", linFree, linTotal, sw); y += lh;
    DrawMemBar(f, x, y, (int)sw, "HEA", heapFree, heapTotal, sw); y += lh;
    DrawMemBar(f, x, y, (int)sw, "PGE", pagesFree, pagesTotal, sw); y += lh;
    DrawMemBar(f, x, y, (int)sw, "ALL", allFree, allTotal, sw); y += lh;

    {
        double vtotal = 6144.0;
        double vratio = (double)vramFree / vtotal;
        if (vratio < 0.0) vratio = 0.0;
        if (vratio > 1.0) vratio = 1.0;
        Color vcol = PressureColor(vratio);
        char vbuf[64];
        snprintf(vbuf, sizeof(vbuf), "VRAM FREE %lldKB", vramFree);
        DrawTextEx(f, vbuf, (Vector2){(float)x, (float)y}, 10, 1, (Color){220,220,220,255});
        int barH = 6; int barY = y + 2; int barX = x + 150; int barW = (int)sw - barX - 4;
        if (barW > 40) {
            DrawRectangle(barX, barY, barW, barH, (Color){50,50,60,255});
            int fillW = (int)(barW * vratio);
            if (fillW < 0) fillW = 0;
            if (fillW > barW) fillW = barW;
            DrawRectangle(barX, barY, fillW, barH, vcol);
            DrawRectangleLinesEx((Rectangle){(float)barX, (float)barY, (float)barW, (float)barH}, 1, (Color){90,90,100,255});
        }
        y += lh;
    }

    {
        double ur = util / 100.0;
        if (ur < 0.0) ur = 0.0; if (ur > 1.0) ur = 1.0;
        Color ucol = (ur < 0.6) ? (Color){40,200,80,255} : (ur < 0.85 ? (Color){230,200,40,255} : (Color){230,60,50,255});
        char ubuf[96];
        snprintf(ubuf, sizeof(ubuf), "CPU %.1f%% slp %.3fms", util, avgSleep);
        DrawTextEx(f, ubuf, (Vector2){(float)x, (float)y}, 10, 1, (Color){220,220,220,255});
        int barH = 6; int barY = y + 2; int barX = x + 150; int barW = (int)sw - barX - 4;
        if (barW > 40) {
            DrawRectangle(barX, barY, barW, barH, (Color){50,50,60,255});
            int fillW = (int)(barW * ur);
            DrawRectangle(barX, barY, fillW, barH, ucol);
            DrawRectangleLinesEx((Rectangle){(float)barX, (float)barY, (float)barW, (float)barH}, 1, (Color){90,90,100,255});
        }
        y += lh;
    }

    DrawTextEx(f, TextFormat("FT %.2fms FPS %.1f TPS %.1f", Global.avgFrameTimeMs, Global.avgFPS, Global.avgHZ), (Vector2){(float)x, (float)y}, 10, 1, YELLOW);
    y += lh;

    DrawRectangle(0, hudHeight - 2, (int)sw, 1, (Color){80,80,100,255});

    std::vector<std::string> snapshot;
    {
        _multithread_mutex_lock(&mtx);
        snapshot = lines;
        _multithread_mutex_unlock(&mtx);
    }
    int ly = logStartY;
    for (size_t i = 0; i < snapshot.size(); i++) {
        DrawAnsiText(f, snapshot[i].c_str(), (float)x, (float)ly, 10, 1, (Color){220,220,220,255});
        ly += logLineHeight;
        if (ly > (int)sh - logLineHeight) break;
    }
}

void consoleui_init() {
    g_onScreenConsole.init();
}

void consoleui_shutdown() {
    g_onScreenConsole.shutdown();
}

void consoleui_draw() {
    g_onScreenConsole.draw();
}

static OnScreenConsoleBuf *g_consoleBuf = nullptr;
static std::streambuf *g_oldCout = nullptr;
static std::streambuf *g_oldCerr = nullptr;

void consoleui_capture_install() {
    if (g_consoleBuf) return;
    g_consoleBuf = new OnScreenConsoleBuf();
    g_oldCout = std::cout.rdbuf(g_consoleBuf);
    g_oldCerr = std::cerr.rdbuf(g_consoleBuf);
}

void consoleui_capture_restore() {
    if (!g_consoleBuf) return;
    if (g_oldCout) std::cout.rdbuf(g_oldCout);
    if (g_oldCerr) std::cerr.rdbuf(g_oldCerr);
    delete g_consoleBuf;
    g_consoleBuf = nullptr;
    g_oldCout = nullptr;
    g_oldCerr = nullptr;
}

#endif
