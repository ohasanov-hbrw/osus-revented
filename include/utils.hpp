#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <chrono>

#define RAYMATH_IMPLEMENTATION
# define M_PI 3.14159265358979323846f
//extern "C" {
    #include "raymath.h"
//}

//Update the variables needed for the scrolling
void updateUpDown();

//Get the scaling needed for the current window size
void GetScale();
//Get the absolute Mouse position
void GetMouse(); //No Tom, you will never be able to catch Jerry :'(
//Get the states of the main keys
void GetKeys();

//Scale a number based on the current scaling factor
float Scale(float);

//Scale a Rectangle based on the current scaling factor
Rectangle ScaleRect(Rectangle);
//Scale a Vector based on the current scaling factor and also add the origin offset
Vector2 ScaleCords(Vector2);

//Scale the X position and add the origin offset
float ScaleCordX(float);
//Scale the Y position and add the origin offset
float ScaleCordY(float);

//Get the center point of a rectangle
Vector2 GetCenter(Rectangle);
//Get the "origin" of a rectangle for raylib drawing functions and return a vector
Vector2 GetRaylibOrigin(Rectangle);
//Get the "origin" of a rectangle for raylib drawing functions and return a rectangle
Rectangle GetRaylibOriginR(Rectangle);

//Draw a centered and scaled Texture
void DrawTextureCenter(Texture2D, float, float, float, Color);
//Draw centered and scaled combo numbers
void DrawCNumbersCenter(int, float, float, float, Color);
//lmao
void DrawTextCenter(const char *, float, float, float, Color);
void DrawTextLeft(const char *, float, float, float, Color);
//Draw scaled combo numbers from left to right
void DrawCNumbersLeft(int, float, float, float, Color);
//Draw a centered, scaled and rotated Texture
void DrawTextureRotate(Texture2D, float, float, float, float, Color);
//max trigo
void DrawTextureOnCircle(Texture2D, float, float, float, float, float, float, Color);
//Draw a slider texture (exclusively for a slider object)
void DrawTextureSlider(Texture2D, float, float, Color, float);

void DrawSpinnerMeter(Texture2D, float);

void DrawSpinnerBack(Texture2D, Color);

//Clip a number between two other numbers
float clip( float, float, float);
//Interpolate more points in between two points
Vector2 lerp(Vector2, Vector2, float);
//Return a vector with both of the cordinates set to a float
Vector2 vectorize(float);
//Calculate the distance between two points
float distance(Vector2 &, Vector2 &);
//Binary search algorithm
int Search(std::vector<float>, float,int,int);

Vector2 getPointOnCircle(float, float, float, float);

std::string getSampleSetFromInt(int);

//Add extra operators because raylib sucks
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

float easeInOutCubic(float);
bool AreSame(double, double);

std::vector<std::string> ParseNameFolder(std::string);
std::vector<std::string> ParseNameFile(std::string);

void initTimer();
void pauseTimer();
void resumeTimer();
double getTimer();
void addOffsetTimer(unsigned long long int);
void updateTimer();

bool IsTextureReady(Texture2D);
bool IsRenderTextureReady(RenderTexture2D);