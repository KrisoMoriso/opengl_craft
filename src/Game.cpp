#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream> // ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ResourceManager.h"
#include "Game.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "stb_image.h"

#define SKYBLUE    0.4f, 0.749f, 1.0f, 1.0f  // Sky Blue

Game::Game(){
    setupWindow();
    if (m_window == nullptr) printf("failed to init glfw window");
    setupImGui(m_window);
    m_shader = Shader("../src/shaders/voxel.vs.glsl", "../src/shaders/voxel.fs.glsl");
    setupBuffers();
    loadTextures();
    m_camera = Camera(m_window, {0.0f,0.0f,-3.0f});
}

void Game::mainLoop(){
    while (!glfwWindowShouldClose(m_window)){
        m_camera.updateAndHandleCamera();

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));

        auto view = m_camera.getViewMatrix();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glm::mat4 mvp = projection * view * model;

        int mvpUniformLocation = glGetUniformLocation(m_shader.m_shaderProgramID, "mvp");
        glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        glClearColor(SKYBLUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_shader.m_shaderProgramID);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        int vertexColorLocation = glGetUniformLocation(m_shader.m_shaderProgramID, "ourColor");
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        displayImGui();
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Game::terminateGame(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}


void Game::loadTextures(){
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

void Game::setupBuffers(){
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

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Game::setupWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Opengl craft", nullptr, nullptr);
    m_window = window;
    if (window == nullptr){
        printf("Failed to create window");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("Failed to init glad");
        return;
    }
    framebufferSizeCallback(window, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetScrollCallback(m_window, [](GLFWwindow* windowLambda, double deltaX, double deltaY){
        auto* gameInstance = (Game*)glfwGetWindowUserPointer(windowLambda);
        if (gameInstance == nullptr) return;
        gameInstance->m_camera.setScrollDelta(deltaX, deltaY);
    });
    glEnable(GL_CULL_FACE);
}
void Game::setupImGui(GLFWwindow* window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
}
void Game::displayImGui(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiWindowFlags window_flags;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Always);
    bool open = true;
    ImGui::Begin("Debug", &open, window_flags);
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.0f", io.Framerate);
    glm::vec3 cameraPos = m_camera.getDebugInfo().position;
    ImGui::Text("X:%f Y:%f Z%f", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::End();
    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
