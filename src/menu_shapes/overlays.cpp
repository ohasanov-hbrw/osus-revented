#include <vector>
#include "raylib.h"
#include "menu_shapes/shapes.hpp"
#include "globals.hpp"



//UI cannot be not hardcoded...


std::vector<Vector2> Create_Shape_Rectangle_With_Trapezoid(enum SHAPES_POSITION_ANCHORS anchor, bool fillX, bool fillY, float width, float height, float x, float y, float edgeHeight, float edgeWidth, float edgeRampWidth){
    std::vector<Vector2> points;
    float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
    float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
    float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
    float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

    switch (anchor) {
        case BOTTOM_RIGHT:
            points.push_back(Vector2{leftMostX, bottomMostY - height});
            points.push_back(Vector2{leftMostX, bottomMostY});
            points.push_back(Vector2{rightMostX - edgeWidth - edgeRampWidth, bottomMostY - height});
            points.push_back(Vector2{rightMostX - edgeWidth - edgeRampWidth, bottomMostY});
            points.push_back(Vector2{rightMostX - edgeWidth, bottomMostY - edgeHeight});
            points.push_back(Vector2{rightMostX - edgeWidth, bottomMostY});
            points.push_back(Vector2{rightMostX, bottomMostY - edgeHeight});
            points.push_back(Vector2{rightMostX, bottomMostY});
            break;
        case BOTTOM_LEFT:
            points.push_back(Vector2{leftMostX, bottomMostY - edgeHeight});
            points.push_back(Vector2{leftMostX, bottomMostY});
            points.push_back(Vector2{leftMostX + edgeWidth, bottomMostY - edgeHeight});
            points.push_back(Vector2{leftMostX + edgeWidth, bottomMostY});
            points.push_back(Vector2{leftMostX + edgeWidth + edgeRampWidth, bottomMostY - height});
            points.push_back(Vector2{leftMostX + edgeWidth + edgeRampWidth, bottomMostY});
            points.push_back(Vector2{rightMostX, bottomMostY - height});
            points.push_back(Vector2{rightMostX, bottomMostY});
            break;
        default:
            points.push_back(Vector2{leftMostX, topMostY});
            points.push_back(Vector2{leftMostX, topMostY + edgeHeight});
            points.push_back(Vector2{leftMostX + edgeWidth, topMostY});
            points.push_back(Vector2{leftMostX + edgeWidth, topMostY + edgeHeight});
            points.push_back(Vector2{leftMostX + edgeWidth + edgeRampWidth, topMostY});
            points.push_back(Vector2{leftMostX + edgeWidth + edgeRampWidth, topMostY + height});
            points.push_back(Vector2{rightMostX, topMostY});
            points.push_back(Vector2{rightMostX, topMostY + height});
            break;
    }
    return points;
}

void Update_Shape_Rectangle_With_Trapezoid(enum SHAPES_POSITION_ANCHORS anchor, bool fillX, bool fillY, float width, float height, float x, float y, float edgeHeight, float edgeWidth, float edgeRampWidth, std::vector<Vector2>* points){
    if(points->size() != 8)
        return;
    float leftMostX = 0 - Global.ZeroPoint.x / Global.Scale;
    float rightMostX = 640 + Global.ZeroPoint.x / Global.Scale;
    float topMostY = 0 - Global.ZeroPoint.y / Global.Scale;
    float bottomMostY = 480 + Global.ZeroPoint.y / Global.Scale;

    switch (anchor) {
        case BOTTOM_RIGHT:
            (*points)[0] = (Vector2{leftMostX, bottomMostY - height});
            (*points)[1] = (Vector2{leftMostX, bottomMostY});
            (*points)[2] = (Vector2{rightMostX - edgeWidth - edgeRampWidth, bottomMostY - height});
            (*points)[3] = (Vector2{rightMostX - edgeWidth - edgeRampWidth, bottomMostY});
            (*points)[4] = (Vector2{rightMostX - edgeWidth, bottomMostY - edgeHeight});
            (*points)[5] = (Vector2{rightMostX - edgeWidth, bottomMostY});
            (*points)[6] = (Vector2{rightMostX, bottomMostY - edgeHeight});
            (*points)[7] = (Vector2{rightMostX, bottomMostY});
            break;
        case BOTTOM_LEFT:
            (*points)[0] = (Vector2{leftMostX, bottomMostY - edgeHeight});
            (*points)[1] = (Vector2{leftMostX, bottomMostY});
            (*points)[2] = (Vector2{leftMostX + edgeWidth, bottomMostY - edgeHeight});
            (*points)[3] = (Vector2{leftMostX + edgeWidth, bottomMostY});
            (*points)[4] = (Vector2{leftMostX + edgeWidth + edgeRampWidth, bottomMostY - height});
            (*points)[5] = (Vector2{leftMostX + edgeWidth + edgeRampWidth, bottomMostY});
            (*points)[6] = (Vector2{rightMostX, bottomMostY - height});
            (*points)[7] = (Vector2{rightMostX, bottomMostY});
            break;
        default:
            (*points)[0] = (Vector2{leftMostX, topMostY});
            (*points)[1] = (Vector2{leftMostX, topMostY + edgeHeight});
            (*points)[2] = (Vector2{leftMostX + edgeWidth, topMostY});
            (*points)[3] = (Vector2{leftMostX + edgeWidth, topMostY + edgeHeight});
            (*points)[4] = (Vector2{leftMostX + edgeWidth + edgeRampWidth, topMostY});
            (*points)[5] = (Vector2{leftMostX + edgeWidth + edgeRampWidth, topMostY + height});
            (*points)[6] = (Vector2{rightMostX, topMostY});
            (*points)[7] = (Vector2{rightMostX, topMostY + height});
            break;
    }
}