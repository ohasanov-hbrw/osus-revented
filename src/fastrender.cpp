#include <fastrender.hpp>
//This file includes functions for rendering the mouse

//Mouse Trails are disabled right now, I don't really see people using them anyway...

// Just add some empty points to the mouse trail array
void initMouseTrail(){
    while(MouseTrail.size() < 150){
        MouseTrail.push_back({-10,-10});
    }
}

// "Queue" stuff while using a vector? Why not?
void updateMouseTrail(){
    FrameTimeCounterTrail += Global.FrameTime * 1.5;
    while(FrameTimeCounterTrail > 1.0f){
        FrameTimeCounterTrail -= 1.0f;
        indexTrail++;
        indexTrail = indexTrail % Global.MouseTrailSize;
        MouseTrail[indexTrail] = Global.MousePosition;
    }
}

// Render the custom cursor
void renderMouse(){
    if(Global.MousePosition.x == 0.0f && Global.MousePosition.y == 0.0f)
        return;
    DrawTextureCenter(&Global.cursor, Global.MousePosition.x, Global.MousePosition.y , 0.3f /*1.0f???*/, {255,255,255,255});
    Vector2 LastPosition = {-10,-10};
    for(int i = 0; i <= Global.MouseTrailSize - 1; i++){
        if(MouseTrail[(indexTrail+i) % Global.MouseTrailSize].x != LastPosition.x or MouseTrail[(indexTrail+i) % Global.MouseTrailSize].y != LastPosition.y ){ 
            LastPosition = MouseTrail[(indexTrail+i) % Global.MouseTrailSize];
            //Color MouseTrailer = {255,255,255,(unsigned char)((float)i*(255.f/(float)Global.MouseTrailSize))};
            //DrawCircleV(ScaleCords(MouseTrail[(indexTrail+i) % Global.MouseTrailSize]), Scale(3.0f), MouseTrailer);
            //DrawTextureCenter(Global.cursorTrail, MouseTrail[(indexTrail+i) % Global.MouseTrailSize].x, MouseTrail[(indexTrail+i) % Global.MouseTrailSize].y , 0.3f, MouseTrailer);
        }
    }
    
}


void DrawLoadingCircle(Vector2 center, float radius, float thickness, double time, Color color){
    time /= 1000.;
    double baseRotation = time *300.f; 
    double arcLength = 150.0f + sinf(time * 4.0f) * 120.0f;
    double dynamicOffset = time * 100.0f + sinf(time * 2.0f) * 50.0f;
    
    double startAngle = baseRotation + dynamicOffset;

    #define ARC_SEGMENTS 30
    #define VERTEX_COUNT (ARC_SEGMENTS * 2)
    Vector2 points[VERTEX_COUNT];

    double innerRadius = radius - thickness;
    double outerRadius = radius;

    for (int i = 0; i < ARC_SEGMENTS; i++)
    {
        double progress = (double)i / (ARC_SEGMENTS - 1);
        double angleRad = (startAngle + (progress * arcLength)) * DEG2RAD;

        double cosA = cosf(angleRad);
        double sinA = sinf(angleRad);

        // Vertex 2i: Outer point
        points[2 * i].x = center.x + cosA * outerRadius;
        points[2 * i].y = center.y + sinA * outerRadius;

        // Vertex 2i + 1: Inner point
        points[2 * i + 1].x = center.x + cosA * innerRadius;
        points[2 * i + 1].y = center.y + sinA * innerRadius;
    }
    DrawTriangleStrip(points, VERTEX_COUNT, color);
}