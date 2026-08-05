cmake_minimum_required(VERSION 3.20)

# 1. Project Declaration (CXX for C++20, C for thirdparty/glad/src/gl.c)
project(GraphicsManagerApp VERSION 1.0 LANGUAGES CXX C)

# Force C++20 standard for C++ sources
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include FetchContent module for remote dependencies
include(FetchContent)

# ==============================================================================
# 2. Fetch External Dependencies (GLM & GLFW)
# ==============================================================================

# Fetch GLM (Header-only math library)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_PROGRESS TRUE
)

# Fetch GLFW (Windowing & OpenGL Context Creation)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_PROGRESS TRUE
)

# Disable building GLFW docs, tests, and examples to optimize build speed
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Download and populate GLM and GLFW
FetchContent_MakeAvailable(glm glfw)

# Locate System OpenGL
find_package(OpenGL REQUIRED)

# ==============================================================================
# 3. Target Definition & Sources
# ==============================================================================

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Shader.cpp
    src/Primitives2D.cpp
    src/GraphicsManager.cpp
    
    # GLAD C source file
    thirdparty/glad/src/gl.c
)

# ==============================================================================
# 4. Include Directories
# ==============================================================================

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include
)

# ==============================================================================
# 5. Compiler Warnings Configuration
# ==============================================================================

# Using $<COMPILE_LANGUAGE:CXX> ensures strict C++ flags don't pollute thirdparty gl.c
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        $<$<COMPILE_LANGUAGE:CXX>:/W4 /permissive- /utf-8>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnon-virtual-dtor>
    )
endif()

# ==============================================================================
# 6. Target Linking & Properties
# ==============================================================================

# Prevent GLFW from including system OpenGL headers (resolves GLAD conflicts)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw        # GLFW target populated by FetchContent
    glm::glm    # GLM target populated by FetchContent
    OpenGL::GL  # System OpenGL target
)

# Optional: Copy shaders directory into the build directory if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Output binary location setting
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)