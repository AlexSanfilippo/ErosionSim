#version 430 core
layout(location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

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


void main()
{ 

    //NO SSBO
    //gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0); //camera   

    //EXPERIMENTAL
    //gl_Position = projection * view * model * vec4(aPos.x, hMap.testVec[0], aPos.z, 1.0);
    

    //EXPERIMENT WITH INDEXING 
    int index = 0;
    if(aPos.x > 2.0){
        index = 1;
    }
    //gl_Position = projection * view * model * vec4(aPos.x, hMap.testVec[int(clamp(aPos.x,0,3))], aPos.z, 1.0);

    //JUST HEIGHTS
    //+ int(aPos.z*20)*64
    gl_Position = projection * view * model * vec4(aPos.x, hMap.testVec[int(aPos.x*40)], aPos.z, 1.0);
    //gl_Position = projection * view * model * vec4(aPos.x, hMap.testVec[aPos.x*128][aPos.z*128], aPos.z, 1.0);


    //float tilt = clamp(0.5f*distance(aNorm, vec3(0.0,1.0,0.0)),0.0f,1.0f); //let slope of terrain triangle effect color

    vec3 flatColor = vec3(0.231, 0.812, 0.118); //grass
    vec3 vertColor = vec3(0.522f, 0.357f, 0.196f);//vec3(0.612, 0.541, 0.498); //dirt or rock
    //ourColor = mix(flatColor, vertColor, tilt); //kinda ?

    float tilt = 1.0f - -1.0f*aNorm.y; //0 for flat
   
    //current best
    //ourColor = max(0.5f*vec3(1.0f-tilt*2.25f)*flatColor*(1.0f-0.5f*aPos.y), 1.0f*vec3(tilt)*vertColor);
    
    //NB: color brightness is tied to tilt-at-which-switch-color, therefore, cannot currently "decrease greeness" w/o
    //darkening the whole color pallete.  I may have to use conditions
    
   
    //ourColor = 1.f*aNorm; //looks super cool, but not what we want

    //SabastionLague-like Version
    //-------------
    
    float grassSlopeThreshold = 0.03; //(sin(time*0.8f)+1.f)/2.f;
    float grassBlendAmount = 0.999;
    float slope = 1.0f - -1.f * aNorm.y;
    float grassBlendHeight = grassSlopeThreshold * (1.0f-grassBlendAmount);
    float grassWeight = 1.0f-clamp((slope-grassBlendHeight)/(grassSlopeThreshold-grassBlendHeight),0.0f, 1.0f);
    ourColor = 1.0f*flatColor*clamp((1.0f-1.0f*aPos.y),0.33,.8)*(0.5f+aNorm.x)* grassWeight + clamp(0.25f+aNorm.x,.5f,0.8f)*(1.0f-grassWeight)*vertColor; //*tilt*2.25
    
    
}