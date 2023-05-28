#version 460 core

//input is one pixel of the image
//work group
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3

void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.yx);

    //value.x and .y store the [0,1] normalized coordinate of the pixel
    //value.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
    //value.y = float(texelCoord.y) / (gl_NumWorkGroups.y);
    vec4 rgba = imageLoad(imgOutput, texelCoord); //works: load in the height map image
    //value.x = cos(texelCoord.x)*cos(texelCoord.y); //r


    //value.x = rgba.x * 1.0001f; //works: slowly moves heights map "up"
    value.x = rgba.x;

    //just playing with compute shader - this makes the map "sink" into a flat map
    /*
    if(rgba.x > 1.0f)
    {
        value.x = rgba.x;
    }
    else
    {
        value.x = rgba.x / 1.001f;
    }
    */

    value.y = 0; //blue, holds water

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value);
}