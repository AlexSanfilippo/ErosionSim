#version 460 core

/**
 This shader handles step 3.3: sediment transport step
 */

 //input is one pixel of the image
 //work group
 layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//output to the image.  format must match here and in host program
layout(rgba32f, binding = 0) uniform image2D imgOutput; //changing binding to change image: 0, 2, or 3
layout(rgba32f, binding = 2) uniform image2D imgOutput2; //velocity



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

    //value.x = rgba.x;

    float dT = 0.01f; //time step


    value = rgba;

    //i think this is likely what is wrong with my deposition:  in this shader, the s value in Texture0 is set
    //to 0.  and/or, no transport occurs, so on vertices with low velocity, they at most gain back the exact amount
    //of sediment they lost.  Thus, high v water erodes, but low b water experiences no change.  
    //I should try changing the texelCoord to be normCoord, then convert back, similar to how we did 
    //it in the normal calculation.  
    //value.b = imageLoad(imgOutput, ivec2(float(normCoord.x) - v.x * dT, float(normCoord.y) - v.y * dT)).b;

   
    vec2 uv = vec2(float(texelCoord.x), float(texelCoord.y)); //TPTP


    //THIS DOES NOT WORK
    //v /= 128.0f;
    //value.b = imageLoad(imgOutput, ivec2(vec2(uv.x - (v.x) * dT, uv.y - (v.y) * dT))).b;

    //---------try taking one cell-step back rather than scale by v, or set v tween 0 and 1

    //logically, should be minus the sign, but this seems to work?  I don't understand
    //actually, appears to deposit in strange places (petals along cardinal XZ axes)  too, but also in bowl center  
    //using VS, sediment is NOT accumulating in areas of low vel, or moving out of areas of high vel
    //Thought: may need to "ping pong", ie, write NEW sediment values to a second texture, then swap textures
    ivec2 pos = ivec2(texelCoord.x - sign(v.x), texelCoord.y - sign(v.y));
    //ivec2 pos = ivec2(texelCoord.x - 1, texelCoord.y - 1);
    //value.b = imageLoad(imgOutput, pos).b;
    if(imageLoad(imgOutput,pos).g > 0.0f) //Do NOT push sediment onto land!
    {
        value.b = imageLoad(imgOutput2, pos).b;

    }
     //new: load s_1 from texture2
    
    /*
    if (pos.x > 128 || pos.x < 0 || pos.y > 128 || pos.y < 0)
    {
        value.b = 0.0f;
    }
    */




        /*
        vec2 pos = vec2(uv.x - v.x * dT, uv.y - v.y * dT);// * vec2(128.0f, 128.0f);    
        if (pos.x > 128 || pos.x < 0 || pos.y > 128 || pos.y < 0)
        {
            //TEMP
            value.b = 0.05f;
            //supposed to do: find value.b by using 4 nearest neighbors.. unsure what this means by "interpolation", average?


            //this is definetly not the solution, but a stand-in
            float R = imageLoad(imgOutput, ivec2(texelCoord.x+1, texelCoord.y)).b;
            float L = imageLoad(imgOutput, ivec2(texelCoord.x - 1, texelCoord.y)).b;
            float T = imageLoad(imgOutput, ivec2(texelCoord.x, texelCoord.y + 1)).b;
            float B = imageLoad(imgOutput, ivec2(texelCoord.x, texelCoord.y - 1)).b;
            value.b = (R+L+T+B) / 4.f;
        }
        */
        /*
        if(pos.x > 128  || pos.y > 128)
        {
            value.b = 0.5f;
        }
        */
        //NEED TO ADD BOUNDARY CONDITIONS


        //write to image, at this texelCoord, the 4f vector of color data
        imageStore(imgOutput, texelCoord, value);
        //imageStore(imgOutput, ivec2(64,64), value); //TP
    //imageStore(imgOutput, texelCoord, v);
}