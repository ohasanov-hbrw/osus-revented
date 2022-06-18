#pragma once
#include "raylib.h"
#include <vector>
extern "C" {
#include "raymath.h"
}

void GetScale();
void GetMouse(); //No Tom, you will never be able to catch Jerry :'(
void GetKeys();

float Scale(float);

Rectangle ScaleRect(Rectangle);
Vector2 ScaleCords(Vector2);

float ScaleCordX(float);
float ScaleCordY(float);

Vector2 GetCenter(Rectangle);
Vector2 GetRaylibOrigin(Rectangle);
Rectangle GetRaylibOriginR(Rectangle);

float clip( float, float, float);
Vector2 lerp(Vector2, Vector2, float);
Vector2 vectorize(float);
float distance(Vector2 &, Vector2 &);
int Search(std::vector<float>, float,int,int);


inline Vector2 operator + (Vector2 p0, Vector2 p1){
    return Vector2Add(p0, p1);
}

inline Vector2 operator - (Vector2 p0, Vector2 p1){
    return Vector2Subtract(p0, p1);
}

inline Vector2 operator * (Vector2 p0, Vector2 p1){
    return Vector2Multiply(p0, p1);
}

inline Vector2 operator / (Vector2 p0, Vector2 p1){
    return Vector2Divide(p0, p1);
}