#version 460 core

/**
 This shader calculates normals of the vertices using the height map
These values should be read by the VS to update the mesh normals in place of the current system
 */

//input is one pixel of the image
//work group
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput0;
layout(rgba32f, binding = 3) uniform image2D imgOutput3; //normal vector image


float textureOffset = 1.0f; //for normal calculation via heightmap from toyshader
float normalStrength = 20.0f;

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

/*normalize a vector by dividing it by its magnitude.  need for normals*/
vec3 normalize(vec3 v)
{
    return v / sqrt(v.x * v.x + v.y * v.y + v.z + v.z);
}


float sum(vec2 v){
    return v.x + v.y;
}


void main()
{
    

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy); 
    vec3 norms;


    

    

//Method we Gleamed from ToyShader example by "NoxWings"
//vec2 texNormalMap(in vec2 uv)

    vec2 s = 1.0/vec2(128.f,128.f);
    vec2 uv = vec2(float(texelCoord.x), float(texelCoord.y))/vec2(129.f,129.f);
    

    //float p = texture(heightMap, uv).x;
    float p = sum(imageLoad(imgOutput0, ivec2(uv * vec2(128.f, 128.f))).rg);

    //float h1 = texture(heightMap, uv + s * vec2(textureOffset,0)).x;
    float h1 = sum(imageLoad(imgOutput0, ivec2( (uv + s * vec2(textureOffset, 0.0f)) * vec2(128.f, 128.f)  )).rg);

    //float v1 = texture(heightMap, uv + s * vec2(0,textureOffset)).x;
    float v1 = sum(imageLoad(imgOutput0, ivec2( (uv + s * vec2(0.0f, textureOffset)) * vec2(128.f, 128.f)   )).rg);

    
       
   	vec2 normal = (p - vec2(h1, v1));
    //normal = vec2(p - h1, p - v1);
    
    //very important
    normal *= normalStrength;
    normal += 0.5f;

    

    

    norms = vec3(normal, 1.0f);
    
   
    

    //ISSUE FOUND: TILT was using LAND+WATER height, needs to use LAND TILT
    //UPDATE: I think this was a mistake.  water that flows down hill into a pool should not continue with velocity
    //as though going down hill.. should crash into water
    float tilt =  sqrt( pow(0.5f - normal.x, 2) +  pow(0.5f - normal.y, 2)); //works for lighting, but not for 'a' in 3.3 

  
    p = imageLoad(imgOutput0, ivec2(uv * vec2(128.f, 128.f))).r;  
    h1 = imageLoad(imgOutput0, ivec2((uv + s * vec2(textureOffset, 0.0f)) * vec2(128.f, 128.f))).r;   
    v1 = imageLoad(imgOutput0, ivec2((uv + s * vec2(0.0f, textureOffset)) * vec2(128.f, 128.f))).r;



    vec2 normalLand = (p - vec2(h1, v1));
    //normal = vec2(p - h1, p - v1);

    //very important
    normalLand *= normalStrength;
    normalLand += 0.5f;

    float tiltLand = sqrt(pow(0.5f - normalLand.x, 2) + pow(0.5f - normalLand.y, 2));

    //TP: testing that our tilt calculation matches our expectations
    /*
    if(tiltLand== 0.0)
    {
        norms = vec3(1.0f, 0.0f, 0.0f);
    }
    */
    //norms = vec3(normalLand, 1.0f);

    //write normal to the texture, plus the tilt in the fourth channel
    imageStore(imgOutput3, texelCoord, vec4(norms.x, norms.y, norms.z, tiltLand));

    

}