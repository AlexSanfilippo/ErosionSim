#version 460 core

/**
 This shader does step 3.2 (move water around) -- third pass, calculate velocity field UV using flux f
 */

 //input is one pixel of the image
 //work group
    layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput0; //changing binding to change image: 0, 2, or 3
layout(rgba32f, binding = 1) uniform image2D imgOutput1; //flux (outflow)
layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity

void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 rgba = imageLoad(imgOutput0, texelCoord); //works: load in the height map image
    vec4 lrtb = imageLoad(imgOutput1, texelCoord);
    vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water

    float dT = 0.004f; //time step
    

    float lX = 1.0f;
    float lY = 1.0f; //distance between cells in X and Y directions --UNSURE OF VALUES


    //eqn 8: update water flow through current cell

    vec2 dW; //double checked: this is correct
    dW.x = (imageLoad(imgOutput1, ivec2(texelCoord.x - 1, texelCoord.y)).g - lrtb.r +
        lrtb.g - imageLoad(imgOutput1, ivec2(texelCoord.x + 1, texelCoord.y)).r) / 2.f;
    dW.y = (imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y - 1)).b - lrtb.a +
        lrtb.b - imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y + 1)).a) / 2.f;

    
    float d_bar = (rgba.g + imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y)).a) / 2.f; //(d1 + d2) / 2.f;

    v.x = dW.x / (d_bar * lY); //u
    v.y = dW.y / (d_bar * lX); //v

    


    
    imageStore(imgOutput2, texelCoord, v); //water velocity
} 