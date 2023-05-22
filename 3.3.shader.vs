#version 330 core
layout(location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

out vec3 ourColor;

//Coord. Sys. chapter
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



void main()
{ 
    gl_Position = projection * view * model * vec4(aPos, 1.0); //camera   
    
    //float tilt = clamp(0.5f*distance(aNorm, vec3(0.0,1.0,0.0)),0.0f,1.0f); //let slope of terrain triangle effect color

    vec3 flatColor = vec3(0.231, 0.812, 0.118); //grass
    vec3 vertColor = vec3(0.612, 0.541, 0.498);//vec3(0.522f, 0.357f, 0.196f); //dirt or rock
    //ourColor = mix(flatColor, vertColor, tilt); //kinda ?

    float tilt = 1.0f - -1.0f*aNorm.y; //0 for flat
   
    
    ourColor = max(0.5f*vec3(1.0f-tilt*2.25f)*flatColor*(1.0f-0.5f*aPos.y), vec3(tilt)*vertColor);
    
    //ourColor = (1.0f - tilt)*flatColor + tilt*vertColor;
   
    //ourColor = aNorm; //looks super cool, but not what we want
    //ourColor = vec3(aNorm.y*-1.f);
    
    //ourColor = clamp((aPos.y-0.5)*2., 0.0f, 1.0f);
}