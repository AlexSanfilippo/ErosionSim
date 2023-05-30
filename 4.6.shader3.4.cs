#version 460 core

/**
 This shader handles step 3.3: sediment transport step
 */

 //input is one pixel of the image
 //work group
 layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3
layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity



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
    vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water

    //value.x = rgba.x;

    float dT = 0.1f; //time step


    value = rgba;

    //i think this is likely what is wrong with my deposition:  in this shader, the s value in Texture0 is set
    //to 0.  and/or, no transport occurs, so on vertices with low velocity, they at most gain back the exact amount
    //of sediment they lost.  Thus, high v water erodes, but low b water experiences no change.  
    //I should try changing the texelCoord to be normCoord, then convert back, similar to how we did 
    //it in the normal calculation.  
    value.b = imageLoad(imgOutput, ivec2(float(texelCoord.x) - v.x * dT, float(texelCoord.y) - v.y * dT)).b;


    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value);
    //imageStore(imgOutput, texelCoord, v);
}