#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"
#include "stb_image.h"
#define SKYBLUE    0.4f, 0.749f, 1.0f, 1.0f  // Sky Blue
void Renderer::init(){
    m_shader = Shader("../src/shaders/voxel.vs.glsl", "../src/shaders/voxel.fs.glsl");
    loadTextures();
    setupBuffers();
}

void Renderer::render(glm::mat4 viewMatrix){
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));

    auto view = viewMatrix;

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glm::mat4 mvp = projection * view * model;

    int mvpUniformLocation = glGetUniformLocation(m_shader.m_shaderProgramID, "mvp");
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    glClearColor(SKYBLUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader.m_shaderProgramID);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glDrawArrays(GL_TRIANGLES, 0, 3);
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


float vertices[] = {
    // positions         // colors
    0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
   -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,    // top

    // positions         // colors
    1.5f, -1.5f, -1.0f,  1.0f, 0.0f, 0.0f,   // bottom right
   -1.5f, -1.5f, -1.0f,  0.0f, 1.0f, 0.0f,   // bottom left
    -1.5f,  -1.5f, 1-.0f,  0.0f, 0.0f, 1.0f    // top
};

float texCoords[] = {
    1.0f, 0.0f,  // lower-right corner
    0.0f, 0.0f,  // lower-left corner
    0.0f, 1.0f   // top-center corner
};

void Renderer::setupBuffers(){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(2);

}