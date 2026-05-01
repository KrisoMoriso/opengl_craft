#pragma once
// #include <glad/glad.h>
#include "MeshSSBO.h"
#include "Shader.h"
#include "glm/glm.hpp"


class Renderer {
public:
    Renderer(){}
    void init();
    void render(glm::mat4 viewMatrix);
private:
    Shader m_shader;
    MeshSSBO m_mesh;
    GLuint m_texture = 0;
    void loadTextures();
    void setupBuffers();
};
