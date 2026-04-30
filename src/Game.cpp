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


Game::Game(){
    setupWindow();
    if (m_window == nullptr) printf("failed to init glfw window");
    setupImGui(m_window);
    m_camera = Camera(m_window, {0.0f,0.0f,3.0f});
    m_renderer = Renderer();
    m_renderer.init();
}

void Game::mainLoop(){
    while (!glfwWindowShouldClose(m_window)){
        m_camera.updateAndHandleCamera();
        m_renderer.render(m_camera.getViewMatrix());
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




void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Game::setupWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    static constexpr int WIDTH = 1400, HEIGHT = 800;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Opengl craft", nullptr, nullptr);
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
    framebufferSizeCallback(window, WIDTH, HEIGHT);
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
