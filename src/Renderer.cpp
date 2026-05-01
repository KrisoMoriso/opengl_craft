#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"

#include "MeshSSBO.h"
#include "stb_image.h"
#define SKYBLUE    0.4f, 0.749f, 1.0f, 1.0f  // Sky Blue
void Renderer::init(){
    m_shader = Shader("../src/shaders/voxel.vs.glsl", "../src/shaders/voxel.fs.glsl");
    loadTextures();
    setupBuffers();
}

void Renderer::render(glm::mat4 viewMatrix){
    glClearColor(SKYBLUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));

    auto view = viewMatrix;

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glm::mat4 mvp = projection * view * model;

    m_mesh.draw(mvp, m_shader.m_shaderProgramID, m_texture);
}

void Renderer::loadTextures(){
    //textures
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../textures/block/dirt.png", &width, &height, &nrChannels, 0);
    if (data == nullptr){
        printf("failed to load texture");
        return;
    }
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}



void Renderer::setupBuffers(){
    MeshSSBO::VoxelFaceData voxelFaceData;
    voxelFaceData.x = 0.0f;
    voxelFaceData.y = 0.0f;
    voxelFaceData.z = 0.0f;
    voxelFaceData.faceId = 0;
    voxelFaceData.atlasX = 0.0f;
    voxelFaceData.atlasY = 0.0f;
    voxelFaceData.pad1 = 0.0f;
    voxelFaceData.pad2 = 0.0f;
    voxelFaceData.ao0 = 140 ;
    voxelFaceData.ao1 = 210;
    voxelFaceData.ao2 = 210;
    voxelFaceData.ao3 = 210;

    static GLuint emptyVAO;
    glGenVertexArrays(1, &emptyVAO);
    glBindVertexArray(emptyVAO);
    std::vector<MeshSSBO::VoxelFaceData> dataVector;
    dataVector.push_back(voxelFaceData);
    m_mesh.upload(dataVector);
}