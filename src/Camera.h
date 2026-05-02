#pragma once
#include <glm/glm.hpp>

#include "GLFW/glfw3.h"

class Camera{
public:
    Camera(){}
    Camera(GLFWwindow* window, glm::vec3 position);
    // Camera(GLFWwindow* window);
    void updateAndHandleCamera();
    glm::mat4 getViewMatrix();
    struct CameraDebugInfo{
        glm::vec3 position;
    };
    CameraDebugInfo getDebugInfo();
    void setScrollDelta(double deltaX, double deltaY);
    glm::vec3 m_position = {0, 0, 0};
private:
    static constexpr float YAW         = -90.0f;
    static constexpr float PITCH       =  0.0f;
    static constexpr float SPEED       =  0.05f;
    static constexpr float SENSITIVITY =  0.1f;
    static constexpr float FOV         =  90.0f;
    GLFWwindow* m_window = nullptr;
    glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_right = {0,0,0};
    glm::vec3 m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    // euler Angles
    float m_yaw = YAW;
    float m_pitch = PITCH;
    // camera options
    float m_movementSpeed = SPEED;
    float m_mouseSensitivity = SENSITIVITY;
    float m_FOV = FOV;
    void updateInternalVectors();
    void processInputs();

    double m_scrollDeltaX = 0;
    double m_scrollDeltaY = 0;
    bool m_firstMouseMove = true;
    double m_lastMouseCursorX = 0;
    double m_lastMouseCursorY = 0;
    void processMouseInputs();
};
