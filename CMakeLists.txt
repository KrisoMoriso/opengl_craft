cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 20)

project(opengl_craft CXX C) # C added because glad.c is a C file

# Enforce C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ---------------------------------------------------------
# DEPENDENCY: GLFW (Window & Input)
# ---------------------------------------------------------
# Disable GLFW tests and examples so they don't clutter your build time
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

# ---------------------------------------------------------
# DEPENDENCY: GLM (Math)
# ---------------------------------------------------------
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# ---------------------------------------------------------
# DEPENDENCY: Dear ImGui (Engine UI)
# ---------------------------------------------------------
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.90.4 # You can change this to a newer tag or 'docking' branch if needed
)
FetchContent_MakeAvailable(imgui)

# ImGui requires us to explicitly compile its source files and backends
set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        # Backends specifically for our GLFW + OpenGL stack
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ---------------------------------------------------------
# PROJECT EXECUTABLE
# ---------------------------------------------------------
set(SOURCES
        src/main.cpp
        src/libraries/glad.c
        src/ResourceManager.cpp
        src/ResourceManager.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Tell CMake where to find our local headers (glad and stb_image)
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

# Link the fetched libraries
# Note: glfw automatically links against required OS libraries (like gdi32/opengl32 on Windows/MinGW)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm)