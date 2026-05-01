#pragma once
// #include <glad/glad.h>
#include <vector>
#include <glm/matrix.hpp>

class MeshSSBO {
public:
    MeshSSBO()= default;
    struct VoxelFaceData {
        float x, y, z;
        float faceId;

        float atlasX, atlasY;
        float pad1, pad2;
        float ao0, ao1, ao2, ao3;
    };
    void upload(const std::vector<VoxelFaceData> &dataVector);
    void draw(glm::mat4& mvp, GLuint shaderProgramID, GLuint textureID);
private:
    GLuint m_ssboID;


};
