#pragma once
#include "raylib.h"

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