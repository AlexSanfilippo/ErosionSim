#version 460 core

/**
 This shader handles step 3.5: evaporation
 */

 //input is one pixel of the image
 //work group
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3
//layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity



void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.yx);


    vec4 rgba = imageLoad(imgOutput, texelCoord); //works: load in the height map image

    //value.x = rgba.x;

    float dT = 0.02f; //time step


    value = rgba;
    float K_e = 0.05f; //evap. constant
    value.g = value.g * (1 - K_e * dT);

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value);
    
}