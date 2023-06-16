#version 460 core

/*
 This shader does step 3.2: update the flux vector "f" and store in texture1
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
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 rgba = imageLoad(imgOutput0, texelCoord); //works: load in the height map image
    vec4 lrtb = imageLoad(imgOutput1, texelCoord);
    //vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water

    float dT = 0.004f; //time step
    //UPDATE FLUX (eqn 2)
    float A = 0.4f; //cross-sectional area.   //NB: decreasing A -> slower velocity  //default: 1.  but 0.1 for big moving waves
    float g = 9.81f; //acceleration from gravity (m/s^2)
    float l = 0.1f; //length of virtual pipe  //NB: does not effect velocity
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
        
        lrtb[i] = max(0.0f, lrtb[i] + dT * A * (g * d_h)/(l)); 

        

        sumLRTB += lrtb[i];
    }

    //NO_SLIP BOUNDARY

    //size -= 10;
    float lowEnd = 0.0f;
    float highEnd = 0.0f;
    float sizeB = size - highEnd; // - 10.f;
    if (texelCoord.y >= sizeB-1)
    {
        sumLRTB -= lrtb.b;
        lrtb.b = 0.0f;
    }
    if (texelCoord.y <= lowEnd)
    {
        sumLRTB -= lrtb.a;
        lrtb.a = 0.0f;
    }
    if (texelCoord.x >= sizeB - 1)
    {
        sumLRTB -= lrtb.g;
        lrtb.g = 0.0f;
    }
    if (texelCoord.x <= lowEnd)
    {
        sumLRTB -= lrtb.r;
        lrtb.r = 0.0f;
    }
    
    

    float lX = 1.0f; 
    float lY = 1.0f; //distance between cells in X and Y directions --UNSURE OF VALUES
    //calculate and factor in scaling factor K
    float K = min(1.0f, (rgba.g *lX*lY)/ (sumLRTB)); //dT causing issues here

    for(int i = 0; i < 4; i++)
    {
        lrtb[i] *= K;
    }


    
    value = rgba; 
    
    


   
    imageStore(imgOutput1, texelCoord, lrtb); //flux
    
}