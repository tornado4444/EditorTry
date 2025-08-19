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

if(NOT EXISTS "${LIBRARIES_DIR}")
    message(FATAL_ERROR "Folder libraries not found! Create a folder libraries in the root of the project.")
endif()

find_package(OpenGL REQUIRED)

file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})

if(EXISTS "${LIBRARIES_DIR}/glfw/CMakeLists.txt")
    message(STATUS "Found GLFW in libraries/glfw")
    
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    add_subdirectory("${LIBRARIES_DIR}/glfw")
    set(GLFW_LIBRARIES glfw)
else()
    message(FATAL_ERROR "GLFW cannot find libraries/glfw/.")
endif()

set(GLEW_INCLUDE_DIR "${LIBRARIES_DIR}/glew/include")
if(EXISTS "${GLEW_INCLUDE_DIR}/GL/glew.h")
    message(STATUS "Found GLEW.")
    
    add_definitions(-DGLEW_STATIC)
    
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
    message(FATAL_ERROR "GLEW cannot find.")
endif()

set(GLM_INCLUDE_DIR "${LIBRARIES_DIR}/glm")
if(EXISTS "${GLM_INCLUDE_DIR}/glm/glm.hpp")
    message(STATUS "Found GLM in libraries/glm")
else()
    message(FATAL_ERROR "GLM not found in libraries/glm/. Download GLM and extract to this folder.")
endif()

set(STB_INCLUDE_DIR "${LIBRARIES_DIR}/stb")
if(EXISTS "${STB_INCLUDE_DIR}/stb_image.hpp")
    message(STATUS "Found STB in libraries/stb")
else()
    message(WARNING "STB not found in libraries/stb/. Download stb_image.h and place in this folder.")
endif()

set(JSON_INCLUDE_DIR "${LIBRARIES_DIR}/json")
if(EXISTS "${JSON_INCLUDE_DIR}/json.h")
    message(STATUS "Found JSON in libraries/json")
elseif(EXISTS "${JSON_INCLUDE_DIR}/json.hpp")
    message(STATUS "Found JSON (nlohmann) in libraries/json")
elseif(EXISTS "${JSON_INCLUDE_DIR}/single_include/nlohmann/json.hpp")
    message(STATUS "Found JSON (nlohmann single_include) in libraries/json")
    set(JSON_INCLUDE_DIR "${JSON_INCLUDE_DIR}/single_include")
else()
    message(WARNING "JSON not found in libraries/json/. Download json.h or nlohmann/json.hpp and place in this folder.")
endif()

set(IMGUI_INCLUDE_DIR "${LIBRARIES_DIR}/imgui")
if(EXISTS "${IMGUI_INCLUDE_DIR}/imgui.h")
    message(STATUS "Found ImGui in libraries/imgui")
    
    set(IMGUI_SOURCES
        "${IMGUI_INCLUDE_DIR}/imgui.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_demo.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_draw.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_tables.cpp"
        "${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp"
    )
    
    set(IMGUI_BACKENDS_DIR "${IMGUI_INCLUDE_DIR}/backends")
    set(GLFW_BACKEND_FILE "${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp")
    set(OPENGL3_BACKEND_FILE "${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp")
    
    if(EXISTS "${GLFW_BACKEND_FILE}" AND EXISTS "${OPENGL3_BACKEND_FILE}")
        list(APPEND IMGUI_SOURCES
            "${GLFW_BACKEND_FILE}"
            "${OPENGL3_BACKEND_FILE}"
        )
        message(STATUS "Found required ImGui backends: GLFW and OpenGL3")
    else()
        message(FATAL_ERROR "CRITICAL ERROR: ImGui backend files not found!")
    endif()
    
else()
    message(FATAL_ERROR "ImGui not found in libraries/imgui/. Download ImGui and extract to this folder.")
endif()

set(IMGUIZMO_INCLUDE_DIR "${LIBRARIES_DIR}/imguizmo")
if(EXISTS "${IMGUIZMO_INCLUDE_DIR}/ImGuizmo.h")
    message(STATUS "Found ImGuizmo in libraries/imguizmo")
    
    set(IMGUIZMO_SOURCES
        "${IMGUIZMO_INCLUDE_DIR}/ImGuizmo.cpp"
        "${IMGUIZMO_INCLUDE_DIR}/ImCurveEdit.cpp"  
        "${IMGUIZMO_INCLUDE_DIR}/ImGradient.cpp"
        "${IMGUIZMO_INCLUDE_DIR}/ImSequencer.cpp"
    )
    
    foreach(imguizmo_file ${IMGUIZMO_SOURCES})
        if(NOT EXISTS ${imguizmo_file})
            message(WARNING "Cannot find the ImGuizmo file: ${imguizmo_file}")
        endif()
    endforeach()
    
else()
    message(FATAL_ERROR "ImGuizmo not found in libraries/imguizmo/")
endif()

file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/src/*.hpp" 
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

set(STB_IMPLEMENTATION_FILE "${CMAKE_BINARY_DIR}/stb_implementation.cpp")
file(WRITE ${STB_IMPLEMENTATION_FILE}
"#define STB_IMAGE_IMPLEMENTATION
#include \"stb_image.hpp\"
")

list(APPEND SOURCES ${STB_IMPLEMENTATION_FILE})
list(APPEND SOURCES ${IMGUI_SOURCES})
list(APPEND SOURCES ${IMGUIZMO_SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/font"
    "${CMAKE_SOURCE_DIR}/src/Logger"
    "${CMAKE_SOURCE_DIR}/shaders"
    "${GLM_INCLUDE_DIR}"
    "${GLEW_INCLUDE_DIR}"
    "${STB_INCLUDE_DIR}"
    "${JSON_INCLUDE_DIR}"
    "${IMGUI_INCLUDE_DIR}"
    "${IMGUI_INCLUDE_DIR}/backends"
    "${IMGUIZMO_INCLUDE_DIR}"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    GLM_ENABLE_EXPERIMENTAL
    GLEW_STATIC
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        gdi32
        user32
        kernel32
        shell32
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit" 
        "-framework CoreVideo"
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_DL_LIBS}
        pthread
        m
        X11
        Xrandr
        Xi
        Xcursor
    )
endif()

if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    foreach(source IN ITEMS ${SOURCES})
        get_filename_component(source_path "${source}" PATH)
        file(RELATIVE_PATH source_path_rel "${CMAKE_SOURCE_DIR}/src" "${source_path}")
        string(REPLACE "/" "\\" group_path "${source_path_rel}")
        if(NOT group_path STREQUAL "")
            source_group("Source Files\\${group_path}" FILES "${source}")
        else()
            source_group("Source Files" FILES "${source}")
        endif()
    endforeach()
    
    foreach(header IN ITEMS ${HEADERS})
        get_filename_component(header_path "${header}" PATH)
        file(RELATIVE_PATH header_path_rel "${CMAKE_SOURCE_DIR}/src" "${header_path}")
        string(REPLACE "/" "\\" group_path "${header_path_rel}")
        if(NOT group_path STREQUAL "")
            source_group("Header Files\\${group_path}" FILES "${header}")
        else()
            source_group("Header Files" FILES "${header}")
        endif()
    endforeach()
    
    source_group("ImGui" FILES ${IMGUI_SOURCES})
    source_group("imguizmo" FILES ${IMGUIZMO_SOURCES})
    
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
                 
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
endif()