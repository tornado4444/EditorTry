cmake_minimum_required(VERSION 3.10)
project(Rethink LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# GLEW configuration
add_definitions(-DGLEW_STATIC)
set(GLEW_BUILD_SHARED OFF CACHE BOOL "Build GLEW as a static library" FORCE)

# Add subdirectories for GLFW and GLEW
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glew)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(glfw PRIVATE
        _WIN32
        WIN32
        _WINDOWS
    )
endif()

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Recursively get all source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

# Copy texture directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})

# Set up GLEW as a static imported library
add_library(glew_static STATIC IMPORTED)
set_target_properties(glew_static PROPERTIES
    IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/thirdparty/glew/lib/Debug/x64/glew32sd.lib"
    IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/thirdparty/glew/lib/Release/x64/glew32s.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/thirdparty/glew/include"
)

# Main project source files
set(SOURCES
    src/main.cpp
    src/Window.cpp
    src/Engine.cpp
    src/Logger/Logger.cpp
    src/font/Font.cpp
    src/Transform.cpp
    src/Shader.cpp
    Game/Camera.cpp
    thirdparty/stb/stb_image.cpp
)

# Create executable
add_executable(Rethink ${SOURCES} "Game/Texture.cpp")

# Set include directories
target_include_directories(Rethink PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/thirdparty/glew/include
)

# Enable experimental GLM features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Enable static GLEW compilation
target_compile_definitions(Rethink PRIVATE GLEW_STATIC)

# Define texture path as a preprocessor definition
target_compile_definitions(Rethink PRIVATE TEXTURE_PATH="${CMAKE_SOURCE_DIR}/textures/sand.png")

# Link libraries
target_link_libraries(Rethink PRIVATE
    ${OPENGL_LIBRARIES}
    glew_static
    glfw
)
