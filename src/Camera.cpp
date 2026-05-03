#include <glad/glad.h>
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Game.h"


Camera::Camera(GLFWwindow* window, glm::vec3 position ){
    m_window = window;
    m_position = position;
}
void Camera::updateAndHandleCamera(){
    processInputs();
    updateInternalVectors();
}

glm::mat4 Camera::getViewMatrix(){
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

Camera::CameraDebugInfo Camera::getDebugInfo(){
    CameraDebugInfo debugInfo;
    debugInfo.position = m_position;
    return debugInfo;
}

void Camera::setScrollDelta(double deltaX, double deltaY){
    m_scrollDeltaX = deltaX;
    m_scrollDeltaY = deltaY;
}


void Camera::processInputs(){
    glm::vec3 levelFront = {m_front.x, 0.0f, m_front.z};
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS){
        m_position += m_movementSpeed * levelFront;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS){
        m_position -= m_movementSpeed * levelFront;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS){
        m_position -= m_movementSpeed * m_right;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS){
        m_position += m_movementSpeed * m_right;
    }
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        m_position -= m_movementSpeed * m_worldUp;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS){
        m_position += m_movementSpeed * m_worldUp;
    }

    processMouseInputs();
}

void Camera::processMouseInputs(){
    double mouseCursorX;
    double mouseCursorY;
    glfwGetCursorPos(m_window, &mouseCursorX, &mouseCursorY);

    if (m_firstMouseMove) {
        m_lastMouseCursorX = mouseCursorX;
        m_lastMouseCursorY = mouseCursorY;
        m_firstMouseMove = false;
    }
    double deltaX = mouseCursorX - m_lastMouseCursorX;
    double deltaY = mouseCursorY - m_lastMouseCursorY;
    m_lastMouseCursorX = mouseCursorX;
    m_lastMouseCursorY = mouseCursorY;

    deltaX *= SENSITIVITY;
    deltaY *= SENSITIVITY;

    m_yaw  += deltaX;
    m_pitch -= deltaY;

    if (m_pitch > 89.9999f){
        m_pitch = 89.9999f;
    }
    if (m_pitch < -89.9999f){
        m_pitch = -89.9999f;
    }
    m_movementSpeed += m_scrollDeltaY * 0.02;
    m_scrollDeltaX = 0;
    m_scrollDeltaY = 0;

}


void Camera::updateInternalVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}
