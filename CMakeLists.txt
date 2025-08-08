cmake_minimum_required(VERSION 3.20)
project(Editor LANGUAGES C CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Опции компиляции
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Определяем тип сборки по умолчанию
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Пути к локальным библиотекам
set(LIBRARIES_DIR "${CMAKE_SOURCE_DIR}/libraries")

# Проверяем наличие библиотек
if(NOT EXISTS "${LIBRARIES_DIR}")
    message(FATAL_ERROR "Папка libraries не найдена! Создайте папку libraries в корне проекта.")
endif()

# Настройка OpenGL (системная библиотека)
find_package(OpenGL REQUIRED)

file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})

# GLFW - локальная сборка
if(EXISTS "${LIBRARIES_DIR}/glfw/CMakeLists.txt")
    message(STATUS "Найден GLFW в libraries/glfw")
    
    # Отключаем ненужные части GLFW
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    add_subdirectory("${LIBRARIES_DIR}/glfw")
    set(GLFW_LIBRARIES glfw)
else()
    message(FATAL_ERROR "GLFW не найден в libraries/glfw/. Скачайте GLFW и распакуйте в эту папку.")
endif()

# GLEW - настройка как в рабочем примере
set(GLEW_INCLUDE_DIR "${LIBRARIES_DIR}/glew/include")
if(EXISTS "${GLEW_INCLUDE_DIR}/GL/glew.h")
    message(STATUS "Найден GLEW в libraries/glew")
    
    # Настройка GLEW как статической библиотеки
    add_definitions(-DGLEW_STATIC)
    
    if(WIN32)
        # Для Windows - используем импортированную статическую библиотеку
        add_library(glew_static STATIC IMPORTED)
        
        # Определяем архитектуру и устанавливаем пути
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
        # Для Linux/Mac
        find_package(GLEW REQUIRED)
        set(GLEW_LIBRARIES GLEW::GLEW)
    endif()
else()
    message(FATAL_ERROR "GLEW не найден в libraries/glew/. Скачайте GLEW и распакуйте в эту папку.")
endif()

# GLM - header-only библиотека
set(GLM_INCLUDE_DIR "${LIBRARIES_DIR}/glm")
if(EXISTS "${GLM_INCLUDE_DIR}/glm/glm.hpp")
    message(STATUS "Найден GLM в libraries/glm")
else()
    message(FATAL_ERROR "GLM не найден в libraries/glm/. Скачайте GLM и распакуйте в эту папку.")
endif()

# STB - создаем отдельный файл реализации
set(STB_INCLUDE_DIR "${LIBRARIES_DIR}/stb")
if(EXISTS "${STB_INCLUDE_DIR}/stb_image.hpp")
    message(STATUS "Найден STB в libraries/stb")
else()
    message(WARNING "STB не найден в libraries/stb/. Скачайте stb_image.h и поместите в эту папку.")
endif()

# JSON - header-only библиотека
set(JSON_INCLUDE_DIR "${LIBRARIES_DIR}/json")
if(EXISTS "${JSON_INCLUDE_DIR}/json.h")
    message(STATUS "Найден JSON в libraries/json")
elseif(EXISTS "${JSON_INCLUDE_DIR}/json.hpp")
    message(STATUS "Найден JSON (nlohmann) в libraries/json")
elseif(EXISTS "${JSON_INCLUDE_DIR}/single_include/nlohmann/json.hpp")
    message(STATUS "Найден JSON (nlohmann single_include) в libraries/json")
    set(JSON_INCLUDE_DIR "${JSON_INCLUDE_DIR}/single_include")
else()
    message(WARNING "JSON не найден в libraries/json/. Скачайте json.h или nlohmann/json.hpp и поместите в эту папку.")
endif()

# Собираем исходные файлы из всех подпапок
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/src/*.hpp" 
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Проверяем наличие исходников
if(NOT SOURCES)
    message(FATAL_ERROR "Исходные файлы не найдены в папке src/. Создайте папку src и добавьте файл main.cpp")
endif()

# КЛЮЧЕВОЕ ОТЛИЧИЕ: Создаем отдельный файл для STB реализации
set(STB_IMPLEMENTATION_FILE "${CMAKE_BINARY_DIR}/stb_implementation.cpp")
file(WRITE ${STB_IMPLEMENTATION_FILE}
"#define STB_IMAGE_IMPLEMENTATION
#include \"stb_image.hpp\"
")

# Добавляем файл реализации STB к источникам
list(APPEND SOURCES ${STB_IMPLEMENTATION_FILE})

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} "src/Mesh.cpp")

# Добавляем директории заголовочных файлов
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/font"
    "${CMAKE_SOURCE_DIR}/src/Logger"
    "${CMAKE_SOURCE_DIR}/shaders"
    "${GLM_INCLUDE_DIR}"
    "${GLEW_INCLUDE_DIR}"
    "${STB_INCLUDE_DIR}"
    "${JSON_INCLUDE_DIR}"
)

# Включаем экспериментальные функции GLM
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    GLM_ENABLE_EXPERIMENTAL
    GLEW_STATIC
)

# Линкуем библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

# Дополнительные системные библиотеки
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
else() # Linux
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

# Настройки для Visual Studio
if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    # Группируем файлы по папкам
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
    
    # Устанавливаем как стартовый проект
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
                 
    # Отключаем предупреждения MSVC
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
endif()