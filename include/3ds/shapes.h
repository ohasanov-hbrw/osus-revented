#pragma once
#include "raylibDefinitions.h"
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);

// Check if point is inside circle
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);

// Check if point is inside a triangle defined by three points (p1, p2, p3)
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);

// Check collision between two rectangles
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);

// Check collision between two circles
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);

// Check collision between circle and rectangle
// NOTE: Reviewed version to take into account corner limit case
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);

// Check the collision between two lines defined by two points each, returns collision point by reference
bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint);

// Check if point belongs to line created between two points [p1] and [p2] with defined margin in pixels [threshold]
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold);

// Get collision rectangle for two rectangles collision
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);