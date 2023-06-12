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

    


    //old borken version
    /*
    vec2 uv = vec2(float(texelCoord.x), float(texelCoord.y)); 
    ivec2 pos = ivec2(vec2(uv.x - (v.x) * dT, uv.y - (v.y) * dT)); //
    value.b = imageLoad(imgOutput2, pos).b;
    */

    //Version from Github
    //-------------------
    
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec4 rgba = imageLoad(imgOutput, texelCoord); //works: load in the height map image
    vec4 v = imageLoad(imgOutput2, texelCoord); //velocity of water
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    
    //use a uniform later on
    vec2 textSize = vec2(128, 128); //size of the texture

    value = rgba;

    float dT = 1.0f; //not optimized.  small values produce no sediment.  around 10 seems sort of correct(?)
    //vec4 self_bds = texture(T1_bds, UV);
    //vec2 vel = texture(T3_v, UV).xy;
    //scale up to xy coords so we can apply the vel*delta_t step
    vec2 XY = texelCoord;// * textSize; 
    float x_step = clamp(XY.x - v.x * dT, 0, textSize.x);
    float y_step = clamp(XY.y - v.y * dT, 0, textSize.y);

    //remember to divide back down to sample with UV coords
    /*
    float s_top_left = imageLoad(imgOutput2, ivec2(floor(x_step) / textSize.x, floor(y_step) / textSize.y)).z;
    float s_top_right = imageLoad(imgOutput2, ivec2(ceil(x_step) / textSize.x, floor(y_step) / textSize.y)).z;
    float s_bot_left = imageLoad(imgOutput2, ivec2(floor(x_step) / textSize.x, ceil(y_step) / textSize.y)).z;
    float s_bot_right = imageLoad(imgOutput2, ivec2(ceil(x_step) / textSize.x, ceil(y_step) / textSize.y)).z;
    */

    //imageLoad uses non-normalized pixel coords.  Thus we should NOT divide by textSize here or in XY step
    float s_top_left = imageLoad(imgOutput2, ivec2(floor(x_step) , floor(y_step) )).z;
    float s_top_right = imageLoad(imgOutput2, ivec2(ceil(x_step), floor(y_step) )).z;
    float s_bot_left = imageLoad(imgOutput2, ivec2(floor(x_step) , ceil(y_step) )).z;
    float s_bot_right = imageLoad(imgOutput2, ivec2(ceil(x_step) , ceil(y_step) )).z;
    //first interp
    float s_top = (x_step - floor(x_step)) * s_top_right + (ceil(x_step) - x_step) * s_top_left;
    float s_bot = (x_step - floor(x_step)) * s_bot_right + (ceil(x_step) - x_step) * s_bot_left;

    //second interp for final val
    value.z = (y_step - floor(y_step)) * s_top + (ceil(y_step) - y_step) * s_bot;

    //tp
    //value.z = 0.01f;

    imageStore(imgOutput, texelCoord, value);




}