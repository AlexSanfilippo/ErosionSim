# version 460 core

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

    //boundary detection  WORKS
    if (r_b == 0 || l_b == 0 || d_b == 0 || u_b == 0)
    {
        return 0.0f;
    }





    float dbdx = (r_b - l_b) / (2.0 * l_xy.x / texture_size.x);
    float dbdy = (d_b - u_b) / (2.0 * l_xy.y / texture_size.y);

    return sqrt(dbdx * dbdx + dbdy * dbdy) / sqrt(1 + dbdx * dbdx + dbdy * dbdy);
}

void main()
{
    


    //vec4 rgba = imageLoad(imgOutput, texelCoord); //works: load in the height map image
    vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water
    //float tilt = imageLoad(imgOutput3, texelCoord).a; //tilt of land

   
   
    
    v.a = max(find_sin_alpha(), 0.01);

    imageStore(imgOutput2, texelCoord, v);
}