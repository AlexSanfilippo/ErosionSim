/*Alexaner Sanfilippo
* 30th March, 2023
* BRIEF: Generate a simple textured primative 3D shapes/objects with normals.  Include drawing, setup, and erase "mesh" functions
* TODO: Add many more shapes: Rectangle, N-sided prism (open and closed), triangular pyramid, torus, egg
*       -Add functions to "bake" rotation, and translation into the position vertices (see our L-system tree turtle code, turtle.h )
*/

#ifndef PRIMATIVE_H
#define PRIMATIVE_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include <headers/shader_s.h>

#include <string>
#include <vector>

#include <fstream>
#include <sstream>
#include <iostream>


/*Each Vertex is a point in 3D space with a normal and texture coordinate*/
struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 text;
};

/*The triangle's used in collision detection between the player and the environment*/
struct CollisionTriangle{
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
};

enum ShapeType {
    CUBE,
    PYRAMID_SQUARE,
    TRIANGLE, 
    SPHERES,
    HEMISPHERE,
    ELLIPSOID,
    EGG

};

using namespace std;
struct TextureStruct {
    unsigned int id; //an id number
    string type; //diffuse, specular, roughness, etc...
    string path; //file name and location
};

const double PI = 3.1415926535;






class Primative {
public:
    //cube properties
    float size; //length of each segment of the cube 

    glm::vec3 position = glm::vec3(0.0f, 0.0f ,0.0f); //primatives position in world space
    

    //vertices of cube as std::vector objects (Translate to arrays to draw in constructor)
    vector <float> verticesVec;
    vector <unsigned int> indicesVec;

    int verticesSize, indicesSize;
    float* vertices;
    unsigned int* indices;

    unsigned int numTriangles;

    unsigned int VAO;

    //texture within the class
    vector <TextureStruct> textures; //holds all the texture structs
    unsigned int diffuseMap;
    unsigned int specularMap;

    ShapeType shapeType;


    glm::vec2 sphereDetailLevel = glm::vec2(8,8);
    glm::vec3 ellipsoidRadii = glm::vec3(1.0,1.0,1.0);

    //a vector containing the collision triangles of this primative mesh
    vector <CollisionTriangle> collisionGeometry;

    // constructor
    //Cube(vector<float> curVertices, vector<unsigned int> curIndices)
    //creats a cube with segment size "size" at the world origin (sit upon XZ plane)
    //calls generate() to get the 24 vertices, then processes for drawing
    //Cube(float cubeSize)
    Primative(ShapeType type, float shapeSize, vector<TextureStruct> texts)
    {
        shapeType = type;
        textures = texts; 

        //temp, later use loadTexture to load path from 'textures' according to type
        //LOAD TEXTURES (generate, bind, tiling, etc)
        //diffuseMap = loadTexture("C:/Users/2015a/Documents/C++ Projects/OpenGLProjects/Include/textures/4k JPEG/Metal_Grid_01/Metal_Grid_01_basecolor.jpg");
        //specularMap = loadTexture("C:/Users/2015a/Documents/C++ Projects/OpenGLProjects/Include/textures/4k JPEG/Metal_Grid_01/Metal_Grid_01_normalOGL.jpg");
        
        for (int i = 0; i < textures.size(); i++) {
            if (textures[i].type == "diffuse") {               
                diffuseMap = loadTexture(textures[i].path);
            }
            else if (textures[i].type == "specular" || textures[i].type == "normal") {
                specularMap = loadTexture(textures[i].path);
            }
            else {
                cout << "Texture type not recognized.\n     0: diffuse\n     1:specular(normal)\n";
            }  
        }
        
        size = shapeSize;
        
        /*STEP 1: Call generate()*/
        if (shapeType == CUBE) {
            generateCube();
        }
        else if (shapeType == PYRAMID_SQUARE) {
            generatePyramidSquare();
        }
        else if (shapeType == TRIANGLE) {
            generateTriangle();
        }
        else if (shapeType == SPHERES) {
            generateSphere();
        }
        else if (shapeType == HEMISPHERE) {
            generateHemisphere();
        }
        else if (shapeType == ELLIPSOID) {
            generateEllipsoid();
        }
        else if (shapeType == EGG) {
            generateEgg();
        }
    

        //needed to draw
        numTriangles = indicesVec.size() / 3; //tells us how many to draw
   

        
        /*Translate vector into array of vertices and indices to be drawn*/
        //dynamically created array of vertices
        verticesSize = verticesVec.size();
        vertices = new float[verticesSize];
        //populate the vertices
        for (int i = 0; i < verticesSize; i++) {
            vertices[i] = verticesVec[i];

        }
        //repeat for indices
        indicesSize = indicesVec.size();
        indices = new unsigned int[indicesSize];
        //populate the indices array
        for (int i = 0; i < indicesSize; i++) {
            indices[i] = indicesVec[i];
        }
        
    }

    /*CONSTRUCTOR: specify world position*/
    Primative(ShapeType type, float shapeSize, vector<TextureStruct> texts, glm::vec3 pos)
    {
        shapeType = type;
        position = pos;

        textures = texts;

        //temp, later use loadTexture to load path from 'textures' according to type
        //LOAD TEXTURES (generate, bind, tiling, etc)
        //diffuseMap = loadTexture("C:/Users/2015a/Documents/C++ Projects/OpenGLProjects/Include/textures/4k JPEG/Metal_Grid_01/Metal_Grid_01_basecolor.jpg");
        //specularMap = loadTexture("C:/Users/2015a/Documents/C++ Projects/OpenGLProjects/Include/textures/4k JPEG/Metal_Grid_01/Metal_Grid_01_normalOGL.jpg");

        for (int i = 0; i < textures.size(); i++) {
            if (textures[i].type == "diffuse") {
                diffuseMap = loadTexture(textures[i].path);
            }
            else if (textures[i].type == "specular" || textures[i].type == "normal") {
                specularMap = loadTexture(textures[i].path);
            }
            else {
                cout << "Texture type not recognized.\n     0: diffuse\n     1:specular(normal)\n";
            }
        }

        size = shapeSize;

        /*STEP 1: Call generate()*/
        if (shapeType == CUBE) {
            generateCube();
        }
        else if (shapeType == PYRAMID_SQUARE) {
            generatePyramidSquare();
        }
        else if (shapeType == TRIANGLE) {
            generateTriangle();
        }
        else if (shapeType == SPHERES) {
            generateSphere();
        }
        else if (shapeType == HEMISPHERE) {
            generateHemisphere();
        }
        else if (shapeType == ELLIPSOID) {
            generateEllipsoid();
        }
        else if (shapeType == EGG) {
            generateEgg();
        }

        //needed to draw
        numTriangles = indicesVec.size() / 3; //tells us how many to draw



        /*Translate vector into array of vertices and indices to be drawn*/
        //dynamically created array of vertices
        verticesSize = verticesVec.size();
        vertices = new float[verticesSize];
        //populate the vertices
        for (int i = 0; i < verticesSize; i++) {
            vertices[i] = verticesVec[i];

        }
        //repeat for indices
        indicesSize = indicesVec.size();
        indices = new unsigned int[indicesSize];
        //populate the indices array
        for (int i = 0; i < indicesSize; i++) {
            indices[i] = indicesVec[i];
        }

    }


    /*CONSTRUCTOR: specity level of detail LOD (for spheroids)*/
    Primative(ShapeType type, float shapeSize, glm::vec2 LOD, glm::vec3 eradii, vector<TextureStruct> texts, glm::vec3 pos)
    {
        shapeType = type;
        position = pos;

        textures = texts;

        sphereDetailLevel = LOD;
        ellipsoidRadii = eradii;

        //temp, later use loadTexture to load path from 'textures' according to type
        //LOAD TEXTURES (generate, bind, tiling, etc)
        //diffuseMap = loadTexture("C:/Users/2015a/Documents/C++ Projects/OpenGLProjects/Include/textures/4k JPEG/Metal_Grid_01/Metal_Grid_01_basecolor.jpg");
        //specularMap = loadTexture("C:/Users/2015a/Documents/C++ Projects/OpenGLProjects/Include/textures/4k JPEG/Metal_Grid_01/Metal_Grid_01_normalOGL.jpg");

        for (int i = 0; i < textures.size(); i++) {
            if (textures[i].type == "diffuse") {
                diffuseMap = loadTexture(textures[i].path);
            }
            else if (textures[i].type == "specular" || textures[i].type == "normal") {
                specularMap = loadTexture(textures[i].path);
            }
            else {
                cout << "Texture type not recognized.\n     0: diffuse\n     1:specular(normal)\n";
            }
        }

        size = shapeSize;

        /*STEP 1: Call generate()*/
        if (shapeType == CUBE) {
            generateCube();
        }
        else if (shapeType == PYRAMID_SQUARE) {
            generatePyramidSquare();
        }
        else if (shapeType == TRIANGLE) {
            generateTriangle();
        }
        else if (shapeType == SPHERES) {
            generateSphere();
        }
        else if (shapeType == HEMISPHERE) {
            generateHemisphere();
        }
        else if (shapeType == ELLIPSOID) {
            generateEllipsoid();
        }
        else if (shapeType == EGG) {
            generateEgg();
        }
        //needed to draw
        numTriangles = indicesVec.size() / 3; //tells us how many to draw



        /*Translate vector into array of vertices and indices to be drawn*/
        //dynamically created array of vertices
        verticesSize = verticesVec.size();
        vertices = new float[verticesSize];
        //populate the vertices
        for (int i = 0; i < verticesSize; i++) {
            vertices[i] = verticesVec[i];

        }
        //repeat for indices
        indicesSize = indicesVec.size();
        indices = new unsigned int[indicesSize];
        //populate the indices array
        for (int i = 0; i < indicesSize; i++) {
            indices[i] = indicesVec[i];
        }

    }

    /*Copy Constructor*/
    /*
    Primative(const Primative &p) {
        cout << "[Primative.h]   CALLED CPY CON\n";
        //COPY MEMBERS
        this->shapeType = p.shapeType;
        this->position = p.position;
        this->textures = p.textures;

        //COPY VERTICES
        this->verticesSize = p.verticesVec.size();
        vertices = new float[this->verticesSize];
        //populate the vertices
        cout << "cpy v\n";
        for (int i = 0; i < verticesSize; i++) {
            //cout << "i=" << i << endl;
            this->verticesVec.push_back(p.verticesVec[i]);
            this ->vertices[i] = p.verticesVec[i];
        }

        //COPY INDICES
        cout << "cpy i\n";
        this ->indicesSize = p.indicesVec.size();
        indices = new unsigned int[this->indicesSize];
        //populate the indices array
        for (int i = 0; i < indicesSize; i++) {
            this->indicesVec.push_back(p.indicesVec[i]);
            this->indices[i] = p.indicesVec[i];
        }
        cout << "[Primative.h]  END CALLED CPY CON\n";
    }
    */

    /*DEFAULT CONSTRUCTOR*/
    Primative() {

    }
    void PrimativeCopier(const Primative* p, Primative *pNew) {
        cout << "[Primative.h]   CALLED CPY CON\n";
        //COPY MEMBERS
        pNew->shapeType = p->shapeType;
        pNew->position = p->position;
        pNew->textures = p->textures;


        //TEXTURE SETUP
        for (int i = 0; i < textures.size(); i++) {
            if (textures[i].type == "diffuse") {
                diffuseMap = loadTexture(textures[i].path);
            }
            else if (textures[i].type == "specular" || textures[i].type == "normal") {
                specularMap = loadTexture(textures[i].path);
            }
            else {
                cout << "Texture type not recognized.\n     0: diffuse\n     1:specular(normal)\n";
            }
        }

        //COPY VERTICES
        pNew->verticesSize = p->verticesVec.size();
        vertices = new float[pNew->verticesSize];
        //populate the vertices
        cout << "cpy v\n";
        for (int i = 0; i < verticesSize; i++) {
            //cout << "i=" << i << endl;
            pNew->verticesVec.push_back(p->verticesVec[i]);
            pNew->vertices[i] = p->verticesVec[i];
        }

        //COPY INDICES
        cout << "cpy i\n";
        pNew->indicesSize = p->indicesVec.size();
        indices = new unsigned int[pNew->indicesSize];
        //populate the indices array
        for (int i = 0; i < indicesSize; i++) {
            pNew->indicesVec.push_back(p->indicesVec[i]);
            pNew->indices[i] = p->indicesVec[i];
        }

        numTriangles = indicesVec.size() / 3; //Cannot draw without this information

        cout << "[Primative.h]  END CALLED CPY CON\n";
    }
    void readVertices() {
        for (int i = 0; i < verticesVec.size(); i++) {
            if (i % 8 == 0) {
                cout << "\n";
            }
            cout << vertices[i] << ", ";           
        }
    }

    /*MODEL GENERATOR: generates position, normal, and tex coord values of the cube model*/
    void generateCube() {
        //cout << "[cube] in cube generate\n";

        

       
        float halfSize = size * 0.5;
        
        //create 8 vertices (place into mesh multiple times to get 24 vertices)

        vector <Vertex> basicVertices = {Vertex(), Vertex(), Vertex(), Vertex(), Vertex(), 
                                    Vertex(), Vertex(), Vertex()}; //a,b,c,d,a2,b2,c2,d2

     
        //fill in position info
        basicVertices[0].pos = glm::vec3(halfSize, -halfSize, -halfSize);
        basicVertices[1].pos = glm::vec3(halfSize, -halfSize, halfSize);
        basicVertices[2].pos = glm::vec3(-halfSize, -halfSize, halfSize);
        basicVertices[3].pos = glm::vec3(-halfSize, -halfSize, -halfSize);
        basicVertices[4].pos = glm::vec3(halfSize, halfSize, -halfSize);
        basicVertices[5].pos = glm::vec3(halfSize, halfSize, halfSize);
        basicVertices[6].pos = glm::vec3(-halfSize, halfSize, halfSize);
        basicVertices[7].pos = glm::vec3(-halfSize, halfSize, -halfSize);

     
        
      

        /*Create the 24 vertices*/
        //store the 6 vertices of each face seperately... compose at end
        
        //each face has 4 vertices
        vector <Vertex> yn = { Vertex(), Vertex(), Vertex(), Vertex() };
        vector <Vertex> yp = { Vertex(), Vertex(), Vertex(), Vertex() };
        vector <Vertex> xn = { Vertex(), Vertex(), Vertex(), Vertex() };
        vector <Vertex> xp = { Vertex(), Vertex(), Vertex(), Vertex() };
        vector <Vertex> zn = { Vertex(), Vertex(), Vertex(), Vertex() };
        vector <Vertex> zp = { Vertex(), Vertex(), Vertex(), Vertex() };
        
        //vector <Vertex> yn, yp, xn, xp, zn, zp;
        vector <vector <Vertex> > faces = {yn, yp, xn, xp, zn, zp}; //  p/n = plus, negative axis
        
        

        //NEW method of assign position to Vertex
        //face yn
        faces[0][0] = basicVertices[1];      
        faces[0][1] = basicVertices[2];       
        faces[0][2] = basicVertices[3];       
        faces[0][3] = basicVertices[0];
        //face yp     
        faces[1][0] = basicVertices[4];        
        faces[1][1] = basicVertices[7];       
        faces[1][2] = basicVertices[6];        
        faces[1][3] = basicVertices[5];
        //xn       
        faces[2][0] = basicVertices[6];
        faces[2][1] = basicVertices[7];
        faces[2][2] = basicVertices[3];
        faces[2][3] = basicVertices[2];
        //xp
        faces[3][0] = basicVertices[4];
        faces[3][1] = basicVertices[5];
        faces[3][2] = basicVertices[1];
        faces[3][3] = basicVertices[0];
        //zn
        faces[4][0] = basicVertices[7];
        faces[4][1] = basicVertices[4];
        faces[4][2] = basicVertices[0];
        faces[4][3] = basicVertices[3];
        //zp
        faces[5][0] = basicVertices[5];
        faces[5][1] = basicVertices[6];
        faces[5][2] = basicVertices[2];
        faces[5][3] = basicVertices[1];

        //assign texture coordinates
        for (int i = 0; i < 6; i++) { //for each face
           
            faces[i][0].text = glm::vec2(0, 0);
            faces[i][1].text = glm::vec2(1, 0);
            faces[i][2].text = glm::vec2(1, 1);
            faces[i][3].text = glm::vec2(0, 1);
            
        }
     

        //calculate normals
        for (int i = 0; i < 6; i++) { //for each face
            for (int j = 0; j < 4; j++) {

                //grab two other points to calcuate normal of this point
                int right = j - 1;
                int left = j + 1;
                if (right < 0) {
                    right = 3;
                }
                if (left > 3) {
                    left = 0;
                }
                
                //calculate and assign the normal value of this vertex
                faces[i][j].norm = calcNormal(faces[i][j].pos, faces[i][right].pos, faces[i][left].pos);
                                
            }
        }
        
        /*Final Step: Translate faces into verticesVec and indicesVec*/
        for (int i = 0; i < 6; i++) { //for each face
            for (int j = 0; j < 4; j++) {
                
                //POS
                for (int k = 0; k < 3; k++) {
                    verticesVec.push_back(faces[i][j].pos[k]);                   
                }
                //NORMAL
                for (int k = 0; k < 3; k++) {
                    verticesVec.push_back(faces[i][j].norm[k]);
                }
                //TEXTURE COORDS
                for (int k = 0; k < 2; k++) {
                    verticesVec.push_back(faces[i][j].text[k]);
                }
                //indices
                if (j != 2) {
                    indicesVec.push_back(j + (i * 4)); 
                }
                else {
                    indicesVec.push_back(j + (i * 4));
                    indicesVec.push_back(j + (i * 4));
                }
            }
            indicesVec.push_back(0 + (i * 4)); //final indices (0th vertex of this face)
        }

        /*generate collision geometry (make conditional later on)*/
        
        
    }

    /*MODEL GENERATOR: generates position, normal, and tex coord values of the square pyramid model--COMPLETED*/
    void generatePyramidSquare() {
       


        float halfSize = size * 0.5;

        //create 8 vertices (place into mesh multiple times to get 24 vertices)

        vector <Vertex> basicVertices = { Vertex(), Vertex(), Vertex(), Vertex(), Vertex()}; //a,b,c,d,e


        //fill in position info
        basicVertices[0].pos = glm::vec3(halfSize, -halfSize, -halfSize);
        basicVertices[1].pos = glm::vec3(halfSize, -halfSize, halfSize);
        basicVertices[2].pos = glm::vec3(-halfSize, -halfSize, halfSize);
        basicVertices[3].pos = glm::vec3(-halfSize, -halfSize, -halfSize);
        basicVertices[4].pos = glm::vec3(0.0f, halfSize, 0.0f); //E
       
       


        /*Create the 16 vertices*/
        //store the vertices of each face seperately... compose at end

        //each face has 4 vertices
        vector <Vertex> xn = { Vertex(), Vertex(), Vertex() };
        vector <Vertex> xp = { Vertex(), Vertex(), Vertex() };
        vector <Vertex> zn = { Vertex(), Vertex(), Vertex() };
        vector <Vertex> zp = { Vertex(), Vertex(), Vertex() };
        vector <Vertex> base = { Vertex(), Vertex(), Vertex(), Vertex() };

        //vector <Vertex> yn, yp, xn, xp, zn, zp;
        vector <vector <Vertex> > faces = { xn, xp, zn, zp, base }; //  p/n = plus, negative axis
       

        //assign position to Vertex
        
        //xn       
        faces[0][0] = basicVertices[2];
        faces[0][1] = basicVertices[4];
        faces[0][2] = basicVertices[3];
      
        //xp
        faces[1][0] = basicVertices[0];
        faces[1][1] = basicVertices[4];
        faces[1][2] = basicVertices[1];
      
        //zn
        faces[2][0] = basicVertices[3];
        faces[2][1] = basicVertices[4];
        faces[2][2] = basicVertices[0];
   
        //zp
        faces[3][0] = basicVertices[1];
        faces[3][1] = basicVertices[4];
        faces[3][2] = basicVertices[2];
        //base
        faces[4][0] = basicVertices[0];
        faces[4][1] = basicVertices[1];
        faces[4][2] = basicVertices[2];
        faces[4][3] = basicVertices[3];
    
    
        //assign texture coordinates
        for (int i = 0; i < 4; i++) { //for each triangular face

            faces[i][0].text = glm::vec2(0, 0);
            faces[i][1].text = glm::vec2(0.5, 1);
            faces[i][2].text = glm::vec2(1, 0);
        }
        //BASE
        faces[4][0].text = glm::vec2(0, 0);
        faces[4][1].text = glm::vec2(1, 0);
        faces[4][2].text = glm::vec2(1, 1);
        faces[4][3].text = glm::vec2(0, 1);
       

        //calculate normals
        for (int i = 0; i < faces.size(); i++) { //for each face
            int faceSize = faces[i].size();
           
            for (int j = 0; j < faceSize; j++) {
                
                //grab two other points to calcuate normal of this point
                int right = j - 1;
                int left = j + 1;
                if (right < 0) {
                    right = faceSize-1;
                }
                if (left > faceSize-1) {
                    left = 0;
                }

                //calculate and assign the normal value of this vertex
                faces[i][j].norm = calcNormal(faces[i][j].pos, faces[i][right].pos, faces[i][left].pos);

            }
        }
       
        /*Final Step: Translate faces into verticesVec and indicesVec*/
        for (int i = 0; i < 4; i++) { //for each triangle face
            //int faceSize = faces[i].size();
            for (int j = 0; j < 3; j++) { //for each triangle face's vertices

                //POS
                for (int k = 0; k < 3; k++) {
                    verticesVec.push_back(faces[i][j].pos[k]);
                }
                //NORMAL
                for (int k = 0; k < 3; k++) {
                    verticesVec.push_back(faces[i][j].norm[k]);
                }
                //TEXTURE COORDS
                for (int k = 0; k < 2; k++) {
                    verticesVec.push_back(faces[i][j].text[k]);
                }

                indicesVec.push_back(j + (i * 3)); //all triangle indices
                    
            }
            //indicesVec.push_back(0 + (i * 3)); //final indices (0th vertex of this face)  -- not sure if needed
        }
     
        //base vertices
        for (int j = 0; j < 4; j++) {
            int i = 4;
            //POS
            for (int k = 0; k < 3; k++) {
                verticesVec.push_back(faces[i][j].pos[k]);
            }
            //NORMAL
            for (int k = 0; k < 3; k++) {
                verticesVec.push_back(faces[i][j].norm[k]);
            }
            //TEXTURE COORDS
            for (int k = 0; k < 2; k++) {
                verticesVec.push_back(faces[i][j].text[k]);
            }
        }
     
        //base indices
        indicesVec.push_back(13);
        indicesVec.push_back(14);
        indicesVec.push_back(15);
        indicesVec.push_back(15);
        indicesVec.push_back(12);
        indicesVec.push_back(13);

        


    }
    /*MODEL GENERATOR: generates position, normal, and tex coord values of the square pyramid model--COMPLETE*/
    void generateTriangle() {




        float halfSize = size * 0.5; //radius

        //create 3 vertices (only 3)

        vector <Vertex> basicVertices = { Vertex(), Vertex(), Vertex()}; //a,b,c,d,e


        //fill in position info
        

        //RADIANS -- correct
        
        basicVertices[0].pos = glm::vec3(halfSize * cos(2.0*PI/4.0) + position.x, 0.0f + position.y, halfSize * sin(2.0 * PI / 4.0) + position.z);
        basicVertices[2].pos = glm::vec3(halfSize  * cos(11.0*PI / 6.0) + position.x, 0.0f + position.y, halfSize  * sin(11.0*PI / 6.0) + position.z);
        basicVertices[1].pos = glm::vec3(halfSize * cos(7.0 * PI / 6.0) + position.x, 0.0f + position.y, halfSize * sin(7.0 * PI / 6.0) + position.z);
        //DEGREES
        /*
        basicVertices[0].pos = glm::vec3(cos(0.0), 0.0f, sin(0.0));
        basicVertices[1].pos = glm::vec3(halfSize * cos(120), 0.0f, halfSize * sin(120));
        basicVertices[2].pos = glm::vec3(halfSize  * cos(240), 0.0f, halfSize * sin(240));
        */

        /*Create the 16 vertices*/
        //store the vertices of each face seperately... compose at end

        //1 face, 3 vertices
        vector <Vertex> onlyFace = { Vertex(), Vertex(), Vertex() };
       

        //vector <Vertex> 
        vector <vector <Vertex> > faces = { onlyFace }; 


        //assign position to Vertex

        //onlyFace      
        faces[0][0] = basicVertices[0];
        faces[0][1] = basicVertices[1];
        faces[0][2] = basicVertices[2];


        //assign texture coordinates
        faces[0][0].text = glm::vec2(0.5, 1.0);
        faces[0][1].text = glm::vec2(0, 0);
        faces[0][2].text = glm::vec2(1, 0);
     

        //calculate normals
        for (int i = 0; i < faces.size(); i++) { //for each face
            int faceSize = faces[i].size();
            cout << "i=" << i << endl;
            for (int j = 0; j < faceSize; j++) {
                cout << "j=" << j << endl;
                //grab two other points to calcuate normal of this point
                int right = j - 1;
                int left = j + 1;
                if (right < 0) {
                    right = faceSize - 1;
                }
                if (left > faceSize - 1) {
                    left = 0;
                }

                //calculate and assign the normal value of this vertex
                faces[i][j].norm = calcNormal(faces[i][j].pos, faces[i][right].pos, faces[i][left].pos);

            }
        }
  
        /*Final Step: Translate faces into verticesVec and indicesVec*/
        for (int i = 0; i < 1; i++) { //for each triangle face
            //int faceSize = faces[i].size();
            CollisionTriangle colTri;
            for (int j = 0; j < 3; j++) { //for each triangle face's vertices

                //POS
                
                for (int k = 0; k < 3; k++) {
                    verticesVec.push_back(faces[i][j].pos[k]);
                    colTri.p1[k] = faces[i][j].pos[k];
                }
                //NORMAL
                for (int k = 0; k < 3; k++) {
                    verticesVec.push_back(faces[i][j].norm[k]);
                }
                //TEXTURE COORDS
                for (int k = 0; k < 2; k++) {
                    verticesVec.push_back(faces[i][j].text[k]);
                }

                indicesVec.push_back(j + (i * 3)); //all triangle indices

            }
            collisionGeometry.emplace_back(colTri);
            //indicesVec.push_back(0 + (i * 3)); //final indices (0th vertex of this face)  -- not sure if needed
        }
    }

    /*MODEL GENERATOR: generates position, normal, and tex coord values of the square pyramid model--WIP*/
    void generateSphere() {
        cout << "generateSphere()\n";
        float radius = size * 0.5; //radius


        //get these from constructor later on
        int stackCount = sphereDetailLevel[0];
        int sectorCount = sphereDetailLevel[1];

        //int verticesCount = stackCount * sectorCount;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;

        //hemisphere attempt
        //sectorStep /= 2;


        float sectorAngle, stackAngle;
        float  lengthInv = 1.0f / radius;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz;    // vertex normal
        float s, t;                                     // vertex texCoord

        /*Create the 16 vertices*/
        //store the vertices of each face seperately... compose at end



        for (int i = 0; i <= stackCount; i++) {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle) + position.z;              // r * sin(u)
            for (int j = 0; j <= sectorCount; j++) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                 // vertex position (x, y, z)
                x = xy * cosf(sectorAngle) + position.x;             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle) + position.y;             // r * cos(u) * sin(v)
                verticesVec.push_back(x);
                verticesVec.push_back(y);
                verticesVec.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                verticesVec.push_back(nx);
                verticesVec.push_back(ny);
                verticesVec.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                verticesVec.push_back(s);
                verticesVec.push_back(t);
            }

        }


        /*Generate Indices*/

        int k1, k2;

        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    indicesVec.push_back(k1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1))
                {
                    indicesVec.push_back(k1 + 1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                //indicesVec.push_back(k1);
                //indicesVec.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    //indicesVec.push_back(k1);
                    //indicesVec.push_back(k1 + 1);
                }
            }
        }
    }

    /*MODEL GENERATOR: generates position, normal, and tex coord values of the Ellipsoid model--COMPLETE*/
    void generateEllipsoid() {
        
        //float radius = size * 0.5; //radius

        ellipsoidRadii.x *= size;
        ellipsoidRadii.y *= size;
        ellipsoidRadii.z *= size;

        //get these from constructor later on
        int stackCount = sphereDetailLevel[0];
        int sectorCount = sphereDetailLevel[1];

        //int verticesCount = stackCount * sectorCount;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;

        //hemisphere attempt
        //sectorStep /= 2;


        float sectorAngle, stackAngle;
        float  lengthInvX = 1.0f / ellipsoidRadii.x;
        float  lengthInvY = 1.0f / ellipsoidRadii.y;
        float  lengthInvZ = 1.0f / ellipsoidRadii.z;

        float x, y, z, x1, y1;                              // vertex position
        float nx, ny, nz;    // vertex normal
        float s, t;                                     // vertex texCoord

        /*Create the 16 vertices*/
        //store the vertices of each face seperately... compose at end



        for (int i = 0; i <= stackCount; i++) {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            //xy = ellipsoidRadii.x * cosf(stackAngle);   // r * cos(u)   //MAY NEED TO BREAK THIS UP
            x1 = ellipsoidRadii.x * cosf(stackAngle);
            y1 = ellipsoidRadii.y * cosf(stackAngle);   
            z = ellipsoidRadii.z * sinf(stackAngle) + position.z;              // r * sin(u)
            for (int j = 0; j <= sectorCount; j++) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                 // vertex position (x, y, z)
                x = x1 * cosf(sectorAngle) + position.x;             // r * cos(u) * cos(v)
                y = y1 * sinf(sectorAngle) + position.y;             // r * cos(u) * sin(v)
                verticesVec.push_back(x);
                verticesVec.push_back(y);
                verticesVec.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInvX;
                ny = y * lengthInvY;
                nz = z * lengthInvZ;
                verticesVec.push_back(nx);
                verticesVec.push_back(ny);
                verticesVec.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                verticesVec.push_back(s);
                verticesVec.push_back(t);
            }

        }


        /*Generate Indices*/

        int k1, k2;

        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    indicesVec.push_back(k1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1))
                {
                    indicesVec.push_back(k1 + 1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                //indicesVec.push_back(k1);
                //indicesVec.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    //indicesVec.push_back(k1);
                    //indicesVec.push_back(k1 + 1);
                }
            }
        }
    }

    /*MODEL GENERATOR: generates position, normal, and tex coord values of the Egg model--WIP*/
    void generateEgg() {

        //float radius = size * 0.5; //radius

        ellipsoidRadii.x = size;
        ellipsoidRadii.y = size;
        ellipsoidRadii.z = size * 1.675f;

        //float zRadiusB = ellipsoidRadii.z * 0.650f; //.5 may not be ideal, play with
        float zRadiusB = ellipsoidRadii.x;

        //get these from constructor later on
        int stackCount = sphereDetailLevel[0];
        int sectorCount = sphereDetailLevel[1];

        //int verticesCount = stackCount * sectorCount;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;

        //hemisphere attempt
        //sectorStep /= 2;


        float sectorAngle, stackAngle;
        float  lengthInvX = 1.0f / ellipsoidRadii.x;
        float  lengthInvY = 1.0f / ellipsoidRadii.y;
        float  lengthInvZ = 1.0f / ellipsoidRadii.z;

        float x, y, z, x1, y1;                              // vertex position
        float nx, ny, nz;    // vertex normal
        float s, t;                                     // vertex texCoord

        /*Create the 16 vertices*/
        //store the vertices of each face seperately... compose at end



        for (int i = 0; i <= stackCount; i++) {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            //xy = ellipsoidRadii.x * cosf(stackAngle);   // r * cos(u)   //MAY NEED TO BREAK THIS UP
            x1 = ellipsoidRadii.x * cosf(stackAngle);
            y1 = ellipsoidRadii.y * cosf(stackAngle);
            if (i > stackCount / 2) {
                z = zRadiusB * sinf(stackAngle) + position.z;
            }
            else {
                z = ellipsoidRadii.z * sinf(stackAngle) + position.z; // r * sin(u)
            }             
            for (int j = 0; j <= sectorCount; j++) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                 // vertex position (x, y, z)
                x = x1 * cosf(sectorAngle) + position.x;             // r * cos(u) * cos(v)
                y = y1 * sinf(sectorAngle) + position.y;             // r * cos(u) * sin(v)
                verticesVec.push_back(x);
                verticesVec.push_back(y);
                verticesVec.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInvX;
                ny = y * lengthInvY;
                nz = z * lengthInvZ;
                verticesVec.push_back(nx);
                verticesVec.push_back(ny);
                verticesVec.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                verticesVec.push_back(s);
                verticesVec.push_back(t);
            }

        }


        /*Generate Indices*/

        int k1, k2;

        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    indicesVec.push_back(k1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1))
                {
                    indicesVec.push_back(k1 + 1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                //indicesVec.push_back(k1);
                //indicesVec.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    //indicesVec.push_back(k1);
                    //indicesVec.push_back(k1 + 1);
                }
            }
        }
    }

    /*MODEL GENERATOR: generates position, normal, and tex coord values of the square pyramid model--WIP*/
    //need way of specifying north hemisphere
    void generateHemisphere() {
        
        float radius = size * 0.5; //radius


        //get these from constructor later on
        int stackCount = sphereDetailLevel[0];
        int sectorCount = sphereDetailLevel[1];

        //int verticesCount = stackCount * sectorCount;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;

        //hemisphere 
        stackStep /= 2;


        float sectorAngle, stackAngle;
        float  lengthInv = 1.0f / radius;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz;    // vertex normal
        float s, t;                                     // vertex texCoord

        /*Create the 16 vertices*/
        //store the vertices of each face seperately... compose at end



        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle) + position.z;              // r * sin(u)
            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                    // vertex position (x, y, z)
                x = xy * cosf(sectorAngle) + position.x;             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle) + position.y;             // r * cos(u) * sin(v)
                verticesVec.push_back(x);
                verticesVec.push_back(y);
                verticesVec.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                verticesVec.push_back(nx);
                verticesVec.push_back(ny);
                verticesVec.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                verticesVec.push_back(s);
                verticesVec.push_back(t);
            }

        }


        /*Generate Indices*/

        int k1, k2;

        for (int i = 0; i < stackCount; i++)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; j++, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    indicesVec.push_back(k1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount))
                {
                    indicesVec.push_back(k1 + 1);
                    indicesVec.push_back(k2);
                    indicesVec.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                //indicesVec.push_back(k1);
                //indicesVec.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    //indicesVec.push_back(k1);
                    //indicesVec.push_back(k1 + 1);
                }
            }
        }
    }
    /*Debug utility function -- returns the number of vertices that the plant mesh has received from turtle*/
    int pingNumVertices() {

        return verticesVec.size();
    }

    // render the mesh
    void draw(Shader& shader)
    {
        //bind textures for this cube
        
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);


        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6 * numTriangles /*num vertices*/, GL_UNSIGNED_INT, 0); //fromOldTurtle source
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

    /*ATTEMPT TO FIX MEMORY LEAK: */
    void cleanUp() {
        //HUGE memory leak fixed by cleaning up these buffers!
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        //large memory leak cleaned up by deleting these vertices/indices, but  all leaks are not fixed 
        delete[] vertices;
        delete[] indices;
        //cout << "[PlantMesh] clean Up called...\n";
    }

    //private:
        // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); //from mesh.h
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * verticesSize, &vertices[0], GL_STATIC_DRAW); //from old turtle.cpp
        //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesSize, &indices[0], GL_STATIC_DRAW);//from old turtle.cpp

        // set the vertex attribute pointers
        // vertex Positions
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT/*type*/, GL_FALSE, 8 * sizeof(float)/*stride was 3*/, (void*)0)/*offset*/;
        glEnableVertexAttribArray(0);
        //normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        //texture
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    /*"wrapper" fxn just to keep nomenclature similar*/
    void setup() {
        setupMesh();
    }

    /*Returns a pointer to the array of vertices*/
    /*
    vector <float> getVertices() {
        return vertices;
    }
    */
    /*Returns a pointer to the array of indices*/
    /*
    vector <unsigned int> getIndices() {
        return indices;
    }
    */

    
    //Move the primative to these vertices
    void translateVertices(glm::vec3 nextPos) {
        position = nextPos;
        for (int i = 0; i < sizeof(vertices)/sizeof(float)/8; i++) { //for each vertex
            for (int j = 0; j < 3; j++) { //for the 3 position floats
                vertices[i * 8 + j] += nextPos[j];
            }            
        }
    }

    /*uses method from our turtle1_4.h to bake transformations into mesh vertices*/
    void bakeTransformations(glm::vec3 newPosition, float newRotation /*in degrees*/, glm::vec3 axis, float scale = 1.0f) {
        cout << "in bake tranformations\n";
        int numVerts = verticesVec.size()/8;  //orig
        //int numVerts = sizeof(verticesVec) / 8;
        cout << "sizeof(verticesVec)" << sizeof(verticesVec) << endl;
        cout << "verticesVec.size() " << verticesVec.size() << endl;
        cout << "numVerts = " << numVerts << endl;
        
        //2. Translation of vertices
        for (int i = 0; i < numVerts; i++) { //for each vertex
            //for (int j = 0; j < 3; j++) { //for each x,y,z position vertex
            glm::vec4 curPoint = glm::vec4(vertices[i * 8], vertices[i * 8 + 1], vertices[i * 8 + 2], 1.0f); //current point

            
            glm::mat4 trans = glm::mat4(1.0f); //translation matrix
            //1. scale - not working
            trans = glm::scale(trans, glm::vec3(scale, scale, scale));         
            curPoint = trans * curPoint;
            
            //2. rotation
            trans = glm::mat4(1.0f);
            trans = glm::rotate(trans, glm::radians(newRotation), axis); //X
            curPoint = trans * curPoint;
            
            //3. translation (move)
            for (int j = 0; j < 3; j++) {
                curPoint[j] += newPosition[j];  
            }
               
            //4. overwrite original position vertex info with new info
            for (int j = 0; j < 3; j++) {
                //cout << "vertices[i * 8 + j] was" << vertices[i * 8 + j] << "and is now " << curPoint[j] << endl;
                vertices[i * 8 + j] = curPoint[j];
                //update collision geometry too
                //for triangle primative (proof of concept) //need a more robust system for any primative--rm later
                /*
                if(i == 0)
                    collisionGeometry[0].p1[j] = curPoint[j];
                if (i == 1)
                    collisionGeometry[0].p2[j] = curPoint[j];
                if (i == 2)
                    collisionGeometry[0].p3[j] = curPoint[j];
                    */
            }
            
           
        }
        updateCollisionGeometry(); //update collision geometry
        
    }
    
    /*create the initial collision geometry for a new mesh*/
    void createCollisionGeometry() {
        cout << "[createCollisionGeometry] starting\n";
        //update collision geometry using indices
        CollisionTriangle colTri;
        for (int i = 0; i < indicesVec.size(); i++) { //for each triangle vertex
            if (i % 3 == 0) {
                collisionGeometry.push_back(colTri);
                for (int j = 0; j < 3; j++) { //for each 3d coordate of a vertex
                //update each vertex of each collision triangle in this mesh
                    //cout << "i=" << i << ", j=" << j << endl;
                    collisionGeometry[floor(i / 3)].p1[j] = vertices[indicesVec[i] * 8 + j];
                }
            }
            if (i % 3 == 1) {
                for (int j = 0; j < 3; j++) {
                    //cout << "i=" << i << ", j=" << j << endl;
                    collisionGeometry[floor(i / 3)].p2[j] = vertices[indicesVec[i] * 8 + j];
                }
            }
            if (i % 3 == 2) {
                for (int j = 0; j < 3; j++) {
                   // cout << "i=" << i << ", j=" << j << endl;
                    collisionGeometry[floor(i / 3)].p3[j] = vertices[indicesVec[i] * 8 + j];
                }
            }
        }
    }



private:
    /*Uses the indicesVec to generate the collision triangles for a mesh.  Call whenever we move the mesh to update,
    and at the initialization of the collision geometry*/
    void updateCollisionGeometry() {
        cout << "[updateCollisionGeometry] starting\n";
        //update collision geometry using indices
        for (int i = 0; i < indicesVec.size(); i++) { //for each triangle vertex
            if (i % 3 == 0) {
                for (int j = 0; j < 3; j++) { //for each 3d coordate of a vertex
                //update each vertex of each collision triangle in this mesh
                    //cout << "i=" << i << ", j=" << j << endl;
                    collisionGeometry[floor(i / 3)].p1[j] = vertices[indicesVec[i] * 8 + j];
                }
            }
            if (i % 3 == 1) {
                for (int j = 0; j < 3; j++) {
                    //cout << "i=" << i << ", j=" << j << endl;
                    collisionGeometry[floor(i / 3)].p2[j] = vertices[indicesVec[i] * 8 + j];
                }
            }
            if (i % 3 == 2) {
                for (int j = 0; j < 3; j++) {
                    //cout << "i=" << i << ", j=" << j << endl;
                    collisionGeometry[floor(i / 3)].p3[j] = vertices[indicesVec[i] * 8 + j];
                }
            }
        }
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

    // utility function for loading a 2D texture from file
// ---------------------------------------------------
    unsigned int loadTexture(char const* path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    unsigned int loadTexture(std::string path)
    {
        

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
};
#endif
