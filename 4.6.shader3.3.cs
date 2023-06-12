#version 460 core

/**
 This shader handles step 3.3: deposition and erosion
 */

 //input is one pixel of the image
 //work group
 layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3
layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity
layout(rgba32f, binding = 3) uniform image2D imgOutput3; //normals and slope

vec2 texture_size = vec2(128.f, 128.f);
vec2 UV = vec2(gl_GlobalInvocationID.xy);
vec2 l_xy = vec2(1.0f, 1.0f);
//alternative tilt factor "alpha" method found online
float find_sin_alpha()
{
    float self_b = imageLoad(imgOutput, ivec2(UV)).x;
    float r_b = imageLoad(imgOutput, ivec2(UV + vec2(1.0 , 0))).x;
    float l_b = imageLoad(imgOutput, ivec2(UV - vec2(1.0, 0))).x;
    float d_b = imageLoad(imgOutput, ivec2(UV + vec2(0, 1.0 ))).x;
    float u_b = imageLoad(imgOutput, ivec2(UV - vec2(0, 1.0 ))).x;

    float dbdx = (r_b - l_b) / (2.0 * l_xy.x / texture_size.x);
    float dbdy = (r_b - l_b) / (2.0 * l_xy.y / texture_size.y);

    return sqrt(dbdx * dbdx + dbdy * dbdy) / sqrt(1 + dbdx * dbdx + dbdy * dbdy);
}


void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    //normCoord.x and .y store the [0,1] normalized coordinate of the pixel
    vec2 normCoord;
    normCoord.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
    normCoord.y = float(texelCoord.y) / (gl_NumWorkGroups.y);


    vec4 rgba = imageLoad(imgOutput, texelCoord); //works: load in the height map image
    vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water
    float tilt = imageLoad(imgOutput3, texelCoord).a; //tilt of land

    //value.x = rgba.x;

    //float dT = 0.001f; //time step



    //Calculate Sediment Transportation Capacity

    //Possible reasons this is failing
    //1. dissolved sediment "s" in texture0 is never updated
    //2. the tilt value "a" is too important to ignore
    //3. the velocity vector "v", calculated in 3.3c, is incorrect


    //unsure what to set these to - values from Lan Lao on YT
    float K_c = 0.01f; //carrying constant     ?
    float K_s = 0.004f; //dissolving constant     ?
    float K_d = 0.008f; //deposit constant    ?


    //float tilt = imageLoad(imgOutput, texelCoord).a; WRONG TEXTURE
    float a = max(0.01f,tilt*1.f); //local tilt. what to make minimum? //some deposition with 0.9f min)
                                   //a is 0 on flat terrain
    a = max(0.01f, find_sin_alpha()); //TP, from github implementation.  Different results then "our" tilt, but still wrong

    //tp: velocity adjustment - very slow water velocity set to zero to force deposition

    /*
    if( (v.x > -.1 && v.x < .1) && (v.y > -.1 && v.y < .1) )
    {
        v.x = 0.0f;
        v.y = 0.0f;
    }
    */
    value = rgba;

    float vMagn = sqrt((v.x * v.x) + (v.y * v.y));
    float C = K_c * sin(a) * vMagn; //as per paper
    //float C = K_c * (sin(a)/2.f + 0.5f) * vMagn ; // modified by me

    //float C = K_c * (sin(a) + 0.15) * max(0.15, vMagn) * clamp(value.y, 0.0f, 1.0f); //github -- if min 0, no erosion

    //C = 0.00001; //TP
    float s = rgba.b;


    
    if(value.g > 0.0000001f) //my addition, maybe not the best, but map doesn't appear without it
    {
        if (C > s) //dissolve soil into water
        {
            //value.r = max(0.10f, rgba.r - K_s * (C - s)); // (11a), modified with min ground height
            value.r = value.r - K_s * (C - s); //(11a)
            s = s + (K_s * (C - s))*2.0f;     // (11b)
        }       
        else //deposit soil
        {
            value.r = value.r + K_d * (s - C);  //(12a)  
            s = s - K_d * (s - C);   //(12b)
        }       
    }
    
    /*
    else //TP
    {
        value.r = rgba.r + K_d * (s*1000.f - 0.0f);  //(12a)  //either no update or causes map to rise/sink beyond view instantly
        s_1 = 0.0f;   //(12b)
    }
    */
    
    //old
    //value.b = s;
    //new
    v.b = s;
    //value.b = 0.0f;

    

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value); 
    imageStore(imgOutput2, texelCoord, v);
}