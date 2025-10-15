cmake_minimum_required(VERSION 3.20)
project(triangulation_gl3 C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------
# Options (you can tweak later)
# ------------------------------
option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(TRI_USE_GLM_SUBMODULE "Use glm submodule (add_subdirectory)" ON)

# ------------------------------
# GLAD (single-file)
# Expecting:
#   external/glad/glad.c
#   external/glad/glad.h
#   external/glad/KHR/khrplatform.h
# ------------------------------
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/external/glad")
if(EXISTS "${GLAD_DIR}/glad.c")
  add_library(glad STATIC "${GLAD_DIR}/glad.c")
  target_include_directories(glad PUBLIC "${GLAD_DIR}")
elseif(EXISTS "${GLAD_DIR}/src/glad.c")
  # fallback if you used the "src/include" layout
  add_library(glad STATIC "${GLAD_DIR}/src/glad.c")
  target_include_directories(glad PUBLIC "${GLAD_DIR}/include")
else()
  message(FATAL_ERROR "GLAD source not found. Put glad.c next to glad.h in external/glad/")
endif()

# ------------------------------
# GLFW (from submodule)
# ------------------------------
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/external/glfw")
# Reduce GLFW build noise/footprint
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory("${GLFW_DIR}")

# ------------------------------
# GLM (header-only)
# ------------------------------
set(GLM_DIR "${CMAKE_SOURCE_DIR}/external/glm")
if(TRI_USE_GLM_SUBMODULE AND EXISTS "${GLM_DIR}/CMakeLists.txt")
  add_subdirectory("${GLM_DIR}")
  set(TRI_GLM_TARGET glm::glm)
else()
  # simple include-only fallback
  add_library(glm_header_only INTERFACE)
  target_include_directories(glm_header_only INTERFACE "${GLM_DIR}")
  add_library(glm::glm ALIAS glm_header_only)
  set(TRI_GLM_TARGET glm::glm)
endif()

# ------------------------------
# OpenGL (platform-specific)
# ------------------------------
if(APPLE)
  find_library(COCOA_FRAMEWORK Cocoa)
  find_library(IOKIT_FRAMEWORK IOKit)
  find_library(COREVIDEO_FRAMEWORK CoreVideo)
  find_library(OPENGL_FRAMEWORK OpenGL)
elseif(WIN32)
  # On Windows, OpenGL library is opengl32
  set(OPENGL_LIB opengl32)
else()
  find_package(OpenGL REQUIRED)
  set(OPENGL_LIB OpenGL::GL)
endif()

# ------------------------------
# App
# ------------------------------
add_executable(triangulation_gl3
  src/main.cpp
)

target_link_libraries(triangulation_gl3
  PRIVATE
    glad
    glfw                 # target from GLFW submodule
    ${TRI_GLM_TARGET}
)

# Add OpenGL link/frameworks per platform
if(APPLE)
  target_link_libraries(triangulation_gl3 PRIVATE
    ${OPENGL_FRAMEWORK} ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${COREVIDEO_FRAMEWORK})
elseif(WIN32)
  target_link_libraries(triangulation_gl3 PRIVATE ${OPENGL_LIB})
else()
  target_link_libraries(triangulation_gl3 PRIVATE ${OPENGL_LIB})
endif()

# Nice warnings
if(MSVC)
  target_compile_options(triangulation_gl3 PRIVATE /W4 /permissive-)
else()
  target_compile_options(triangulation_gl3 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ------------------------------
# Runtime working directory (optional)
# ------------------------------
set_target_properties(triangulation_gl3 PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
