#version 460 core

/**
 This shader adds water to the height map (value.y) either through a fixed source or a rain() function
 */

//input is one pixel of the image
//work group
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3


//gives us a random rain pattern
float random(vec2 st){
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

//euclidean distance between two 2D vectors
float distance(vec2 v1, vec2 v2)
{
    return sqrt(pow((v1.x - v2.x), 2.f) + pow((v1.y - v2.y), 2.f));
}


void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.yx);
    //normCoord.x and .y store the [0,1] normalized coordinate of the pixel
    vec2 normCoord;
    normCoord.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
    normCoord.y = float(texelCoord.y) / (gl_NumWorkGroups.y);


    vec4 rgba = imageLoad(imgOutput, texelCoord); //works: load in the height map image
  
    value.x = rgba.x;

    float dT = 1.0f; //time step

    //RAIN 
    //value.y = rgba.y + dT * random(rgba.xy);  //pseudo-random rain

    //WATER SOURCE
    vec2 source = vec2(0.9f, 0.9f); //pos of water source- norm coords in [0,1]
    float radius = 0.05f; //radius of water source, square map side length = 2.0f
    float strength = 0.1f; //how much water to add //very unsure how strong this is, probably want value < 1.0f
    float MAXWATER = 0.3f;
    value.y = min(rgba.y + dT*strength * max(0.0f, radius - distance(source, normCoord)), MAXWATER);

    //testing g channel
    //value.y = rgba.g + 1.0f;
  

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value);
}