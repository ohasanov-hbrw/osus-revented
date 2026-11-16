#include "menuElements.hpp"
#include "globals.hpp"
#include "raylib.h"
#include "utils.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include "fastrender.hpp"

void MenuElement::init() {}

void MenuElement::deinit() {}

void MenuElement::update() {}

void MenuElement::render() {}

// void ClickableObject::render() {
//   if (positions.size() < 2)
//     ;
//   else if (positions.size() < 3) {
//     DrawRectangleV(ScaleCords(positions[0]),
//                    ScaleCords(positions[1]) - ScaleCords(positions[0]),
//                    baseColor);
//
//   } else {
//     std::vector<Vector2> temp = positions;
//     for (int i = 0; i < temp.size(); i++) {
//       temp[i] = ScaleCords(temp[i]);
//     }
//     DrawTriangleStrip(temp.data(), positions.size(), baseColor);
//   }
//   if (text != "") {
//     internalBox.Draw(horizontalAlign, verticalAlign, textColor);
//   }
// }

void ClickableObject::render() {
  if (positions.size() < 2)
    return;

  // Scale vertices to screen coordinates
  std::vector<Vector2> scaled;
  scaled.reserve(positions.size());
  for (const auto &p : positions) {
    scaled.push_back(ScaleCords(p));
  }

  // Determine fill colour based on state
  Color fillColor = baseColor;
  if ((clicked || (Global.Key1D && focused && !focusbreak))) {
    // Pressed state – darken
    fillColor = ColorBrightness(baseColor, -0.2f);
  } else if (focused) {
    // Hover state – brighten
    fillColor = ColorBrightness(baseColor, 0.2f);
  }

  // Draw the shape
  if (positions.size() == 2) {
    // Rectangle: top-left = scaled[0], bottom-right = scaled[1]
    Rectangle rect = {scaled[0].x, scaled[0].y, scaled[1].x - scaled[0].x,
                      scaled[1].y - scaled[0].y};
    DrawRectangleRec(rect, fillColor);
  } else {
    // Triangle strip
    DrawTriangleStrip(scaled.data(), scaled.size(), fillColor);
    if (focused) {

      if (boundary.size() > 1) {
        for (size_t i = 0; i < boundary.size(); ++i) {
          size_t next = (i + 1) % boundary.size();
          DrawLineEx(ScaleCords(boundary[i]), ScaleCords(boundary[next]),
                     Scale(2.01f), WHITE);
          DrawCircle(ScaleCordX(boundary[i].x), ScaleCordY(boundary[i].y),
                     Scale(1.01f), WHITE);
        }
      }
    }
  }

  // Draw text if present – centered on the shape's bounding rectangle
  if (!text.empty()) {
    // Rectangle rect = GetBoundingRect(positions);
    // internalBox.SetBox(rect);
    //  Use the alignment you prefer (here we force center)
    internalBox.Draw(horizontalAlign, verticalAlign, textColor);
  }
}

void ClickableObject::update() {
  if (staticobject)
    return;
  bool hover = false;
  Vector2 mouse = Global.MousePosition; // screen coordinates

  // 2. Collision test based on vertex count
  if (positions.size() >= 3) {
    // Triangle strip: test each triangle (i, i+1, i+2)
    for (size_t i = 0; i < positions.size() - 2; ++i) {
      if (IsPointInTriangle(mouse, positions[i], positions[i + 1],
                            positions[i + 2])) {
        hover = true;
        break;
      }
    }
  } else if (positions.size() == 2) {
    // Rectangle: top‑left = scaled[0], bottom‑right = scaled[1]
    Rectangle rect = {positions[0].x, positions[0].y,
                      positions[1].x - positions[0].x,
                      positions[1].y - positions[0].y};
    hover = CheckCollisionPointRec(mouse, rect);
  }
  // else: no collision

  // 3. Button state logic (same as before)
  bool click = Global.MouseInFocus && Global.Key1P;

  if (hover && click) {
    focused = true;
    clicked = true;
    focusbreak = false;
  } else if (hover) {
    focused = true;
    clicked = false;
  } else {
    focused = false;
    clicked = false;
    focusbreak = true;
  }

  // Trigger action on release while focused
  if (hover && !focusbreak && Global.Key1R)
    action = true;
  else
    action = false;
}

void ClickableObject::init() { boundary = GetTriangleStripBoundary(positions); }

void ClickableObject::deinit() {
  positions.clear();
  boundary.clear();
  positions.shrink_to_fit();
  boundary.shrink_to_fit();
}

bool initFunctionRan = false;

void FancyScrollingList::init() {
  numberOfObjects =
      (int)((positions[1].y - positions[0].y) / objectDistance) + 5;
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "nubmer of objects "
            << numberOfObjects << std::endl;
  objects.reserve(numberOfObjects);
  for (int i = 0; i < numberOfObjects; i++) {
    objects.push_back(std::make_unique<ClickableObject>());
    objects[i].get()->positions.push_back(positions[0] +
                                          (Vector2){0, objectFreeSpace});
    objects[i].get()->positions.push_back(
        positions[0] + (Vector2){positions[1].x - positions[0].x,
                                 objectDistance - objectFreeSpace});
    objects[i].get()->text = objectNames.size() > 0
                                 ? objectNames[i % objectNames.size()]
                                 : "Error, object: " + std::to_string(i);
    objects[i].get()->internalBox.SetText(objects[i].get()->text);
    objects[i].get()->internalBox.SetFontSize(textSize);
    objects[i].get()->internalBox.SetSpacing(2);
    objects[i].get()->internalBox.SetWrapWords(false);
    objects[i].get()->textRect = Rectangle{
        objects[i].get()->positions[0].x + textSpacing,
        objects[i].get()->positions[0].y + textSpacing,
        (objects[i].get()->positions[1].x - objects[i].get()->positions[0].x) -
            2.0f * textSpacing,
        objects[i].get()->positions[1].y - objects[i].get()->positions[0].y -
            2.0f * textSpacing};
    objects[i].get()->internalBox.SetBox(objects[i].get()->textRect);
    objects[i].get()->baseColor = baseColor;
    objects[i].get()->textColor = textColor;
    objects[i].get()->staticobject = false;
  }
  updateTextBox = true;
  objectOffsetFull = 0;
  graphicalObjectOffsetFull = 0;
  currentSelection = -objectOffsetFull;
  objectOffset = 0;
  updateTexts = true;
  if (!Global.ScaleUpdated) {
    initFunctionRan = true;
    // std::cout << "calling initial update for list\n";
    update();
    initFunctionRan = false;
    // std::cout << "called initial update for list\n";
  }
  updateTexts = false;
}

void FancyScrollingList::reinit() {
  for (int i = 0; i < objects.size(); i++) {
    objects[i]->deinit();
    objects[i].reset();
  }
  objects.clear();
  objects.shrink_to_fit();
  numberOfObjects =
      (int)((positions[1].y - positions[0].y) / objectDistance) + 5;
  std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "nubmer of objects "
            << numberOfObjects << std::endl;
  objects.reserve(numberOfObjects);
  for (int i = 0; i < numberOfObjects; i++) {
    // std::cout << "adding obnject " << i << std::endl;
    objects.push_back(std::make_unique<ClickableObject>());
    objects[i].get()->positions.push_back(positions[0] +
                                          (Vector2){0, objectFreeSpace});
    objects[i].get()->positions.push_back(
        positions[0] + (Vector2){positions[1].x - positions[0].x,
                                 objectDistance - objectFreeSpace});
    objects[i].get()->text = objectNames.size() > 0
                                 ? objectNames[i % objectNames.size()]
                                 : "Error, object: " + std::to_string(i);
    objects[i].get()->internalBox.SetText(objects[i].get()->text);
    objects[i].get()->internalBox.SetFontSize(textSize);
    objects[i].get()->internalBox.SetSpacing(2);
    objects[i].get()->internalBox.SetWrapWords(false);
    objects[i].get()->textRect = Rectangle{
        objects[i].get()->positions[0].x + textSpacing,
        objects[i].get()->positions[0].y + textSpacing,
        (objects[i].get()->positions[1].x - objects[i].get()->positions[0].x) -
            2.0f * textSpacing,
        objects[i].get()->positions[1].y - objects[i].get()->positions[0].y -
            2.0f * textSpacing};
    objects[i].get()->internalBox.SetBox(objects[i].get()->textRect);
    objects[i].get()->baseColor = baseColor;
    objects[i].get()->textColor = textColor;
    objects[i].get()->staticobject = false;
  }
  updateTextBox = true;
  objectOffsetFull = 0;
  graphicalObjectOffsetFull = 0;
  currentSelection = -objectOffsetFull;
  objectOffset = 0;
  updateTexts = true;
  if (!Global.ScaleUpdated) {
    initFunctionRan = true;
    // std::cout << "calling initial update for list\n";
    update();
    initFunctionRan = false;
    // std::cout << "called initial update for list\n";
  }
  updateTexts = false;
}

void FancyScrollingList::deinit() {
  for (int i = 0; i < objects.size(); i++) {
    objects[i]->deinit();
    objects[i].reset();
  }
  objects.clear();
  objects.shrink_to_fit();
  positions.clear();
  positions.shrink_to_fit();
}

void FancyScrollingList::update() {
  objectOffsetFull += frameChange;

  if (objectOffsetFull > 0) {
    objectOffsetFull = 0;
  }

  if (objectOffsetFull <
      0 - std::max(1, (int)objectNames.size()) + 1) { // + 1 critical!
    objectOffsetFull = 0 - std::max(1, (int)objectNames.size()) + 1;
  }
  frameChange = 0;
  if (Global.ScaleUpdated) {
    for (int i = 0; i < objects.size(); i++) {
      objects[i]->deinit();
      objects[i].reset();
    }
    objects.clear();
    objects.shrink_to_fit();
    init();
    initFunctionRan = true;
    updateTextBox = true;
    updateTexts = true;
  }

  // Mouse interaction. The drag/inertia model is the same one used by the
  // WIP menus: mouse delta drives an acceleration value which is damped every
  // frame, with a snap-towards-the-nearest-item term once it has settled.
  auto logicalIndexAtY = [&](float y) -> int {
    for (int i = 0; i < (int)objects.size(); i++) {
      int logical = i - (int)objects.size() / 2 - graphicalObjectOffsetFull;
      if (logical < 0 || logical >= (int)objectNames.size())
        continue;
      if (y >= objects[i]->positions[0].y && y <= objects[i]->positions[1].y)
        return logical;
    }
    return -1;
  };

  // The centred card is the one whose middle is closest to the middle of the
  // list. Deriving it from the offset signs is error prone, so use geometry.
  auto logicalIndexNearestCenter = [&]() -> int {
    float listCenterY =
        positions[0].y + (positions[1].y - positions[0].y) / 2.0f;
    int best = -1;
    float bestDistance = std::numeric_limits<float>::max();
    for (int i = 0; i < (int)objects.size(); i++) {
      int logical = i - (int)objects.size() / 2 - graphicalObjectOffsetFull;
      if (logical < 0 || logical >= (int)objectNames.size())
        continue;
      float cardCenterY =
          (objects[i]->positions[0].y + objects[i]->positions[1].y) / 2.0f;
      float distance = std::abs(cardCenterY - listCenterY);
      if (distance < bestDistance) {
        bestDistance = distance;
        best = logical;
      }
    }
    return best;
  };

  bool inList = false;
  if (positions.size() >= 2 && Global.MouseInFocus && Global.enableMouse) {
    Rectangle area = {positions[0].x, positions[0].y,
                      positions[1].x - positions[0].x,
                      positions[1].y - positions[0].y};
    if (mouseInputRect.width > 0 && mouseInputRect.height > 0)
      area = mouseInputRect;
    inList = CheckCollisionPointRec(Global.MousePosition, area);
  }

  if (Global.Key1P && inList) {
    mousePressedOnList = true;
    mouseDragging = false;
    centerItemActivated = false;
    mousePressStartY = Global.MousePosition.y;
    mouseLastY = Global.MousePosition.y;
    mouseTotalDragDistance = 0.0f;
    dragVelocity = 0.0;
    pressedLogicalIndex = logicalIndexAtY(Global.MousePosition.y);
  }

  if (Global.Key1D && mousePressedOnList && Global.MouseInFocus) {
    float deltaY = Global.MousePosition.y - mouseLastY;
    mouseTotalDragDistance += std::abs(deltaY);
    if (mouseTotalDragDistance > 1.0f)
      mouseDragging = true;
    if (mouseDragging)
      dragVelocity = deltaY;
  }

  // Leaving the window counts as letting go, so a drag can't get stuck.
  if (mousePressedOnList && (Global.Key1R || !Global.MouseInFocus)) {
    if (Global.MouseInFocus) {
      float deltaY = Global.MousePosition.y - mouseLastY;
      mouseTotalDragDistance += std::abs(deltaY);
      if (mouseDragging || mouseTotalDragDistance > 1.0f) {
        dragVelocity += deltaY; // carry the last bit of movement as a flick
      } else if (pressedLogicalIndex >= 0) {
        if (pressedLogicalIndex == logicalIndexNearestCenter()) {
          centerItemActivated = true;
        } else {
          // Clicking another card scrolls it to the centre.
          objectOffsetFull = -pressedLogicalIndex;
          objectOffset = 0;
          dragVelocity = 0;
        }
      }
    }
    mousePressedOnList = false;
    mouseDragging = false;
  }

  // Keep the last Y fresh even outside the scroller so re-entry can't produce
  // one huge bogus delta.
  if (Global.MouseInFocus)
    mouseLastY = Global.MousePosition.y;

  objectOffset += dragVelocity;

  // Snap towards the nearest item when the list is (almost) at rest.
  if (!mouseDragging && !(std::abs(dragVelocity) >= 0.05)) {
    double clampingPosition = objectOffset;
    while (clampingPosition < 0.0)
      clampingPosition += objectDistance;
    while (clampingPosition >= objectDistance)
      clampingPosition -= objectDistance;
    double snapAccel = 0.0;
    if (clampingPosition >= objectDistance / 2.0) {
      snapAccel = (Global.FrameTime / 1000.0) *
                  (10.0 * (objectDistance - clampingPosition));
      if (clampingPosition + snapAccel >= objectDistance)
        snapAccel = objectDistance - clampingPosition;
    } else {
      snapAccel = -(Global.FrameTime / 1000.0) * (10.0 * clampingPosition);
      if (clampingPosition + snapAccel <= 0.0)
        snapAccel = -clampingPosition;
    }
    objectOffset += snapAccel;
  }

  dragVelocity +=
      ((-dragVelocity) / 2.0f) * ((Global.FrameTime / 1000.0f) * 8.0f);

  objectOffsetFull += (int)trunc((objectOffset / objectDistance));

  // bool updateTexts = false;
  if ((int)trunc((objectOffset / objectDistance)) != 0) {
    updateTexts = true;
  }

  objectOffset -= trunc((objectOffset / objectDistance)) * objectDistance;

  // Stop the scroll momentum at either end of the list.
  {
    int minLogicalOffset = -std::max(1, (int)objectNames.size()) + 1;
    if (objectOffsetFull > 0)
      objectOffsetFull = 0;
    if (objectOffsetFull < minLogicalOffset)
      objectOffsetFull = minLogicalOffset;
    if ((objectOffsetFull >= 0 && dragVelocity > 0) ||
        (objectOffsetFull <= minLogicalOffset && dragVelocity < 0)) {
      dragVelocity = 0;
      objectOffset = 0;
    }
  }

  double distance =
      (objectOffsetFull - graphicalObjectOffsetFull) * objectDistance +
      (objectOffset - graphicalObjectOffset);
  updateTextBox = true;
  if (!(std::abs(distance) < 0.5 && !initFunctionRan)) {
    // Define the distance at which full speed is reached
    double maxDistance =
        objectDistance * 8.0; // Increased to handle fast scrolls better

    // Normalize distance between -1.0 and 1.0
    double t = std::max(-1.0, std::min(1.0, distance / maxDistance));
    double absT = std::abs(t);

    // Quadratic Ease In Out (faster response than Cubic)
    double easeFactor = (absT < 0.5)
                            ? (2.0 * absT * absT)
                            : (1.0 - std::pow(-2.0 * absT + 2.0, 2.0) / 2.0);

    double sign = (distance > 0) ? 1.0 : -1.0;
    double baseSpeed = 250.0f; // Increased base speed to eliminate lag

    // Calculate step, ensuring a minimum speed so it doesn't crawl at the end
    double minSpeed = 300;
    double step = easeFactor * baseSpeed * objectDistance;

    if (step < minSpeed) {
      step = minSpeed; // Prevents the infinite slow crawl at the end
    }
    step *= (Global.FrameTime / 1000.f);
    graphicalObjectOffset += step * sign;
  }

  if (std::abs(distance) < 5 && !initFunctionRan)
    updateTextBox = false;

  if (initFunctionRan)
    initFunctionRan = false;
  int change = (int)trunc((graphicalObjectOffset / (0.5 * objectDistance)));
  graphicalObjectOffsetFull += change;

  if (change != 0) {
    updateTexts = true;
    updateTextBox = true;
  }

  graphicalObjectOffset -= change * objectDistance;

  float centeringOffset = objectDistance / 2.;

  currentSelection = -objectOffsetFull;

  for (int i = 0; i < objects.size(); i++) {
    objects[i].get()->positions[0] =
        positions[0] - (Vector2){0, (positions[0].y - positions[1].y) / 2} +
        (Vector2){0, objectDistance * (float)(i - 1 - (int)objects.size() / 2) +
                         objectFreeSpace + graphicalObjectOffset} +
        (Vector2){0, centeringOffset};
    objects[i].get()->positions[1] =
        positions[0] - (Vector2){0, (positions[0].y - positions[1].y) / 2} +
        (Vector2){positions[1].x - positions[0].x,
                  objectDistance * (float)(i - 0 - (int)objects.size() / 2) -
                      objectFreeSpace + graphicalObjectOffset} +
        (Vector2){0, centeringOffset};
    if (objects[i].get()->positions[0].y + objectFreeSpace < positions[0].y) {
      objects[i].get()->baseColor =
          Fade(baseColor,
               clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y +
                                            objectFreeSpace)) /
                         (objectDistance * 1.5)),
                    0.0f, 1.0f));
      objects[i].get()->textColor =
          Fade(textColor,
               clip(1 - ((positions[0].y - (objects[i].get()->positions[0].y +
                                            objectFreeSpace)) /
                         (objectDistance * 1.5)),
                    0.0f, 1.0f));
    } else if (objects[i].get()->positions[0].y > positions[1].y) {
      objects[i].get()->baseColor =
          Fade(baseColor,
               clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) /
                         (objectDistance * 1.5)),
                    0.0f, 1.0f));
      objects[i].get()->textColor =
          Fade(textColor,
               clip(1 - ((objects[i].get()->positions[0].y - positions[1].y) /
                         (objectDistance * 1.5)),
                    0.0f, 1.0f));
    } else {
      objects[i].get()->baseColor = baseColor;
      objects[i].get()->textColor = textColor;
    }

    double distanceFromCenter =
        (objects[i].get()->positions[0].y + objectDistance / 2) -
        (positions[0].y + ((positions[1].y - positions[0].y) / 2.0f));
    objects[i].get()->positions[0].x +=
        (distanceFromCenter / 50.0) * (distanceFromCenter / 50.0);
    if (updateTexts) {
      objects[i].get()->text =
          objectNames.size() > 0
              ? 
              //"ID: " +
              //      std::to_string(std::max(0, i - (int)objects.size() / 2- graphicalObjectOffsetFull) %
              //                     objectNames.size()) + " " +
                    objectNames[std::max(0, i - (int)objects.size() / 2- graphicalObjectOffsetFull) %
                                objectNames.size()]
              : "Error, object: " + std::to_string(i - (int)objects.size() / 2 -
                                                   graphicalObjectOffsetFull);
      objects[i].get()->internalBox.SetText(objects[i].get()->text);
    } 
    // The card moves every frame, so its text box has to follow every frame.
    // SetBox only re-lays out when the size drifts by 5 units, so this is cheap.
    objects[i].get()->textRect =
        Rectangle{objects[i].get()->positions[0].x + textSpacing,
                  objects[i].get()->positions[0].y + textSpacing,
                  (objects[i].get()->positions[1].x -
                   objects[i].get()->positions[0].x) -
                      2.0f * textSpacing,
                  objects[i].get()->positions[1].y -
                      objects[i].get()->positions[0].y - 2.0f * textSpacing};
    objects[i].get()->internalBox.SetBox(objects[i].get()->textRect);
    // Cards outside the list's input area must not hover or react to clicks.
    if (inList) {
      objects[i].get()->update();
    } else {
      objects[i].get()->focused = false;
      objects[i].get()->clicked = false;
      objects[i].get()->focusbreak = true;
    }
    // Clicking a card is handled by the list itself (scroll-to-centre or
    // centre activation); don't let the card's own action leak into the menu.
    objects[i].get()->action = false;
  }
  if (updateTexts)
    updateTexts = false;
  if (updateTextBox)
    updateTextBox = false;
}

void FancyScrollingList::render() {
  //DrawTextEx(&Global.DefaultFont, TextFormat("OffsetFull: %d Graphical: %.0f", graphicalObjectOffsetFull, graphicalObjectOffset),
  //{static_cast<float>((int)Scale(5)), static_cast<float>((int)Scale(400))},
  //Scale(20.05), Scale(2), BLUE);
  for (int i = 0; i < objects.size(); i++) {
    // if (i - objectOffsetFull + hardCodedOffset >= 0)
    if (i - (int)objects.size() / 2 - graphicalObjectOffsetFull >= 0 &&
        i - (int)objects.size() / 2 - graphicalObjectOffsetFull <
            std::max(1, (int)objectNames.size()) &&
        objects[i]->positions[1].y > positions[0].y &&
        objects[i]->positions[0].y < positions[1].y)
      objects[i]->render();
  }
}



void CircularSliderElement::init() {
  sliderValue = 0.5;
  textColor = WHITE;
  horizontalAlign = HAlign::Center;
  verticalAlign = VAlign::Middle;
  internalBox.SetText("Master\nVolume");
  internalBox.SetFontSize(20);
  internalBox.SetSpacing(2.005);
  //internalBox.SetBox();
}

void CircularSliderElement::deinit() {
  internalBox.Unload();
}

void CircularSliderElement::update() {
  if(positions.empty())
    return;
  focused = CheckCollisionPointCircle(Global.MousePosition, positions[0], radius);
  if(renderObject && focused){
    sliderValue += 0.05 * Global.Wheel;
    sliderValue = clip(sliderValue, 0.0, 1.0);
    Global.Wheel = 0;
  }
  
}

void CircularSliderElement::render() {
  if(!positions.empty() && renderObject){
    float startAngle = 150.f;
    float endAngle = startAngle + 240. * sliderValue;
    DrawCircleV(ScaleCords(positions[0]), Scale(radius), baseColor);
    DrawThickCircleLines(ScaleCords(positions[0]), Scale(radius - 2.5), startAngle, endAngle, std::max(3, (int)(30 * sliderValue)), Scale(10) ,focused ? Color{sliderColor.r * 1.5, sliderColor.g * 1.5, sliderColor.b * 1.5, sliderColor.a} : sliderColor);
    internalBox.Draw(horizontalAlign, verticalAlign, textColor);
  }
}