#pragma once
#include <string>


class Shader {
public:
    Shader(){}
    unsigned int m_shaderProgramID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void useShaderProgram();
};

