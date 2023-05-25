#version 430 core
out vec4 FragColor;

in vec3 ourColor; // the input variable from the vertex shader (same name and same type)  


void main()
{
    FragColor = vec4(ourColor, 1.0f);
}