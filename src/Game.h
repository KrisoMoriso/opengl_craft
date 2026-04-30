#pragma once
#include "Camera.h"
#include "GLFW/glfw3.h"
#include "Shader.h"


class Game {
public:
    Game();
    void mainLoop();
    void terminateGame();
    void loadTextures();

    Game(const Game&) = delete;
    void operator=(const Game&) = delete;

private:
    GLFWwindow* m_window;
    Shader m_shader;
    Camera m_camera;
    void setupWindow();
    unsigned int m_texture;
    void setupBuffers();
    void setupImGui(GLFWwindow* window);
    void displayImGui();
    float m_timeOfLastFrame;

};
