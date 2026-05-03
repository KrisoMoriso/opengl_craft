#include <glad/glad.h>
#include "MeshSSBO.h"

#include "Camera.h"
#include "Camera.h"
#include "glm/gtc/type_ptr.hpp"

void MeshSSBO::upload(const std::vector<VoxelFaceData>& dataVector){
    glGenBuffers(1, &m_ssboID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VoxelFaceData) * dataVector.size(), dataVector.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    m_size = dataVector.size();
}

void MeshSSBO::unload(){
    glDeleteBuffers(1, &m_ssboID);
}

void MeshSSBO::draw(glm::mat4 &mvp, GLuint shaderProgramID, GLuint textureID, GLuint grassColormapTextureID){
    glUseProgram(shaderProgramID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboID);
    int mvpUniformLocation = glGetUniformLocation(shaderProgramID, "mvp");
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    glUseProgram(shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    glUniform1i(glGetUniformLocation(shaderProgramID, "texture0"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grassColormapTextureID);
    glUniform1i(glGetUniformLocation(shaderProgramID, "grassColormap"), 1);
    glDrawArrays(GL_TRIANGLES, 0, 6 * m_size);
}
