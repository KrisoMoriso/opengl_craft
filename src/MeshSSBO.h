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

        float textureLayer, temperature;
        float behaviourFlag, humidity;
        //0 = Normal, 1 = Grass Top, 2 = Grass Side, 3 = Water, 4 = WaterTop
        float ao0, ao1, ao2, ao3;
    };
    void upload(const std::vector<VoxelFaceData> &dataVector);
    void unload();
    void draw(glm::mat4& mvp, GLuint shaderProgramID, GLuint textureID, GLuint grassColormapTextureID);
    size_t GetSize(){ return m_size;}
private:
    size_t m_size = 0;
    GLuint m_ssboID = 0;


};
