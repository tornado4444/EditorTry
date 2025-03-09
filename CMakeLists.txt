cmake_minimum_required(VERSION 3.10)
project(Rethink LANGUAGES C CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Настройка GLEW
add_definitions(-DGLEW_STATIC)
set(GLEW_BUILD_SHARED OFF CACHE BOOL "Build GLEW as a static library" FORCE)

# Добавление поддиректорий для GLFW и GLEW
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glew)

# Настройки для Windows
if(WIN32)
    target_compile_definitions(glfw PRIVATE
        _WIN32
        WIN32
        _WINDOWS
    )
endif()

# Поиск OpenGL
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})

# Сборка GLEW как статической библиотеки
add_library(glew_static STATIC IMPORTED)
set_target_properties(glew_static PROPERTIES
    IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/thirdparty/glew/lib/Debug/x64/glew32sd.lib"
    IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/thirdparty/glew/lib/Release/x64/glew32s.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/thirdparty/glew/include"
)

# Сборка основного проекта
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

add_executable(Rethink ${SOURCES} "Game/Texture.cpp")

# Установка директорий для включения заголовочных файлов
target_include_directories(Rethink PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/thirdparty/glew/include
)

# Включение экспериментальных функций GLM
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Статическая сборка для GLEW
target_compile_definitions(Rethink PRIVATE GLEW_STATIC)
target_compile_definitions(Rethink PRIVATE TEXTURE_PATH="${CMAKE_SOURCE_DIR}/textures/sand.png")

# Линковка библиотек
target_link_libraries(Rethink PRIVATE
    ${OPENGL_LIBRARIES}
    glew_static
    glfw
)