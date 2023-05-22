/**
* Author: Alexander Sanfilippo
* Date 29 Jan, 2023
* Breif: A class to generate vertex data for a procedural map mesh
* 
*/

#ifndef MAP_H
#define MAP_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

/*Add Matric Algebra Functions*/
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>



double E = 2.718281828459045;


using namespace std;
class Map
{
public:

	unsigned int size;
	unsigned int octaves;
	unsigned int frequency;
	unsigned int height;

	//float smooth = 1.0;
	float hMax, hMin;

	vector <float> vertices;
	
	vector <float> heights;

	

	//Mesh drawing
	unsigned int VAO;

	//Data handling
	int verticesSize;
	//cout << "in main(), verticesSize = " << verticesSize << endl;
	float* verticesArray;
	int numMapVertices;
	

	Map(int seed = 0, unsigned int mySize = 8, unsigned int myOctaves = 2, unsigned int myFreq = 4, float smooth = 1.0, float scale=1.0){
		srand(seed);

		frequency = myFreq;
		size = mySize;
		octaves = myOctaves;
		numMapVertices = size * size * 6;

		//cout << "Map Constructor called with size " << size << endl;

		//put height map data in here
		heights = noiseMap(frequency, size, octaves, scale);

		//OCTAVE CALCULATIION
		//vector <float> base = noiseMap(frequency, size, octaves, scale);
		vector < vector <float> > octaveLayers;
		octaveLayers.push_back(heights);
		unsigned freqOctaves = frequency;
		for (int i = octaves; i > 0; i--) { //build our octave layers
			freqOctaves *= 2;
			octaveLayers.push_back(noiseMap(freqOctaves, size, octaves, scale));
		}
		//cout << "made octaves\n";
		hMax = -10;
		hMin = 1000000;
		for (int i = 0; i < size; i++) { //apply octave layers
			for (int j = 0; j < size; j++) {
				for (int k = 1; k < octaves+1; k++) {
					heights[i * size + j] += octaveLayers[k][i*size+j] * pow(0.5,k);
				}
				heights[i * size + j] = sigmoid(pow(heights[i * size + j], smooth)); // 0 to 1
				if (heights[i * size + j] > hMax) {
					hMax = heights[i * size + j];
				}
				if (heights[i * size + j] < hMin) {
					hMin = heights[i * size + j];
				}
			}
		}
		//cout << "applied octaves\n";
		
		//cout << "MAX HEIGHT " << hMax << ",  MIN HEIGHT " << hMin << endl;
		//cout << "in constructor, height Map has size " << heights.size() << " for size = " << size << endl;

		//initialize a flat map
		//vertices
		float spacing = scale/(float)size; //lets put coords between 0 an 1
		float i, j;
		unsigned int triangleOrientation = 0;
		for (i = 0.0f; i < size; i++) {
			
			//cout << "i = " << i << endl;
			for (j = 0.0f; j < size-1; j++) { //was size -1
				//cout << "j=" << j << endl;
				if (triangleOrientation == 0) {
					/*
					//point A
					vertices.push_back(i*spacing);
					vertices.push_back(heights[i*size + j]); //height value
					vertices.push_back(j * spacing);
					//COLOR ATTRIBUTE
					vertices.push_back(colorMap(heights[i * size + j]).x);
					vertices.push_back(colorMap(heights[i * size + j]).y);
					vertices.push_back(colorMap(heights[i * size + j]).z);
					//point B
					vertices.push_back(i * spacing);
					vertices.push_back(heights[i * size + j + 1]); //height value
					vertices.push_back((j + 1.0f) * spacing);
					//COLOR ATTRIBUTE
					vertices.push_back(colorMap(heights[i * size + j + 1]).x);
					vertices.push_back(colorMap(heights[i * size + j + 1]).y);
					vertices.push_back(colorMap(heights[i * size + j + 1]).z);
					//Point C
					vertices.push_back((i + 1.0f) * spacing);
					vertices.push_back(heights[(i+1) * size + j]); //height value
					vertices.push_back(j * spacing);
					//COLOR ATTRIBUTE
					vertices.push_back(colorMap(heights[(i + 1) * size + j]).x);
					vertices.push_back(colorMap(heights[(i + 1) * size + j]).y);
					vertices.push_back(colorMap(heights[(i + 1) * size + j]).z);
					*/

					//NORMAL VERSION
					//a) calculate 3 vertices of this triangle
					//b) calculate normal using the 3 vertices
					//c) write both position and normal to "vertices"
					vector <glm::vec3> tv(3); //temp vertices
					//point A
					tv[0].x = (i * spacing);
					tv[0].y = (heights[i * size + j]); //height value
					tv[0].z = (j * spacing);					
					//point B
					tv[1].x = (i * spacing);
					tv[1].y = (heights[i * size + j + 1]); //height value
					tv[1].z = ((j + 1.0f) * spacing);					
					//Point C
					tv[2].x = ((i + 1.0f) * spacing);
					tv[2].y = (heights[(i + 1) * size + j]); //height value
					tv[2].z = (j * spacing);
					//normal calculation
					vector <glm::vec3> tn(3); //temp normals
					tn[0] = glm::normalize(calcNormal(tv[0], tv[2], tv[1])); //these may be wrong winding order, unsure
					tn[1] = glm::normalize(calcNormal(tv[1], tv[0], tv[2]));
					tn[2] = glm::normalize(calcNormal(tv[2], tv[1], tv[0]));
					//Finally, store this info into vertices
					for (int k = 0; k < 3; k++) { //per vertex
						//add position
						for (int m = 0; m < 3; m++) {
							vertices.push_back(tv[k][m]);
						}
						//add normals
						for (int m = 0; m < 3; m++) {
							vertices.push_back(tn[k][m]);
						}

					}

				}
				else {
					/*
					//point D
					vertices.push_back(i * spacing);
					vertices.push_back(heights[i * size + j]); //height value
					vertices.push_back(j * spacing);
					//COLOR ATTRIBUTE
					vertices.push_back(colorMap(heights[i * size + j]).x);
					vertices.push_back(colorMap(heights[i * size + j]).y);
					vertices.push_back(colorMap(heights[i * size + j]).z);
					
					//point E
					vertices.push_back(i * spacing);
					vertices.push_back(heights[i * size + j + 1]); //height value
					vertices.push_back((j + 1.0f) * spacing);
					//COLOR ATTRIBUTE
					vertices.push_back(colorMap(heights[i * size + j + 1]).x);
					vertices.push_back(colorMap(heights[i * size + j + 1]).y);
					vertices.push_back(colorMap(heights[i * size + j + 1]).z);
					
					//Point F
					vertices.push_back((i - 1.0f) * spacing);
					vertices.push_back(heights[(i-1) * size + j + 1]); //height value
					vertices.push_back((j + 1.0f) * spacing);
					//COLOR ATTRIBUTE
					vertices.push_back(colorMap(heights[(i - 1) * size + j + 1]).x);
					vertices.push_back(colorMap(heights[(i - 1) * size + j + 1]).y);
					vertices.push_back(colorMap(heights[(i - 1) * size + j + 1]).z);
					*/

					vector <glm::vec3> tv(3); //temp vertices
					//point D
					tv[0].x = (i * spacing);
					tv[0].y = (heights[i * size + j]); //height value
					tv[0].z = (j * spacing);
					
					//point E
					tv[1].x = (i * spacing);
					tv[1].y = (heights[i * size + j + 1]); //height value
					tv[1].z = ((j + 1.0f) * spacing);
					
					//Point F
					tv[2].x = ((i - 1.0f) * spacing);
					tv[2].y = (heights[(i - 1) * size + j + 1]); //height value
					tv[2].z = ((j + 1.0f) * spacing);
					
					//normal calculation
					vector <glm::vec3> tn(3); //temp normals
					tn[0] = glm::normalize(calcNormal(tv[0], tv[1], tv[2])); //these may be wrong winding order, unsure
					tn[1] = glm::normalize(calcNormal(tv[1], tv[2], tv[0]));
					tn[2] = glm::normalize(calcNormal(tv[2], tv[0], tv[1]));
					//Finally, store this info into vertices
					for (int k = 0; k < 3; k++) { //per vertex
						//add position
						for (int m = 0; m < 3; m++) {
							vertices.push_back(tv[k][m]);
						}
						//add normals
						//cout << "norm has values:  ";
						for (int m = 0; m < 3; m++) {
							vertices.push_back(tn[k][m]);
							//cout << tn[k][m] << ", ";
						}
						//cout << endl;

					}
				}
				
			}
			if (triangleOrientation == 0) {
				triangleOrientation = 1;
			}
			else {
				triangleOrientation = 0;
				if(i != size-1)
					i -= 1;
			}
		}


		
		/*=====Translate vector of positions to array=====*/
		//dynamically created array of vertices
		verticesSize = vertices.size();
		verticesArray = new float[verticesSize];
		//populate the vertices
		for (int i = 0; i < verticesSize; i++) {
			verticesArray[i] = vertices[i];

		}


		//call setupMesh() to create OpenGL buffers
		setupMesh();
	}

	

	/*Applies color based on the vector's height*/
	glm::vec3 colorMap(float height) {
		float third = (hMax - hMin) / 3;
		glm::vec3 color;
		

		//first attempt at land colors
		float colors2[18] = {0.0, 77.0/255.0, 0.0, 
							0.0, 128.0/255.0, 0.0, 
							0.17647, 0.70196, 0.00000,
							204.0/255.0, 200.0/255.0, 0.0,
							204.0/255.0, 153.0/255.0, 0.0,
							1.00000, 0.40000, 0.00000 };
		//attempting a more muted pallet
		float colors3[18] = { 0.20000, 0.34902, 0.15294,
							0.19608, 0.38039, 0.14118,
							0.35686, 0.50196, 0.13725,
							0.57647, 0.50980, 0.16471,
							0.63922, 0.60392, 0.42353,
							0.86275, 0.79608, 0.83529 };
		//less yellow
		float colors4[18] = { 0.20000, 0.34902, 0.15294,
							0.19608, 0.38039, 0.14118,
							0.35686, 0.50196, 0.13725,
							0.34902, 0.34118, 0.32941,
							0.63922, 0.60392, 0.42353,
							0.86275, 0.79608, 0.83529 };
		//Water
		float colors[18] = { 59.0/255.0, 64.0/255.0, 130.0/255.0,
							0.20000, 0.34902, 0.15294,
							0.19608, 0.38039, 0.14118,
							0.35686, 0.50196, 0.13725,
							0.34902, 0.34118, 0.32941,
							
							0.86275, 0.79608, 0.83529 };

		height -= hMin;
		float top = hMax-hMin;
		float maxColor = 1.0f;
		float x = maxColor / top;
		height *= x;
		float hColor = height;
		hColor *= x;
		hColor += 0.2; //tween .2 and .8
		float hFactor;
		if (height < 0.2) {
			hFactor = height/0.2;
			color = glm::vec3((1 - hFactor)*(colors[0]) + hFactor * (colors[3]), \
				              (1 - hFactor) * (colors[1]) + hFactor * (colors[4]), \
				              (1 - hFactor) * (colors[2]) + hFactor * (colors[5]));
		}
		else if (height > 0.2 && height < 0.4) {
			hFactor = (height - 0.2)/ 0.2;
			color = glm::vec3((1 - hFactor) * (colors[3]) + hFactor * (colors[6]), \
				(1 - hFactor) * (colors[4]) + hFactor * (colors[7]), \
				(1 - hFactor) * (colors[5]) + hFactor * (colors[8]));
			//color = glm::vec3(0.0, 128.0/255.0, 0.0);
		}
		else if (height > 0.4 && height < 0.6) {
			hFactor = (height - 0.4) / 0.2;
			color = glm::vec3((1 - hFactor) * (colors[6]) + hFactor * (colors[9]), \
				(1 - hFactor) * (colors[7]) + hFactor * (colors[10]), \
				(1 - hFactor) * (colors[8]) + hFactor * (colors[11]));
			//color = glm::vec3(89.0/255.0, 179.0/255.0, 0.0);
		}
		else if (height > 0.6 && height < 0.8) {
			hFactor = (height - 0.6) / 0.2;
			color = glm::vec3((1 - hFactor) * (colors[9]) + hFactor * (colors[12]), \
				(1 - hFactor) * (colors[10]) + hFactor * (colors[13]), \
				(1 - hFactor) * (colors[11]) + hFactor * (colors[14]));

			//color = glm::vec3(172.0/255.0, 230.0/255.0, 0.0);
		}
		else if (height > 0.8) {
			hFactor = (height - 0.8) / 0.2;
			color = glm::vec3((1 - hFactor) * (colors[12]) + hFactor * (colors[15]), \
				(1 - hFactor) * (colors[13]) + hFactor * (colors[16]), \
				(1 - hFactor) * (colors[14]) + hFactor * (colors[17]));
			//color = glm::vec3(204.0/255.0, 153.0/255.0, 0.0);
		}
		else { //should never render
			color = glm::vec3(204.0/255.0, 0.0, 204.0/255.0);
		}
		
		
		

		return color;
	}

	/*Sigmoid function.  returns value between 0 and 1*/
	double sigmoid(double x) {
		//cout << "inside sigmoid\n";
		return 1/(1 + pow(E, -x));
	}
	/*Generates 2D noise*/
	vector <float> noiseMap(unsigned int frequency, unsigned int size, unsigned int octaves, float scale) {
		//cout << "in noiseMap()\n";
		vector <float> heightMap; // vector of heights for our map
		//1. Generate random points to interpolate between
		vector <float> randomMap; // map of random values we interpolate between
		for (int i = 0; i < frequency; i++) {
			for (int j = 0; j < frequency; j++) {
				//float r = (((float)rand() / RAND_MAX) * 2.0f) - 1.0f;
				float r = (((float)rand() / RAND_MAX) );
				//cout << "r = " << r << endl;
				randomMap.push_back(r);
				
			}
		}
		//cout << "Made random map success\n";
		//cout << "randomMap has size " << randomMap.size() << endl; //seems fine
		//2. Interpolate between random points in randomMap
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				
					//convert heightMap point to randomMap point equivalent
					float y = (float)i / (float)size * (float)(frequency - 1);
					float x = (float)j / (float)size * (float)(frequency - 1);
					int x1, x2, y1, y2;
					x1 = (int)x;
					y2 = (int)y;
					x2 = x1 + 1;
					y1 = y2 + 1;
					
					float Q12 = randomMap[x1 + y2 *frequency];
					float Q22 = randomMap[x2 + y2 * frequency];
					float Q11 = randomMap[x1 + y1 * frequency];
					float Q21 = randomMap[x2 + y1 * frequency];
					float heightValue = ((y2 - y) / (y2 - y1) * ((x2 - x) / (x2 - x1) * Q11 + (x - x1) / \
						(x2 - x1) * Q21) + (y - y1) / (y2 - y1) * ((x2 - x) / (x2 - x1) * Q12 + (x - x1) / (x2 - x1) * Q22));
					//cout << "x1:" << x2 << " y1:" << y1 << " x2:" << x2 << " y2:" << y2 << endl;
					//cout << "height is " << heightValue << endl;
					heightMap.push_back(heightValue);
			}
		}
		//cout << "made height map success\n";
		return heightMap;
	}
	
	//add a function to generate height data based on N octaves, and apply to the map vertices data
	
	/*return vertices*/
	vector <float> getVertices() {
		return vertices;
	}

	/*Call to draw the map in the render loop*/
	void draw(Shader& shader)
	{
		// draw mesh
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, numMapVertices/*num vertices*/);
		glBindVertexArray(0);
	}

	/*Returns height at an x,z coordinate on the map*/
	//NOTE: In main, pass X coord to worldZ and Z coord to worldX!!!!
	float getHeight(float worldX, float worldZ, float mapscale/*map size in world*/) {
		
		//1. turn world coords into grid coordinates (ie, which square we are in)
		int gridX = floor(size * (worldX / mapscale));
		int gridZ = floor(size * (worldZ / mapscale));
		
		//cout << "[map.h] given world coords z" << worldX << ", x" << worldZ << endl;
		//cout << "[map.h]we are in grid cell " << gridZ << "Z, " << gridX << "X" << endl;
		//2. get coords within the square (to determine which triangle)
		float squareX = size*(worldX/mapscale) - gridX;
		//squareX = 0.0; //TP
		float squareZ = size*(worldZ/mapscale) - gridZ;
		//tp
		//cout << "in map.getHeight() squareX=" << squareX << " and squareZ=" << squareZ << endl;
		float height = 0.0f;
		if (squareZ <= (1 - squareX)) { //then we are in Triangle 1 (left hand side) ABC
			//cout << "TRIANGLE ABC\n";
			glm::vec3 vertA = glm::vec3(0.0f, heights[gridX + gridZ * size], 0.0f);
			glm::vec3 vertB = glm::vec3(0.0f, heights[gridX + ((gridZ+1) * size) ], 1.0f);
			glm::vec3 vertC = glm::vec3(1.0f, heights[(gridX + 1) + gridZ * size], 0.0f);
			//apply the barry centric position formula
			float det = (vertB.z - vertC.z) * (vertA.x - vertC.x) + (vertC.x - vertB.x) * (vertA.z - vertC.z);
			float aa = ((vertB.z - vertC.z) * (squareX - vertC.x) + (vertC.x - vertB.x) * (squareZ - vertC.z)) / det;
			float ab = ((vertC.z - vertA.z) * (squareX- vertC.x) + (vertA.x - vertC.x) * (squareZ - vertC.z)) / det;
			float ac = 1.0f - aa - ab;
			height = aa * vertA.y + ab * vertB.y + ac * vertC.y;
		}
		else { //TRIANGLE DEF
			//cout << "TRIANGLE DEF\n";
			glm::vec3 vertA = glm::vec3(0.0f, heights[gridX  + ((gridZ+1) *size)], 1.0f);
			glm::vec3 vertB = glm::vec3(1.0f, heights[(gridX + 1) + ((gridZ+1) * size)], 1.0f);
			glm::vec3 vertC = glm::vec3(1.0f, heights[(gridX + 1) + gridZ * (size)], 0.0f);
			//apply the barry centric position formula
			float det = (vertB.z - vertC.z) * (vertA.x - vertC.x) + (vertC.x - vertB.x) * (vertA.z - vertC.z);
			float aa = ((vertB.z - vertC.z) * (squareX - vertC.x) + (vertC.x - vertB.x) * (squareZ - vertC.z)) / det;
			float ab = ((vertC.z - vertA.z) * (squareX - vertC.x) + (vertA.x - vertC.x) * (squareZ - vertC.z)) / det;
			float ac = 1.0f - aa - ab;
			height = aa * vertA.y + ab * vertB.y + ac * vertC.y;

		}
		//cout << "[map.h] end of getHeight()\n";
		return height * mapscale - hMin * mapscale;



	}

	/*When we delete the map, we should make sure to free the memory by calling this function!*/
	void cleanUp() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		//NO EBO

		delete[] verticesArray;
		vertices.clear();
		heights.clear();
		//vertices.erase(vertices.begin());
		//heights.erase(heights.begin());
		
	}

	/*probably not needed-Wrapper for setupMesh*/
	void setup() {
		setupMesh();
	}
private:
	// render data 
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		
		//VBO bind calls for map
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verticesArray[0]) * verticesSize, &verticesArray[0], GL_STATIC_DRAW);
		
		
		// set the vertex attribute pointers
		
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1/*location*/, 3/*number of values*/, GL_FLOAT, GL_FALSE, 6 * sizeof(float)/*size*/, (void*)(3 * sizeof(float))/*offset*/);
		glEnableVertexAttribArray(1);


		glBindVertexArray(0);
	}

	/*calculate normals for proper Phong lighting*/
	glm::vec3 calcNormal(glm::vec3 A, glm::vec3 B, glm::vec3 C) {
		glm::vec3 normal; //return variable: the normal of this triangle

		glm::vec3 U(B.x - A.x, B.y - A.y, B.z - A.z);
		glm::vec3 V(C.x - A.x, C.y - A.y, C.z - A.z);

		normal.x = U.y * V.z - U.z * V.y;
		normal.y = U.z * V.x - U.x * V.z;
		normal.z = U.x * V.y - U.y * V.x;

		return normal;
	}
};
#endif