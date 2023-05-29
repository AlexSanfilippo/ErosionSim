#version 430 core
layout(location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;

//Coord. Sys. chapter
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;

struct mapStruct
{
	vec3 pos;
	vec3 norm;
};

layout(std430, binding = 3) buffer mapVertices
{
    
    //mapStruct ms;

    //xperimental
    float testVec[];
}hMap;

//texture with height map
//uniform sampler2D tex;
layout(binding=0) uniform sampler2D tex;
//texture with normals 
//uniform sampler2D texNormals;
layout(binding=3) uniform sampler2D texNormals;

void main()
{  

    //NEW - read normals from texture
    vec2 TexCoordsN = vec2(aTexCoord.y, aTexCoord.x);
    vec3 texNorm = texture(texNormals, TexCoordsN).rgb;




    //TEXTURE VERSION WORKING
    vec2 TexCoords = vec2(aTexCoord.y, aTexCoord.x);
    vec4 texCol = texture(tex, TexCoords).rgba;
    gl_Position = projection * view * model * vec4(aPos.x, texCol.r + texCol.g, aPos.z, 1.0);
    //gl_Position = projection * view * model * vec4(aPos.x, texCol.r, aPos.z, 1.0); //water height not displayed on mesh
    


    vec3 flatColor = vec3(0.231, 0.812, 0.118); //grass
    //vec3 vertColor = vec3(0.522f, 0.357f, 0.196f);//vec3(0.612, 0.541, 0.498); //dirt or rock-brown version
    vec3 vertColor = vec3(0.612, 0.541, 0.498); //grey version
    //ourColor = mix(flatColor, vertColor, tilt); //kinda ?

    //float tilt = 1.0f - -1.0f*aNorm.y; //0 for flat
   
    //current best
    //ourColor = max(0.5f*vec3(1.0f-tilt*2.25f)*flatColor*(1.0f-0.5f*aPos.y), 1.0f*vec3(tilt)*vertColor);
    
    //NB: color brightness is tied to tilt-at-which-switch-color, therefore, cannot currently "decrease greeness" w/o
    //darkening the whole color pallete.  I may have to use conditions
    
   
    ourColor = 1.f*texNorm; //looks super cool, but not what we want

    //SabastionLague-like Version
    //-------------
    
    /*
    
    float grassSlopeThreshold = 0.03; //(sin(time*0.8f)+1.f)/2.f;
    float grassBlendAmount = 0.999;
    


    //change 3 instances of texNorm/aNorm below to turn on/off normal vector texture's effect
    vec3 tempNormVal = texNorm; //texNorm
    float slope = 1.0f - -1.f * tempNormVal.y;
    float grassBlendHeight = grassSlopeThreshold * (1.0f-grassBlendAmount);
    float grassWeight = 1.0f-clamp((slope-grassBlendHeight)/(grassSlopeThreshold-grassBlendHeight),0.0f, 1.0f);
    ourColor = 1.5f*flatColor*clamp((1.0f-1.0f*aPos.y),0.33,.8)*(0.5f+tempNormVal.x)* grassWeight + 1.5f*(clamp(0.25f+tempNormVal.x,.025f,0.99f)*(1.0f-grassWeight)*vertColor); //*tilt*2.25
    
    //outer condition colors water blue
    if(texCol.g > 0.0001){
        //ourColor = mix(ourColor, vec3(0.f, 0.2f, max(texCol.g,0.45)), 0.25+texCol.g);
        //ourColor = mix(0.6f*ourColor, vec3(0.f, 0.2f, 0.45), 0.25+texCol.g); //attempt at transparent water
        ourColor = mix(mix(ourColor, vec3(0.5f, 0.5f, 0.85), 0.0+texCol.g), vec3(0.f, 0.2f, 0.45), 0.75f + texCol.g); //simple depth color
    }
    
    */

    
}