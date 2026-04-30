#pragma once
// #include <glad/glad.h>
#include "Shader.h"
#include "glm/glm.hpp"


class Renderer {
public:
    Renderer(){}
    void init();
    void render(glm::mat4 viewMatrix);
private:
    struct VoxelFaceData {
        float x, y, z;
        float faceId;         // 0.0 to 5.0

        float atlasX, atlasY; // Atlas offsets
        float pad1, pad2;     // Empty padding to align perfectly to 16 bytes

        float ao0, ao1, ao2, ao3; // Your AO values (passed as floats 0.0 to 255.0)
    };
    Shader m_shader;
    unsigned int m_texture = 0;
    void loadTextures();
    void setupBuffers();
};
