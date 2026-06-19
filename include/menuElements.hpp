#pragma once
#include <vector>
#include "raylib.h"
#include <string>
#include <memory>
#include "utils.hpp"

enum class HAlign { Left, Center, Right };
enum class VAlign { Top, Middle, Bottom };

class TextBoxSmart {
public:
    TextBoxSmart() 
        : m_text(""), m_box{0, 0, 0, 0}, m_fontSize(10), m_spacing(1), m_wrapWords(true) {
        // m_cachedLines starts empty automatically
    }

    // Setters that trigger re-calculation only when mutated
    void SetBox(const Rectangle& newBox) {
        if (m_box.x != newBox.x || m_box.y != newBox.y || m_box.width != newBox.width || m_box.height != newBox.height) {
            m_box = newBox;
            UpdateLayout();
        }
    }

    void SetText(const std::string& newText) {
        if (m_text != newText) {
            m_text = newText;
            UpdateLayout();
        }
    }

    void SetFontSize(float fontSize) { m_fontSize = fontSize; UpdateLayout(); }
    void SetSpacing(float spacing) { m_spacing = spacing; UpdateLayout(); }
    void SetWrapWords(bool wrap) { m_wrapWords = wrap; UpdateLayout(); }

    // Pure layout rendering (extremely fast, zero allocation)
    void Draw(HAlign hAlign, VAlign vAlign, Color color) const {
        Font font = GetFontDefault();
        float lineHeight = (float)Scale(m_fontSize);
        float totalTextHeight = m_cachedLines.size() * lineHeight;
        
        float startY = ScaleRect(m_box).y;
        if (vAlign == VAlign::Middle) {
            startY = ScaleRect(m_box).y + (ScaleRect(m_box).height - totalTextHeight) / 2.0f;
        } else if (vAlign == VAlign::Bottom) {
            startY = ScaleRect(m_box).y + ScaleRect(m_box).height - totalTextHeight;
        }

        for (size_t i = 0; i < m_cachedLines.size(); ++i) {
            float currentY = startY + (i * lineHeight);
            
            // Bounding box vertical safety crop
            if (currentY >= ScaleRect(m_box).y && (currentY + lineHeight) <= (ScaleRect(m_box).y + ScaleRect(m_box).height)) {
                float xPos = ScaleRect(m_box).x;
                
                if (hAlign != HAlign::Left) {
                    Vector2 textSize = MeasureTextEx(&font, m_cachedLines[i].c_str(), (float)Scale(m_fontSize), (float)Scale(m_spacing));
                    if (hAlign == HAlign::Center) {
                        xPos = ScaleRect(m_box).x + (ScaleRect(m_box).width - textSize.x) / 2.0f;
                    } else if (hAlign == HAlign::Right) {
                        xPos = ScaleRect(m_box).x + ScaleRect(m_box).width - textSize.x;
                    }
                }
                
                DrawTextEx(&font, m_cachedLines[i].c_str(), Vector2{ xPos, currentY }, (float)Scale(m_fontSize), (float)Scale(m_spacing), color);
            }
        }
    }

    const Rectangle& GetBox() const { return ScaleRect(m_box); }

private:
    std::string m_text;
    Rectangle m_box;
    float m_fontSize;
    float m_spacing;
    bool m_wrapWords;
    
    // The performance cache
    std::vector<std::string> m_cachedLines;

    // Heavy string processing isolated here
    void UpdateLayout() {
        m_cachedLines.clear();
        Font font = GetFontDefault();
        float lineHeight = (float)Scale(m_fontSize); 

        int maxLinesPossible = static_cast<int>(ScaleRect(m_box).height / lineHeight);
        if (maxLinesPossible < 1) maxLinesPossible = 1;

        size_t cursor = 0;
        size_t textLength = m_text.length();

        while (cursor < textLength && m_cachedLines.size() < static_cast<size_t>(maxLinesPossible)) {
            if (m_text[cursor] == '\n') {
                m_cachedLines.emplace_back("");
                cursor++;
                continue;
            }

            size_t takeChars = 0;
            size_t lastSpace = std::string::npos;
            
            while (cursor + takeChars < textLength) {
                char nextChar = m_text[cursor + takeChars];
                if (nextChar == '\n') break;
                if (nextChar == ' ') lastSpace = takeChars;

                std::string testLine = m_text.substr(cursor, takeChars + 1);
                Vector2 size = MeasureTextEx(&font, testLine.c_str(), (float)Scale(m_fontSize), (float)Scale(m_spacing));

                if (size.x > ScaleRect(m_box).width) break;
                takeChars++;
            }

            if (takeChars == 0) takeChars = 1; 

            if (m_wrapWords && (cursor + takeChars < textLength) && m_text[cursor + takeChars] != ' ' && m_text[cursor + takeChars] != '\n') {
                if (lastSpace != std::string::npos && lastSpace > 0) {
                    takeChars = lastSpace;
                }
            }

            bool isLastVisibleLine = (m_cachedLines.size() == static_cast<size_t>(maxLinesPossible - 1));
            bool textOverflows = (cursor + takeChars < textLength);

            if (isLastVisibleLine && textOverflows) {
                std::string cutText = m_text.substr(cursor, takeChars);
                while (!cutText.empty()) {
                    std::string testEllipsis = cutText + "...";
                    Vector2 size = MeasureTextEx(&font, testEllipsis.c_str(), (float)Scale(m_fontSize), (float)Scale(m_spacing));
                    if (size.x <= ScaleRect(m_box).width) {
                        m_cachedLines.push_back(testEllipsis);
                        break;
                    }
                    cutText.pop_back();
                }
                if (cutText.empty()) m_cachedLines.push_back(std::string("...").substr(0, takeChars));
                break; 
            } else {
                m_cachedLines.push_back(m_text.substr(cursor, takeChars));
                cursor += takeChars;
                while (cursor < textLength && m_text[cursor] == ' ') {
                    cursor++;
                }
            }
        }
    }
};


enum MenuObjectModes{
    CLICKABLEOBJECT_BUTTON = 0,
    CLICKABLEOBJECT_TOGGLE,
    CLICKABLEOBJECT_INACTIVE
};

class MenuElement {
public:
    
    bool drawOutline = false;
    
    Color outlineColor;
    Color outlineColorFocused;
    
    Color baseColor;
    Color baseColorFocused;
    Color baseColorFocusedActive;
    
    Color textColor;
    Color textColorActive;
    std::string text = "";
    TextBoxSmart internalBox;
    Rectangle textRect = {0,0,0,0};
    double textSpacing = 5.0f;

    double textSize = 20.0f;

    std::vector<Vector2> positions;
    
    MenuElement() = default;
    virtual ~MenuElement() = default;

    virtual void render();
    virtual void update();

    virtual void init();
    virtual void deinit();
};

class ClickableObject : public MenuElement {
private:
    
public:
    enum MenuObjectModes mode = CLICKABLEOBJECT_INACTIVE;
    
    
    ClickableObject() = default;
    ~ClickableObject() = default;
    void render() override;
    void update() override;

    void init() override;
    void deinit() override;
};

class FancyScrollingList : public MenuElement {
private:
    
public:
    enum MenuObjectModes mode = CLICKABLEOBJECT_INACTIVE;
    double objectDistance = 80.0f;
    double objectFreeSpace = 4.0f;
    int objectOffsetFull = 0;
    double objectOffset = 0;

    int graphicalObjectOffsetFull = 0;
    double graphicalObjectOffset = 0;
    double graphicalVelocity = 0;

    const int hardCodedOffset = -2;
    int numberOfObjects = 0;
    bool updateTextBox = false;
    bool updateTexts = false;

    int currentSelection = 0;
    std::vector<std::string> objectNames;
    std::vector<std::unique_ptr<ClickableObject>> objects;
    FancyScrollingList() = default;
    ~FancyScrollingList() = default;
    void render() override;
    void update() override;

    void init() override;
    void deinit() override;
};


