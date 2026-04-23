#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream> // ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ResourceManager.h"

#define SKYBLUE    0.4f, 0.749f, 1.0f, 1.0f  // Sky Blue

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* setupWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Opengl craft", nullptr, nullptr);
    if (window == nullptr){
        printf("Failed to create window");
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("Failed to init glad");
        return nullptr;
    }
    framebufferSizeCallback(window, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    return window;
}
void setupImGui(GLFWwindow* window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
}
void displayImGui(){
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
    ImGui::End();
    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

void setupShaders(){
    //vertex
    std::string voxelDotVS = ResourceManager::loadFileIntoString("../src/shaders/voxel.vs.glsl");
    const char * vertexShaderCode = voxelDotVS.c_str();
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    glCompileShader(vertexShader);
    int  successVertex;
    char infoLogVertex[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successVertex);
    if(!successVertex)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLogVertex);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLogVertex << std::endl;
    }
    //fragment
    std::string voxelDotFS = ResourceManager::loadFileIntoString("../src/shaders/voxel.fs.glsl");
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char * fragmentShaderCode = voxelDotFS.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    glCompileShader(fragmentShader);
    int  successFragment;
    char infoLogFragment[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successFragment);
    if(!successFragment)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLogFragment);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLogFragment << std::endl;
    }
    //program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    int  successLinking;
    char infoLogLinking[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &successLinking);
    if(!successLinking)
    {
        glGetProgramInfoLog(vertexShader, 512, NULL, infoLogLinking);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLogLinking << std::endl;
    }
    //delete
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void setupBuffers(){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
}

int main(){
    GLFWwindow* window = setupWindow();
    if (window == nullptr) return -1;
    setupImGui(window);
    setupShaders();
    setupBuffers();

    while (!glfwWindowShouldClose(window)){

        glClearColor(SKYBLUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        displayImGui();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}


