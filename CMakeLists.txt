cmake_minimum_required(VERSION 3.20)
project(Editor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(LIBRARIES_DIR "${CMAKE_SOURCE_DIR}/libraries")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

if(EXISTS "${CMAKE_SOURCE_DIR}/textures")
    file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/models")
    file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})
endif()

find_package(OpenGL REQUIRED)


find_package(glfw3 QUIET)
if(glfw3_FOUND)
    message(STATUS "Using system GLFW")
    set(GLFW_LIBRARIES glfw)
elseif(EXISTS "${LIBRARIES_DIR}/glfw/CMakeLists.txt")
    message(STATUS "Using local GLFW from libraries/glfw")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory("${LIBRARIES_DIR}/glfw")
    set(GLFW_LIBRARIES glfw)
else()
    message(FATAL_ERROR "GLFW not found! Install with: sudo pacman -S glfw OR put GLFW in libraries/glfw/")
endif()

find_package(GLEW QUIET)
if(GLEW_FOUND)
    message(STATUS "Using system GLEW")
    set(GLEW_LIBRARIES GLEW::GLEW)
elseif(EXISTS "${LIBRARIES_DIR}/glew/include/GL/glew.h")
    message(STATUS "Using local GLEW from libraries/glew")
    add_definitions(-DGLEW_STATIC)
    set(GLEW_INCLUDE_DIR "${LIBRARIES_DIR}/glew/include")
    
    if(WIN32)
        add_library(glew_static STATIC IMPORTED)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_target_properties(glew_static PROPERTIES
                IMPORTED_LOCATION_DEBUG "${LIBRARIES_DIR}/glew/lib/Debug/x64/glew32sd.lib"
                IMPORTED_LOCATION_RELEASE "${LIBRARIES_DIR}/glew/lib/Release/x64/glew32s.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
            )
        else()
            set_target_properties(glew_static PROPERTIES
                IMPORTED_LOCATION_DEBUG "${LIBRARIES_DIR}/glew/lib/Debug/Win32/glew32sd.lib"
                IMPORTED_LOCATION_RELEASE "${LIBRARIES_DIR}/glew/lib/Release/Win32/glew32s.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
            )
        endif()
        set(GLEW_LIBRARIES glew_static)
    else()
        find_package(GLEW REQUIRED)
        set(GLEW_LIBRARIES GLEW::GLEW)
    endif()
else()
    message(FATAL_ERROR "GLEW not found! Install with: sudo pacman -S glew")
endif()

find_package(glm QUIET)
if(glm_FOUND)
    message(STATUS "Using system GLM")
elseif(EXISTS "${LIBRARIES_DIR}/glm/glm/glm.hpp")
    message(STATUS "Using local GLM from libraries/glm")
    set(GLM_INCLUDE_DIR "${LIBRARIES_DIR}/glm")
else()
    message(FATAL_ERROR "GLM not found! Install with: sudo pacman -S glm OR put GLM in libraries/glm/")
endif()


find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Using system nlohmann_json")
    set(JSON_LIBRARIES nlohmann_json::nlohmann_json)
elseif(EXISTS "${LIBRARIES_DIR}/json/json.h")
    message(STATUS "Using local JSON from libraries/json (json.h)")
    set(JSON_INCLUDE_DIR "${LIBRARIES_DIR}/json")
elseif(EXISTS "${LIBRARIES_DIR}/json/json.hpp")
    message(STATUS "Using local JSON from libraries/json (json.hpp)")
    set(JSON_INCLUDE_DIR "${LIBRARIES_DIR}/json")
elseif(EXISTS "${LIBRARIES_DIR}/json/single_include/nlohmann/json.hpp")
    message(STATUS "Using local JSON from libraries/json (single_include)")
    set(JSON_INCLUDE_DIR "${LIBRARIES_DIR}/json/single_include")
else()
    message(WARNING "JSON not found, but continuing...")
endif()

if(EXISTS "${LIBRARIES_DIR}/stb/stb_image.hpp")
    message(STATUS "Found STB (.hpp) in libraries/stb")
    set(STB_INCLUDE_DIR "${LIBRARIES_DIR}/stb")
    set(STB_IS_HPP TRUE)
elseif(EXISTS "${LIBRARIES_DIR}/stb/stb_image.h")
    message(STATUS "Found STB (.h) in libraries/stb")
    set(STB_INCLUDE_DIR "${LIBRARIES_DIR}/stb")  
    set(STB_IS_HPP FALSE)
else()
    message(FATAL_ERROR "STB not found! Put stb_image.h or stb_image.hpp in libraries/stb/")
endif()

if(EXISTS "${LIBRARIES_DIR}/imgui/imgui.h")
    message(STATUS "Found ImGui in libraries/imgui")
    set(IMGUI_INCLUDE_DIR "${LIBRARIES_DIR}/imgui")
    
    set(IMGUI_SOURCES
        "${IMGUI_INCLUDE_DIR}/imgui.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_demo.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_draw.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_tables.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp"
        "${IMGUI_INCLUDE_DIR}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_INCLUDE_DIR}/backends/imgui_impl_opengl3.cpp"
    )
else()
    message(FATAL_ERROR "ImGui not found in libraries/imgui/!")
endif()

if(EXISTS "${LIBRARIES_DIR}/imguizmo/ImGuizmo.h")
    message(STATUS "Found ImGuizmo in libraries/imguizmo")
    set(IMGUIZMO_INCLUDE_DIR "${LIBRARIES_DIR}/imguizmo")
    
    set(IMGUIZMO_SOURCES
        "${IMGUIZMO_INCLUDE_DIR}/ImGuizmo.cpp"
        "${IMGUIZMO_INCLUDE_DIR}/ImCurveEdit.cpp"  
        "${IMGUIZMO_INCLUDE_DIR}/ImGradient.cpp"
        "${IMGUIZMO_INCLUDE_DIR}/ImSequencer.cpp"
    )
else()
    message(WARNING "ImGuizmo not found in libraries/imguizmo/, continuing without it...")
    set(IMGUIZMO_SOURCES "")
endif()


file(GLOB_RECURSE SOURCES 
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.c"
)

file(GLOB_RECURSE HEADERS
    "${SRC_DIR}/*.hpp" 
    "${SRC_DIR}/*.h"
)

set(STB_IMPLEMENTATION_FILE "${CMAKE_BINARY_DIR}/stb_implementation.cpp")
if(STB_IS_HPP)
    file(WRITE ${STB_IMPLEMENTATION_FILE}
"#define STB_IMAGE_IMPLEMENTATION
#include \"stb_image.hpp\"
")
else()
    file(WRITE ${STB_IMPLEMENTATION_FILE}
"#define STB_IMAGE_IMPLEMENTATION
#include \"stb_image.h\"
")
endif()

list(APPEND SOURCES ${STB_IMPLEMENTATION_FILE})
list(APPEND SOURCES ${IMGUI_SOURCES})
if(IMGUIZMO_SOURCES)
    list(APPEND SOURCES ${IMGUIZMO_SOURCES})
endif()


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

set(INCLUDE_DIRS
    "${SRC_DIR}"
    "${SRC_DIR}/font"
    "${SRC_DIR}/Logger"
    "${CMAKE_SOURCE_DIR}/shaders"
    "${STB_INCLUDE_DIR}"
    "${IMGUI_INCLUDE_DIR}"
    "${IMGUI_INCLUDE_DIR}/backends"
)

if(GLM_INCLUDE_DIR)
    list(APPEND INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
endif()

if(GLEW_INCLUDE_DIR)
    list(APPEND INCLUDE_DIRS "${GLEW_INCLUDE_DIR}")
endif()

if(JSON_INCLUDE_DIR)
    list(APPEND INCLUDE_DIRS "${JSON_INCLUDE_DIR}")
endif()

if(IMGUIZMO_INCLUDE_DIR)
    list(APPEND INCLUDE_DIRS "${IMGUIZMO_INCLUDE_DIR}")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    GLM_ENABLE_EXPERIMENTAL
)

if(GLEW_INCLUDE_DIR)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
endif()

set(LINK_LIBRARIES
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::GL
)

if(JSON_LIBRARIES)
    list(APPEND LINK_LIBRARIES ${JSON_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 user32 kernel32 shell32)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS} pthread m X11 Xrandr Xi Xcursor)
endif()