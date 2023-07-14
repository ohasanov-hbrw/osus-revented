#version 110

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 textureSize;
uniform float transparency;

vec4 calculateColor(vec4 texel){
    vec4 outputColor = vec4(0,0,0,0);
    if(texel.r > 0.09){
        if(texel.a > 0.25){
            outputColor = vec4(texel.a*0.3,texel.a*0.3,texel.a*0.3,0.7 * transparency);
        }
        else{
            float opacity = 1.0 * transparency;
            float color = 1.0;
            if(texel.a == 0.0){
                opacity = 0.0;
            }
            else if(texel.a > 0.22){
                color = 0.7;
            }
            else if(texel.a > 0.2){
                color = 0.8;
            }
            else if(texel.a > 0.19){
                color = 0.9;
            }
            outputColor = vec4(color,color,color,opacity);
        }
    }
    else{
        outputColor = vec4(0,0,0,0);
    }
    return outputColor;
}

void main()
{
    vec4 outcolor = calculateColor(texture2D(texture0, fragTexCoord));
    gl_FragColor = outcolor;
}