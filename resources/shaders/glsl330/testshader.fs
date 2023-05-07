#version 330
in vec2 fragTexCoord;                    // screen position <-1,+1>
in vec4 fragColor;

uniform sampler2D texture0;          // texture to blur
uniform vec4 colDiffuse;
uniform float xs,ys;            // texture resolution
uniform float r;                // blur radius
out vec4 finalColor;          // fragment output color
//---------------------------------------------------------------------------
void main(){
    float x,y,xx,yy,rr=r*r,dx,dy,w,w0;
    w0=0.3780/pow(r,1.975);
    vec2 p;
    vec4 col=vec4(0.0,0.0,0.0,0.0);
    for (dx=1.0/xs,x=-r,p.x=0.5+(fragTexCoord.x*0.5)+(x*dx);x<=r;x++,p.x+=dx){ xx=x*x;
        for (dy=1.0/ys,y=-r,p.y=0.5+(fragTexCoord.y*0.5)+(y*dy);y<=r;y++,p.y+=dy){ yy=y*y;
        if (xx+yy<=rr)
        {
        w=w0*exp((-xx-yy)/(2.0*rr));
        col+=texture2D(texture0,p)*w;
        }}}
    finalColor=col;
}