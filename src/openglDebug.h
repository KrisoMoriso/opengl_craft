#pragma once
#include <glad/glad.h>
#include <iostream>


// Clears all currently queued OpenGL errors
void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

// Checks for errors and prints them with file and line info
bool GLCheckError(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::string errorStr;
        switch (error) {
        case GL_INVALID_ENUM:                  errorStr = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 errorStr = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             errorStr = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                errorStr = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               errorStr = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 errorStr = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errorStr = "INVALID_FRAMEBUFFER_OPERATION"; break;
        default:                               errorStr = "UNKNOWN_ERROR"; break;
        }

        std::cerr << "[OpenGL Error] (" << errorStr << " - 0x" << std::hex << error << std::dec << ")"
                  << "\nFunction: " << function
                  << "\nFile: "     << file
                  << "\nLine: "     << line << std::endl;
        return false;
    }
    return true;
}

// The Macro: Use this to wrap your OpenGL calls
#ifdef _DEBUG // Only run checks in debug builds for performance
// __debugbreak() is MSVC specific. Use __builtin_trap() for GCC/Clang
#define GLDebug(x) GLClearError();\
x;\
if (!GLCheckError(#x, __FILE__, __LINE__)) __debugbreak();
#else
#define GLDebug(x) x
#endif