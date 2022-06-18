#include <fastrender.hpp>

void initMouseTrail(){
    while(MouseTrail.size() < 150){
        MouseTrail.push_back({-10,-10});
    }
}

void updateMouseTrail(){
    FrameTimeCounterTrail += GetFrameTime()*1000.0f;
    while(FrameTimeCounterTrail > 1.0f){
        FrameTimeCounterTrail -= 1.0f;
        indexTrail++;
        indexTrail = indexTrail % Global.MouseTrailSize;
        MouseTrail[indexTrail] = Global.MousePosition;
    }
}


void renderMouse(){
    Vector2 LastPosition = {-10,-10};
    if(Global.MouseInFocus){
        DrawCircleV(ScaleCords(Global.MousePosition), Scale(3.0f), GREEN);
        for(int i = Global.MouseTrailSize - 1; i >= 0; i--){
            if(MouseTrail[(indexTrail+i) % Global.MouseTrailSize].x != LastPosition.x or MouseTrail[(indexTrail+i) % Global.MouseTrailSize].y != LastPosition.y ){ 
                LastPosition = MouseTrail[(indexTrail+i) % Global.MouseTrailSize];
                Color MouseTrailer = {0,255,0,(float)i*(255.f/(float)Global.MouseTrailSize)};
                DrawCircleV(ScaleCords(MouseTrail[(indexTrail+i) % Global.MouseTrailSize]), Scale(3.0f), MouseTrailer);
            }
        }
    }
}