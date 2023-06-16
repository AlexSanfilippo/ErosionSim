/*
* Current Issues: 
*               1) Deposition Either not occuring or not occuring as expected
*                   -land coming out with many "pock-marks"
*                   -axis-alligned erosion patterns
*                   -no deposition at bottom of hills/cliffs as shows in paper's images/videos
*               2) code needs to be cleaned up and properly commented
*               3) write up on github
*               4) videos to youtube
*               Possible causes:
*                   a) in shader 3.4, sediment transportation, we are not reading/writing to file correct.  must use same
*                       method as used in normal shader 
*               
*Date:29 Jan, 2023
*Author: Alexander Sanfilippo
* Brief: This file contains the main function for my first attempt at a Hydraulic Erosion Simulation.  The Sim 
* is based on the paper [paper name and author], and used OpenGL's compute shaders to read and write to/from
* textures.  
*/


/*TRING TO MAKE CHANGES SHOW UP ON GITHUB*/


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <local_headers/shader_s.h>

#include <iostream>

/*Add Matric Algebra Functions*/
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

/*Texture Loading*/
#include "headers/stb_image.h"

/*Camera*/
#include <headers/camera.h>

/*Map*/
#include <local_headers/map.h>

/*Compute Shader*/
#include <local_headers/shader_c.h>

//function headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);




// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
/*[CAMERA] Create Camera Object*/
Camera ourCam = Camera(glm::vec3(0.0f, 130.0f, 35.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;



//turn on rotating camera mode
bool rotateCamera = false;
//change camera rotation radius in-program UP and DOWN keys
float radius = 180.0f;






//MAP
float mapScale = 40.0;
float mapScaleY = mapScale*4.0f;
bool changeMap = false;
bool freqUp = false;
bool freqDown = false;

//EROSION SIM CONTROLS
bool pauseSim = true;
bool evaporate = false; //control whether shader 3.5 is called or not during main loop
int vsMode = 0; //vertex shader mode. 0 for default. 1 for flat. 2 for velocity, 3 for sediment


struct mapStruct
{
    glm::vec3 pos;
    float pad0;
    glm::vec3 norm;
    float pad1;
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Map Generator V0.1 by Alexander Sanfilippo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //for camera-mouse input
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //discrete key press functionality 
    glfwSetKeyCallback(window, key_callback);
    

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }




    /*=======================Create Compute Shader========================*/

    //construct compute shader object using our class
    ComputeShader ourComputeShader3_1("4.6.shader3.1.cs");
    ComputeShader ourComputeShader3_2("4.6.shader3.2.cs");
    ComputeShader ourComputeShader3_2b("4.6.shader3.2.b.cs");
    ComputeShader ourComputeShader3_2c("4.6.shader3.2.c.cs");
    ComputeShader ourComputeShader3_3("4.6.shader3.3.cs");
    ComputeShader ourComputeShader3_4("4.6.shader3.4.cs");
    ComputeShader ourComputeShader3_5("4.6.shader3.5.cs");
    ComputeShader ourComputeShaderNormals("4.6.shaderNormals.cs");
    


    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("3.3.shader.vs", "3.3.shader.fs");
    
    
    /*Map Properties*/
    unsigned int size = 128; //resolution, 
    unsigned int octaves = 2; //LOWEST = 1
    float smooth = 3.5; //higher -> bumpier.  closer to 0 -> flatter
    int seed = 1245; //2000  //10366 //1998 for reddit demo (1999 is nice too) //12478 nice lake //1245 nice river
    unsigned int frequency = 3; //cannot be under 2
    int numMapVertices = size * size * 6;
    float scale = 5.25f; //stretch map out over XZ plane while perserving height

    bool getUserInput = false;
    if (getUserInput) {
        cout << "ENTER SIZE: ";
        cin >> size;
        cout << "ENTER : OCTAVES";
        cin >> octaves;
        cout << "ENTER : SEED";
        cin >> seed;
        cout << "\n~~~Generating Map...~~~\n";
    }
    
    Map map = Map(seed, size, octaves, frequency, smooth, scale);
   
    //Wireframe or fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_FILL FOR SOLID POLYGONS GL_LINE FOR WIREFRAME

    //change provoking vertex
    //glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    //SSBO STUFF: Create & Fill a vector of structs to send to the SSBO in the compute shader
    mapStruct* mapVertices = new mapStruct[numMapVertices];
    int verticesCount = 0;
    float* vertices = new float[numMapVertices]; //just heights
    for (int i = 0; i < numMapVertices; i++) {
        vector <float> vertexData = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
        for (int j = 0; j < 6; j++) {
            vertexData[j] = sin(i)*10.f;//map.verticesArray[i / 6 + j]; //VERY unsure this is correct
        }
        if (i < 40) {
            //cout << "vertexData[1] = " << vertexData[1] << endl;
        }
        mapVertices[i].pos.x = vertexData[0];
        mapVertices[i].pos.y = vertexData[1]; //  1000*sin(i);
        mapVertices[i].pos.z = vertexData[2];
        mapVertices[i].norm.x = vertexData[3];
        mapVertices[i].norm.y = vertexData[4];
        mapVertices[i].norm.z = vertexData[5];
        if (i < 40) {
            //cout << "mapVertices[i].pos.y = " << mapVertices[i].pos.y << endl;
        }
        vertices[0] = map.verticesArray[i];
    }

    
    
    float* justHeights = new float[size*size]; //just heights
    float* initVel = new float[size * size]; //initial velocity set to 0
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            
            //actual height map
            justHeights[i * size + j] = map.heights[i * size + j]; //normalized height values [0,1]
            //bowl map
            //justHeights[i * size + j] = sqrt(pow(float(i) - float(size)/2.0f,2) + pow(float(j) - float(size) / 2.0f, 2))/64.f;
            //bowl map-more round
            //justHeights[i * size + j] = -cos(sqrt(pow(float(i) - float(size) / 2.0f, 2) + pow(float(j) - float(size) / 2.0f, 2)) / 64.f);
            //flat map
            //justHeights[i * size + j] = 0.5f;
            //cliff single
            /*
            if (float(i) > 0.6f*float(size)) {
            justHeights[i * size + j] = float(i) / float(size) + 0.5f;
            }
            else {
                justHeights[i * size + j] = 0.5f;
            }
            */
            //cliff double    
            /*
            if (float(i) > 0.6f*float(size)) {
                justHeights[i * size + j] = float(i) / float(size) + 0.5f;
            }
            else if (float(i) > 0.33f * float(size)) {
                justHeights[i * size + j] = float(i) / float(size) + 0.2f;
            }
            else {
                justHeights[i * size + j] = 0.5f; 
            }
            */


            initVel[i * size + j] = 0.0f;
        }        
    }

    

    
  

  

    //WRITE HEIGHT DATA TO A TEXTURE (ATTEMPT)
    // texture size
    const unsigned int TEXTURE_WIDTH = size, TEXTURE_HEIGHT = size;
    //...
    unsigned int texture0;

    glGenTextures(1, &texture0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA,
        GL_FLOAT, justHeights);
    */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED,
        GL_FLOAT, justHeights);
    glBindImageTexture(0, texture0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    unsigned int texture1;

    glGenTextures(1, &texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED,
        GL_FLOAT, initVel);
    glBindImageTexture(1, texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    unsigned int texture2;

    glGenTextures(1, &texture2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED,
        GL_FLOAT, initVel);
    glBindImageTexture(2, texture2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    //This Texture stores vertex normals in the rgb channels.  a is zero
    unsigned int texture3;

    glGenTextures(1, &texture3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED,
        GL_FLOAT, NULL);
    glBindImageTexture(3, texture3, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    
    
   

    //[COORD SYS]
    //3D projection
    glm::mat4 model = glm::mat4(1.0f);
 
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f)/*FOV*/, 1200.0f / 800.0f/*aspect ratio*/, 0.1f, 10000.0f);

    glEnable(GL_DEPTH_TEST); //enable depth testing




    
    int shCount = 0;
    int shCountValue = 3; //set to 1 to run shader every frame

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        
        
        
        //the background color
        //glClearColor(0.2f, 0.3f, 0.5f, 1.0f); // old teal: 0.2f, 0.3f, 0.3f, 1.0f
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //BLACK/grey

        //clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        

        shCount += 1;
        if (shCount == shCountValue) {
            shCount = 0;

            if (!pauseSim) {
                /*COMPUTE SHADER*/ //need to change for SSBO
                ourComputeShader3_1.use();

                //time for random rain
                float timeValue = glfwGetTime();
                int TIME = glGetUniformLocation(ourComputeShader3_1.ID, "time");
                glUniform1f(TIME, timeValue);
                //uniform for 
                glm::vec2 rp = glm::vec2(rand() / float(RAND_MAX), rand() / float(RAND_MAX));
                int rainPos = glGetUniformLocation(ourComputeShader3_1.ID, "rainPos");
                glUniform2f(rainPos, rp.x, rp.y);

                glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
                //glDispatchCompute(32,1,1);
                // make sure writing to image has finished before read
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            //second comp shader     (3.2 in 3 passes)   
            ourComputeShader3_2.use();
            //time for random rain
            //float timeValue = glfwGetTime();
            int SIZE = glGetUniformLocation(ourComputeShader3_2.ID, "size");
            glUniform1f(SIZE, size);
            glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            ourComputeShader3_2b.use();
            glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            //calculate water velocity height map (t2) from flux height map (t1)
            ourComputeShader3_2c.use();
            glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



            //erosion and deposition           
            ourComputeShader3_3.use();
            glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


            
            //sediment transport   -- disable until fix 3.3           
            ourComputeShader3_4.use();
            glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            

            //evaporation 
            if (evaporate) {
                ourComputeShader3_5.use();
                glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }



            //Calculate normals of map shader -- change to not update every loop later on

            ourComputeShaderNormals.use();
            glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        
            


        

        ourShader.use();



        //BIND HEIGHT TEXTURE
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0);
        //BIND NORMAL VECTOR TEXTURE
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture3);   //VS NOT READING THIS
        
       
        /*Generate New Map */  
        if (changeMap) {
            
            changeMap = false;

            map.cleanUp(); //prevent memory leak
            map.~Map();
            seed += 1;
            map = Map(seed, size, octaves, frequency, smooth, scale);
          
        }
        if (freqDown) {

            freqDown = false;

            map.cleanUp(); //prevent memory leak
            map.~Map();
            if (frequency > 1) {
                //frequency--;
                size /= 2;
                //octaves--;
            }
            
            map = Map(seed, size, octaves, frequency, smooth, scale);

        }
        if (freqUp) {

            freqUp = false;

            map.cleanUp(); //prevent memory leak
            map.~Map();
            //frequency++;
            size *= 2;
            //octaves++;
            map = Map(seed, size, octaves, frequency, smooth, scale);

        }
       
        

        // camera/view transformation
        glm::mat4 view = ourCam.GetViewMatrix();
 
        /*Controlable Camera*/
        
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // input
        // -----
        processInput(window);

        glm::mat4 projection = glm::perspective(glm::radians(ourCam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 2000.0f);
        
        view = ourCam.GetViewMatrix();
        

        /*Rotating Camera*/
        if (rotateCamera) {
            //radius;
            float spinSpeed = 0.33;
            float camX = sin(glfwGetTime()*spinSpeed) * radius;
            float camZ = cos(glfwGetTime()*spinSpeed) * radius;
            
            view = glm::lookAt(glm::vec3(camX, 200.0, camZ)/*Pos*/, glm::vec3(0.0, 0.0, 0.0)/*lookAt*/, glm::vec3(0.0, 1.0, 0.0));
            
        }
        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        
        
        float timeVar = 1.0f *  (float)glfwGetTime();
        
        glm::mat4 model = glm::mat4(1.0f);
            
        model = glm::translate(model, glm::vec3(-mapScale*2.3, -map.hMin*mapScaleY, -mapScale*2.3)); //move map back to XZ plane
        float angle = 0.0f;
                       
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        model = glm::scale(model, glm::vec3(mapScale, mapScaleY, mapScale));
        int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        ourShader.use();
            
         

        float timeValue = glfwGetTime();
            
        int TIME = glGetUniformLocation(ourShader.ID, "time");
        //(ourProgram);
        glUniform1f(TIME, timeValue);

            
        map.draw(ourShader);
        
        
        


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    

    
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    
    //CAMERA INPUT PROCESSING WITH CAMERA CLASS

    //"sprint" input logic
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        ourCam.MovementSpeed = 8.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
        ourCam.MovementSpeed = SPEED;
    }
   

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ourCam.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ourCam.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ourCam.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ourCam.ProcessKeyboard(RIGHT, deltaTime);
    

    //mouse input not handled in this function!!!
    
    
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        if (evaporate == true) {
            evaporate = false;
        }
        else {
            evaporate = true;
        }
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        if (rotateCamera == true) {
            rotateCamera = false;
        }
        else {
            rotateCamera = true;
        }
        
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        if (pauseSim == true) {
            pauseSim = false;
        }
        else {
            pauseSim = true;
        }

    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        radius += 1;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        radius -= 1;
    }
    //spawn new map - not compatable with erosion sim as of 12 june, 2023
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        changeMap = true;
       
    }
    
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    ourCam.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ourCam.ProcessMouseScroll(static_cast<float>(yoffset));
}



