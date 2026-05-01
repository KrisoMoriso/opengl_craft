#include <glad/glad.h>
#include "MeshSSBO.h"
#include "glm/gtc/type_ptr.hpp"

void MeshSSBO::upload(const std::vector<VoxelFaceData>& dataVector){
    glGenBuffers(1, &m_ssboID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VoxelFaceData) * dataVector.size(), dataVector.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void MeshSSBO::draw(glm::mat4 &mvp, GLuint shaderProgramID, GLuint textureID){
    glUseProgram(shaderProgramID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboID);
    int mvpUniformLocation = glGetUniformLocation(shaderProgramID, "mvp");
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    glUseProgram(shaderProgramID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}
