#version 110

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 textureSize;
uniform float transparency;

vec4 calculateColor(vec4 texel){
    vec4 outputColor = vec4(0,0,0,0);
    if(texel.r > 0.06){
        if(texel.a > 0.25){
            outputColor = vec4(texel.a*0.3,texel.a*0.3,texel.a*0.4,0.7 * transparency);
        }
        else if(texel.a > 0.20){
            float tempData = (0.25 - texel.a) * 20.0;
            outputColor = vec4(tempData + 0.075 ,tempData + 0.075 ,tempData + 0.075 , (0.7 + tempData * 0.3) * transparency);
        }
        else{
            float opacity = 1.0 * transparency;
            float color = 1.0;
            //if(texel.a == 0.0){
            //    opacity = 0.0;
            //}
            //else if(texel.a > 0.22){
            //    color = 0.7;
            //}
            //else if(texel.a > 0.2){
            //    color = 0.8;
            //}
            //else if(texel.a > 0.19){
            //    color = 0.9;
            //}
            
            //if(texel.r < 0.08){
            //    opacity = 0.2 * transparency;
            //}
            //else if(texel.r < 0.09){
            //    opacity = 0.3 * transparency;
            //}
            //else if(texel.r < 0.11){
            //    opacity = 0.5 * transparency;
            //}
            //else if(texel.r < 0.12){
            //    opacity = 0.6 * transparency;
            //}
            //else if(texel.r < 0.13){
            //    opacity = 0.7 * transparency;
            //}
            
            if(texel.r < 0.11){
                float tempData = (texel.r - 0.06) * 20.0;
                opacity = tempData * transparency;
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
    //if(outcolor.a == 0.0){
    //    
    //}
    //else{
    //    vec4 outcolorr = calculateColor(texture2D(texture0, fragTexCoord + vec2(1.0 / textureSize.x,0))) / 5.0;
    //    vec4 outcolorl = calculateColor(texture2D(texture0, fragTexCoord - vec2(1.0 / textureSize.x,0))) / 5.0;
    //    vec4 outcoloru = calculateColor(texture2D(texture0, fragTexCoord + vec2(0,1.0 / textureSize.y))) / 5.0;
    //    vec4 outcolord = calculateColor(texture2D(texture0, fragTexCoord - vec2(0,1.0 / textureSize.y))) / 5.0;
    //    
    //    gl_FragColor = outcolor + outcolorr + outcolorl + outcoloru + outcolord;
    //}
    gl_FragColor = outcolor;
}