#version 460 core

/**
 This shader calculates normals of the vertices using the height map
These values should be read by the VS to update the mesh normals in place of the current system
 */

//input is one pixel of the image
//work group
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput;
layout(rgba32f, binding = 3) uniform image2D imgOutput3; //normal vector image



/*Calculate the normal of a vertex*/
vec3 calcNormal(vec3 A, vec3 B, vec3 C){
    vec3 normal; //return variable: the normal of this triangle

    vec3 U = vec3(B.x - A.x, B.y - A.y, B.z - A.z);
    vec3 V = vec3(C.x - A.x, C.y - A.y, C.z - A.z);

    normal.x = U.y * V.z - U.z * V.y;
    normal.y = U.z * V.x - U.x * V.z;
    normal.z = U.x * V.y - U.y * V.x;

    return normal; 
}

void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.yx);
    //normCoord.x and .y store the [0,1] normalized coordinate of the pixel
    vec2 normCoord;
    normCoord.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
    normCoord.y = float(texelCoord.y) / (gl_NumWorkGroups.y);


    vec4 rgbaA = imageLoad(imgOutput, texelCoord); //works: load in the height map image

    //load the two neighboring heights - need to know which triangle orientation we are in..
    //or do we? just changes coloring pattern
    

    //get vector of each 3 points of triangle (sum water AND ground)
    /*
    vec3 A = vec3(normCoord.x, rgbaA.r + rgbaA.g, normCoord.y);
    vec3 B = vec3(normCoord.x + 1, rgbaB.r + rgbaB.g, normCoord.y);
    vec3 C = vec3(normCoord.x, rgbaC.r + rgbaC.g, normCoord.y + 1);
    */

    vec3 A;
    vec3 B;
    vec3 C;

    float distance = 1.0f; //distance between points - did not fix the issue

    vec2 tcScaled = distance * texelCoord.xy;
    
    if (texelCoord.y % 2 == 0)
    {
        if (texelCoord.x % 2 == 0)
        {
            vec4 rgbaB = imageLoad(imgOutput, ivec2(texelCoord.x + 1, texelCoord.y));
            vec4 rgbaC = imageLoad(imgOutput, ivec2(texelCoord.x, texelCoord.y + 1));
            A = vec3(float(tcScaled.x), rgbaA.r + rgbaA.g, float(tcScaled.y));
            B = vec3(float(tcScaled.x + 1), rgbaB.r + rgbaB.g, float(tcScaled.y));
            C = vec3(float(tcScaled.x), rgbaC.r + rgbaC.g, float(tcScaled.y + 1));
            
        }
        else //(tcScaled.x % 2 == 1)
        {
            vec4 rgbaB = imageLoad(imgOutput, ivec2(texelCoord.x - 1, texelCoord.y));
            vec4 rgbaC = imageLoad(imgOutput, ivec2(texelCoord.x - 1, texelCoord.y - 1));
            A = vec3(tcScaled.x, rgbaA.r + rgbaA.g, tcScaled.y);
            B = vec3(tcScaled.x - 1, rgbaB.r + rgbaB.g, tcScaled.y);
            C = vec3(tcScaled.x - 1, rgbaC.r + rgbaC.g, tcScaled.y - 1);
        }        
    }
    else
    {
        if (texelCoord.x % 2 == 0)
        {
            vec4 rgbaB = imageLoad(imgOutput, ivec2(texelCoord.x, texelCoord.y - 1));
            vec4 rgbaC = imageLoad(imgOutput, ivec2(texelCoord.x + 1, texelCoord.y - 1));
            A = vec3(float(tcScaled.x), rgbaA.r + rgbaA.g, float(tcScaled.y));
            B = vec3(float(tcScaled.x), rgbaB.r + rgbaB.g, float(tcScaled.y - 1));
            C = vec3(float(tcScaled.x + 1), rgbaC.r + rgbaC.g, float(tcScaled.y - 1));

        }
        else //(texelCoord.x % 2 == 1)
        {
            vec4 rgbaB = imageLoad(imgOutput, ivec2(texelCoord.x - 1, texelCoord.y));
            vec4 rgbaC = imageLoad(imgOutput, ivec2(texelCoord.x, texelCoord.y - 1));
            A = vec3(tcScaled.x, rgbaA.r + rgbaA.g, tcScaled.y);
            B = vec3(tcScaled.x - 1, rgbaB.r + rgbaB.g, tcScaled.y);
            C = vec3(tcScaled.x, rgbaC.r + rgbaC.g, tcScaled.y - 1);
        }
    }

   



    //send 3 points to a normal calculation function
    vec3 norms = calcNormal(A, B, C);
    


    //write normal to the texture
    imageStore(imgOutput3, texelCoord, vec4(norms, 1.0f));

    

}