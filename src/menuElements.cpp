#include "menuElements.hpp"
#include "utils.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include "raylib.h"



// Scoped enums for alignment
enum class HAlign { Left, Center, Right };
enum class VAlign { Top, Middle, Bottom };

// Helper to draw a single line with horizontal alignment
void DrawLineAligned(std::string_view text, const Rectangle& box, float yPos, HAlign hAlign, int fontSize, int spacing, Color color) {
    Font font = GetFontDefault();
    // Null-terminate safely for Raylib's C-string API
    std::string nullTerminatedText(text); 
    Vector2 textSize = MeasureTextEx(&font, nullTerminatedText.c_str(), (float)fontSize, (float)spacing);
    
    float xPos = box.x;
    if (hAlign == HAlign::Center) {
        xPos = box.x + (box.width - textSize.x) / 2.0f;
    } else if (hAlign == HAlign::Right) {
        xPos = box.x + box.width - textSize.x;
    }

    DrawTextEx(&font, nullTerminatedText.c_str(), Vector2{ xPos, yPos }, (float)fontSize, (float)spacing, color);
}

// Main TextBox function
void DrawTextBox(const std::string& text, const Rectangle& box, HAlign hAlign, VAlign vAlign, int fontSize, int spacing, Color color, bool wrapWords) {
    Font font = GetFontDefault();
    
    // Calculate character layout bounds dynamically from font metrics
    float glyphWidth = font.recs[0].width * ((float)fontSize / font.baseSize);
    float charAdvance = glyphWidth + spacing;
    float lineHeight = (float)fontSize; 

    // Dynamic interpolation of limits based on the textbox dimensions
    int maxCharsPerLine = static_cast<int>(box.width / charAdvance);
    if (maxCharsPerLine < 1) maxCharsPerLine = 1;

    int maxLinesPossible = static_cast<int>(box.height / lineHeight);
    if (maxLinesPossible < 1) maxLinesPossible = 1;

    std::vector<std::string> lines;
    size_t cursor = 0;
    size_t textLength = text.length();

    // --- Dynamic Text Wrapping & Splitting ---
    while (cursor < textLength && lines.size() < static_cast<size_t>(maxLinesPossible)) {
        // Handle explicit newlines
        if (text[cursor] == '\n') {
            lines.emplace_back("");
            cursor++;
            continue;
        }

        size_t remainingChars = textLength - cursor;
        size_t takeChars = (remainingChars > static_cast<size_t>(maxCharsPerLine)) ? maxCharsPerLine : remainingChars;

        if (wrapWords && remainingChars > static_cast<size_t>(maxCharsPerLine)) {
            size_t lastSpace = std::string::npos;
            for (size_t i = 0; i < takeChars; i++) {
                if (text[cursor + i] == ' ') lastSpace = i;
            }

            // Split at space if found; otherwise hard-wrap the long word
            if (lastSpace != std::string::npos && lastSpace > 0) {
                takeChars = lastSpace;
            }
        }

        bool isLastVisibleLine = (lines.size() == static_cast<size_t>(maxLinesPossible - 1));
        bool textOverflows = (textLength - cursor > takeChars);

        if (isLastVisibleLine && textOverflows) {
            // Truncate and append ellipsis
            if (takeChars > 3) {
                lines.push_back(text.substr(cursor, takeChars - 3) + "...");
            } else {
                lines.push_back(std::string("...").substr(0, takeChars));
            }
            break; // Textbox filled up completely
        } else {
            lines.push_back(text.substr(cursor, takeChars));
            cursor += takeChars;
            
            // Advance past trailing whitespace if word wrapping hit a boundary
            if (wrapWords && cursor < textLength && text[cursor] == ' ') {
                cursor++;
            }
        }
    }

    // --- Vertical Alignment Execution ---
    float totalTextHeight = lines.size() * lineHeight;
    float startY = box.y;

    if (vAlign == VAlign::Middle) {
        startY = box.y + (box.height - totalTextHeight) / 2.0f;
    } else if (vAlign == VAlign::Bottom) {
        startY = box.y + box.height - totalTextHeight;
    }

    // --- Rendering ---
    for (size_t i = 0; i < lines.size(); i++) {
        float currentY = startY + (i * lineHeight);
        // Safety bounds check
        if (currentY >= box.y && (currentY + lineHeight) <= (box.y + box.height)) {
            DrawLineAligned(lines[i], box, currentY, hAlign, fontSize, spacing, color);
        }
    }
}

void MenuElement::init() {
    
}

void MenuElement::deinit() {
   
}

void MenuElement::update() {
    
}

void MenuElement::render() {
   
}

void ClickableObject::render() {
    if(positions.size() < 2);
    else if(positions.size() < 3){
        DrawRectangleV(ScaleCords(positions[0]), ScaleCords(positions[1]) - ScaleCords(positions[0]), baseColor);
        textRect = Rectangle{positions[0].x + textSpacing, positions[0].y + textSpacing, positions[1].x - positions[0].x - 2.0f*textSpacing, positions[1].y - positions[0].y - 2.0f*textSpacing};
    }
    else{
        std::vector<Vector2> temp = positions;
        for(int i = 0; i < temp.size(); i++){
            temp[i] = ScaleCords(temp[i]);
        }
        DrawTriangleStrip(temp.data(), positions.size(), baseColor);
        
    }
    if(text != ""){
        DrawTextBox(text, ScaleRect(textRect), HAlign::Left, VAlign::Middle, Scale(textSize), Scale(1), textColor, false);
    }
}

void ClickableObject::update() {
    
}

void ClickableObject::init() {

}

void ClickableObject::deinit() {
    positions.clear();
    positions.shrink_to_fit(); 
}


void FancyScrollingList::init() {
    numberOfObjects = (int)((positions[1].y - positions[0].y) / objectDistance) + 5;
    std::cout << "nubmer of objects " << numberOfObjects << std::endl;
    objects.reserve(numberOfObjects);
    for(int i = 0; i < numberOfObjects; i++){
        objects.push_back(std::make_unique<ClickableObject>());
        objects[i].get()->positions.push_back(positions[0] + (Vector2){0, objectFreeSpace});
        objects[i].get()->positions.push_back(positions[0] + (Vector2){positions[1].x - positions[0].x, objectDistance - objectFreeSpace});
        objects[i].get()->text = "Object: " + std::to_string(i);
        objects[i].get()->baseColor = baseColor;
    }
}

void FancyScrollingList::deinit() {
    for(int i = 0; i < objects.size(); i++){
        objects[i]->deinit();
        objects[i].reset();
    }
    objects.clear();
    objects.shrink_to_fit();  
    positions.clear();
    positions.shrink_to_fit(); 
}

void FancyScrollingList::update() {
    objectOffsetFull += (int)trunc((objectOffset / objectDistance));


    
    bool updateTexts = false;
    if((int)trunc((objectOffset / objectDistance)) != 0){
        updateTexts = true;
    }






    objectOffset -= trunc((objectOffset / objectDistance)) * objectDistance;
    for(int i = 0; i < objects.size(); i++){
        objects[i].get()->positions[0] = positions[0] + (Vector2){0, objectDistance * (i - 2) + objectFreeSpace + objectOffset};
        objects[i].get()->positions[1] = positions[0] + (Vector2){positions[1].x - positions[0].x, objectDistance * (i - 1) - objectFreeSpace + objectOffset};
        if(objects[i].get()->positions[0].y + objects[i].get()->positions[1].y < positions[0].y){
            objects[i].get()->baseColor = Fade(baseColor, clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y + objects[i].get()->positions[1].y)) / (objectDistance * 1.5)), 0.0f, 1.0f));
            objects[i].get()->textColor = Fade(textColor, clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y + objects[i].get()->positions[1].y)) / (objectDistance * 1.5)), 0.0f, 1.0f));
        }
        else if(objects[i].get()->positions[0].y > positions[1].y){
            objects[i].get()->baseColor = Fade(baseColor, clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) / (objectDistance * 1.5)), 0.0f, 1.0f));
            objects[i].get()->textColor = Fade(textColor, clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) / (objectDistance * 1.5)), 0.0f, 1.0f));
        }
        else{
            objects[i].get()->baseColor = baseColor;
            objects[i].get()->textColor = textColor;
        }

        double distanceFromCenter = (objects[i].get()->positions[0].y + objectDistance / 2) - (positions[0].y + ((positions[1].y - positions[0].y) / 2.0f));
        objects[i].get()->positions[0].x += (distanceFromCenter / 50.0) * (distanceFromCenter / 50.0);
        if(updateTexts)
            objects[i].get()->text = "Object: " + std::to_string(i - objectOffsetFull + hardCodedOffset);
    }
}

void FancyScrollingList::render() {
   for(int i = 0; i < objects.size(); i++){
//if (i - objectOffsetFull + hardCodedOffset >= 0)
            objects[i]->render();
    }
}