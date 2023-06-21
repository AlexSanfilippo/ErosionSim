#version 460 core

/*
 This shader handles step 3.3: deposition and erosion
 */

 //input is one pixel of the image
 //work group
 layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3
layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity
layout(rgba32f, binding = 3) uniform image2D imgOutput3; //normals and slope

ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

vec2 texture_size = vec2(128.f, 128.f);
vec2 UV = vec2(gl_GlobalInvocationID.xy);
vec2 uv = vec2(float(texelCoord.x), float(texelCoord.y)) / vec2(129.f, 129.f);
vec2 l_xy = vec2(1.0f, 1.0f);
vec2 s = 1.0 / texture_size;
//alternative tilt factor "alpha" method found online
/*
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
*/
//new version that uses indexing method from normal shader
//seems to be working, but issue around edges
float find_sin_alpha()
{
  

    //land height values of 4 cardinal neighbors
    // looks different, but not better neccessarily
    /*
    float r_b = imageLoad(imgOutput, ivec2((uv + s * vec2(1.0f, 0.0f)) * vec2(128.f, 128.f))).x;
    float l_b = imageLoad(imgOutput, ivec2((uv - s * vec2(1.0f, 0.0f)) * vec2(128.f, 128.f))).x;
    float d_b = imageLoad(imgOutput, ivec2((uv + s * vec2(0.0f, 1.0f)) * vec2(128.f, 128.f))).x;
    float u_b = imageLoad(imgOutput, ivec2((uv - s * vec2(0.0f, 1.0f)) * vec2(128.f, 128.f))).x;
    */
    
    float r_b = imageLoad(imgOutput, ivec2(UV + vec2(1.0, 0))).x;
    float l_b = imageLoad(imgOutput, ivec2(UV - vec2(1.0, 0))).x;
    float d_b = imageLoad(imgOutput, ivec2(UV + vec2(0, 1.0))).x;
    float u_b = imageLoad(imgOutput, ivec2(UV - vec2(0, 1.0))).x;

    float b = imageLoad(imgOutput, ivec2(UV + vec2(0.0, 0))).x;

    //boundary detection  WORKS
    if (r_b == 0 || l_b == 0 || d_b == 0 || u_b == 0)
    {
        return 0.0f;
    }





    //float dbdx = (r_b - l_b) / (2.0 * l_xy.x / texture_size.x);
    //float dbdy = (d_b - u_b) / (2.0 * l_xy.y / texture_size.y);

    //not any better,but different erosion pattern
    float dbdx = ( (r_b - l_b) + ((b-r_b) + (b-l_b) )/2.f ) / (2.0 * l_xy.x / texture_size.x);
    float dbdy = ( (d_b - u_b) + ((b - d_b) + (b - u_b))/2.f )/ (2.0 * l_xy.y / texture_size.y);

    return sqrt(dbdx * dbdx + dbdy * dbdy) / sqrt(1 + dbdx * dbdx + dbdy * dbdy);
}

float getSmoothAlpha()
{

    float r_a = imageLoad(imgOutput2, ivec2(UV + vec2(1.0, 0))).w;
    float l_a = imageLoad(imgOutput2, ivec2(UV - vec2(1.0, 0))).w;
    float d_a = imageLoad(imgOutput2, ivec2(UV + vec2(0, 1.0))).w;
    float u_a = imageLoad(imgOutput2, ivec2(UV - vec2(0, 1.0))).w;

    float a = imageLoad(imgOutput2, ivec2(UV + vec2(0.0, 0))).w;


    return (r_a + l_a + d_a + u_a + a )/ 5.0f; ;
}

void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    //absolute texel coord (ie, not normalized)
    
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
    float K_c = 0.001f; //carrying constant     ?
    float K_s = 0.002f; //dissolving constant     ?
    float K_d = 0.002f; //deposit constant    ?


    //float tilt = imageLoad(imgOutput, texelCoord).a; WRONG TEXTURE
    //float a = max(0.0f,tilt*1.f); //local tilt. what to make minimum? //some deposition with 0.9f min)
    //a is 0 on flat terrain
    //float a = find_sin_alpha(); //TP, from github implementation.  sort of works


    //float a = max(find_sin_alpha(), 0.01); //0.25 ok results //.01 ok too, maybe better

    float a = max(getSmoothAlpha(), 0.0f);
    


    //CONCLUSION: our tilt value "a" is wrong, both for our version and the github-found version


    //unsure if this is better or worse
    //float a = 1.0f - clamp(find_sin_alpha(), 0.0f, 1.0f); //attempt at inversion


    value = rgba;

    float vMagn = sqrt((v.x * v.x) + (v.y * v.y));
    //float C = K_c * sin(a) * vMagn; //as per paper (failing to deposit and erode correctly)

    //float C = K_c * a * vMagn; //changed sin(a) -> a.  much smoother erosion on cliffs, but direction biased erosion too.
    float C = K_c * vMagn;// * (1.0f + value.y);// * (1.0f - clamp(value.y, 0.0f, 1.0f));//* vMagn ; 
    //float C = K_c * a * vMagn; //modified
    //float C = K_c * (sin(a)/2.f + 0.5f) * vMagn ; // modified by me

    //float C = K_c * (a + 0.15) * max(0.15, vMagn) * clamp(value.y, 0.0f, 1.0f); //github -- if min 0, no erosion


    float s = rgba.b;


    //if initialized channels yxw of texture0 to 0.0f, this out condition may not be neccessary 
    if(value.g > 0.00001f) //my addition, maybe not the best, but map doesn't appear without it
    {
        if (C > s) //dissolve soil into water
        {
            //value.r = max(0.10f, rgba.r - K_s * (C - s)); // (11a), modified with min ground height
            value.r = value.r - K_s * (C - s); //(11a)
            s = s + (K_s * (C - s));     // (11b)
        }       
        else //deposit soil
        {
            value.r = value.r + K_d * (s - C);  //(12a)  
            s = s - K_d * (s - C);   //(12b)
        }       
    }
    
   
    
    
    //update sediment in texture 2, not texture 0.  sediment written to texture 0 in shader 3.4
    v.z = s;
    

    

    //write to image, at this texelCoord, the 4f vector of color data
    imageStore(imgOutput, texelCoord, value); 
    imageStore(imgOutput2, texelCoord, v);
}