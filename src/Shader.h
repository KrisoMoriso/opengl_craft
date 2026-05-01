#pragma once
#include <glad/glad.h>
class Shader {
public:
    Shader(){}
    GLuint m_shaderProgramID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void useShaderProgram();
};

