#version 460 core

/**
 This shader does step 3.2 (move water around) -- second pass, update water height d with flux f
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
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);

    vec4 rgba = imageLoad(imgOutput0, texelCoord); //works: load in the height map image
    vec4 lrtb = imageLoad(imgOutput1, texelCoord);
    vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water

    float dT = 0.01f; //time step
    
    float sumLRTB = 0.f;

    float lX = 1.0f;
    float lY = 1.0f; //distance between cells in X and Y directions --UNSURE OF VALUES
    


    for (int i = 0; i < 4; i++)
    {
        sumLRTB += lrtb[i];
    }

    //3.2.2: water surface and velocity field update
    
    //Get F_in from neighboring cells
    float Rin = imageLoad(imgOutput1, ivec2(texelCoord.x - 1, texelCoord.y)).g;    
    float Tin = imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y - 1)).b;
    float Lin = imageLoad(imgOutput1, ivec2(texelCoord.x + 1, texelCoord.y)).r;
    float Bin = imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y + 1)).a;
    

    //ivec2 texelCoord2 = ivec2(gl_GlobalInvocationID.x - 1, gl_GlobalInvocationID.y);
    //float Rin = imageLoad(imgOutput1, ivec2(texelCoord2)).g;


    float dV = dT * ((Rin + Tin + Lin + Bin) - sumLRTB);
    /*
    if(Rin != lrtb.g) //PROOF: That our neighbor is NOT a different value.  implies we are not reading in neighbor in image
    {
        dV = 0.0001;
    }
    else
    {
        dV = 0.0f;
    }
    */
    //eqn 7: update water height wrt water in/out flow dV
    float d2 = rgba.g + dV / (lX * lY); 
    float d1 = rgba.g;
    //value.x = rgba.r;
    rgba.g = d2;
    rgba.a = d1;


    //TP: Attempt to accumulate velocity
    //v += dV / (lX * lY); //no change in behavoir


    imageStore(imgOutput0, texelCoord, rgba); //b,d,s
    //imageStore(imgOutput1, texelCoord, lrtb); //flux
    //imageStore(imgOutput2, texelCoord, v); //water velocity
}

//eqn 8: update water flow through current cell
/*
vec2 dW;
dW.x = (imageLoad(imgOutput1, ivec2(texelCoord.x - 1, texelCoord.y)).g - lrtb.r +
    lrtb.g - imageLoad(imgOutput1, ivec2(texelCoord.x + 1, texelCoord.y)).r) / 2.f;
dW.y = (imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y - 1)).b - lrtb.a +
    lrtb.b - imageLoad(imgOutput1, ivec2(texelCoord.x, texelCoord.y + 1)).a) / 2.f;

float d_bar = (d1 + d2) / 2.f;

v.x = dW.x / d_bar * lY; //u
v.y = dW.y / d_bar * lX; //v


value.r = rgba.r;
value.g = d2;
*./

//write to image, at this texelCoord, the 4f vector of color data
imageStore(imgOutput0, texelCoord, value); //b,d,s
imageStore(imgOutput1, texelCoord, lrtb); //flux
//imageStore(imgOutput2, texelCoord, v); //water velocity
} */