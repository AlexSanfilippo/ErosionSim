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
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    //absolute texel coord (ie, not normalized)
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    //normCoord.x and .y store the [0,1] normalized coordinate of the pixel
    vec2 normCoord;
    normCoord.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
    normCoord.y = float(texelCoord.y) / (gl_NumWorkGroups.y);


    vec4 rgbaA = imageLoad(imgOutput0, texelCoord); //works: load in the height map image

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
    vec3 norms;


    /*
    float distance = 1.0f; //distance between points - did not fix the issue

    vec2 tcScaled = distance * texelCoord.xy;
    
    if (texelCoord.y > -50.5f)
    {
        if (texelCoord.x % 2 == 0)
        {
            vec4 rgbaB = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y + 1));
            vec4 rgbaC = imageLoad(imgOutput0, ivec2(texelCoord.x + 1, texelCoord.y));
            A = vec3(float(tcScaled.x), rgbaA.r + rgbaA.g, float(tcScaled.y));
            B = vec3(float(tcScaled.x), rgbaB.r + rgbaB.g, float(tcScaled.y + 1));
            C = vec3(float(tcScaled.x + 1), rgbaC.r + rgbaC.g, float(tcScaled.y));

            //tp
            norms = normalize(calcNormal(A, B, C));
            //norms = vec3(0.70711f, -0.0f, 0.0f);


        }
        
        else //(tcScaled.x % 2 == 1)
        {
            vec4 rgbaB = imageLoad(imgOutput0, ivec2(texelCoord.x - 1, texelCoord.y));
            vec4 rgbaC = imageLoad(imgOutput0, ivec2(texelCoord.x - 1, texelCoord.y - 1));
            A = vec3(tcScaled.x, rgbaA.r + rgbaA.g, tcScaled.y);
            B = vec3(tcScaled.x - 1, rgbaB.r + rgbaB.g, tcScaled.y);
            C = vec3(tcScaled.x - 1, rgbaC.r + rgbaC.g, tcScaled.y - 1);

            norms = normalize(calcNormal(A, B, C));

        } 
        
    }
    else
    {
        if (texelCoord.x % 2 == 0)
        {
            vec4 rgbaB = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y - 1));
            vec4 rgbaC = imageLoad(imgOutput0, ivec2(texelCoord.x + 1, texelCoord.y - 1));
            A = vec3(float(tcScaled.x), rgbaA.r + rgbaA.g, float(tcScaled.y));
            B = vec3(float(tcScaled.x), rgbaB.r + rgbaB.g, float(tcScaled.y - 1));
            C = vec3(float(tcScaled.x + 1), rgbaC.r + rgbaC.g, float(tcScaled.y - 1));

            norms = normalize(calcNormal(A, B, C));

        }
        else //(texelCoord.x % 2 == 1)
        {
            vec4 rgbaB = imageLoad(imgOutput0, ivec2(texelCoord.x - 1, texelCoord.y));
            vec4 rgbaC = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y - 1));
            A = vec3(tcScaled.x, rgbaA.r + rgbaA.g, tcScaled.y);
            B = vec3(tcScaled.x - 1, rgbaB.r + rgbaB.g, tcScaled.y);
            C = vec3(tcScaled.x, rgbaC.r + rgbaC.g, tcScaled.y - 1);

            norms = normalize(calcNormal(A, B, C));
        }
    }
    */

    //TP-manually send normals to color the mesh
    /*
    A = vec3(0.2f, 0.2f, 0.2f);
    B = vec3(0.3f, 0.3f, 0.3f);
    C = vec3(0.5f, 0.5f, 0.2f);
    norms = calcNormal(A, B, C); //is returning correct, after below normalization is ran
    norms = norms / sqrt(norms.x*norms.x + norms.y*norms.y + norms.z + norms.z); //normalize
    //norms = vec3(-0.7f, 0.70f, 0.0f); //norm IS being sent to VS, and used to color each vertex
    */

    /*
    //Now that we know calcNormal is working, and texture reading/writing here and in VS is working, the issue MUST be the
    //inputs we give to the calcNormal function
    //this method sort of works but not really.  norm.y always 1.
    
    vec4 rgbaB = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y + 1));
    vec4 rgbaC = imageLoad(imgOutput0, ivec2(texelCoord.x + 1, texelCoord.y));
    A = vec3(0.f, rgbaA.r + rgbaA.g, 0.f);
    B = vec3(1.f, rgbaB.r + rgbaB.g, 1.f);
    C = vec3(1.f, rgbaC.r + rgbaC.g , 0.f);
    norms = calcNormal(A, B, C); //is returning correct, after below normalization is ran
    norms = norms / sqrt(norms.x * norms.x + norms.y * norms.y + norms.z + norms.z); //normalize
    //norms -= 0.5f;
    norms = vec3(mix(0.0f, 0.8f, pow(norms.y,4)));  // x and z are zero... only norm.y is anything
    */
    //NEW METHOD: use 4 cardinal neighbors to this height on height map to get the slope of this vertex, then convert into
    // the vec3 normal vector
    /*
    vec4 rgbaL = imageLoad(imgOutput0, ivec2(texelCoord.x - 1, texelCoord.y));
    vec4 rgbaR = imageLoad(imgOutput0, ivec2(texelCoord.x + 1, texelCoord.y));
    vec4 rgbaT = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y + 1));
    vec4 rgbaB = imageLoad(imgOutput0, ivec2(texelCoord.x, texelCoord.y - 1));


    //our current point in height map, an 4 cardinal neighbors

    texelCoord = ivec2(0, 0);

    vec3 up2 = vec3(texelCoord.x, texelCoord.y + 1, rgbaT.r + rgbaT.g);
    vec3 down = vec3(texelCoord.x, texelCoord.y - 1, rgbaB.r + rgbaB.g);
    vec3 left = vec3(texelCoord.x - 1, texelCoord.y, rgbaL.r + rgbaL.g);
    vec3 right = vec3(texelCoord.x + 1, texelCoord.y, rgbaR.r + rgbaR.g);
    vec3 point = vec3(texelCoord.x, texelCoord.y, rgbaA.r + rgbaA.g);

    vec3 lp = point - left;
    vec3 pr = right - point;
    vec3 up = point - up2;
    vec3 pd = down - point;

    //cross products
    vec3 upxlp = vec3(lp.z, up.z, -1.f);
    vec3 upxpr = vec3(pr.z, up.z, -1.f);
    vec3 pdxlp = vec3(lp.z, pd.z, -1.f);
    vec3 pdxpr = vec3(pr.z, pd.z, -1.f);

    //sum
    norms = upxlp + upxpr + pdxlp + pdxpr; //returns black
    //normalize
    //norms = normalize(norms);
    norms = -1.f*vec3(norms.x, norms.z, norms.y); //change order so height is y
    norms = vec3(0.f, 0.f, 1.f);///BLACK MAP WHy?!?!?

    //norms = vec3(normCoord.y, 0.f,  normCoord.x); //gives a blue-red color gradient
    */

    

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
    
    //TP
    //norms = vec3(h1);
    

    //ISSUE FOUND: TILT was using LAND+WATER height, needs to use LAND TILT
    //UPDATE: I think this was a mistake.  water that flows down hill into a pool should not continue with velocity
    //as though going down hill.. should crash into water
    float tilt =  sqrt( pow(0.5f - normal.x, 2) +  pow(0.5f - normal.y, 2)); //NOT QUITE working

  
    p = imageLoad(imgOutput0, ivec2(uv * vec2(128.f, 128.f))).r;  
    h1 = imageLoad(imgOutput0, ivec2((uv + s * vec2(textureOffset, 0.0f)) * vec2(128.f, 128.f))).r;   
    v1 = imageLoad(imgOutput0, ivec2((uv + s * vec2(0.0f, textureOffset)) * vec2(128.f, 128.f))).r;



    vec2 normalLand = (p - vec2(h1, v1));
    //normal = vec2(p - h1, p - v1);

    //very important
    normalLand *= normalStrength;
    normalLand += 0.5f;

    float tiltLand = sqrt(pow(0.5f - normalLand.x, 2) + pow(0.5f - normalLand.y, 2));


    //write normal to the texture, plus the tilt in the fourth channel
    imageStore(imgOutput3, texelCoord, vec4(norms.x, norms.y, norms.z, tilt));

    

}