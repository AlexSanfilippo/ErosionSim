#version 460 core

/**
 This shader does step 3.2 (move water around)
 */

 //input is one pixel of the image
 //work group
    layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput0; //changing binding to change image: 0, 2, or 3
layout(rgba32f, binding = 1) uniform image2D imgOutput1; //flux (outflow)
//layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity
uniform float size;
void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 rgba = imageLoad(imgOutput0, texelCoord); //works: load in the height map image
    vec4 lrtb = imageLoad(imgOutput1, texelCoord);
    //vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water

    float dT = 0.001f; //time step
    //UPDATE FLUX (eqn 2)
    float A = 1.0f; //cross-sectional area.
    float g = 9.81f; //acceleration from gravity (m/s^2)
    float l = 1.0f; //length of virtual pipe
    float sumLRTB = 0.f;
    for(int i = 0; i < 4; i++){
        vec4 rgba_1;
        if (i == 0){
            rgba_1 = imageLoad(imgOutput0, ivec2(texelCoord.x - 1, texelCoord.y));
        }
        if (i == 1){
            rgba_1 = imageLoad(imgOutput0, ivec2(texelCoord.x + 1, texelCoord.y));
        }
        if (i == 2)
        {
            rgba_1 = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y + 1));
        }
        if (i == 3)
        {
            rgba_1 = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y - 1));
        }
        float d_h = rgba.r + rgba.g - (rgba_1.r + rgba_1.g); //height difference
        //tp
        lrtb[i] = max(0.0f, lrtb[i] + dT * A * (g * d_h)/(l)); //

        

        sumLRTB += lrtb[i];
    }

    //NO_SLIP BOUNDARY
    
    if (texelCoord.y >= size-1)
    {
        sumLRTB -= lrtb.b;
        lrtb.b = 0.0f;
    }
    if (texelCoord.y <= 0)
    {
        sumLRTB -= lrtb.a;
        lrtb.a = 0.0f;
    }
    if (texelCoord.x >= size - 1)
    {
        sumLRTB -= lrtb.g;
        lrtb.g = 0.0f;
    }
    if (texelCoord.x <= 0)
    {
        sumLRTB -= lrtb.r;
        lrtb.r = 0.0f;
    }
    


    float lX = 1.0f; 
    float lY = 1.0f; //distance between cells in X and Y directions --UNSURE OF VALUES
    //calculate and factor in scaling factor K
    float K = min(1.f, (rgba.g *lX*lY)/ sumLRTB);
    for(int i = 0; i < 4; i++)
    {
        lrtb[i] *= K;
    }

  
    //3.2.2: water surface and velocity field update
    /*
    //eqn 6: change in velocity
    float Rin = imageLoad(imgOutput1, ivec2(texelCoord.x - 1, texelCoord.y)).r;
    float Tin = imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y - 1)).a;
    float Lin = imageLoad(imgOutput1, ivec2(texelCoord.x + 1, texelCoord.y)).g;
    float Bin = imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y + 1)).b;
    float dV = dT*((Rin + Tin + Lin + Bin) - sumLRTB);

    //eqn 7: update water height
    float d2 = rgba.g + dV / (lX * lY); //going below zero here, not good
    float d1 = rgba.g;

    //eqn 8: update water flow through current cell
    vec2 dW;
    dW.x = (imageLoad(imgOutput1, ivec2(texelCoord.x - 1, texelCoord.y)).g - lrtb.r +  
        lrtb.g - imageLoad(imgOutput1, ivec2(texelCoord.x + 1, texelCoord.y)).r) / 2.f;
    dW.y = (imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y - 1)).b - lrtb.a +  
        lrtb.b - imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y + 1)).a) / 2.f;

    float d_bar = (d1 + d2) / 2.f;

    v.x = dW.x / d_bar * lY; //u
    v.y = dW.y / d_bar * lX; //v

    */
    value.r = rgba.r;
    value.g = rgba.g;
    //value.y = rgba.y; //blue, holds water

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput0, texelCoord, value); //b,d,s
    imageStore(imgOutput1, texelCoord, lrtb); //flux
    //imageStore(imgOutput2, texelCoord, v); //water velocity
}