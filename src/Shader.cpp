#include "Shader.h"

#include <iostream>

#include "ResourceManager.h"
#include "glad/glad.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath){
    //vertex
    std::string voxelDotVS = ResourceManager::loadFileIntoString(vertexPath); //"../src/shaders/voxel.vs.glsl"
    const char * vertexShaderCode = voxelDotVS.c_str();
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    glCompileShader(vertexShader);
    int  successVertex;
    char infoLogVertex[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successVertex);
    if(!successVertex)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLogVertex);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLogVertex << std::endl;
    }
    //fragment
    std::string voxelDotFS = ResourceManager::loadFileIntoString(fragmentPath); //"../src/shaders/voxel.fs.glsl"
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char * fragmentShaderCode = voxelDotFS.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    glCompileShader(fragmentShader);
    int  successFragment;
    char infoLogFragment[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successFragment);
    if(!successFragment)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLogFragment);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLogFragment << std::endl;
    }
    //program
    m_shaderProgramID = glCreateProgram();
    glAttachShader(m_shaderProgramID, vertexShader);
    glAttachShader(m_shaderProgramID, fragmentShader);
    glLinkProgram(m_shaderProgramID);
    glUseProgram(m_shaderProgramID);
    int  successLinking;
    char infoLogLinking[512];
    glGetProgramiv(m_shaderProgramID, GL_LINK_STATUS, &successLinking);
    if(!successLinking)
    {
        glGetProgramInfoLog(vertexShader, 512, NULL, infoLogLinking);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLogLinking << std::endl;
    }
    //delete
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
