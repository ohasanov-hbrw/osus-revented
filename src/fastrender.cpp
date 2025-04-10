#include <fastrender.hpp>
//This file includes functions for rendering the mouse

//Mouse Trails are disabled right now, I don't really see people using them anyway...
void initMouseTrail(){
    while(MouseTrail.size() < 150){
        MouseTrail.push_back({-10,-10});
    }
}

void updateMouseTrail(){
    FrameTimeCounterTrail += Global.FrameTime * 1.5;
    while(FrameTimeCounterTrail > 1.0f){
        FrameTimeCounterTrail -= 1.0f;
        indexTrail++;
        indexTrail = indexTrail % Global.MouseTrailSize;
        MouseTrail[indexTrail] = Global.MousePosition;
    }
}


void renderMouse(){
    if(Global.MousePosition.x == 0.0f && Global.MousePosition.y == 0.0f)
        return;
    DrawTextureCenter(&Global.cursor, Global.MousePosition.x, Global.MousePosition.y , 0.3f /*1.0f???*/, {255,255,255,255});
    Vector2 LastPosition = {-10,-10};
    for(int i = 0; i <= Global.MouseTrailSize - 1; i++){
        if(MouseTrail[(indexTrail+i) % Global.MouseTrailSize].x != LastPosition.x or MouseTrail[(indexTrail+i) % Global.MouseTrailSize].y != LastPosition.y ){ 
            LastPosition = MouseTrail[(indexTrail+i) % Global.MouseTrailSize];
            Color MouseTrailer = {255,255,255,(unsigned char)((float)i*(255.f/(float)Global.MouseTrailSize))};
            //DrawCircleV(ScaleCords(MouseTrail[(indexTrail+i) % Global.MouseTrailSize]), Scale(3.0f), MouseTrailer);
            //DrawTextureCenter(Global.cursorTrail, MouseTrail[(indexTrail+i) % Global.MouseTrailSize].x, MouseTrail[(indexTrail+i) % Global.MouseTrailSize].y , 0.3f, MouseTrailer);
        }
    }
    
}