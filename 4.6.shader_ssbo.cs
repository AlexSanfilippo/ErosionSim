# version 460 core


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//define an SSBO storage block to store the height map data
struct mapStruct
{
	vec3 pos;
	vec3 norm;
};

layout(std430, binding = 3) buffer mapVertices
{
	//mapStruct hMap[];	

    //below ran, but flat map:
    //vec3 pos;
	//vec3 norm;

	//runs fine, but flat
	//mapStruct ms;

	//experimental
	float testVec[];
	
}hMap; 

void main()
{
	//int index = gl_GlobalInvocationID.x;
	//hMap.pos.y = 1000*sin(index);
	//hMap.pos;
	//hMap.ms.pos.y += 1.0f; // + 0.000000001f*hMap.ms.pos.y;  //DOES actually write to SSBO, which VS successfully reads from
	//hMap.ms[0] += -1;
	

	//hMap.testVec[gl_GlobalInvocationID.x] = 1.00f;

	//hMap.testVec[0] = gl_GlobalInvocationID.x;

	//hMap[gl_GlobalInvocationID.x];
}