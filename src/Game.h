#pragma once
#include <glad/glad.h>
#include "Camera.h"
#include "Renderer.h"
#include "GLFW/glfw3.h"
#include "ThreadPool.h"

class Game {
public:
    Game();
    void mainLoop();
    void terminateGame();
    Game(const Game&) = delete;
    void operator=(const Game&) = delete;
private:
    const int RENDER_DISTANCE = 20;
    const int WORLD_SEED = 12521;
    ThreadPool m_threadPool;
    Renderer m_renderer;
    GLFWwindow* m_window;
    Camera m_camera;
    World m_world;
    void setupWindow();
    void setupImGui(GLFWwindow* window);
    void displayImGui();
    float m_timeOfLastFrame;

};
