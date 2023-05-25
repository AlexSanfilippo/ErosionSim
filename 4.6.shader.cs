#version 460 core

//input is one pixel of the image
//work group
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput;

void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    //value.x and .y store the [0,1] normalized coordinate of the pixel
    value.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
    value.y = float(texelCoord.y) / (gl_NumWorkGroups.y);

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value);
}