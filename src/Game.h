#pragma once
#include <glad/glad.h>
#include "Camera.h"
#include "Renderer.h"
#include "GLFW/glfw3.h"
#include "Shader.h"


class Game {
public:
    Game();
    void mainLoop();
    void terminateGame();

    Game(const Game&) = delete;
    void operator=(const Game&) = delete;

private:
    GLFWwindow* m_window;
    Camera m_camera;
    Renderer m_renderer;
    void setupWindow();
    void setupImGui(GLFWwindow* window);
    void displayImGui();
    float m_timeOfLastFrame;

};
